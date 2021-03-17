#include <retroTerm.h> //Include terminal library
retroTerm terminal; //Create a terminal instance
uint8_t widgetId = 0; //Record the widget IDs globally
uint32_t numberOfClicks = 1; //Record the number of clicks

void setup() {
  Serial.begin(115200);   //Initialise the Serial stream
  terminal.begin(Serial); //Initialise the library
  terminal.eraseScreen(); //Clear the screen
  terminal.hideCursor(); //Hide the terminal cursor
  terminal.enableMouse(); //Capture the mouse so it can be used with widgets
  terminal.setScrollWindow(8,16); //Set up somewhere to show the events without disrupting the widget
  widgetId = terminal.newTextInput(1, 1, 20, 5, F("Widget 1 label"), COLOUR_GREEN, OUTER_BOX | BOX_SINGLE_LINE | LABEL_IN_BOX | LABEL_CENTRED ); //Create a green widget
  //terminal.setWidgetContent(widgetId,F("Option 1\rOption 2\rOption 3")); //ListBox options are specified as a string or char array with options separated by \r
  terminal.widgetShortcutKey(widgetId,f1Pressed); //Assign a shortcut key of F1, once selected with this or the mouse up and down keys can be used
  terminal.widgetValue(widgetId,1); //Set the first widget true or 'selected'
  terminal.showWidget(widgetId); //Make the widget visible, all widgets start 'invisible' for later display
}

void loop() {
  if(terminal.widgetClicked(widgetId)){ //This clears the 'click' on checking
    terminal.scroll("Listbox click " + String(numberOfClicks++)); //Print inside the scroll window
  }
  if(terminal.contentChanged(widgetId)){ //This clears the 'changed' flag on checking
    terminal.scroll(terminal.retrieveContent(widgetId)); //Print inside the scroll window
  }
  //if(terminal.keyPressed() == enterPressed){ //The 'enter' key is not captured by the widget and can be read normally
  if(terminal.keyPressed()){
    terminal.scroll(terminal.retrieveContent(widgetId)); //Print inside the scroll window
    terminal.clearContent(widgetId); //Clear the content and start again
  }
  terminal.houseKeeping();  //You MUST run housekeeping to show/detect any changes or events
}
