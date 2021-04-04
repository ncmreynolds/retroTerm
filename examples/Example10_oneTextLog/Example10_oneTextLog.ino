#include <retroTerm.h> //Include terminal library
retroTerm terminal; //Create a terminal instance
uint8_t widgetId = 0; //Record the widget IDs globally
uint32_t numberOfClicks = 1; //Record the number of clicks
uint32_t lastRandomContentTime = 0;
uint32_t randomInterval = 1000;

void setup() {
  Serial.begin(115200);   //Initialise the Serial stream
  terminal.begin(Serial); //Initialise the library
  terminal.eraseScreen(); //Clear the screen
  terminal.hideCursor(); //Hide the terminal cursor
  terminal.enableMouse(); //Capture the mouse so it can be used with widgets
  terminal.setScrollWindow(18,24); //Set up somewhere to show the events without disrupting the widget
  widgetId = terminal.newTextLog(1, 1, 20, 10, F("Widget 1 label"), COLOUR_GREEN, OUTER_BOX | BOX_SINGLE_LINE | LABEL_IN_BOX | LABEL_CENTRED ); //Create a green widget
  terminal.appendWidgetContent(widgetId,F("First line")); //Append text at the bottom of the window
  terminal.widgetShortcutKey(widgetId,f1Pressed); //Assign a shortcut key of F1, once selected with this or the mouse up and down keys can be used
  terminal.showWidget(widgetId); //Make the widget visible, all widgets start 'invisible' for later display
}

void loop() {
  if(terminal.widgetClicked(widgetId)){ //This clears the 'click' on checking
    terminal.scroll("Text display click " + String(numberOfClicks++)); //Print inside the scroll window
  }
  if(millis() - lastRandomContentTime >= randomInterval)
  {
    lastRandomContentTime = millis();
    terminal.appendWidgetContent(widgetId,"Random " + String(randomInterval));
    randomInterval = random(1000,5000);
  }
  terminal.houseKeeping();  //You MUST run housekeeping to show/detect any changes or events
}
