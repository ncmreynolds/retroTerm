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
  terminal.setScrollWindow(17,20); //Set up somewhere to show the events without disrupting the widget
  widgetId = terminal.newTextDisplay(1, 1, 40, 15, F("Widget 1 label"), COLOUR_GREEN, OUTER_BOX | BOX_SINGLE_LINE | LABEL_IN_BOX | LABEL_CENTRED ); //Create a green widget
  terminal.setWidgetContent(widgetId,F("\
# Heading level 1\r\
## Heading level 2\r\
### Heading level 3\r\
\r\
An sincerity so __extremity__ he additions. Her yet there truth merit. Mrs all projecting favourable now unpleasing. Son law garden chatty temper. Oh _children_ provided to mr elegance marriage strongly. Off can admiration prosperous now devonshire diminution law. \r\
\r\
- First item\r\
- Second item\r\
- Third item\r\
- Fourth item\r\
\r\
Him boisterous invitation dispatched had connection inhabiting projection. By mutual an mr danger garret edward an. Diverted as strictly exertion addition no _disposal_ by stanhill. This call wife do so sigh no gate felt. You and abode __spite__ order get. Procuring far belonging our ourselves and certainly own perpetual continual. It elsewhere of sometimes or my certainty. Lain no as five or at high. Everything travelling set how law literature. \r\
\r\
")); //Text display widget show text with limited markdown capability. The \r are needed to make each line/paragraph.
  terminal.widgetShortcutKey(widgetId,f1Pressed); //Assign a shortcut key of F1, once selected with this or the mouse up and down keys can be used
  terminal.showWidget(widgetId); //Make the widget visible, all widgets start 'invisible' for later display
}

void loop() {
  if(terminal.widgetClicked(widgetId)){ //This clears the 'click' on checking
    terminal.scroll("Text display click " + String(numberOfClicks++)); //Print inside the scroll window
  }
  terminal.houseKeeping();  //You MUST run housekeeping to show/detect any changes or events
}
