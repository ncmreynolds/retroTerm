# retroTerm

retroTerm is an Arduino library for creating user interfaces in a terminal emulator with a microcontroller.

Now that many microcontrollers have Bluetooth or Wi-Fi it might seem superfluous but there are also many that do not and occasions when you want to configure or interact with them without having a network available.

retroTerm allows you to create clickable 'buttons', 'checkboxes', 'list boxes' and so on and simple line editing of text.

The name comes from being able to create 'retro terminal' interfaces like this, which can be interacted freely with using the mouse and keyboard, within the fundamental limitations of them running on a microcontroller.

![](chatTerminal.png)

## Table of Contents

1. [About retoTerm](#about-the-project)
   1. [Terminal control features](#terminal-control-features)
   1. [Widget features](#widget-features)
1. [Project status](#project-status)
1. [Getting Started](#getting-started)
	1. [Dependencies](#dependencies)
	1. [Getting the Source](#getting-the-source)
	1. [Installation](#installation)
	1. [Usage](#usage)
1. [Versioning](#versioning)
  1. [Version history](#version-history)
1. [How to Get Help](#how-to-get-help)
1. [Further Reading](#further-reading)
1. [Contributing](#contributing)
1. [License](#license)
1. [Authors](#authors)
1. [Acknowledgements](#acknowledgements)

## About retroTerm

retroTerm is an Arduino library for creating user interfaces in a terminal emulator with a microcontroller.

By default this is expected to be the Serial interface, but it could also be any Stream. Testing is done with the popular terminal emulator [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) as it is open source, available across several platforms and supports many terminal features others do not. Please use PuTTY first before reporting issues. The Arduino IDE terminal is **not** usable with retroTerm, it supports almost no ANSI/VT terminal features.

Both direct control of the terminal and the use of 'widgets' that allow for an almost-GUI are possible, but mixing the two requires care. It tracks the current cursor state and tries to reduce the number of state changes sent to improve performance over the serial connection.

This project was originally created to help with making a fake 'mainframe' that somebody can interact with running on an ESP8266.

### Terminal control features

- Detect size and type
- Set the terminal 'title'
- Clear/reset the terminal
- Move the cursor to specific co-ordinates
- Hide/show the cursor
- Limit scrolling to certain regions
- Set various character colours/attributes
- Print at specific co-ordinates
- Draw boxes
- Enable 'capture' of the mouse and record clicks

### Widget features

Creation and management of GUI-esque objects

* [Buttons](examples/Example04_singleButton/README.md)
* [Checkboxes](examples/Example05_singleCheckbox/README.md)
* [Radio buttons](examples/Example06_twoRadioButtons/README.md)
* [List boxes](examples/Example07_oneListBoxSixOptions/README.md)
* [Text boxes](examples/Example08_oneTextInput/README.md) (editable text) single line editing of text, with support for 'expected' behaviour like home/end/insert/delete and the arrow keys
* [Text boxes](examples/Example09_oneTextDisplay/README.md) (fixed text) with scrollbars, word wrapping and basic markdown support
* [Text boxes](examples/Example10_oneTextLog/README.md) (changing text) for 'logging' windows that scroll content
* Keyboard shortcuts for use without a mouse
* Basic styling for widget outlines/labels

As you might expect, this library can use a lot of working memory. There is explicit support for storing strings in flash memory using the `F()` macro on several architectures. You should use flash storage whenever possible for things like labels on buttons that will not change.

**[Back to top](#table-of-contents)**

## Project Status

This is the very first public release, which is working for the specific project I wrote it for. However I'm aware my naming of methods, approaches to passing arguments and so on are messy and inconsistent so some of this may change frequently. This is reflected in the version numbering, which is currently 0.1.2.

**[Back to top](#table-of-contents)**

## Getting Started

From the project page, select 'Code' -> 'Download ZIP' and save the file somewhere appropriate.

You can then install in the Arduino IDE by choosing 'Sketch' -> 'Include Library' -> 'Add .ZIP Library...'.

After installation, the retroTerm examples will be available under "File" -> "Examples" -> "retroTerm" in the Arduino IDE.

You should try the first two example sketches to check it works with your terminal emulator, then try the 'Alarm Clock' example which is a very simple application that will work on most microcontrollers.

[Capability Test](examples/Example01_terminalCapabilityTests/README.md) - basic terminal features

[Widget Test](examples/Example02_widgetTests/README.md) - widget features

[Alarm Clock](examples/Example03_alarmClock/README.md) - a simple interactive application

There are then further examples that serve as a demonstration of how to use each individual type of widget.

### Dependencies

This library has no specific dependencies, but you will need a fully featured terminal emulator that can connect to your target microcontroller over a serial or USB-serial interface. Testing is done with [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) and it is strongly recommend you start with using this.

It is only fully tested on the handful of microcontrollers below and some of these have specific code, usually related to storing strings/character arrays in flash memory to save working memory. Other microcontrollers may use working memory for all strings.

- Arduino AVR (Arduino "Uno" and "Mega2560" tested) Note the full widget example is now overly large for an Uno.
- ESP8266 (WeMos D1 mini tested).
- ESP32 ("Espressif ESP32-S2-Saola-1R" and "WeMos WiFi&Bluetooth Battery" tested).
- RP2040 (Raspberry Pi Pico tested with this [unofficial Arduino support](https://github.com/earlephilhower/arduino-pico)).
- ARM Cortex-M4 (Teensy 3.1 tested).

Note that the Espressif ESP8266/ESP32 bootloader sends characters to the serial port on reset, which may cause the bell to sound or be seen as an 'XOFF' that prevents further input. If compiled on ESP8266/ESP32 the library sends an 'XON' at initialisation.

Any device that does its own USB support (eg. Raspberry Pi Pico and Teensy) rather than having a separate USB to UART device **will** lose some initial output to the terminal at start up. You should take this into account by perhaps delaying output, waiting for a keypress etc. before starting your retroTerm UI. If tried immediately on these, probing the terminal size, enabling the mouse and so on will fail.

### Getting the Source

This project is [hosted on GitHub](https://github.com/ncmreynolds/retroTerm).

### Installation

From the project page, select 'Code' -> 'Download ZIP' and save the file somewhere appropriate.

You can then install in the Arduino IDE by choosing 'Sketch' -> 'Include Library' -> 'Add .ZIP Library...'.

### Usage

[Documentation](documentation/README.md) for retroTerm is included as markdown in the 'documentation' directory.

Broadly to create widgets you need something along lines of the code below. This will create a single button at co-ordinates 1,1 that detects mouse clicks and prints below when it happens.

This example (which is in examples\Example04_singleButton) shows a lot of core concepts.

```c++
#include <retroTerm.h> //Include terminal library
retroTerm terminal; //Create a terminal instance
uint8_t buttonId = 0; //Record the button ID globally
uint32_t numberOfClicks = 1; //Record the number of clicks

void setup() {
  Serial.begin(115200);   //Initialise the Serial stream
  terminal.begin(Serial); //Initialise the library
  terminal.eraseScreen(); //Clear the screen
  terminal.hideCursor(); //Hide the terminal cursor
  terminal.enableMouse(); //Capture the mouse so it can be used with widgets
  terminal.setScrollWindow(4,12); //Set up somewhere to show the events without disrupting the button
  buttonId = terminal.newButton(1, 1, 15, 3, F("Button label"), COLOUR_GREEN, OUTER_BOX | BOX_SINGLE_LINE); //Create a green button in a box
  terminal.widgetShortcutKey(buttonId,f1Pressed); //Assign a shortcut key of F1
  terminal.showWidget(buttonId); //Make the button visible, all widgets start 'invisible' for later display
}

void loop() {
  if(terminal.widgetClicked(buttonId)) //This clears the 'click' on checking
    {
      terminal.scroll("Button click " + String(numberOfClicks++)); //Print inside the scroll window
    }
  terminal.houseKeeping();  //You MUST run housekeeping to show/detect any changes or events
}
```

The code includes the library then assigns a global variable for the widget. Widgets have simple numeric IDs from 1-255, 0 is considered invalid. If you delete a button its ID may be re-used.

Effective setup for displaying widgets needs quite a few steps, but is then simpler to interact with.

- Pass the library the Stream used for display in `begin()`. Normally this is 'Serial' but it could be another Stream, for example Serial2.
- Erase the screen to clear any mess from startup. Not necessary on all microcontrollers but ESP8266 & ESP32 output boot messages.
- Hide the cursor, for tidiness. If you use a widget that allows text editing it will make the cursor visible while editing.
- Enable the mouse, otherwise you will have to rely on keyboard shortcuts for interaction.
- Set a scroll window. This is not a widget, but reduces the scrolling region of the terminal, making for trivial diagnostic/logging output.
- Create the widget.
- Assign a shortcut key to the widget. If you press the shortcut key it registers as a 'click' as if clicked with the mouse.
- Make the widget visible.

Once in the loop all it does is.

- Check for button clicks and print in the scrolling window when this happens.
- Run a housekeeping routine. This does all screen updating and processing of inputs, normally you should run it frequently. The more frequently it runs, the more responsive the application will be to inputs. If you are expecting no inputs, it is OK to run it only when you have changed the widgets.

This example should look like this in a terminal.

![](examples/Example04_singleButton/images/singleButton.png)

**[Back to top](#table-of-contents)**

## Versioning

This project uses [Semantic Versioning](http://semver.org/) to be compatible with the Arduino library standards.

### Version history

#### 0.1.2

First public release.

For a full list of available versions, see the [repository tag list](https://github.com/ncmreynolds/retroTerm/tags). There is also a [change log](documentation/CHANGELOG.md) included in the documentation.

## How to Get Help

Drop me a message on retroTerm@arcanium.london, I can't promise to be super-responsive but I'll try.

## Contributing

I'd love to get help with this work, either drop me a message here or on retroTerm@arcanium.london. I'm new to using git for collaboration so please be patient. Ongoing work is happening in the 'dev' branch for a later release.

**[Back to top](#table-of-contents)**

## Further Reading

Serial 'dumb' terminals got quite smart over time, I took information from a variety of sources to help write this library including the ones linked below. Broadly it uses features you would expect in a VT420.

[VT100.net](https://www.vt100.net/)

[ascii-table.com](http://ascii-table.com/ansi-escape-sequences-vt-100.php)

[Paul Bourke's list of VT commands](http://braun-home.net/michael/info/misc/VT100_commands.htm)

[libvterm-ctrl](https://github.com/BryanHaley/libvterm-ctrl)

**[Back to top](#table-of-contents)**

## License

Copyright (c) 2021 Nick Reynolds

This project is licensed under the GNU Lesser General Public License - see [LICENSE.md](LICENSE.md) file for details.

**[Back to top](#table-of-contents)**

## Authors

* **[Nick Reynolds](https://github.com/ncmreynolds)** 

**[Back to top](#table-of-contents)**

## Acknowledgments

This project wouldn't be useful without the excellent open source terminal emulator [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/), which I have used for what feels like decades.

Markdown documentation mostly written in [Typora](https://typora.io/).

**[Back to top](#table-of-contents)**