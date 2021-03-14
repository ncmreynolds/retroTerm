void textInputExample()
{
  
  newPage(F("Text input field demo"));
  terminal.setScrollWindow(2,6);     //Set up somewhere to show the events without disrupting the buttons
  terminal.hideCursor();
  terminal.enableMouse();
  uint8_t red =   terminal.newTextInput(terminal.columns()/4,  9, terminal.columns()/2, 1, F("Red:"),   COLOUR_WHITE | BACKGROUND_COLOUR_RED, SHORTCUT_INLINE);    //Create three new widgets and record the IDs
  uint8_t green = terminal.newTextInput(terminal.columns()/4, 11, terminal.columns()/2, 4, F("Green"), COLOUR_GREEN, OUTER_BOX);  //The method returns 0 if it fails
  uint8_t blue =  terminal.newTextInput(terminal.columns()/4, 15, terminal.columns()/2, 5, F("Blue (Password)"),  COLOUR_BLUE, OUTER_BOX | BOX_DOUBLE_LINE | LABEL_IN_BOX | LABEL_CENTRED | SHORTCUT_INLINE | PASSWORD_FIELD);   //Colour is the colour of the frame, lettering can be different
  if(red)
  {
    terminal.labelAttributes(red, COLOUR_WHITE | ATTRIBUTE_BRIGHT | BACKGROUND_COLOUR_RED);     //Make the label text more emphasised, for better contrast
    terminal.scroll(F("Red text input field created"));
    terminal.showWidget(red);                                               //Make this widget visible
    terminal.widgetShortcutKey(red,f10Pressed);
    terminal.setWidgetContent(red,F("Red text input"));                     //Populate the field at the start
  }
  if(green)
  {
    terminal.labelAttributes(green, COLOUR_GREEN | ATTRIBUTE_BRIGHT); //Make the label text more emphasised, for better contrast
    terminal.scroll(F("Green text input field created"));
    terminal.showWidget(green);                                       //Make this widget visible
    terminal.widgetShortcutKey(green,f11Pressed);
    terminal.setWidgetContent(green,F("Green text input"));           //Populate the field at the start
  }
  if(blue)
  {
    terminal.labelAttributes(blue, COLOUR_BLUE | ATTRIBUTE_BRIGHT);    //Make the label text more emphasised, for better contrast
    terminal.scroll(F("Blue text input field created"));
    terminal.showWidget(blue);                                         //Make this widget visible
    terminal.widgetShortcutKey(blue,f12Pressed);
    terminal.setWidgetContent(blue,F("Blue text input"));              //Populate the field at the start
  }
  terminal.scroll(F("Press enter when finished to see the inputs"));   //It is for the application to decide what to do with everything that isn't a line editing key
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    if(terminal.widgetClicked(red))
    {
      terminal.scroll(F("Red text input field selected"));
    }
    if(terminal.widgetClicked(green))
    {
      terminal.scroll(F("Green text input field selected"));
    }
    if(terminal.widgetClicked(blue))
    {
      terminal.scroll(F("Blue text input field selected"));
    }
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  if(red && terminal.contentChanged(red))
  {
    terminal.scroll("Red string changed - " + String(terminal.retrieveContent(red)));
  }
  if(green && terminal.contentChanged(green))
  {
    terminal.scroll("Green string changed - " + String(terminal.retrieveContent(green)));
  }
  if(blue && terminal.contentChanged(blue))
  {
    terminal.scroll("Blue string changed - " + String(terminal.retrieveContent(blue)));
  }
  //Make the widgets inactive so they don't accept any more keypresses
  if(red)
  {
    terminal.widgetActive(red, false);
  }
  if(green)
  {
    terminal.widgetActive(green, false);
  }
  if(blue)
  {
    terminal.widgetActive(blue, false);
  }
  terminal.readKeypress();  //Gobble up the keypress so the script will wait to show the strings
  timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  //Delete the widgets for finishing
  terminal.deleteWidget(red);     //It is safe to try and delete a non-existent widget, the method will check before attempting to de-allocate memory etc.
  terminal.deleteWidget(green);
  terminal.deleteWidget(blue);
  terminal.readKeypress();        //Gobble up the keypress before running the next example
  terminal.disableMouse();
}
