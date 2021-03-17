#include <retroTerm.h> //Include terminal library
retroTerm terminal; //Create a terminal instance
uint8_t widgetId1 = 0; //Record the widget IDs globally
uint8_t widgetId2 = 0; //Record the widget IDs globally
uint32_t numberOfClicks = 1; //Record the number of clicks

void setup() {
  Serial.begin(115200);   //Initialise the Serial stream
  terminal.begin(Serial); //Initialise the library
  terminal.eraseScreen(); //Clear the screen
  terminal.hideCursor(); //Hide the terminal cursor
  terminal.enableMouse(); //Capture the mouse so it can be used with widgets
  terminal.setScrollWindow(4,12); //Set up somewhere to show the events without disrupting the widget
  widgetId1 = terminal.newRadioButton(1, 1, 20, 1, F("Widget 1 label"), COLOUR_GREEN); //Create a green widget
  widgetId2 = terminal.newRadioButton(1, 2, 20, 1, F("Widget 2 label"), COLOUR_GREEN); //Create a green widget
  terminal.widgetShortcutKey(widgetId1,f1Pressed); //Assign a shortcut key of F1
  terminal.widgetShortcutKey(widgetId2,f2Pressed); //Assign a shortcut key of F2
  //terminal.state(widgetId1,true); //Set the first widget true or 'selected'
  terminal.widgetValue(widgetId1,1); //Set the first widget true or 'selected'
  terminal.showWidget(widgetId1); //Make the widget visible, all widgets start 'invisible' for later display
  terminal.showWidget(widgetId2); //Make the widget visible, all widgets start 'invisible' for later display
}

void loop() {
  if(terminal.widgetClicked(widgetId1) || terminal.widgetClicked(widgetId2)){ //This clears the 'click' on checking
    if(terminal.state(widgetId1) == true)
    {
      terminal.scroll("Radio button 1, click " + String(numberOfClicks++)); //Print inside the scroll window
    }
    else if(terminal.state(widgetId2) == true)
    {
      terminal.scroll("Radio button 2, click " + String(numberOfClicks++)); //Print inside the scroll window
    }
  }
  terminal.houseKeeping();  //You MUST run housekeeping to show/detect any changes or events
}
