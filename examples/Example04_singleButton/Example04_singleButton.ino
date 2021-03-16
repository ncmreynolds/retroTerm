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
