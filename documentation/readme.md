# retroTerm Documentation

Documentation is broken down into three main sections, initialisation/housekeeping, terminal control/interaction and widget creation/control. There are also some notes on memory management and known issues.

## Table of Contents

1. [Initialisation and housekeeping](#initialisation-and-housekeeping)
   1. [Initialisation](#initialisation)
   1. [Housekeeping](#housekeeping)
   1. [Probing the terminal](#probing-the-terminal)
1. [Terminal control](#terminal-control)
	1. [Reset/clear](#reset-clear)
	1. [Title](#title)
	1. [Cursor movement](#cursor-movement)
	1. [Attributes and colours](#attributes-and-colours)
	1. [Bell](#bell)
1. [Output](#output)
   1. [Printing](#printing)
1. [Input](#input)
   1. [Keyboard](#keyboard)
   1. [Mouse](#mouse)
1. [Widgets](#widgets)
	1. [Widget IDs](#widget-ids)
	1. [Creating widgets](#creating-widgets)
	1. [Deleting widgets](#deleting-widgets)
	1. [Show & Hide](#show-hide)
	1. [Content control](#content-control)
1. [Memory management](#memory-management)
1. [Known-issues](#known-issues)

## Initialisation and housekeeping

With no dependencies, all you need to include the library is a standard #include. You should then declare an instance of the class, here it is called 'terminal' but it can be whatever you wish.

```c++
#include <retroTerm.h>
retroTerm terminal;
```
In principle it can be used with any Stream class in Arduino but is only tested on Serial.

**[Back to top](#table-of-contents)**

### Initialisation

You must first do any initialisation of the Stream you plan to use then pass it to the begin() of retroTerm so it is used.

```c++
Serial.begin(115200);
terminal.begin(Serial);
```

**[Back to top](#table-of-contents)**

### Housekeeping

In order to keep any widgets up to date and take input from the terminal there is a housekeeping function, `houseKeeping()`. 

This function only needs to be run when you expect changes or input, but the more often it is run, the more responsive the application can be. Typically it should be in the `loop()` of an Arduino sketch, but if your code spends lots of time in other functions it will need to be run there too. Sketches that rely on `delay()` a great deal will not be responsive and should be avoided.

```c++
terminal.houseKeeping();
```

### Probing the terminal

```c++
bool probeSize();
uint8_t columns();
uint8_t lines();
```
It should be possible to find the number of columns and number of lines the terminal has. The library tries to probe this by moving the cursor to 255,255 and then requesting the current position. The expected behaviour is it will move as far as possible and this can be used to determine the size. Once this is done, the cursor will move back to its previous position. The `probeSize()` may take a second or so to complete, this function blocks until it receives a response or times out.

If `probeSize()` returns true, you can be fairly sure the size has been correctly probed and can use `columns()` and `lines()` to size the output you send form this. If `probeSize()` returns false, the library assumes a terminal of 80 columns x 24 lines.

As is standard on VT style terminals, the origin of the terminal is 1,1 at the top left corner. There is no column or line zero and this is sometimes used in the library to report an error.

```c++
bool probeType();
char* type();
```

Similar to probing the size, it is possible to probe the terminal type which is stored as a char array by the library, this is not overly useful without further work.

**[Back to top](#table-of-contents)**

## Terminal control

The most basic methods in retroTerm apply to the whole terminal or just its 'current state', more complicated behaviour is built from these.

**[Back to top](#table-of-contents)**

### Reset/clear

```c++
void reset();
```

This method resets all the terminal settings to default and clears the currently display contents.

```c++
void eraseScreen();
```

This method clears the currently displayed terminal content. It does not change colours, attributes and so on.

**[Back to top](#table-of-contents)**

### Title

```c++
void setTitle(variableContent title);
```

Some terminal emulators allow you to set the 'title' of the window the terminal emulator, you can use this method to do so. The method is templated so accepts Strings, char arrays, or any reasonable thing you'd send to a `print()`, including strings stored in Flash with an F() macro.

**[Back to top](#table-of-contents)**

### Cursor control

As is standard for VT style terminal, all output happens from a 'cursor' that has a specific position and state. This is reference to an origin of 1,1 which is the top left of the terminal.

```c++
void moveCursorTo(uint8_t column, uint8_t row);
```

The `moveCursorTo()` method moves the cursor directly to the specified column and row.

```c++
void hideCursor();
void showCursor();
```

Most terminal have the facility to 'hide' the cursor, which is often a 'blob' on the terminal. If printing output in many different places on the terminal, this stops the cursor seeming the 'flicker' as it moves around.

```c++
void saveCursorPosition();
void restoreCursorPosition();
```

Most terminals also have the ability to save and restore the state of the cursor, which also includes [attributes and colours](attributes-and-colours). This allows you to save the cursor state, output whatever you want somewhere else then return. This is only 'one layer deep' so you should always save, output and restore.

```c++
void requestCursorPosition();
```

```c++
void setScrollWindow(uint8_t, uint8_t);
```

**[Back to top](#table-of-contents)**

### Attributes and Colours

```c++
void color(uint8_t);
void colour(uint8_t);
```
These synonym functions (US/English) set the foreground colour of any output sent to the terminal. The foreground and background colour are set by a bitmask, which has convenience shortcuts you can OR together. The bitmask is chosen so 'terminal default' is 0x00.

```c++
constexpr const uint16_t COLOUR_BLACK = 			0x0008;
constexpr const uint16_t COLOUR_RED = 				0x0009;
constexpr const uint16_t COLOUR_GREEN =				0x000a;
constexpr const uint16_t COLOUR_YELLOW = 			0x000b;
constexpr const uint16_t COLOUR_BLUE = 				0x000c;
constexpr const uint16_t COLOUR_MAGENTA = 			0x000d;
constexpr const uint16_t COLOUR_CYAN = 				0x000e;
constexpr const uint16_t COLOUR_WHITE = 			0x000f;
constexpr const uint16_t BACKGROUND_COLOUR_BLACK = 	0x0080;
constexpr const uint16_t BACKGROUND_COLOUR_RED = 	0x0090;
constexpr const uint16_t BACKGROUND_COLOUR_GREEN = 	0x00a0;
constexpr const uint16_t BACKGROUND_COLOUR_YELLOW = 0x00b0;
constexpr const uint16_t BACKGROUND_COLOUR_BLUE = 	0x00c0;
constexpr const uint16_t BACKGROUND_COLOUR_MAGENTA =0x00d0;
constexpr const uint16_t BACKGROUND_COLOUR_CYAN = 	0x00e0;
constexpr const uint16_t BACKGROUND_COLOUR_WHITE = 	0x00f0;
```

So to set the colour for later out put to 'green on a black background' you might use.

`terminal.colour(COLOUR_GREEN | BACKGROUND_COLOUR_BLACK);`

Terminal output also has various 'attributes' which may not be settable on all terminals.

```c++
uint16_t attributes();
void attributes(uint16_t);
void resetAttributes();
```
Again there are convenience shortcuts you OR together to set these attributes.

```
constexpr const uint16_t ATTRIBUTE_BRIGHT =			0x0100;
constexpr const uint16_t ATTRIBUTE_BOLD =			0x0200;
constexpr const uint16_t ATTRIBUTE_FAINT =			0x0400;
constexpr const uint16_t ATTRIBUTE_UNDERLINE =		0x0800;
constexpr const uint16_t ATTRIBUTE_BLINK =			0x1000;
constexpr const uint16_t ATTRIBUTE_INVERSE =		0x2000;
constexpr const uint16_t ATTRIBUTE_DOUBLE_WIDTH =	0x4000;
constexpr const uint16_t ATTRIBUTE_DOUBLE_SIZE =	0x8000;

```

**[Back to top](#table-of-contents)**

### Bell

```c++
void enableBell();
void disableBell();
void soundBell();
```

The library includes basic methods for enabling/disableing and sounding the terminal bell. This is useful for warnings.

**[Back to top](#table-of-contents)**

## Output

```c++
void print(variableContent content)
```

```c++
void println(variableContent content)
```

```c++
void printCentred(variableContent content)
void printCentred(uint8_t y, variableContent content)
```

```c++
void printAt(uint8_t x, uint8_t y, variableContent content)
void printAt(uint8_t x, uint8_t y, variableContent content, uint16_t specificAttributes)
```

```c++
void scroll(variableContent content)
void scroll(variableContent content, bool centred)
```

**[Back to top](#table-of-contents)**



## Input

The retroTerm library normally expects to capture all input from the terminal, rather than the rest of the application reading directly from the Stream. If you do the latter, you may get unreliable behaviour.

Common non-alphanumeric keyboard presses (function keys, arrows etc.) have constants associated with them to make them easy to handle but if widgets are in use then often a widget will act on these keypresses and the application won't see them. This is particularly true of anything specified as a keyboard shortcut or keyboard navigation key (tab, back tab, arrows, page up, page down, enter etc.) that has a commonly understood meaning in a MS Windows style GUI.

**[Back to top](#table-of-contents)**

## Keyboard

```c++
bool userIsTyping();
```
This method returns true if the terminal has received a keypress recently. It is a useful way to wait until somebody has 'finished typing'.

```c++
bool keyPressed();
```
This method returns true if there is a keypress waiting to be read.

```c++
uint8_t readKeypress()
```
Returns the ASCII code of the keypress, plus assorted other keys mapped into 0-31. There is currently no unicode/UTF-8 support for input from the keyboard.

**[Back to top](#table-of-contents)**

## Mouse

```c++
void enableMouse();
void disableMouse();
```
These two methods enable and disable mouse captue, if the terminal supports it.

```c++
uint8_t mouseX();
uint8_t mouseY();
```
These return the last reported located of the mouse pointer. Depending on terminal support this is usually where it was last clicked, rather than where it currently is.

```c++
bool mouseButtonDown();
bool mouseButtonUp();
bool mouseWheelDown();
bool mouseWheelUp();
```
These methods return true on the relevant mouse button event. They clear the button status once read.


**[Back to top](#table-of-contents)**

## Widgets

**[Back to top](#table-of-contents)**

### Widget IDs

Widget IDs are simple 8-bit unsigned integers (uin8_t ) to allow you to store them trivially in variables, arrays and so on. An ID of 0 denotes unassigned or a failure to assign due to lack of memory. You should always check an ID is non-zero when creating a new widget.

If a widget is deleted then its ID may be subsequently re-used, so you should ensure you no longer associate that ID with the deleted widget. Deleting widgets is not great for heap fragmentation if they contain large amounts of text content.

**[Back to top](#table-of-contents)**

### Creating widgets

**[Back to top](#table-of-contents)**

### Deleting widgets

**[Back to top](#table-of-contents)**

### Show & Hide

**[Back to top](#table-of-contents)**

### Content control

**[Back to top](#table-of-contents)**

## Memory management

**[Back to top](#table-of-contents)**

## Known issues

**[Back to top](#table-of-contents)**