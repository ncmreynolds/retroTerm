# retroTerm Documentation

Documentation is broken down into three main sections, initialisation/housekeeping, terminal control/interaction and widget creation/control. There are also some notes on memory management and known issues.

# Table of Contents

1. [Initialisation and housekeeping](#initialisation-and-housekeeping)
   1. [Initialisation](#initialisation)
   1. [Housekeeping](#housekeeping)
   1. [Probing the terminal](#probing-the-terminal)
1. [Terminal control](#terminal-control)
	1. [Reset/clear](#reset-clear)
	1. [Title](#title)
	1. [Cursor movement](#cursor-movement)
	1. [Attributes and colours](#attributes-and-colours)
1. [Output](#output)
   1. [Printing](#printing)
   2. [Boxes](#boxes)
   3. [Bell](#bell)
1. [Input](#input)
   1. [Keyboard](#keyboard)
   1. [Mouse](#mouse)
1. [Widgets](#widgets)
	1. [Widget IDs](#widget-ids)
	1. [Z-order](#z-order)
	1. [Styles](#styles)
	1. [Creating widgets](#creating-widgets)
	1. [Deleting widgets](#deleting-widgets)
	1. [Show and hide widgets](#show-and-hide-widgets)
	1. [Content control](#content-control)
1. [Memory management](#memory-management)
1. [Known-issues](#known-issues)

# Initialisation and housekeeping

With no dependencies beyond usual internal libraries of the Arduino core, all you need to include the library is a standard #include. You should then declare an instance of the class, here it is called 'terminal' but it can be whatever you wish.

```c++
#include <retroTerm.h>
retroTerm terminal;
```
In principle it can be used with any Stream class in Arduino but is only tested on Serial.

**[Back to top](#table-of-contents)**

## Initialisation

You must first do any initialisation of the Stream you plan to use then pass a reference to the begin() of retroTerm so it can be used.

```c++
Serial.begin(115200);
terminal.begin(Serial);
```

In testing a baud rate of 115200 has proved responsive and reliable. Higher baud rates seem to cause the occasional loss of control characters and unreliable behaviour.

**[Back to top](#table-of-contents)**

## Housekeeping

In order to keep any widgets up to date and take input from the terminal there is a housekeeping function, `houseKeeping()`. 

This function only needs to be run when you expect changes or input, but the more often it is run, the more responsive the application can be. Typically it should be in the `loop()` of an Arduino sketch, but if your code spends lots of time in other functions it will need to be run there too. Sketches that rely on `delay()` a great deal will not be responsive and should be avoided.

```c++
terminal.houseKeeping();
```

## Probing the terminal

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

# Terminal control

The most basic methods in retroTerm apply to the whole terminal or just its 'current state', more complicated behaviour is built from these.

**[Back to top](#table-of-contents)**

## Reset/clear

```c++
void reset();
```

This method resets all the terminal settings to default and clears the currently display contents.

```c++
void eraseScreen();
```

This method clears the currently displayed terminal content. It does not change colours, attributes and so on.

**[Back to top](#table-of-contents)**

## Title

```c++
void setTitle(variableContent title);
```

Some terminal emulators allow you to set the 'title' of the window the terminal emulator, you can use this method to do so. The method is templated so accepts Strings, char arrays, or any reasonable thing you'd send to a `print()`, including strings stored in Flash with an F() macro.

**[Back to top](#table-of-contents)**

## Cursor control

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

Most terminals also have the ability to save and restore the state of the cursor, which also includes [attributes and colours](#attributes-and-colours). This allows you to save the cursor state, output whatever you want somewhere else then return. This is only 'one layer deep' so you should always save, output and restore.

```c++
void requestCursorPosition();
```

This method refreshes the current cursor position from the terminal application. The response can take a short while so your code should wait at least a second before assuming an update has happened.

```c++
void setScrollWindow(uint8_t, uint8_t);
```

This method sets the scrolling region of the screen to between two specified rows (inclusive). Printing in this region that overflows will not scroll the whole terminal but just the lines specified. This is very useful for a 'logging' style window that is endlessly added to. Note it still scrolls the whole width of the screen.

As this is done in the terminal application, not by the library redrawing areas it is faster and more efficient than scrolling in a widget.

**[Back to top](#table-of-contents)**

## Attributes and Colours

There are two ways to set colours and attributes in the library, eight colour and 256-colour.

### Eight colour attributes

The default control over colour in the library is through the use of 'attributes'. These use the most basic form of colour support in the terminal emulator and are most likely to be supported.

Attributes can handle eight foreground colours, eight background colours and a mix of other visual effects. Many of the library output methods like `print()` can be passed attributes and will change them for you in one method and all widgets expect to use attributes.

The attributes are set by a bitmask, which has convenience shortcuts you can OR together. The bitmask is chosen so 'terminal default' is 0x00. There are synonyms with the US spelling of 'colour'.

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
constexpr const uint16_t ATTRIBUTE_BRIGHT =			0x0100;
constexpr const uint16_t ATTRIBUTE_BOLD =			0x0200;
constexpr const uint16_t ATTRIBUTE_FAINT =			0x0400;
constexpr const uint16_t ATTRIBUTE_UNDERLINE =		0x0800;
constexpr const uint16_t ATTRIBUTE_BLINK =			0x1000;
constexpr const uint16_t ATTRIBUTE_INVERSE =		0x2000;
constexpr const uint16_t ATTRIBUTE_DOUBLE_WIDTH =	0x4000;
constexpr const uint16_t ATTRIBUTE_DOUBLE_SIZE =	0x8000;
```

You pass the combined attribute value to one of these methods.

Beware the 'double width' and 'double size' attributes. They apply to the whole row, two rows for double size and can be hard to work with.

```c++
void attributes(uint16_t);
void defaultAttributes(uint16_t);
void resetAttributes();
```

So to set the colour for later output to 'green on a black background' you might use.

`terminal.colour(COLOUR_GREEN | BACKGROUND_COLOUR_BLACK);`

You can also retrieve the current set attributes.

```
uint16_t attributes();
uint16_t defaultAttributes();
```

### 256-colours

The other method of setting colours is from the 256-colour palette. Not all terminals will support 256-colour mode. This is not supported by widgets (yet).

```c++
void color(uint8_t);
void colour(uint8_t); //A synonym for US users
void clearColour();
```

**[Back to top](#table-of-contents)**

# Output

When sending output to the terminal, try not to mix straightforward print methods with widgets, as the library makes no effort to maintain output that isn't a widget if it is overwritten etc. Both styles of access to the terminal work simultaneously but you will need care to ensure they do not conflict.

## Printing

```c++
void print(variableContent content);
```

A good old fashioned print method, which outputs at the current cursor position, with the current [attributes](#attributes-and-colours). This method is templated so will output most types you send it as you'd expect without having to do things like `itoa()`.

```c++
void println(variableContent content);
```

Similar to `print()` this does a newline after the print, which may cause the terminal or scroll region to scroll up.

```c++
void printCentred(variableContent content);
void printCentred(uint8_t y, variableContent content);
```

These two methods will centre printed content. The first prints at the current row, the second centred on a specific row.

```c++
void printAt(uint8_t x, uint8_t y, variableContent content);
void printAt(uint8_t x, uint8_t y, variableContent content, uint16_t specificAttributes);
```

These two methods print at a specified column and row. The second variant will print with the specified [attributes](#colours-and-attributes).

```c++
void scroll(variableContent content);
void scroll(variableContent content, bool centred);
```

These two methods print at the bottom of the scrolling region, after moving the existing content up one line. These are fast and efficient compared to scrolling the content of a widget as the terminal emulator handles the scrolling and the contents do not need to be redrawn by the library.

This is one of the occasions where you might want to mix widgets and direct printing to the terminal.

See `setScrollWindow()` to set the scrolling region.

**[Back to top](#table-of-contents)**

## Boxes

As well as simple text, the methods for drawing boxes used by widgets are available directly.

```c++
void drawBox(uint8_t, uint8_t, uint8_t, uint8_t, uint16_t, uint8_t);
void drawBoxWithScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint32_t, uint16_t, uint8_t);
void drawBoxWithTitle(uint8_t, uint8_t, uint8_t, uint8_t, const char *, uint16_t, uint8_t);
void drawBoxWithTitleAndScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, const char *, uint32_t, uint32_t, uint16_t, uint8_t);
```

The four methods, do as described. Each is overload so the [attributes](#attributes-and-colours), [style](#styles) or title text can be omitted. The minimum parameters are column, row, width and height.

```c++
void centredTextBox(String);
void centredTextBox(String, uint16_t);
void centredTextBox(String, uint8_t);
void centredTextBox(String, uint16_t, uint8_t);
```

These four methods stick a box right in the centre of the screen with some text in. They're a relic of before the library supported widgets and deprecated.

**[Back to top](#table-of-contents)**

## Bell

```c++
void enableBell();
void disableBell();
void soundBell();
```

These are basic methods for enabling/disabling and sounding the terminal bell.

**[Back to top](#table-of-contents)**

# Input

The retroTerm library normally expects to capture all input from the terminal, rather than the rest of the application reading directly from the Stream. If you do the latter, you may get unreliable behaviour.

Common non-alphanumeric keyboard presses (function keys, arrows etc.) have constants associated with them to make them easy to handle but if widgets are in use then often a widget will act on these keypresses and the application won't see them. This is particularly true of anything specified as a keyboard shortcut or keyboard navigation key (tab, back tab, arrows, page up, page down, enter etc.) that has a commonly understood meaning in a MS Windows style GUI.

**[Back to top](#table-of-contents)**

## Keyboard

```c++
bool userIsTyping();
```
This method returns true if the terminal has received a keypress or mouse activity recently. It is a useful to help your code to wait until somebody has 'finished typing' or using the mouse before doing something.

```c++
bool keyPressed();
```
This method returns true if there is a keypress waiting to be read.

```c++
uint8_t readKeypress()
```
Returns the ASCII code of the next keypress, plus assorted other keys mapped into 0-31. There is currently no unicode/UTF-8 support for input from the keyboard.

### Key codes

The key codes used in the library are the traditional ASCII character set with some common keys on the PC keyboard mapped over little-used entries.

```c++
constexpr const uint8_t escapePressed =		0;
constexpr const uint8_t f1Pressed =			1;
constexpr const uint8_t f2Pressed =			2;
constexpr const uint8_t f3Pressed =			3;
constexpr const uint8_t f4Pressed =			4;
constexpr const uint8_t f5Pressed =			5;
constexpr const uint8_t f6Pressed =			6;
constexpr const uint8_t f7Pressed =			7;
constexpr const uint8_t f8Pressed =			8;
constexpr const uint8_t f9Pressed =			9;
constexpr const uint8_t f10Pressed =		10;
constexpr const uint8_t f11Pressed =		11;
constexpr const uint8_t f12Pressed =		12;
constexpr const uint8_t backspacePressed =	13;
constexpr const uint8_t insertPressed =		14;
constexpr const uint8_t homePressed =		15;
constexpr const uint8_t pageUpPressed =		16;
constexpr const uint8_t deletePressed =		17;
constexpr const uint8_t endPressed =		18;
constexpr const uint8_t pageDownPressed =	19;
constexpr const uint8_t tabPressed = 		20;
constexpr const uint8_t backTabPressed =	21;	//Shift tab
constexpr const uint8_t leftPressed =		22;
constexpr const uint8_t upPressed =			23;
constexpr const uint8_t downPressed =		24;
constexpr const uint8_t rightPressed =		25;
constexpr const uint8_t breakPressed =		26;
constexpr const uint8_t returnPressed =		27;
constexpr const uint8_t fsPressed =			28;
constexpr const uint8_t enterPressed =		29;
constexpr const uint8_t noKeyPressed =		127; //Normally you shouldn't see this, used as no key
```



The input buffer is limited, so if doing your own keyboard processing your code should check for this regularly.

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

# Widgets

The main purpose of this library is to allow the use of GUI-like widgets in the normally very 'dull' world of the Serial terminal. It's possible to create nicely interactive user interfaces with the selection of widgets available, although it does eat a chunk of your device's memory.

In this release there are eight widgets available. You are strongly recommended to try the example code to see how they behave. Broadly they behave like components you'd find in an older HTML form, or MS-Windows application so long as your terminal emulator supports the full range of behaviours.

Due to the limited memory of most microcontrollers, text widgets should be used with care.

- Button
- Checkbox
- Radio button
- List box
- Text input (A single line editing field for free form text entry)
- Text display (Display of static unchanging text information. This widget handles basic MarkDown for styling the content)
- Text log (Regularly updating text information like a 'log' or 'chat' window. New text can be added at the top or bottom, scrolling the existing text)



**[Back to top](#table-of-contents)**

## Widget IDs

Widget IDs are simple 8-bit unsigned integers (uin8_t) to allow you to store them trivially in variables, arrays and so on. An ID of 0 denotes unassigned or a failure to assign due to lack of memory. You should always check an ID is non-zero when creating a new widget.

If a widget is deleted then its ID may be subsequently re-used, so you should ensure you no longer associate that ID with the deleted widget. Deleting widgets is not great for [memory management](#memory-management) and heap fragmentation will occur especially if they contain large amounts of text content.

You can check the current number of widgets and maximum with these two methods.

```c++
uint8_t numberOfWidgets();
uint8_t maximumNumberOfWidgets();
```

**[Back to top](#table-of-contents)**

## Z-order

There is no configurable Z-order (yet), only an implicit one based on later widgets being 'higher'. Overlapping widgets are discouraged as it will cause much redrawing on the terminal but it does work.

## Styles

Widgets have some very basic ability to 'style' them.

**[Back to top](#table-of-contents)**

## Creating widgets




**[Back to top](#table-of-contents)**

## Deleting widgets

You are advised to hide and show widgets as necessary rather than deleting them unless your device is short on memory or you are very clear they will not be used again.

To check a specific widget ID is valid and in use you can check it the following method.

```c++
bool widgetExists(uint8_t widgetId);
```

If you're certain you would like to delete a widget, use the following method.

```c++
bool deleteWidget(uint8_t widgetId);
```

**[Back to top](#table-of-contents)**

## Show and hide widgets

All widget starts as 'hidden' and need to be 'shown'. Ordinarily you are advised to hide widgets rather than delete them, if memory permits.

```c++
void showWidget(uint8_t widgetId);
void hideWidget(uint8_t widgetId);
void showWidget(uint8_t widgetId, bool visible);
```

If you need to check the visibility of a specific widget you can use this method.

```c++
bool widgetVisible(uint8_t widgetId);
```

These next methods are pretty self explanatory. Normally the library manages which widget are likely to need refreshing based on content changes, moves, resizes and any overlaps. The `refreshAllWidgets()` method forces a complete redraw from lowest to highest visible widget ID and may be visibly slow.

```c++
void hideAllWidgets();
void showAllWidgets();
void refreshAllWidgets();
```

**[Back to top](#table-of-contents)**

## Moving and resizing

```c++
void moveWidget(uint8_t widgetId, uint8_t column, uint8_t row);
void resizeWidget(uint8_t widgetId, uint8_t width, uint8_t height);
```

These methods allow you to move or resize a widget after creation. Both cause a refresh of the widget in the terminal and potentially clipping of existing content or a blank space on the right hand side of a scrolling text widget where previously text was clipped.

**[Back to top](#table-of-contents)**

## Labels

Every widget can have a 'label' attached. Its appearance varies by widget and is affected by its [style](#styles) it is recommended you try the example code to see how they look in use. To help with [memory management](#memory-management) it is strongly advised you use the F() macro to set the label.

If you set the label further times, it will replace the previous one, freeing storage on heap if it was used.

```c++
bool setWidgetLabel(uint8_t widgetId, char* label);
bool setWidgetLabel(uint8_t widgetId, String label);
bool setWidgetLabel(uint8_t widgetId, const __FlashStringHelper* label);
```

Once set, if you really want, you can also delete the label with the following method.

```c++
bool deleteWidgetLabel(const uint8_t widgetId);
```

**[Back to top](#table-of-contents)**

## Content

```c++
bool setWidgetContent(uint8_t widgetId, char*);								//Add/change widget content char array
bool setWidgetContent(uint8_t widgetId, String);							//Add/change widget content String version
bool setWidgetContent(uint8_t widgetId, const char*);						//Add/change widget content string literal version
bool setWidgetContent(uint8_t widgetId, const __FlashStringHelper*);		//Add/change widget content PROGMEM version
bool deleteWidgetContent(uint8_t widgetId);									//Delete the widget content, returns true if there was one to delete
bool appendWidgetContent(uint8_t widgetId, char*);							//Add/change widget content char array
bool appendWidgetContent(uint8_t widgetId, String);							//Add/change widget content String version
bool appendWidgetContent(uint8_t widgetId, const char*);					//Add/change widget content string literal version
bool appendWidgetContent(uint8_t widgetId, const __FlashStringHelper*);		//Add/change widget content PROGMEM version
bool prependWidgetContent(uint8_t widgetId, char*);							//Add/change widget content char array
bool prependWidgetContent(uint8_t widgetId, String);						//Add/change widget content String version
bool prependWidgetContent(uint8_t widgetId, const char*);					//Add/change widget content string literal version
bool prependWidgetContent(uint8_t widgetId, const __FlashStringHelper*);	//Add/change widget content PROGMEM version
uint32_t contentOffset(uint8_t widgetId);									//Current content offset (0 if invalid widget)
bool contentOffset(uint8_t widgetId, uint32_t);								//Set current content offset

```

**[Back to top](#table-of-contents)**

## Shortcuts

The library can attach a keyboard shortcut to a widget that allows it to be selected or 'clicked' with the keyboard. The placement and look of the shortcut is affected by the widget's [style](#styles). It is recommended you try the example code to see how they look.

The shortcut key is set using the same [key codes](#key-codes) as other [keyboard](#keyboard) methods. The commonly expected shortcuts are the 'F1' to 'F12' keys but others are possible.

```c++
void widgetShortcutKey(uint8_t widgetId, uint8_t shortcutKey);
```

Any time the widget is shown and the key is pressed it 'clicks' the relevant widget and that keypress **cannot be read** with `readKeypress()` and **does not affect** `keyPressed()`. While the widget is hidden the shortcut key can be read normally.

**[Back to top](#table-of-contents)**

## Values

Many widgets can return a numeric value that tells you something about their state.

```c++
bool state(uint8_t widgetId);
void state(uint8_t widgetId, bool);
uint8_t widgetValue(uint8_t widgetId);
void widgetValue(uint8_t widgetId, uint8_t);
uint8_t numberOfOptions(uint8_t widgetId);
```

If a widget is intrinsically 'binary' like a checkbox, use state() to read it. Widgets with more values, for example a list box, are read with `widgetValue()`.

When you need to set the state or value of a widget, pass that value (either boolean or 0-255 uint8_t) to the same method.

Ordinarily you should know the number of options in a list box but the method `numberOfOptions` can report them back.

**[Back to top](#table-of-contents)**

## Events

```c++
bool widgetActive(uint8_t widgetId);
void widgetActive(uint8_t widgetId, bool);
void widgetPassive(uint8_t widgetId);
bool selectWidget(uint8_t widgetId);
bool widgetClicked(uint8_t widgetId);
```

**[Back to top](#table-of-contents)**

# Memory management

## Storing text

As a library that handles large chunks of text, whenever possible assign widget labels and other unchanging text content using the Arduino core `F()` macro. This places the text in the flash memory permanently and the widget data structure only stores a pointer to that.

If you use a String or character array, the widget will store a copy of the text in heap memory leaving less free working memory for your other code. If a chunk of text is something that naturally changes frequently this is unavoidable but will eventually lead to heap fragmentation. Try to restrict this to things that really need it, for example user input fields.

Use of the F() macro is supported and tested on the following architectures.

- AVR
- ESP8266
- ESP32

On other architectures, all text will be stored in heap memory.

**[Back to top](#table-of-contents)**

# Widget count

Different architectures have a different default maximum number of widgets.

- AVR - 20
- ESP8266 - 50
- ESP32 - 50

**[Back to top](#table-of-contents)**

# Known issues

- Widgets do not function properly when using the double width or double size attribute.
- Serial speeds of 230400 and above are unreliable and cause 'phantom typing'.
- Use with a Stream class over TCP/IP causes immediate watchdog errors on ESP8266.
- No explicit support for unicode/UTF-8 input, only ASCII. This is not an insurmountable problem it just needs more work.
- Input in text input widgets is restricted to the visible width. This is not an insurmountable problem it just needs more work.

**[Back to top](#table-of-contents)**