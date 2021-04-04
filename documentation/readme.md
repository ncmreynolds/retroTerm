# retroTerm Documentation

Documentation is broken down into three main sections, initialisation/housekeeping, terminal control/interaction and widget creation/control. There are also some notes on memory management and known issues.

# Table of Contents

1. [Initialisation and housekeeping](#initialisation-and-housekeeping)
   1. [Initialisation](#initialisation)
   1. [Housekeeping](#housekeeping)
   1. [Probing the terminal](#probing-the-terminal)
1. [Terminal control](#terminal-control)
	1. [Reset and clear](#reset-and-clear)
	1. [Title](#title)
	1. [Cursor control](#cursor-control)
	1. [Attributes and colours](#attributes-and-colours)
	   1. [Eight colours](#eight-colour-attributes)
	   1. [256 colours](#256-colours)
1. [Output](#output)
   1. [Printing](#printing)
   2. [Boxes](#boxes)
   3. [Bell](#bell)
1. [Input](#input)
   1. [Keyboard](#keyboard)
      1. [Key codes](#key-codes)
   1. [Mouse](#mouse)
1. [Widgets](#widgets)
	1. [Widget IDs](#widget-ids)
	1. [Z-order](#z-order)
	1. [Styles](#styles)
	1. [Attributes](#widget-attributes)
	1. [Creating widgets](#creating-widgets)
	1. [Deleting widgets](#deleting-widgets)
	1. [Show and hide widgets](#show-and-hide-widgets)
	1. [Moving and resizing](#moving-and-resizing)
	1. [Content](#content)
	1. [Shortcuts](#shortcuts)
	1. [Values](#values)
	1. [Events](#values)
1. [Memory management](#memory-management)
   1. [Storing text](#storing-text)
   1. [Widget limits](#widget-limits)
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
It should be possible to find the number of columns and number of lines the terminal has. The library tries to probe this by moving the cursor to 255,255 and then requesting the current position. The expected behaviour from the terminal emulator is it will move as far as possible and this can be used to determine the size. Once this is done, the cursor will move back to its previous position. The `probeSize()` may take a second or so to complete, this function blocks until it receives a response or times out.

If `probeSize()` returns true, you can be fairly sure the size has been correctly probed and can use `columns()` and `lines()` to size the output you send form this. If `probeSize()` returns false, the library assumes a terminal of 80 columns x 24 lines.

As is standard on VT style terminals, the origin of the terminal is 1,1 at the top left corner. There is no column or line zero and this is sometimes used in the library to report an error.

```c++
bool probeType();
char* type();
```

Similar to probing the size, it is possible to probe the terminal type which is stored as a char array by the library, this is not overly useful without further work to map these types to something more meaningful.

**[Back to top](#table-of-contents)**

# Terminal control

The most basic functions in retroTerm apply to the whole terminal or just its 'current state', more complicated behaviour is built from these.

**[Back to top](#table-of-contents)**

## Reset and clear

```c++
void reset();
```

This function resets all the terminal settings to default and clears the currently display contents.

```c++
void eraseScreen();
```

This function clears the currently displayed terminal content. It does not change colours, attributes and so on.

**[Back to top](#table-of-contents)**

## Title

```c++
void setTitle(variableContent title);
```

Some terminal emulators allow you to set the 'title' of the window the terminal emulator, you can use this function to do so. The function is templated so accepts Strings, char arrays, or any reasonable thing you'd send to a `print()`, including strings stored in Flash with an F() macro.

**[Back to top](#table-of-contents)**

## Cursor control

As is standard for VT style terminal, all output happens from a 'cursor' that has a specific position and state. This is referenced to an origin of 1,1 which is the top left of the terminal.

The `moveCursorTo()` function moves the cursor directly to the specified column and row.

```c++
void moveCursorTo(uint8_t column, uint8_t row);
```
Most terminal haves the facility to 'hide' the cursor, which is often a 'blob' on the terminal. If printing output in many different places on the terminal, this stops the cursor seeming the 'flicker' as it moves around.

```c++
void hideCursor();
void showCursor();
```
Most terminals also have the ability to save and restore the state of the cursor, which also includes [attributes and colours](#attributes-and-colours). This allows you to save the cursor state, output whatever you want somewhere else then return. This is only 'one layer deep' so you should always save, output and restore.

```c++
void saveCursorPosition();
void restoreCursorPosition();
```
You can also request the current cursor position from the terminal application. The response can take a short while so your code should wait at least a second before assuming an update has happened.

```c++
void requestCursorPosition();
uint8_t currentCursorColumn();
uint8_t currentCursorRow();
```
Many terminals allow you to set the scrolling region of the screen to between two specified rows (inclusive). Any printing in this region that overflows will not scroll the whole terminal but just the lines specified. This is very useful for a 'logging' style window that is endlessly added to. Note it still scrolls the whole width of the screen, don't place anything you care about staying visible in the same region.

As this is done in the terminal application, not by the library redrawing areas over the Stream connection it is faster and more efficient than scrolling in a widget.

```c++
void setScrollWindow(uint8_t topRow, uint8_t bottomRow);
```

**[Back to top](#table-of-contents)**

## Attributes and Colours

There are two ways to set colours and attributes in the library, eight colour and 256-colour.

### Eight colour attributes

The default control over colour in the library is through the use of 'attributes'.

Attributes can handle eight foreground colours, eight background colours and a mix of other visual effects.  These use the most basic form of colour support in the terminal emulator and are most likely to be supported. Many of the library output functions like `printAt()` can be passed attributes and will change them for you in one function. All [widgets](#widgets) expect to use attributes.

The attributes are set by a bitmask, which has convenience shortcuts you can OR together. The bitmask is chosen so 'terminal default' is 0x00. There are synonyms with the US spelling of 'colour'.

Any time you set attributes, the library compares them with the current cursor state to reduce the number of changes sent.

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

Beware the 'double width' and 'double size' attributes. They apply to the whole row, two rows for double size and can be hard to work with.

You pass the combined attribute value to one of these functions and it changes any future output.

```c++
void attributes(uint16_t attributes);
void defaultAttributes(uint16_t attributes);
```

So to set the colour for later output to 'green on a black background' you might use.

`terminal.colour(COLOUR_GREEN | BACKGROUND_COLOUR_BLACK);`

You can also retrieve the current set attributes and `resetAttributes()` will set it to the previously specified default.

```c++
uint16_t attributes();
uint16_t defaultAttributes();
void resetAttributes();
```

### 256-colours

The other way of setting foreground colours is from the 256-colour palette. Not all terminals will support 256-colour mode. This is not supported by widgets (yet). You can see the palette of colours by running the example code, it *may* vary by terminal emulator.

When using the 256-colour palette, attributes no longer apply. You can switch back to outputting using attributes with `clearForegroundColour()`.

```c++
void foregroundColour(uint8_t);
void foregroundColor(uint8_t); //A synonym for US users
void clearForegroundColour();
void clearForegroundColor(); //A synonym for US users
```

**[Back to top](#table-of-contents)**

# Output

When sending output to the terminal, try not to mix straightforward print functions with widgets, as the library makes no effort to maintain output that isn't a widget if it is overwritten etc. Both styles of access to the terminal work simultaneously but you will need care to ensure they do not conflict.

## Printing

```c++
void print(variableContent content);
```

A good old fashioned print function, which outputs at the current cursor position, with the current [attributes](#attributes-and-colours). This function is templated so will output most types you send it as you'd expect without having to do things like `itoa()`.

```c++
void println(variableContent content);
```

Similar to `print()` this does a newline after the print, which may cause the terminal or scroll region to scroll up.

```c++
void printCentred(variableContent content);
void printCentred(uint8_t y, variableContent content);
```

These two functions will centre printed content. The first prints at the current row, the second centred on a specific row.

```c++
void printAt(uint8_t x, uint8_t y, variableContent content);
void printAt(uint8_t x, uint8_t y, variableContent content, uint16_t specificAttributes);
```

These two functions print at a specified column and row. The second variant will print with the specified [attributes](#colours-and-attributes).

```c++
void scroll(variableContent content);
void scroll(variableContent content, bool centred);
```

These two functions print at the bottom of the scrolling region, after moving the existing content up one line. These are fast and efficient compared to scrolling the content of a widget as the terminal emulator handles the scrolling and the contents do not need to be redrawn by the library.

This is one of the occasions where you might want to mix widgets and direct printing to the terminal.

See `setScrollWindow()` to set the scrolling region.

**[Back to top](#table-of-contents)**

## Boxes

As well as simple text, the functions for drawing boxes used by widgets are available directly.

```c++
void drawBox(uint8_t row, uint8_t column, uint8_t width, uint8_t height, uint16_t attributes, uint8_t style);
void drawBoxWithScrollbar(uint8_t row, uint8_t column, uint8_t width, uint8_t height, uint32_t currentScrollbarPosition, uint32_t maxScrollbarPostion, uint16_t attributes, uint8_t style);
void drawBoxWithTitle(uint8_t row, uint8_t column, uint8_t width, uint8_t height, variableContent title, uint16_t attributes, uint8_t style);
void drawBoxWithTitleAndScrollbar(uint8_t row, uint8_t column, uint8_t width, uint8_t height, variableContent title, uint32_t currentScrollbarPosition, uint32_t maxScrollbarPosition, uint16_t attributes, uint8_t style);
```

The four functions, do as described. Each is overload so the [attributes](#attributes-and-colours), [style](#styles) or title text can be omitted. The minimum parameters are column, row, width and height.

```c++
void centredTextBox(String text);
void centredTextBox(String text, uint16_t attributes);
void centredTextBox(String text, uint8_t style);
void centredTextBox(String text, uint16_t attributes, uint8_t style);
```

These four functions stick a box right in the centre of the screen with some text in. They're a relic of before the library supported widgets and deprecated.

You can also clear a box with spaces in preparation for some now content, replacing it with the default [attributes](#attributes). If you specify some attributes it will fill the box with these instead.

```c++
void clearBox(uint8_t column, uint8_t row, uint8_t width, uint8_t height);
void clearBox(uint8_t column, uint8_t row, uint8_t width, uint8_t height, uint16_t attributes);
```

**[Back to top](#table-of-contents)**

## Bell

There are basic functions for enabling/disabling and sounding the terminal bell. If enabled the bell is used by some widgets to indicate an error, for example when trying to delete characters from an already empty input etc.

```c++
void enableBell();
void disableBell();
void soundBell();
```

**[Back to top](#table-of-contents)**

# Input

The library normally expects to capture all input from the terminal, rather than the rest of the application reading directly from the Stream. If you do the latter, you may get unreliable behaviour.

Common non-alphanumeric keyboard presses (function keys, arrows etc.) have constants associated with them to make them easy to handle but if widgets are in use then often a widget will act on these keypresses and the application won't see them. This is particularly true of anything specified as a keyboard shortcut or keyboard navigation key (tab, back tab, arrows, page up, page down, enter etc.) that has a commonly understood meaning in a MS Windows style GUI.

**[Back to top](#table-of-contents)**

## Keyboard

```c++
bool userIsTyping();
```
This function returns true if the terminal has received a keypress or mouse activity recently. It is a useful to help your code to wait until somebody has 'finished typing' or using the mouse before doing something.

```c++
bool keyPressed();
```
This function returns true if there is a keypress waiting to be read.

```c++
uint8_t readKeypress()
```
Returns the ASCII code of the next keypress, plus assorted other keys mapped into 0-31. There is currently no Unicode/UTF-8 support for input from the keyboard.

### Key codes

The key codes used in the library are the traditional ASCII character set with some common keys on the PC keyboard mapped over little-used entries. 

Control codes for VT style terminals generally start with an escape character. As a result, if you use the Escape key in your application then the library waits a short while to see if valid control codes are sent, before passing the escape on. This delay is to be expected.

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

The Stream input buffer is limited, so if doing your own keyboard processing your code rather than using [widget events](#events) you should check for input regularly.

**[Back to top](#table-of-contents)**

## Mouse

These two functions enable and disable mouse capture, if the terminal supports it.

Once mouse support is enabled, it usually prevents the mouse doing cut/paste or bringing up the right-click menu in the terminal emulator. It may be worth having a hotkey set up to enable/disable mouse support if you want to be able to occasionally use the usual mouse functionality in the terminal emulator.

```c++
void enableMouse();
void disableMouse();
```
These functions return the last reported location of the mouse pointer. Depending on terminal support this is usually where it was last clicked, rather than where it currently is.

```c++
uint8_t mouseColumn();
uint8_t mouseRow();
```
These functions return true on the relevant mouse button event. They clear the button status once read. If the click was over an active widget then a widget [event](#events) will occur instead.

```c++
bool mouseButtonDown();
bool mouseButtonUp();
bool mouseWheelDown();
bool mouseWheelUp();
```

**[Back to top](#table-of-contents)**

# Widgets

The main purpose of this library is to allow the use of GUI-like widgets in the normally very 'dull' world of the Serial terminal. It's possible to create nicely interactive user interfaces with the selection of widgets available, although it does eat a chunk of your device's memory.

In this release there are eight widgets available. You are strongly recommended to try the example code to see how they behave. Broadly they behave like components you'd find in an older HTML form, or MS-Windows application so long as your terminal emulator supports the full range of behaviours.

Due to the limited memory of most microcontrollers, text widgets should be used with care.

- Button
- Checkbox
- Radio button (all visible ones for one group)
- List box
- Text input (A single line editing field for free form text entry)
- Text display (Display of static unchanging text information. This widget handles basic MarkDown for styling the content)
- Text log (Regularly updating text information like a 'log' or 'chat' window. New text can be added at the top or bottom, scrolling the existing text)

You should try the example code on your hardware to see how each type of widget behaves.

The widgets have been written to behave how you 'expect', for example a large amount text in a text display that does not fit inside the box has a 'scroll bar'. The text and can be paged through with the arrow keys on the keyboard, scroll wheel on the mouse or by clicking at the top/bottom of the scroll bar. It will not be as responsive as a real GUI, but it does behave broadly how people have come to expect.

Likewise, buttons can be clicked, checkboxes checked and so on.

When using a text input, the cursor is visible and can be moved with the arrow keys, home/end and you can do in place editing of the text with delete/backspace. There is not yet any support for cut/paste in a text input though.

**[Back to top](#table-of-contents)**

## Widget IDs

Widget IDs are simple 8-bit unsigned integers (uin8_t) to allow you to store them trivially in variables, arrays and so on. An ID of 0 denotes unassigned or a failure to assign due to lack of memory. You should always check an ID is non-zero when creating a new widget.

If a widget is deleted then its ID may be subsequently re-used, so you should ensure you no longer associate that ID with the deleted widget. Deleting widgets is not great for [memory management](#memory-management) and heap fragmentation will occur especially if they contain large amounts of text content.

You can check the current number of widgets and maximum with these two functions.

```c++
uint8_t numberOfWidgets();
uint8_t maximumNumberOfWidgets();
```

**[Back to top](#table-of-contents)**

## Z-order

There is no configurable Z-order (yet), only an implicit one based on higher widget IDs being 'higher'. You should consider this when creating new widgets as until IDs are re-used, they will be allocated in ascending numerical order.

Overlapping widgets are discouraged as it will cause much redrawing on the terminal, but it does work.

## Styles

Widgets have some very basic ability to 'style' them. To set a style, OR several constants together.

The box style relies on certain Unicode characters being available in the terminal emulator application, you should try the example code to see which works. At a worst case, `BOX_ASCII` will draw a box with standard ASCII characters.

Labels/titles will appear in different places depending on the type of widget. Where a widget is in a box, the title can be inside its own box if you use `LABEL_IN_BOX`.

Labels/titles can also be left-justified (default), centred (`LABEL_CENTRED`) or right-justified (`LABEL_RIGHT_JUSTIFIED`).

Shortcuts are usually displayed on the top-left border of a boxed widget, unless you use `SHORTCUT_INLINE` when it will be inline with the label/title.

If you use `PASSWORD_FIELD` with a text input, it will obscure all typing with blobs. Note it does not obscure the length of the string.

```
constexpr const uint8_t BOX_SINGLE_LINE =			0x00;		//Box drawing with single line
constexpr const uint8_t BOX_DOUBLE_LINE =			0x01;		//Box drawing with double line
constexpr const uint8_t BOX_ASCII =					0x02;		//Box drawing with plain ASCII
constexpr const uint8_t BOX_ASCII_2 =				0x03;		//Box drawing with plain ASCII
constexpr const uint8_t NO_BOX =					0x00;		//Draw an outer box
constexpr const uint8_t OUTER_BOX =					0x04;		//Draw an outer box
constexpr const uint8_t LABEL_IN_BOX =				0x08;		//Label inside separate box
constexpr const uint8_t LABEL_CENTRED =				0x10;		//Label centred, default is left justified
constexpr const uint8_t LABEL_RIGHT_JUSTIFIED =		0x20;		//Label right justified, default is left justified
constexpr const uint8_t SHORTCUT_INLINE =			0x40;		//Shortcut inline with label
constexpr const uint8_t PASSWORD_FIELD =			0x80;		//Prints content as dots, but the arrays hold the real string
```

So for example a widget style could be.

```c++
OUTER_BOX | LABEL_IN_BOX | LABEL_CENTRED | SHORTCUT_INLINE | BOX_DOUBLE_LINE
```

When creating a widget you can set the style, but it can also be changed later with this function, which will cause it to redraw completely.

```c++
void widgetStyle(uint8_t widgetId, uint8_t style);
```

You are advised to look at the example code to see how attributes look in practice.

## Widget attributes

Widgets make heavy use of attributes. The legibility and aesthetic of the UI is really helped by having slight variations between the widget components, for example, the body as `ATTRIBUTE_FAINT` so the outline box isn't glaring and the content as `ATTRIBUTE_BRIGHT` so it 'pops'.

You can check/control the default body, label/title and content attributes separately with the following functions. this is useful for setting a general look for your UI.

```
uint16_t defaultWidgetAttributes();
void defaultWidgetAttributes(uint16_t attributes);

uint16_t defaultLabelAttributes();
void defaultLabelAttributes(uint16_t attributes);

uint16_t defaultContentAttributes();
void defaultContentAttributes(uint16_t attributes);
```

 When creating a new widget if you set attributes they will apply to every part of it, overriding defaults.

If you need to change the widget attributes, you can do so with these functions. They will cause the widget to redraw whichever part changed.

```c++
void widgetAttributes(uint8_t widgetId, uint16_t attributes);
void labelAttributes(uint8_t widgetId, uint16_t attributes);
void contentAttributes(uint8_t widgetId, uint16_t attributes);
```

You are advised to look at the example code running in your terminal emulator to see how attributes look in practice.

**[Back to top](#table-of-contents)**

## Creating widgets

There's a function for creation of each type of widget that returns the newly created widget ID, or 0 if the creation failed. Your code should check the returned ID, not assume the widget was created succesfully.

These functions are templated so you can omit the label, attributes or style. The label can be a char array or ideally defined with the `F()` macro to store it in flash.

```c++
uint8_t newButton(uint8_t column, uint8_t row, uint8_t width, uint8_t height, variableType label, uint16_t attributes, uint8_t style);

uint8_t newCheckbox(uint8_t column, uint8_t row, uint8_t width, uint8_t height, variableType label, uint16_t attributes, uint8_t style);

uint8_t newRadioButton(uint8_t column, uint8_t row, uint8_t width, uint8_t height, variableType label, uint16_t attributes, uint8_t style);

uint8_t newListBox(uint8_t column, uint8_t row, uint8_t width, uint8_t height, variableType label, uint16_t attributes, uint8_t style);

uint8_t newTextInput(uint8_t column, uint8_t row, uint8_t width, uint8_t height, variableType label, uint16_t attributes, uint8_t style);

uint8_t newTextDisplay(uint8_t column, uint8_t row, uint8_t width, uint8_t height, variableType label, uint16_t attributes, uint8_t style);

uint8_t newScrollingTextDisplay(uint8_t column, uint8_t row, uint8_t width, uint8_t height, variableType label, uint16_t attributes, uint8_t style);


```

Widget IDs are allocated sequentially from 1 and once a widget is deleted may be re-used, starting at the lowest available ID.

**[Back to top](#table-of-contents)**

## Deleting widgets

You are advised to hide and show widgets as necessary rather than deleting them unless your device is short on memory or you are very clear they will not be used again.

To check a specific widget ID is valid and in use you can check it the following function.

```c++
bool widgetExists(uint8_t widgetId);
```

If you're certain you would like to delete a widget, use the following function. This returns true if the widget existed and was deleted.

```c++
bool deleteWidget(uint8_t widgetId);
```

**[Back to top](#table-of-contents)**

## Show and hide widgets

All widgets start as 'hidden' and need to be 'shown'. Ordinarily you are advised to hide widgets rather than delete them, if memory permits.

```c++
void showWidget(uint8_t widgetId);
void hideWidget(uint8_t widgetId);
void showWidget(uint8_t widgetId, bool visible);
```

If you need to check the visibility of a specific widget you can use this function.

```c++
bool widgetVisible(uint8_t widgetId);
```

These next functions are pretty self explanatory. Normally the library manages which widget are likely to need refreshing based on content changes, moves, resizes and any overlaps.

The `refreshAllWidgets()` function forces a complete redraw from lowest to highest visible widget ID and may be visibly slow.

The `clearWidget(uint8_t widgetId)` function clears the space occupied by a widget and **does not** flag it to redraw. This is mostly used internally by the library but might be occasionally useful.

```c++
void hideAllWidgets();
void showAllWidgets();
void refreshAllWidgets();
void clearWidget(uint8_t widgetId);
```

**[Back to top](#table-of-contents)**

## Moving and resizing

```c++
void moveWidget(uint8_t widgetId, uint8_t column, uint8_t row);
void resizeWidget(uint8_t widgetId, uint8_t width, uint8_t height);
```

These functions allow you to move or resize a widget after creation. Both cause a refresh of the widget in the terminal and potentially clipping of existing content or a blank space on the right hand side of a scrolling text widget where previously text was clipped.

**[Back to top](#table-of-contents)**

## Labels

Every widget can have a 'label' attached. Its appearance varies by widget and is affected by its [style](#styles). For larger widgets generally it looks like a 'title' but on a button/checkbox it works like a 'label'.

It is recommended you try the example code to see how they look in use. To help with [memory management](#memory-management) it is strongly advised you use the F() macro to set the label and don't change or delete it unless absolutely necessary. If you want to draw attention to a field, change its [attributes](#attributes) or [style](#styles) instead.

If you set the label further times, it will replace the previous one, freeing storage on heap if it was used.

```c++
bool setWidgetLabel(uint8_t widgetId, char* label);
bool setWidgetLabel(uint8_t widgetId, String label);
bool setWidgetLabel(uint8_t widgetId, const __FlashStringHelper* label);
```

Once set, if necessary, you can also delete the label with the following function.

```c++
bool deleteWidgetLabel(const uint8_t widgetId);
```

**[Back to top](#table-of-contents)**

## Content

The text widgets usually show content as well as the title/label. This is set with the functions below, which will accept most text types.

Content in a `textDisplay` is not expected to change often, if at all. If the content will not change you are strongly advised to use the `F()` macro so it is stored in flash memory.

Content in a `textInput` is expected to change and be read once the user is 'finished editing' and any content you supply will be copied into heap memory and expected to change. The maximum length of the edited text is the space visible onscreen.

Content in a `textLog` is expected to change constantly. Any content you supply will be lost once it scrolls off the top or bottom of the widget, which reserves just enough heap memory for what is currently visible. This can still be quite a lot of memory so in many cases it is worth considering careful use of `setScrollWindow()` and `scroll()` although this requires careful placement of widgets around it.

```c++
bool setWidgetContent(uint8_t widgetId, variableType content);
```

For the `textLog` widget **only**, you can append and prepend content at the top/bottom of the widget scrolling the existing content.

```c++
bool appendWidgetContent(uint8_t widgetId, variableType content);
bool prependWidgetContent(uint8_t widgetId, variableType content);
```

If you need to clear the content and free up memory you can use this function. If the widget ID exists and had content to delete it returns true.

```c++
bool deleteWidgetContent(uint8_t widgetId);
```

As a `textDisplay` will often have a chunk of text too big to display in the widget you can set and retrieve the current 'top' of what is displayed in the widget. This is changed by the scrollbar as a user pages through the text.

```c++
uint32_t contentOffset(uint8_t widgetId);
bool contentOffset(uint8_t widgetId, uint32_t offset);
```



**[Back to top](#table-of-contents)**

## Shortcuts

The library can attach a keyboard shortcut to a widget that allows it to be selected or 'clicked' with the keyboard. The placement and look of the shortcut is affected by the widget's [style](#styles). It is recommended you try the example code to see how they look.

The shortcut key is set using the same [key codes](#key-codes) as other [keyboard](#keyboard) functions. The commonly expected shortcuts are the 'F1' to 'F12' keys but others are possible.

```c++
void widgetShortcutKey(uint8_t widgetId, uint8_t shortcutKey);
```

Any time the widget is shown and the key is pressed it 'clicks' the relevant widget and that keypress **cannot be read** with `readKeypress()` and **does not affect** `keyPressed()`. While the widget is hidden the shortcut key can be read normally.

**[Back to top](#table-of-contents)**

## Values

Many widgets can return a numeric value that tells you something about their state.

If a widget is intrinsically 'binary' like a `checkbox`, use `state()` to read it. Widgets with more values, for example a list box, are read with `widgetValue()`. Ordinarily you should know the number of options in a list box but the function `numberOfOptions` can report them back.

```c++
bool state(uint8_t widgetId);
uint8_t widgetValue(uint8_t widgetId);
uint8_t numberOfOptions(uint8_t widgetId);
```

When you need to set the state or value of a widget, pass that value (either boolean or 0-255 uint8_t) to the same function. This will cause the widget to refresh.

```c++
void state(uint8_t widgetId, bool newState);
void widgetValue(uint8_t widgetId, uint8_t newValue);
```

For the `textInput` widget there are a separate set of functions for managing and retrieving the text. The widget traps most keyboard input but you can tell if the text in the field has been edited with `contentChanged()`, which clears the flag once read. This is helpful to avoid unnecessary handling of the input in your code.

```
bool contentChanged(uint8_t widgetId);
```

Your code can retrieve a pointer to the char array used to store the text in the `textInput` with `retrieveContent`. It is up to the application to allocate memory on heap and copy the text out (or not) for further processing of the contents of the `textInput`.

```c++
char* retrieveContent(uint8_t widgetId);
```

It is entirely possible to leave the text stored as part of the widget object and hide the widget. The space allocated on the heap for the text will not be freed unless you actively do so with the following function. This also clears it in the widget if it is shown.

```c++
void clearContent(uint8_t widgetId);
```

**[Back to top](#table-of-contents)**

## Events

Most widgets are capable of generating 'events' that show they've been interacted with in the terminal. To do they must be 'active', all widgets are 'active' by default, but this can be changed.

```c++
void widgetActive(uint8_t widgetId);
void widgetPassive(uint8_t widgetId);
```

The default event is 'clicked', which you can test with the following function. This resets the flag on reading. Typically you would check a widget has been 'clicked' and if it has read its new [value](#values). Shortcut keys and keyboard navigation also generates 'clicks'.

```c++
bool widgetClicked(uint8_t widgetId);
```

At any one time one visible widget will be the 'selected' widget. Clicking an active widget or using its shortcut key makes it the selected one. There might be occasions when you want to select the widget without causing a 'click' and you can use the following function for this.

```c++
bool selectWidget(uint8_t widgetId);
```

Reasons to do this are to shift focus to a specific widget to accept keyboard inputs that aren't defined as a shortcut, for example arrow keys scrolling a text display.

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

## Widget limits

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