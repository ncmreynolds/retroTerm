#include <retroTerm.h> //Include terminal library
retroTerm terminal; //Create a terminal instance
uint8_t button1Id = 0; //Record the button ID globally
uint8_t button2Id = 0; //Record the button ID globally
uint8_t button3Id = 0; //Record the button ID globally
uint32_t numberOfClicks[3] = {1,1,1}; //Record the number of clicks

void processClick(uint8_t widgetClicked) {
  if(widgetClicked == button1Id) {
    terminal.scroll("Button 1 click " + String(numberOfClicks[0]++)); //Print inside the scroll window
  } else if(widgetClicked == button2Id) {
    terminal.scroll("Button 2 click " + String(numberOfClicks[1]++)); //Print inside the scroll window
  } else if(widgetClicked == button3Id) {
    terminal.scroll("Button 3 click " + String(numberOfClicks[2]++)); //Print inside the scroll window
  } else {
    terminal.scroll("Unknown click"); //Print inside the scroll window
  }
}

void setup() {
  Serial.begin(115200);   //Initialise the Serial stream
  terminal.begin(Serial); //Initialise the library
  terminal.eraseScreen(); //Clear the screen
  terminal.hideCursor(); //Hide the terminal cursor
  terminal.enableMouse(); //Capture the mouse so it can be used with widgets
  terminal.setScrollWindow(4,12); //Set up somewhere to show the events without disrupting the button
  button1Id = terminal.newButton(1, 1, 15, 3, F("Button 1"), COLOUR_RED, OUTER_BOX | BOX_SINGLE_LINE); //Create a green button in a box
  button2Id = terminal.newButton(16, 1, 15, 3, F("Button 2"), COLOUR_GREEN, OUTER_BOX | BOX_SINGLE_LINE); //Create a green button in a box
  button3Id = terminal.newButton(31, 1, 15, 3, F("Button 3"), COLOUR_BLUE, OUTER_BOX | BOX_SINGLE_LINE); //Create a green button in a box
  terminal.widgetShortcutKey(button1Id,f1Pressed); //Assign a shortcut key of F1
  terminal.widgetShortcutKey(button2Id,f2Pressed); //Assign a shortcut key of F2
  terminal.widgetShortcutKey(button3Id,f3Pressed); //Assign a shortcut key of F3
  terminal.showWidget(button1Id); //Make the button visible, all widgets start 'invisible' for later display
  terminal.showWidget(button2Id); //Make the button visible, all widgets start 'invisible' for later display
  terminal.showWidget(button3Id); //Make the button visible, all widgets start 'invisible' for later display
  terminal.setClickCallback(processClick);    //retroTerm callback for click events
}

void loop() {
  terminal.houseKeeping();  //You MUST run housekeeping to show/detect any changes or events
}
