/*
 *	It's not ncurses but it is a bundle of stuff to help create ncurses style interfaces
 *
 *	Some Unicode characters are used to draw boxes etc. to help with broader support
 *
 *
 *	VT100 commands and responses cribbed from...
 *
 *	http://braun-home.net/michael/info/misc/VT100_commands.htm
 *
 *	https://www.vt100.net/docs/vt510-rm/contents.html and various other sources online.
 *
 *	This code is  released under the terms of the GNU General Public License as published by the Free Software 
 *	Foundation, either version 3 of the License, or (at your option) any later version.
 *	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 *	the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *	See the GNU General Public License for more details.
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *
 *
 *
 *
 */
 
#define PROCESS_MARKDOWN
#define tabStopWidth 5

#ifndef retroTerm_h
#define retroTerm_h
#include <Arduino.h>

//Some limits
#ifdef __AVR__
constexpr const uint8_t _widgetObjectLimit = 20;
#elif defined(ESP8266)
constexpr const uint8_t _widgetObjectLimit = 50;
#elif defined(ESP32)
constexpr const uint8_t _widgetObjectLimit = 75;
#elif defined(CORE_TEENSY)
constexpr const uint8_t _widgetObjectLimit = 50;
#elif defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_GENERIC_RP2040) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040)
constexpr const uint8_t _widgetObjectLimit = 50;
#else
constexpr const uint8_t _widgetObjectLimit = 30;
#endif

//Defines for event callbacks

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define RETROTERM_CLICK_CALLBACK std::function<void(uint8_t)> clickCallback
#define RETROTERM_TYPING_CALLBACK std::function<void(uint8_t)> typingCallback
#else
#define RETROTERM_CLICK_CALLBACK void (*clickCallback)(uint8_t)
#define RETROTERM_TYPING_CALLBACK void (*typingCallback)(uint8_t)
#endif

//#define retroTerm_DYNAMIC_OBJECT_ALLOCATION			//Uncomment to allow dynamic object storage allocation, which saves memory but may cause heap fragmentation

//Colours and attributes set for printing with the methods above. This is composed as a bitmask for easy saving/comparison, just OR them together to set

constexpr const uint16_t COLOR_BLACK = 				0x0008;	//US spelling
constexpr const uint16_t COLOR_RED = 				0x0009;
constexpr const uint16_t COLOR_GREEN =				0x000a;
constexpr const uint16_t COLOR_YELLOW =	 			0x000b;
constexpr const uint16_t COLOR_BLUE = 				0x000c;
constexpr const uint16_t COLOR_MAGENTA =			0x000d;
constexpr const uint16_t COLOR_CYAN =			 	0x000e;
constexpr const uint16_t COLOR_WHITE =	 			0x000f;

constexpr const uint16_t COLOUR_BLACK = 			0x0008;	//UK spelling
constexpr const uint16_t COLOUR_RED = 				0x0009;
constexpr const uint16_t COLOUR_GREEN =				0x000a;
constexpr const uint16_t COLOUR_YELLOW = 			0x000b;
constexpr const uint16_t COLOUR_BLUE = 				0x000c;
constexpr const uint16_t COLOUR_MAGENTA = 			0x000d;
constexpr const uint16_t COLOUR_CYAN = 				0x000e;
constexpr const uint16_t COLOUR_WHITE = 			0x000f;		

constexpr const uint16_t BACKGROUND_COLOR_BLACK = 	0x0080;	//US spelling
constexpr const uint16_t BACKGROUND_COLOR_RED = 	0x0090;
constexpr const uint16_t BACKGROUND_COLOR_GREEN = 	0x00a0;
constexpr const uint16_t BACKGROUND_COLOR_YELLOW = 	0x00b0;
constexpr const uint16_t BACKGROUND_COLOR_BLUE = 	0x00c0;
constexpr const uint16_t BACKGROUND_COLOR_MAGENTA = 0x00d0;
constexpr const uint16_t BACKGROUND_COLOR_CYAN = 	0x00e0;
constexpr const uint16_t BACKGROUND_COLOR_WHITE = 	0x00f0;

constexpr const uint16_t BACKGROUND_COLOUR_BLACK = 	0x0080;	//UK spelling
constexpr const uint16_t BACKGROUND_COLOUR_RED = 	0x0090;
constexpr const uint16_t BACKGROUND_COLOUR_GREEN = 	0x00a0;
constexpr const uint16_t BACKGROUND_COLOUR_YELLOW = 0x00b0;
constexpr const uint16_t BACKGROUND_COLOUR_BLUE = 	0x00c0;
constexpr const uint16_t BACKGROUND_COLOUR_MAGENTA =0x00d0;
constexpr const uint16_t BACKGROUND_COLOUR_CYAN = 	0x00e0;
constexpr const uint16_t BACKGROUND_COLOUR_WHITE = 	0x00f0;

constexpr const uint16_t ATTRIBUTE_BRIGHT =			0x0100;	//Used for 16 colour mode, foreground colours only, sometimes the same as bold depending on terminal
constexpr const uint16_t ATTRIBUTE_BOLD =			0x0200;	//Don't use bold and faint at the same time
constexpr const uint16_t ATTRIBUTE_FAINT =			0x0400;	//Don't use bold and faint at the same time
constexpr const uint16_t ATTRIBUTE_UNDERLINE =		0x0800;
constexpr const uint16_t ATTRIBUTE_BLINK =			0x1000;
constexpr const uint16_t ATTRIBUTE_INVERSE =		0x2000;
constexpr const uint16_t ATTRIBUTE_DOUBLE_WIDTH =	0x4000;	//Only usable for whole lines, use carefuly to avoid messed up output
constexpr const uint16_t ATTRIBUTE_DOUBLE_SIZE =	0x8000;	//This needs care in use, it is printed as a top and bottom half and can mess up easily. Avoid

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
constexpr const uint8_t fsPressed =			28;	//The ` key on the keyboard
constexpr const uint8_t enterPressed =		29;
constexpr const uint8_t unknownKeyPressed =	31;
constexpr const uint8_t noKeyPressed =		127; //Normally you shouldn't see this, used as no key

//Used to provide keyboard shortcut labels

//constexpr const char *keyLabels[28] = {
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
const char keyLabel_00[] PROGMEM = "Esc";
const char keyLabel_01[] PROGMEM = "F1";
const char keyLabel_02[] PROGMEM = "F2";
const char keyLabel_03[] PROGMEM = "F3";
const char keyLabel_04[] PROGMEM = "F4";
const char keyLabel_05[] PROGMEM = "F5";
const char keyLabel_06[] PROGMEM = "F6";
const char keyLabel_07[] PROGMEM = "F7";
const char keyLabel_08[] PROGMEM = "F8";
const char keyLabel_09[] PROGMEM = "F9";
const char keyLabel_10[] PROGMEM = "F10";
const char keyLabel_11[] PROGMEM = "F11";
const char keyLabel_12[] PROGMEM = "F12";
const char keyLabel_13[] PROGMEM = "Backspace";
const char keyLabel_14[] PROGMEM = "Insert";
const char keyLabel_15[] PROGMEM = "Home";
const char keyLabel_16[] PROGMEM = "PgUp";
const char keyLabel_17[] PROGMEM = "Del";
const char keyLabel_18[] PROGMEM = "End";
const char keyLabel_19[] PROGMEM = "PgDn";
const char keyLabel_20[] PROGMEM = "Tab";
const char keyLabel_21[] PROGMEM = "BackTab";
const char keyLabel_22[] PROGMEM = "Left";
const char keyLabel_23[] PROGMEM = "Up";
const char keyLabel_24[] PROGMEM = "Down";
const char keyLabel_25[] PROGMEM = "Right";
const char keyLabel_26[] PROGMEM = "Break";
const char keyLabel_27[] PROGMEM = "Return";
const char keyLabel_28[] PROGMEM = "FileSeparator";
const char keyLabel_29[] PROGMEM = "Enter";
constexpr const char *keyLabels[30] PROGMEM = {
	keyLabel_00,
	keyLabel_01,
	keyLabel_02,
	keyLabel_03,
	keyLabel_04,
	keyLabel_05,
	keyLabel_06,
	keyLabel_07,
	keyLabel_08,
	keyLabel_09,
	keyLabel_10,
	keyLabel_11,
	keyLabel_12,
	keyLabel_13,
	keyLabel_14,
	keyLabel_15,
	keyLabel_16,
	keyLabel_17,
	keyLabel_18,
	keyLabel_19,
	keyLabel_20,
	keyLabel_21,
	keyLabel_22,
	keyLabel_23,
	keyLabel_24,
	keyLabel_25,
	keyLabel_26,
	keyLabel_27,
	keyLabel_28,
	keyLabel_29
	};
#else
constexpr const char *keyLabels[30] = {
	"Esc",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"Bkspc",
	"Ins",
	"Home",
	"PgUp",
	"Del",
	"End",
	"PgDn",
	"Tab",
	"BackTab",
	"Left",
	"Up",
	"Down",
	"Right",
	"Brk",
	"Rtn",
	"FileSeparator",
	"Enter"
	};
#endif

//Box drawing characters order is top left, top, top right, side, bottom left, bottom right, right tee, left tee, up arrow, down arrow, left arrow, right arrow, shaded block, solid block, marker block, empty square, square with x, empty circle, filled circle
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
//Single line/light
const char retroTermCharacterTable000[] PROGMEM = "\u250c";	//Top left corner
const char retroTermCharacterTable001[] PROGMEM = "\u2500";	//Horizontal
const char retroTermCharacterTable002[] PROGMEM = "\u2510";	//Top right corner
const char retroTermCharacterTable003[] PROGMEM = "\u2502";	//Vertical
const char retroTermCharacterTable004[] PROGMEM = "\u2514";	//Bottom left corner
const char retroTermCharacterTable005[] PROGMEM = "\u2518";	//Bottom right corner
const char retroTermCharacterTable006[] PROGMEM = "\u251c"; //Tee right
const char retroTermCharacterTable007[] PROGMEM = "\u2524"; //Tee left
const char retroTermCharacterTable008[] PROGMEM = "\u25b2";	//Up arrow
const char retroTermCharacterTable009[] PROGMEM = "\u25bc";	//Down arrow
const char retroTermCharacterTable010[] PROGMEM = "\u25c4";
const char retroTermCharacterTable011[] PROGMEM = "\u25ba";
const char retroTermCharacterTable012[] PROGMEM = "\u2591";
const char retroTermCharacterTable013[] PROGMEM = "\u2588";
const char retroTermCharacterTable014[] PROGMEM = "\u25a0";	//Marker block
const char retroTermCharacterTable015[] PROGMEM = "\u25c7";
const char retroTermCharacterTable016[] PROGMEM = "\u25c6";
const char retroTermCharacterTable017[] PROGMEM = "\u25cb";
const char retroTermCharacterTable018[] PROGMEM = "\u25cf";
//Double line/heavy
const char retroTermCharacterTable100[] PROGMEM = "\u2554";	//Top left corner
const char retroTermCharacterTable101[] PROGMEM = "\u2550";	//Horizontal
const char retroTermCharacterTable102[] PROGMEM = "\u2557";	//Top right corner
const char retroTermCharacterTable103[] PROGMEM = "\u2551";	//Vertical
const char retroTermCharacterTable104[] PROGMEM = "\u255a";	//Bottom left corner
const char retroTermCharacterTable105[] PROGMEM = "\u255d";	//Bottom right corner
const char retroTermCharacterTable106[] PROGMEM = "\u2560"; //Tee right
const char retroTermCharacterTable107[] PROGMEM = "\u2563"; //Tee left
const char retroTermCharacterTable108[] PROGMEM = "\u25b2";	//Up arrow
const char retroTermCharacterTable109[] PROGMEM = "\u25bc";	//Down arrow
const char retroTermCharacterTable110[] PROGMEM = "\u25c4";
const char retroTermCharacterTable111[] PROGMEM = "\u25ba";
const char retroTermCharacterTable112[] PROGMEM = "\u2593";
const char retroTermCharacterTable113[] PROGMEM = "\u2588";
const char retroTermCharacterTable114[] PROGMEM = "\u25a0";	//Marker block
const char retroTermCharacterTable115[] PROGMEM = "\u25c7";
const char retroTermCharacterTable116[] PROGMEM = "\u25c6";
const char retroTermCharacterTable117[] PROGMEM = "\u25cb";
const char retroTermCharacterTable118[] PROGMEM = "\u25cf";
//Pure text
const char retroTermCharacterTable200[] PROGMEM = "+";		//Top left corner
const char retroTermCharacterTable201[] PROGMEM = "-";		//Horizontal
const char retroTermCharacterTable202[] PROGMEM = "+";		//Top right corner
const char retroTermCharacterTable203[] PROGMEM = "|";		//Vertical
const char retroTermCharacterTable204[] PROGMEM = "+";		//Bottom left corner
const char retroTermCharacterTable205[] PROGMEM = "+";		//Bottom right corner
const char retroTermCharacterTable206[] PROGMEM = "+";		//Tee right
const char retroTermCharacterTable207[] PROGMEM = "+";		//Tee left
const char retroTermCharacterTable208[] PROGMEM = "^";		//Up arrow
const char retroTermCharacterTable209[] PROGMEM = "v";		//Down arrow
const char retroTermCharacterTable210[] PROGMEM = "<";
const char retroTermCharacterTable211[] PROGMEM = ">";
const char retroTermCharacterTable212[] PROGMEM = "x";
const char retroTermCharacterTable213[] PROGMEM = "#";
const char retroTermCharacterTable214[] PROGMEM = "=";		//Marker block
const char retroTermCharacterTable215[] PROGMEM = "o";
const char retroTermCharacterTable216[] PROGMEM = "*";
const char retroTermCharacterTable217[] PROGMEM = "o";
const char retroTermCharacterTable218[] PROGMEM = "*";

constexpr const char *retroTermCharacterTable[57] PROGMEM = {
  retroTermCharacterTable000, retroTermCharacterTable001, retroTermCharacterTable002, retroTermCharacterTable003, retroTermCharacterTable004, retroTermCharacterTable005, retroTermCharacterTable006, retroTermCharacterTable007, retroTermCharacterTable008, retroTermCharacterTable009, retroTermCharacterTable010, retroTermCharacterTable011, retroTermCharacterTable012, retroTermCharacterTable013, retroTermCharacterTable014, retroTermCharacterTable015, retroTermCharacterTable016, retroTermCharacterTable017, retroTermCharacterTable018,
  retroTermCharacterTable100, retroTermCharacterTable101, retroTermCharacterTable102, retroTermCharacterTable103, retroTermCharacterTable104, retroTermCharacterTable105, retroTermCharacterTable106, retroTermCharacterTable107, retroTermCharacterTable108, retroTermCharacterTable109, retroTermCharacterTable110, retroTermCharacterTable011, retroTermCharacterTable112, retroTermCharacterTable113, retroTermCharacterTable114, retroTermCharacterTable115, retroTermCharacterTable116, retroTermCharacterTable117, retroTermCharacterTable118,
  retroTermCharacterTable200, retroTermCharacterTable201, retroTermCharacterTable202, retroTermCharacterTable203, retroTermCharacterTable204, retroTermCharacterTable205, retroTermCharacterTable206, retroTermCharacterTable207, retroTermCharacterTable208, retroTermCharacterTable209, retroTermCharacterTable210, retroTermCharacterTable211, retroTermCharacterTable212, retroTermCharacterTable213, retroTermCharacterTable214, retroTermCharacterTable215, retroTermCharacterTable216, retroTermCharacterTable217, retroTermCharacterTable218};
  
#else
constexpr const char *retroTermCharacterTable[3][19] = {
	{"\u250c","\u2500","\u2510","\u2502","\u2514","\u2518","\u251c","\u2524","\u25b2","\u25bc","\u25c4","\u25ba","\u2591","\u2588","\u25a0","\u25c7","\u25c6","\u25cb","\u25cf"},	//Single line/light
	{"\u2554","\u2550","\u2557","\u2551","\u255a","\u255d","\u2560","\u2563","\u25b2","\u25bc","\u25c4","\u25ba","\u2593","\u2588","\u25a0","\u25c7","\u25c6","\u25cb","\u25cf"},	//Double line/heavy
	{"+"     , "-"    ,"+"     ,"|"     ,"+"     ,"+"     ,"+"     ,"+"     ,"^"     ,"v"     ,"<"     ,">"     ,"x"     ,"#"     ,"="     ,"o"     ,"*"     ,"o"     ,"*"}
};
#endif


//Style is done as a bitmask, LSB is line style rest is flags
constexpr const uint8_t BOX_SINGLE_LINE =			0x00;		//Box drawing with single line
constexpr const uint8_t BOX_DOUBLE_LINE =			0x01;		//Box drawing with double line
constexpr const uint8_t BOX_ASCII =					0x02;		//Box drawing with plain ASCII
constexpr const uint8_t BOX_ASCII_2 =				0x03;		//Box drawing with plain ASCII
//1 bit for box setting
constexpr const uint8_t NO_BOX =					0x00;		//Draw an outer box
constexpr const uint8_t OUTER_BOX =					0x04;		//Draw an outer box
//2 bits for label setting
constexpr const uint8_t LABEL_IN_BOX =				0x08;		//Label inside separate box
constexpr const uint8_t LABEL_CENTRED =				0x10;		//Label centred, default is left justified
constexpr const uint8_t LABEL_RIGHT_JUSTIFIED =		0x20;		//Label right justified, default is left justified
//2 bits for shortcut settings
constexpr const uint8_t SHORTCUT_INLINE =			0x40;		//Shortcut inline with label
//1 bit for marking password field
constexpr const uint8_t PASSWORD_FIELD =			0x80;		//Prints content as dots, but the arrays hold the real string

class retroTerm
{
	public:

		//Constructor method
		retroTerm();
		//Destructor method
		~retroTerm();

		//Setup methods
		void begin(Stream &);				//Pass a reference to the stream used for the terminal. Often this will be &Serial
		void end();							//Clears any allocated memory

		//Run regularly to make sure things like line editing and mouse capture work. Not necessary for simple printing
		void  houseKeeping();

		//Probing commands that interrogate the terminal and return discovered information
		bool probeSize();					//Tries to set the width/length of the terminal by probing, this can take up to 1s
		uint8_t columns();					//Returns the current number of columns, use after calling probeSize()
		uint8_t lines();					//Returns the current number of lines, use after calling probeSize()
		bool probeType();					//Tries to probe the terminal type, mostly for information, this can take up to 3s. May not be useful!
		char* type();						//Returns the terminal type

		//Terminal 'bell'
		void enableBell();					//Enable the terminal bell. This is used for user feedback in line editing etc. and is enabled by default
		void disableBell();					//Disable the terminal bell. This disables during line editing etc., some other things may still cause the terminal to sound the bell
		void soundBell();					//Sound the terminal bell now. This does not override a disabled bell.
		
		//Cursor control
		void moveCursorTo(uint8_t, uint8_t);//Move the cursor
		void hideCursor();					//Hide the cursor
		void showCursor();					//Show the cursor
		void saveCursorPosition();			//Useful when printing outside the current area, also stores and restores attributes NEVER nest this with a printAt or printing to an object which does this internally
		void restoreCursorPosition();		//Useful when printing outside the current area
		void requestCursorPosition();		//Request the current cursor position from the terminal, this will take ~100ms to be sent by the terminal
		uint8_t currentCursorColumn();		//Current cursor position
		uint8_t currentCursorRow();			//Current cursor position

		//Screen/line control
		void reset();						//Resets the terminal and clears the screen, then applies the default printing attributes and enables the mouse if previously enabled
		void eraseScreen();					//Erase the whole screen
		void eraseLine();					//Erase the current line
		void insertLine();					//Insert a line
		void deleteLine();					//Delete a line
		template <class variableContent>
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR setTitle(variableContent title)	//Set the title bar of the terminal emulator, if supported/allowed
		#else
		void setTitle(variableContent title)	//Set the title bar of the terminal emulator, if supported/allowed
		#endif
		{
			_terminalStream->print(F("\033]0;"));
			_terminalStream->print(title);
			_terminalStream->print("\007");
		}

		//Scrolling region control
		void setScrollWindow(uint8_t, uint8_t);						//Sets the scolling window between two lines, inclusive. The whole width of the screen will scroll!
		//void setScrollWindow(uint8_t, uint8_t, uint8_t, uint8_t);	//Sets the scolling window to a square region. DOES NOT WORK!

		//Window/margin/origin commands
		//void setOriginMode();										//What is this for? Who knows

		//Get/set non-widget printing attributes
		uint16_t attributes();						//Get all the current character attributes for non-widget printing
		void attributes(uint16_t);					//Set all character attributes, colour, bold etc. by ORing constants. tracks current attributes to reduce number of sent control characters
		void resetAttributes();						//Reset all current character attributes to the default
		
		uint16_t defaultAttributes();				//Get the default attributes for non-widget printing, printAt etc.
		void defaultAttributes(uint16_t);			//Set the default attributes, useful at the start of a sketch if you want everything green etc.

		//Set foreground colour in 256-colour mode for non-widget printing. These override foreground attributes if set until all attributes are reset
		void foregroundColour(uint8_t);
		void foregroundColor(uint8_t);
		void clearForegroundColour();
		void clearForegroundColor();

		//Get/set widget printing attributes
		uint16_t defaultWidgetAttributes();			//Get the default attributes for widgets
		void defaultWidgetAttributes(uint16_t);		//Set the default attributes for widgets, useful at the start of a sketch if you want every widget green etc.

		uint16_t defaultLabelAttributes();			//Get the default attributes for widget labels
		void defaultLabelAttributes(uint16_t);		//Set the default attributes for widget labels, useful at the start of a sketch if you want every widget green etc.

		uint16_t defaultContentAttributes();		//Get the default attributes for widget content
		void defaultContentAttributes(uint16_t);	//Set the default attributes for widget content, useful at the start of a sketch if you want every widget green etc.
		
		//Get/set default widget drawing style
		uint16_t defaultWidgetStyle();				//Get the default style
		void defaultWidgetStyle(uint16_t);			//Set the default style, useful at the start of a sketch if you want every widget to have a double border etc.
		
		//Widget control
		void showWidget(uint8_t widgetId);					//Set a widget visible
		void hideWidget(uint8_t widgetId);					//Set a widget invisible
		void showWidget(uint8_t widgetId, bool visible);	//Explicitly set visibility of this widget
		bool widgetVisible(uint8_t widgetId);				//Check the visibility of a specific widget
		
		void hideAllWidgets();								//Make all widgets invisible
		void showAllWidgets();								//Make all widgets visible
		
		void widgetActive(uint8_t widgetId);				//Make this widget active, ie. it can be clicked etc.
		void widgetPassive(uint8_t widgetId);				//Make this widget passive, ie. it cannot be clicked on and is skipped over in keyboard navigation
								
		template <class variableContent>
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR scroll(variableContent content)							//Convenience method to stick content in the scolling window
		#else
		void scroll(variableContent content)											//Convenience method to stick content in the scolling window
		#endif
		{
		  scroll(content,false);
		}
		template <class variableContent>
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR scroll(variableContent content, bool centred)			//Convenience method to stick content in the scolling window, send a 'true' if you want it centred
		#else
		void scroll(variableContent content, bool centred)			//Convenience method to stick content in the scolling window, send a 'true' if you want it centred
		#endif
		{
			saveCursorPosition();
			hideCursor();
			moveCursorTo(1,_scrollWindowBottom);	//Scroll the last line up
			_terminalStream->println();
			restoreCursorPosition();
			//Print the new line
			if(centred)
			{
				printAt((_columns-String(content).length())/2,_scrollWindowBottom,content);
			}
			else
			{
				printAt(1,_scrollWindowBottom,content);
			}
			//Fill in the lines at the side
		}
			
		//Single key inputs, requires periodic calling of houseKeeping() to work, if a widget is selected it may capture some keypresses like the shortcut key
		
		bool userIsTyping();					//True if there has been input from a user recently, use to check for inactivity
		bool keyPressed();						//True if there is a keypress available to read
		uint8_t readKeypress();					//Return the key pressed, no it doesn't support unicode, just ASCII and the constants below to keep it smaller
		retroTerm& setTypingCallback(RETROTERM_TYPING_CALLBACK);	//Set a callback for typing

				
		//Mouse support, requires periodic calling of houseKeeping() to work
		
		void enableMouse();						//Enables mouse capture, if supported by the terminal application
		void disableMouse();					//Disables mouse input
		uint8_t mouseColumn();					//Returns last reported mouse X
		uint8_t mouseRow();						//Returns last reported mouse Y
		bool mouseButtonDown();					//Mouse button down event. Resets on read
		bool mouseButtonUp();					//Mouse button up event. Resets on read
		bool mouseWheelDown();					//Mouse wheel down. Resets on read
		bool mouseWheelUp();					//Mouse wheel up. Resets on read
				
		//Box drawing convenience methods to avoid the tedious looking at unicode tables, they are overloaded heavily so be careful when calling
		
		void drawBox(uint8_t, uint8_t, uint8_t, uint8_t);														//x y w h
		void drawBox(uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);												//x y w h attributes
		void drawBox(uint8_t, uint8_t, uint8_t, uint8_t, uint16_t, uint8_t);									//x y w h attributes style
		void drawBoxWithScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint32_t);						//x y w h scrollbarPosition scrollbarLength
		void drawBoxWithScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint32_t, uint16_t);			//x y w h scrollbarPosition scrollbarLength attributes
		void drawBoxWithScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint32_t, uint16_t, uint8_t);	//x y w h scrollbarPosition scrollbarLength attributes style
		
		void drawBoxWithTitle(uint8_t, uint8_t, uint8_t, uint8_t, const char *);													//x y w h title
		void drawBoxWithTitle(uint8_t, uint8_t, uint8_t, uint8_t, const char *, uint16_t);											//x y w h title attributes
		void drawBoxWithTitle(uint8_t, uint8_t, uint8_t, uint8_t, const char *, uint16_t, uint8_t);									//x y w h title attributes style
		void drawBoxWithTitleAndScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, const char *, uint32_t, uint32_t);					//x y w h title scrollbarPosition scrollbarLength
		void drawBoxWithTitleAndScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, const char *, uint32_t, uint32_t, uint16_t);			//x y w h title scrollbarPosition scrollbarLength attributes
		void drawBoxWithTitleAndScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, const char *, uint32_t, uint32_t, uint16_t, uint8_t);	//x y w h title scrollbarPosition scrollbarLength attributes style
		//PROGMEM variants
		void drawBoxWithTitle(uint8_t, uint8_t, uint8_t, uint8_t, const __FlashStringHelper*);														//x y w h title
		void drawBoxWithTitle(uint8_t, uint8_t, uint8_t, uint8_t, const __FlashStringHelper*, uint16_t);											//x y w h title attributes
		void drawBoxWithTitle(uint8_t, uint8_t, uint8_t, uint8_t, const __FlashStringHelper*, uint16_t, uint8_t);									//x y w h title attributes style
		void drawBoxWithTitleAndScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, const __FlashStringHelper*, uint32_t, uint32_t);						//x y w h title scrollbarPosition scrollbarLength
		void drawBoxWithTitleAndScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, const __FlashStringHelper*, uint32_t, uint32_t, uint16_t);			//x y w h title scrollbarPosition scrollbarLength attributes
		void drawBoxWithTitleAndScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, const __FlashStringHelper*, uint32_t, uint32_t, uint16_t, uint8_t);	//x y w h title scrollbarPosition scrollbarLength attributes style
		
		void clearBox(uint8_t, uint8_t, uint8_t, uint8_t);														//x y w h
		void clearBox(uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);											//x y w h attributes
		void clearWidget(uint8_t);																				//widgetId
		
		void centredTextBox(String);					//label
		void centredTextBox(String, uint16_t);			//label attributes
		void centredTextBox(String, uint8_t);			//label style
		void centredTextBox(String, uint16_t, uint8_t);	//label attributes style
		
		/*
		 *	These classes and methods create and manage widgets, which have a non-zero ID, an ID of zero implies failure/non-existent
		 *
		 *	Heavily overloaded to allow for relying on defaults
		 *
		 *
		 *
		 */

		//enum class _widgetTypes {button, checkbox, radioButton, textInput, textDisplay, textLog, listBox, label, tab, slider};				//Enum for Widget types
		enum class _widgetTypes {button, checkbox, radioButton, listBox, textInput, textDisplay, textLog};				//Enum for Widget types
		 
		//Widget methods common to all types
		uint8_t newWidget(_widgetTypes type, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t attributes, uint8_t style);									//Add unlabelled widget
		uint8_t newWidget(_widgetTypes type, uint8_t x, uint8_t y, uint8_t w, uint8_t h, char* label, uint16_t attributes, uint8_t style);						//Add labelled widget
		uint8_t newWidget(_widgetTypes type, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* label, uint16_t attributes, uint8_t style);				//Add labelled widget PROGMEM variant
		uint8_t newWidget(_widgetTypes type, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const __FlashStringHelper* label, uint16_t attributes, uint8_t style);	//Add labelled widget PROGMEM variant

		bool widgetExists(uint8_t);					//Does a widget ID exist, useful to see if too many have been made
		bool deleteWidget(uint8_t);					//Delete a widget (if it exists, which is checked first)
		uint8_t numberOfWidgets();					//How many widgets are defined, useful for choosing if to destroy things to save memory
		uint8_t maximumNumberOfWidgets();			//Maximum number of widgets, useful for choosing if to destroy things to save memory
		
		void moveWidget(uint8_t widgetId, uint8_t x, uint8_t y);	//Move a widget
		void resizeWidget(uint8_t widgetId, uint8_t w, uint8_t h);	//Resize a widget
		void refreshAllWidgets();									//Set all widgets to redraw completely

		//Widget selection
		bool selectWidget(uint8_t widgetId);				//Select a widget
		void deselectWidget();								//Deselect the current widget and ensure nothing is selected
		
		//Shortcuts
		void widgetShortcutKey(uint8_t widgetId, uint8_t);	//Set a keyboard shortcut on a widget, this interacts like a mouse click

		//Click events
		bool widgetClicked(uint8_t widgetId);					//Is THIS widget clicked, resets on read
		uint8_t widgetClicked();								//Is any widget clicked, returns zero otherwise, resets on read
		retroTerm& setClickCallback(RETROTERM_CLICK_CALLBACK);	//Set a callback for widget clicks

		void widgetAttributes(uint8_t, uint16_t);	//Set widget attributes
		void labelAttributes(uint8_t, uint16_t);	//Set widget label attributes
		void contentAttributes(uint8_t, uint16_t);	//Set widget content attributes
		void widgetStyle(uint8_t, uint8_t);			//Set widget style
		
		uint8_t columnsAvailable(uint8_t widgetId)
		{
			return(_columnsAvailable(widgetId -1));
		}

		//Label methods
		bool setWidgetLabel(uint8_t widgetId, char* label);							//Add/change a label to a widget
		bool setWidgetLabel(uint8_t widgetId, String label);						//Add/change a label to a widget
		bool setWidgetLabel(uint8_t widgetId, const char* label);					//Add/change a label to a widget
		bool setWidgetLabel(uint8_t widgetId, const __FlashStringHelper* label);	//Add/change a label to a widget PROGMEM version
		bool deleteWidgetLabel(const uint8_t widgetId);								//Delete the label for a widget, returns true if there was one to delete

		//Content methods
		bool setWidgetContent(uint8_t widgetId, char*);								//Add/change widget content char array
		bool setWidgetContent(uint8_t widgetId, String);							//Add/change widget content String version
		bool setWidgetContent(uint8_t widgetId, const char*);						//Add/change widget content string literal version
		bool setWidgetContent(uint8_t widgetId, const __FlashStringHelper*);		//Add/change widget content PROGMEM version
		bool deleteWidgetContent(uint8_t widgetId);									//Delete the widget content, returns true if there was one to delete
		bool appendWidgetContent(uint8_t widgetId, char*);							//Add widget content char array
		bool appendWidgetContent(uint8_t widgetId, String);							//Add widget content String version
		bool appendWidgetContent(uint8_t widgetId, const char*);					//Add widget content string literal version
		bool appendWidgetContent(uint8_t widgetId, const __FlashStringHelper*);		//Add widget content PROGMEM version
		bool scrollDownWidgetContent(uint8_t widgetId);								//Add a blank line at the top
		bool prependWidgetContent(uint8_t widgetId, char*);							//Add widget content char array at the top
		bool prependWidgetContent(uint8_t widgetId, String);						//Add widget content String version at the top
		bool prependWidgetContent(uint8_t widgetId, const char*);					//Add widget content string literal version at the top
		bool prependWidgetContent(uint8_t widgetId, const __FlashStringHelper*);	//Add widget content PROGMEM version at the top
		uint32_t contentOffset(uint8_t widgetId);									//Current content offset (0 if invalid widget)
		uint32_t contentSize(uint8_t widgetId);										//Current content size in bytes
		bool contentOffset(uint8_t widgetId, uint32_t);								//Set current content offset
		
		uint8_t lines(uint8_t widgetId);											//Number of lines available for content
		uint8_t columns(uint8_t widgetId);											//Number of columns available for content

		//Stored value methods
		bool state(uint8_t widgetId);									//What is the boolean state of this widget (used for checkboxes/radio buttons)
		void state(uint8_t widgetId, bool);								//Set the boolean state of this widget (used for checkboxes/radio buttons)
		uint8_t widgetValue(uint8_t widgetId);							//Return the current 'value' of the widget, also which option is selected in a listbox
		void widgetValue(uint8_t widgetId, uint8_t);					//Set the current 'value' of the widget, also which option is selected in a listbox
		uint8_t numberOfOptions(uint8_t widgetId);						//Number of options in a listbox, otherwise meaningless

		//Buttons
		
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
		#else
		uint8_t newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
		#endif
		{
			return(newWidget(_widgetTypes::button, x, y, w, h, _defaultAttributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#else
		uint8_t newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::button, x, y, w, h, attributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#else
		uint8_t newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::button, x, y, w, h, attributes, style));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, labelType label)
		#else
		uint8_t newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, labelType label)
		#endif
		{
			return(newWidget(_widgetTypes::button, x, y, w, h, label, _defaultAttributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, labelType label, const uint16_t attributes)
		#else
		uint8_t newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, labelType label, const uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::button, x, y, w, h, label, attributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, labelType label, const uint16_t attributes, const uint8_t style)
		#else
		uint8_t newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, labelType label, const uint16_t attributes, uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::button, x, y, w, h, label, attributes, style));
		}
		
		//Checkboxes
		
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
		#else
		uint8_t newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
		#endif
		{
			return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, _defaultAttributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
		#else
		uint8_t newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, attributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
		#else
		uint8_t newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, attributes, style));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newCheckbox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#else
		uint8_t newCheckbox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#endif
		{
			return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, _defaultAttributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newCheckbox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#else
		uint8_t newCheckbox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, attributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newCheckbox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#else
		uint8_t newCheckbox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, attributes, style));
		}
		
		//Radio buttons

		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newRadioButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
		#else
		uint8_t newTextInput(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
		#endif
		{
			return(newWidget(_widgetTypes::textInput, x, y, w, h, _defaultAttributes, _defaultStyle));
		}		
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#else
		uint8_t newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::radioButton, x, y, w, h, attributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#else
		uint8_t newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::radioButton, x, y, w, h, attributes, style));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newRadioButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#else
		uint8_t newRadioButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#endif
		{
			return(newWidget(_widgetTypes::radioButton, x, y, w, h, label, _defaultAttributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newRadioButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#else
		uint8_t newRadioButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::radioButton, x, y, w, h, label, attributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newRadioButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#else
		uint8_t newRadioButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::radioButton, x, y, w, h, label, attributes, style));
		}

		//Text input areas used for interactive text entry with backspace, delete, home and end keys functional. Other non-text keys, like F1 etc. remain readable as single keys

		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
		#else
		uint8_t newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
		#endif
		{
			return(newWidget(_widgetTypes::textInput, x, y, w, h, _defaultAttributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#else
		uint8_t newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::textInput, x, y, w, h, attributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#else
		uint8_t newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::textInput, x, y, w, h, attributes, style));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextInput(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#else
		uint8_t newTextInput(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#endif
		{
			return(newWidget(_widgetTypes::textInput, x, y, w, h, label, _defaultAttributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextInput(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#else
		uint8_t newTextInput(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::textInput, x, y, w, h, label, attributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextInput(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#else
		uint8_t newTextInput(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::textInput, x, y, w, h, label, attributes, style));
		}

		//Manage the content of the text input areas, above
		
		bool contentChanged(uint8_t widgetId);			//True when an input field has changed since first set up. Reading this clears the flag
		char* retrieveContent(uint8_t widgetId);		//Return a pointer to the string from a text entry field, they don't have to have finished typing
		void clearContent(uint8_t widgetId);			//Clears the string and field where the user was typing
		
		//Text display areas, this is static text not expected to chance just be read, use setWidgetContent to set the text
		
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
		#else
		uint8_t newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
		#endif
		{
			return(newWidget(_widgetTypes::textDisplay, x, y, w, h, _defaultAttributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#else
		uint8_t newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::textDisplay, x, y, w, h, attributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#else
		uint8_t newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::textDisplay, x, y, w, h, attributes, style));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#else
		uint8_t newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#endif
		{
			return(newWidget(_widgetTypes::textDisplay, x, y, w, h, label, _defaultAttributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#else
		uint8_t newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::textDisplay, x, y, w, h, label, attributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#else
		uint8_t newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::textDisplay, x, y, w, h, label, attributes, style));
		}
		
		//Text logs, which are for scrolling 'log' type windows where you can items to top and bottom. They DO NOT store the content long term. Use appendWidgetContent and prependWidget, rather than setWidgetContent

		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
		#else
		uint8_t newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
		#endif
		{
			return(newWidget(_widgetTypes::textLog, x, y, w, h, _defaultAttributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#else
		uint8_t newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::textLog, x, y, w, h, attributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#else
		uint8_t newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::textLog, x, y, w, h, attributes, style));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#else
		uint8_t newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#endif
		{
			return(newWidget(_widgetTypes::textLog, x, y, w, h, label, _defaultAttributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#else
		uint8_t newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::textLog, x, y, w, h, label, attributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#else
		uint8_t newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::textLog, x, y, w, h, label, attributes, style));
		}
		
		//List boxes, a 'select' with all options in a single text string separated by \r

		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
		#else
		uint8_t newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
		#endif
		{
			return(newWidget(_widgetTypes::listBox, x, y, w, h, _defaultAttributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#else
		uint8_t newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::listBox, x, y, w, h, attributes, _defaultStyle));
		}
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#else
		uint8_t newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::listBox, x, y, w, h, attributes, style));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#else
		uint8_t newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label)
		#endif
		{
			return(newWidget(_widgetTypes::listBox, x, y, w, h, label, _defaultAttributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#else
		uint8_t newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes)
		#endif
		{
			return(newWidget(_widgetTypes::listBox, x, y, w, h, label, attributes, _defaultStyle));
		}
		template <typename labelType>
		#if defined(ESP8266) || defined(ESP32)
		uint8_t ICACHE_FLASH_ATTR newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#else
		uint8_t newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, labelType label, uint16_t attributes, uint8_t style)
		#endif
		{
			return(newWidget(_widgetTypes::listBox, x, y, w, h, label, attributes, style));
		}
		
		ICACHE_FLASH_ATTR class widget							//widget is a nested class of retroTerm
		{
			public:
			
				//Constructor method
				#if defined(ESP8266) || defined(ESP32)
				widget()
				#else
				widget()
				#endif
				{
					//Do nothing
				}
				//Destructor method
				#if defined(ESP8266) || defined(ESP32)
				~widget()
				#else
				~widget()
				#endif
				{
					//Do nothing
				}

				_widgetTypes type;					//Differentiate between different widget types

				//State flags
				uint16_t currentState = 0x011C;		//Use a bitmask for boolean state information to reduce the memory footprint of widgets
													//0x0001 signifies 'visible'
													//0x0002 signifies 'displayed'
													//0x0004 signifies 'widget changed and needs redisplaying'
													//0x0008 signifies 'label changed and needs redisplaying'
													//0x0010 signifies 'content changed needs redisplaying'
													//0x0020 signifies 'scrollbar/tracker changed and needs redisplaying'
													//0x0040 signifies 
													//0x0080 signifies 
													//0x0100 signifies 'active' ie. can it be interacted with using keyboard or mouse. Inactive widgets are visible but unresponsive
													//0x0200 signifies 'clicked' with mouse or the shortcut key has been pressed
													#if defined(PROCESS_MARKDOWN)
													//0x0400 signifies bold
													//0x0800 signifies italic
													//0x1000 signifies bold italic
													//0x2000 signifies heading
													#endif
													//0x4000 signifies label is stored in FLASH memory, not SRAM. Used with AVR/ESP8266/ESP32 implementation
													//0x8000 signifies content is stored in FLASH memory, not SRAM. Used with AVR/ESP8266/ESP32 implementation
																										
				uint8_t value = 0;					//Generic field for storing a 'value' such as which option is clicked, has it changed or the state of a button

				//Layout
				uint8_t x = 0;						//Top left corner of the widget, 0 implies non-existence
				uint8_t y = 0;						//
				uint8_t w = 0;						//Width of the widget
				uint8_t h = 0;						//Height of the widget

				//Styling (optional)
				uint16_t attributes;							//Attributes of the 'main', usually outer bit of the widget
				uint8_t style = OUTER_BOX | BOX_SINGLE_LINE;	//Style of the box itself

				//Label (optional)
				char* label = nullptr;				//Text label on widget
				uint16_t labelAttributes;			//Attributes of the text on the label
				
				//Keyboard shortcut (optional)
				uint8_t shortcut = noKeyPressed;	//Shortcut ket, start as nothing, causes a 'click' when used
				
				//Content (optional) used for windows with big chunks of text in, also for the typing buffer
				char* content = nullptr;			//The widget DOES NOT take a copy of the content, so it must be retained in scope by the user application
				uint32_t contentSize = 0;			//Size is the size in bytes
				uint32_t contentLength = 0;			//Length means display 'length' ie. lines of text, number of options in a list box etc.
				uint32_t contentOffset = 0;			//Offset of the section to display, used for scrolling and editing content
				uint16_t contentAttributes;			//Attributes of the text being displayed or edited
				
		};

		//Printing methods - done with templates (hence inside this file) but this should be transparent to a user of the library
		template <class variableContent>
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR print(variableContent content)
		#else
		void print(variableContent content)
		#endif
		{
			if(_currentAttributes & ATTRIBUTE_DOUBLE_SIZE)
			{
				_printDouble(String(content));
			}
			else
			{
				_terminalStream->print(content);
			}
		}
		template <class variableContent>
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR println(variableContent content)
		#else
		void println(variableContent content)
		#endif
		{
			if(_currentAttributes & ATTRIBUTE_DOUBLE_SIZE)
			{
				_printDouble(String(content));
				_terminalStream->println();
			}
			else
			{
				_terminalStream->println(content);
			}
		}
		template <class variableContent>
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR printAt(uint8_t x, uint8_t y, variableContent content)						//Print with current attributes
		#else
		void printAt(uint8_t x, uint8_t y, variableContent content)						//Print with current attributes
		#endif
		{
			printAt(x, y, content, _currentAttributes);
		}
		template <class variableContent>
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR printAt(uint8_t x, uint8_t y, variableContent content, uint16_t specificAttributes)	//Print with specific attributes
		#else
		void printAt(uint8_t x, uint8_t y, variableContent content, uint16_t specificAttributes)	//Print with specific attributes
		#endif
		{
			saveCursorPosition();
			hideCursor();
			attributes(specificAttributes);
			moveCursorTo(x,y);
			if(_currentAttributes & ATTRIBUTE_DOUBLE_SIZE)
			{
				_printDouble(String(content));
			}
			else
			{
				_terminalStream->print(content);
			}
			restoreCursorPosition();
		}
		template <class variableContent>
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR printCentred(variableContent content)					//Print dead centre of the screen
		#else
		void printCentred(variableContent content)					//Print dead centre of the screen
		#endif
		{
			printAt((_columns-String(content).length())/2,_lines/2,content);
		}
		template <class variableContent>
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR printCentred(uint8_t y, variableContent content)			//Print centred on a specific line
		#else
		void printCentred(uint8_t y, variableContent content)			//Print centred on a specific line
		#endif
		{
			printAt((_columns-String(content).length())/2,y,content);
		}
		#if defined(ESP8266) || defined(ESP32)
		void ICACHE_FLASH_ATTR println()												//Good old println() which will _always_ live on
		#else
		void println()												//Good old println() which will _always_ live on
		#endif
		{
			_terminalStream->println();
		}

	protected:	//protected things are here because they are used by widgets

		//Size and position
		uint8_t _columns = 80;						//Number of columns, retrieved by probeSize()
		uint8_t _lines = 24;						//Number of lines, retrieved by  probeSize()

		//Cursor state
		uint8_t _cursorX = 0;						//Current cursor position
		uint8_t _cursorY = 0;						//Current cursor position
		bool _cursorVisible = true;					//Current vibility
		
		//Attributes
		uint16_t _currentAttributes = 				//Default printing attributes
		COLOUR_WHITE | BACKGROUND_COLOUR_BLACK;
		uint16_t _defaultAttributes =				//Default when resetting attributes
		COLOUR_WHITE | BACKGROUND_COLOUR_BLACK;
		uint16_t _defaultWidgetAttributes =			//Default when resetting attributes
		COLOUR_WHITE | BACKGROUND_COLOUR_BLACK;
		uint16_t _defaultLabelAttributes =			//Default when resetting attributes
		COLOUR_WHITE | BACKGROUND_COLOUR_BLACK;
		uint16_t _defaultContentAttributes =		//Default when resetting attributes
		COLOUR_WHITE | BACKGROUND_COLOUR_BLACK;

		//Styles
		uint8_t _currentStyle = BOX_SINGLE_LINE;
		uint8_t _defaultStyle = BOX_SINGLE_LINE;	//Default when setting style for boxes etc.

		//Colours in 256-colour mode, which override foreground attributes if set
		bool _256colourSet = false;
		uint8_t _256colour = 0;
		
		//Cursor state saving						//The terminal can save ONE cursor state and go back to it
		uint16_t _savedAttributes = 0;				//This means we also track it and can avoid sending
		uint8_t _savedCursorX = 0;					//Unnecessary escape sequences
		uint8_t _savedCursorY = 0;
		bool _savedCursorState = true;
		bool _saved256colourSet = false;;
		uint8_t _saved256colour = 0;

		//Widget objects
		#ifdef retroTerm_DYNAMIC_OBJECT_ALLOCATION
		widget * _widgets[_widgetObjectLimit];		//Array of pointers to widgets
		#else
		widget _widgets[_widgetObjectLimit];		//Assign this at startup to avoid fragmentation
		#endif
		uint8_t _numberOfWidgets = 0;				//How many widgets are currently allocated
		//uint8_t _numberOfWidgetShortcuts = 0;		//How many widgets are currently visible
		bool _widgetChanged = false;				//At least one widget has changed so an update is required
		
		//Checkbox objects
		//checkbox _checkboxes[_checkboxObjectLimit];	//Assign this at startup to avoid fragmentation
		//uint8_t _numberOfcheckboxes = 0;			//How many widgets are currently allocated
		//uint8_t _numberOfcheckboxShortcuts = 0;		//How many widgets are currently visible


	private:
	
		Stream *_terminalStream = nullptr;			//The stream used for the terminal
		
		//Terminal type
		bool _terminalTypeReceived = false;			//Used to track if terminal type has been received
		char* _terminalType;						//Terminal type, as recovered from probeType()
	
		//Scrolling Window
		bool _scrollWindowSet = false;
		uint8_t _scrollWindowTop = 1;
		uint8_t _scrollWindowBottom = _lines;

		//Mouse
		uint8_t _mouseStatus = 0x00;				//Track mouse status in a bitmask
													//0x01 = enabled
													//0x02 = primary button down
													//0x04 = primary button up
													//0x08 = middle button down
													//0x10 = mouse wheel roll down
													//0x20 = mouse wheel roll up
													//0x40 = secondary button down
													//0x80 = 
													
		uint8_t _mouseX = 0;						//Last reported mouse X
		uint8_t _mouseY = 0;						//Last reported mouse Y
		//Click recording
		uint8_t _clickedWidget = _widgetObjectLimit;//FIRST widget clicked, resets on read
		RETROTERM_CLICK_CALLBACK;					//Click callback function 

		//Terminal bell
		bool _bellEnabled = true;					//Is the terminal 'bell' enabled
				
		//Cursor state
		bool _cursorPositionReceived = false;		//Used during probing terminal size
		
		//Keyboard reading
		bool _escapeReceived = false;
		uint32_t _escapeReceivedAt = 0;
		char _escapeBuffer[24] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		uint8_t escapeBufferPosition = 0;
		void _resetEscapeBuffer();						//Clears the escape buffer to process more input
		uint8_t _typingBufferMaxLength(uint8_t);		//The maximum length of the typing buffer when in use
		RETROTERM_TYPING_CALLBACK;						//Typing callback function
		
		bool _readInput();								//Reads the incoming data from the terminal and turns it into kepresses, mouse clicks etc. true if something was received and understood
		bool _processInput();							//Look for clicks on widgets, true if one was processed
		void _clickWidget(uint8_t);						//Do per-widget actions on clicks
		bool _findNextClick();							//Clear the current click and find the next one to process, if any
		uint8_t _selectedWidget = _widgetObjectLimit;	//The last selected widget, start as none selected
		void _displayChanges();							//Look for changes and update the terminal
		bool _widgetsOverlap(uint8_t, uint8_t);			//Checks if two widgets overlap each other, used by _displayChanges()
		uint8_t _contentXorigin(uint8_t);				//Where to start printing widget content
		uint8_t _contentYorigin(uint8_t);				//Where to start printing widget content
		uint16_t _textCapacity(uint8_t);				//How much space there is for text in a widget
		uint8_t _linesAvailable(uint8_t);				//Lines available for content in a widget
		uint8_t _columnsAvailable(uint8_t);				//Columns available for content in a widget
		bool _scrollbarNeeded(uint8_t);					//Works out if a scrollbar is necessary to display the content contained in a widget
		void _handleScrollbarClicks(uint8_t);			//Looks for scrollbar clicks and shifts content as necessary
		void _displayWidgetOuterBox(uint8_t);			//Displays the outer box of a widget

		void _displayKeyboardShortcut(uint8_t);			//Show keyboard shortcut on a widget, doing all the lifting of centering etc.
		void _printKeyboardShortcut(uint8_t);			//Does the actual print. Uses appropriate method if shortcut is in a PROGMEM
		bool _shortcutMatches(uint8_t);					//Does case insensitive matching of keyboard shortcuts

		void _displayLabel(uint8_t widgetId);			//Show the label on a widget (will include shortcut if inline)
		void _printLabel(uint8_t);						//Does the actual print. Uses appropriate method if label is in a PROGMEM

		void _displayContent(uint8_t);											//Show content in a widget
		uint8_t _displayLineOfContent(uint8_t widgetIndex, uint32_t offset, bool print);	//Shows ONE LINE of formatted content from the current offset, used by the above function to fill widgets
		uint32_t _previousLineOffset(uint8_t widgetIndex, uint32_t offset);	//Finds the offset of the previous line
		
		uint16_t _labelLength(uint8_t);					//Label length, in bytes. Uses appropriate method if content is in a PROGMEM
		uint16_t _shortcutLength(uint8_t);				//Shortcut length, in bytes. Uses appropriate method if content is in a PROGMEM
		uint16_t _contentSize(uint8_t);					//Content length, in bytes. Uses appropriate method if content is in a PROGMEM
		
		const uint32_t _typingTimeout = 10000ul;
		uint8_t _lastKeypress = noKeyPressed;
		uint32_t _lastInputActivity = 0;
		uint8_t _typingXposition(uint8_t _widgetId);	//Where to place the cursor when typing
		uint8_t _typingYposition(uint8_t _widgetId);	//Where to place the cursor when typing
		
		//Widget utility methods
		bool _widgetExists(uint8_t);					//Does a particular widget exist?
		void _scrollDown(uint8_t);						//Scroll down one line of displayed content (required amount varies with style)
		void _scrollUp(uint8_t);						//Scroll up one line of displayed content (required amount varies with style)

		void _calculateContentLength(uint8_t widgetIndex);						//Calculate content length, which varies by type of widget
		uint8_t _calculateNumberOfOptions(uint8_t widgetIndex);					//Number of options to fit in a listbox, only computed on initial setting of content
		uint8_t _calculateNumberOfLines(uint8_t widgetIndex);					//Number of lines of text to fit in a text box, only computed on initial setting of content
		//Markdown/wordwrap utility methods
		uint8_t _lineSize(uint8_t widgetIndex, uint32_t offset);					//Size (bytes) of current line, WORDWRAP limited from current content offset. Used for smooth scrolling of large text content
		uint8_t _wordLength(uint8_t widgetIndex, uint32_t offset);					//Length of current word from the current content offset. Used for smooth scrolling of text content
		uint8_t _runLength(uint8_t widgetIndex, uint32_t offset, char character);	//Length of a specificed character from the current content offset. Used for smooth scrolling of text content
		uint8_t _whitespaceLength(uint8_t widgetIndex, uint32_t offset);			//Length of current whitespace from the current content offset. Used for smooth scrolling of text content
		char _currentCharacter(uint8_t widgetIndex, uint32_t offset);				//Character at the current content offset. Used in the wordwrap routines to abstract the PROGMEM kerfuffle

		//uint8_t _previousLineLength(uint8_t widgetIndex, uint32_t offset);		//Length of previous line, WORDWRAP limited from current content offset. Used for smooth scrolling of large text content
		//uint8_t _reverseWordLength(uint8_t widgetIndex, uint32_t offset);		//Length of the word STARTING AT THE END!

		
		void _printDouble(char *);					//Internal method for handling double size printing
		void _printDouble(String);					//Internal method for handling double size printing

		//Methods used by widgets etc.
		void _drawBoxLines (uint8_t, uint8_t, uint8_t, uint8_t, bool, bool, uint32_t, uint32_t, uint16_t, uint8_t);	//The method that actually draws the lines to make a box
		void _drawScrollbar(uint8_t, uint8_t, uint8_t, uint8_t, bool, uint32_t, uint32_t, uint16_t, uint8_t);	//Just draw the scrollbar from the box
		void _printUnicodeCharacter(uint8_t, uint8_t);
		#if defined(__AVR__)																						//AVR specific function to pull Unicode characters from a PROGMEM and print them
		void _printProgStr(const char *);																			//Print a char array stored in PROGMEM
		#endif
		
		
};

#endif
