/*
 * retroterm Terminal capability demo.
 * 
 * This sketch helps you check if your terminal emulator can handle all the commands it needs to work with the library.
 * 
 * The sketch cycles through the tests, press a key to skip to the next. The tests are...
 * 
 * Probing the terminal size
 * Basic character attributes bold/faint/underlined/blinking/inverse (blinking does not blink in PuTTY)
 * Double width and double height characters
 * Basic 8-colour support, foreground and background
 * 16-colour support, combining basic forground colours with 'bright'
 * 256-colour support
 * Box drawing characters, as used by the widget library
 * Scroll window, which fixes vertical scrolling to the centre of the terminal
 * Mouse support
 * 
 */

#include <retroTerm.h>    //Include serial terminal library

retroTerm terminal;       //Create a terminal instance

void setup() {
  Serial.begin(115200);
  terminal.begin(Serial);
}

void loop() {
  probingDemo();
  attributeDemo();
  largeFontDemo();
  eightColourDemo();
  sixteenColourDemo();
  twoHundredAndFiftySixColourDemo();
  boxDrawingDemo();
  scrollingWindowDemo();
  mouseDemo();
}

/*
 * The 'newPage' function uses some 'primitves' from the library
 * to clear the screen and show a title at the top of the page before every test
 */

void newPage(String title) {
  terminal.reset();             //Resets the terminal, clearing colours etc.
  terminal.hideCursor();        //Hide the cursor
  terminal.printAt(1,1,title + ", press a key or wait 30s to finish");  //Print the title at the top of the page
  terminal.println();           //Print a blank line
}

/*
 * The 'endPage' function tidies up after the test
 */
void endPage() {
  terminal.houseKeeping();
  if(terminal.keyPressed())
  {
    terminal.readKeypress();
  }
  terminal.houseKeeping();
}
