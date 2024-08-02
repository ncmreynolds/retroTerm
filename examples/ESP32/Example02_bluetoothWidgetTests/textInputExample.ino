void textInputExample()
{
  newPage(F("Text input field test"));
  red = terminal.newTextInput(terminal.columns()/4,  terminal.lines()/2 - 2, terminal.columns()/2, 1, F("Red:"),   COLOUR_WHITE | BACKGROUND_COLOUR_RED, SHORTCUT_INLINE);    //Create three new widgets and record the IDs
  green = terminal.newTextInput(terminal.columns()/4, terminal.lines()/2, terminal.columns()/2, 4, F("Green"), COLOUR_GREEN, OUTER_BOX);  //The method returns 0 if it fails
  blue = terminal.newTextInput(terminal.columns()/4, terminal.lines()/2 + 4, terminal.columns()/2, 5, F("Blue (Password)"),  COLOUR_BLUE, OUTER_BOX | BOX_DOUBLE_LINE | LABEL_IN_BOX | LABEL_CENTRED | SHORTCUT_INLINE | PASSWORD_FIELD);   //Colour is the colour of the frame, lettering can be different
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
    terminal.setWidgetContent(blue,F("Blue password input"));          //Populate the field at the start
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
  terminal.deselectWidget();  //Deselect the currently typed in widget, to prevent more typing
  if(red)
  {
    terminal.widgetPassive(red);  //Stop the widget being clicked on again
  }
  if(green)
  {
    terminal.widgetPassive(green);  //Stop the widget being clicked on again
  }
  if(blue)
  {
    terminal.widgetPassive(blue);  //Stop the widget being clicked on again
  }
  terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  if(terminal.keyPressed())
  {
    terminal.readKeypress();    //Gobble up the enter key so the script will wait to show the strings
  }
  timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  if(terminal.keyPressed())
  {
    terminal.readKeypress();  //Gobble up the keypress so the script will wait for the next page
  }
  endPage();
}
