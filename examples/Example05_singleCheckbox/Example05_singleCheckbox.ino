#include <retroTerm.h> //Include terminal library
retroTerm terminal; //Create a terminal instance
uint8_t widgetId = 0; //Record the widget ID globally
uint32_t numberOfClicks = 1; //Record the number of clicks

void setup() {
  Serial.begin(115200);   //Initialise the Serial stream
  terminal.begin(Serial); //Initialise the library
  terminal.eraseScreen(); //Clear the screen
  terminal.hideCursor(); //Hide the terminal cursor
  terminal.enableMouse(); //Capture the mouse so it can be used with widgets
  terminal.setScrollWindow(4,12); //Set up somewhere to show the events without disrupting the widget
  widgetId = terminal.newCheckbox(1, 1, 20, 3, F("Widget label"), COLOUR_GREEN); //Create a green widget
  terminal.widgetShortcutKey(widgetId,f1Pressed); //Assign a shortcut key of F1
  terminal.showWidget(widgetId); //Make the widget visible, all widgets start 'invisible' for later display
}

void loop() {
  if(terminal.widgetClicked(widgetId)) //This clears the 'click' on checking
    {
      terminal.scroll("Checbox click " + String(numberOfClicks++) + " value " + terminal.widgetValue(widgetId)); //Print inside the scroll window
    }
  terminal.houseKeeping();  //You MUST run housekeeping to show/detect any changes or events
}
