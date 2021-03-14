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

void newPage(String title)
{
  terminal.reset();
  terminal.probeSize();
  terminal.hideCursor();
  terminal.print(title);
  terminal.println();
  terminal.println();
}
void endPage()
{
  terminal.moveCursorTo(1,terminal.lines());
  terminal.print(F("Press any key to continue, or wait 30s"));
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();
    //delay(100);
  }
  terminal.readKeypress();
}
