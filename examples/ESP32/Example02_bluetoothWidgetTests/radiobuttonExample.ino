void radioButtonExample()
{
  newPage(F("Radio button test"));
  red = terminal.newRadioButton(terminal.columns()/3 - 1, terminal.lines()/2, 16, 5, F("Red"),   COLOUR_WHITE, OUTER_BOX | SHORTCUT_INLINE);    //Create three new widgets and record the IDs
  green = terminal.newRadioButton(terminal.columns()/3, terminal.lines()/2+2, 14, 1, F("Green"), COLOUR_WHITE, SHORTCUT_INLINE);  //The method returns 0 if it fails
  blue = terminal.newRadioButton(terminal.columns()/3, terminal.lines()/2+3, 14, 1, F("Blue"),  COLOUR_WHITE, SHORTCUT_INLINE);   //Colour is the colour of the frame, lettering can be different
  if(red)
  {
    terminal.labelAttributes(red, COLOUR_RED | ATTRIBUTE_BRIGHT);     //Make the label text more emphasised, for better contrast
    terminal.scroll(F("Red radio button created"));
    terminal.showWidget(red);                                       //Make this button visible
    terminal.widgetShortcutKey(red,f7Pressed);
    terminal.widgetValue(red,true);                                         //Make this button selected at the start. All visible radio buttons are in a single group
  }
  if(green)
  {
    terminal.labelAttributes(green, COLOUR_GREEN | ATTRIBUTE_BRIGHT); //Make the label text more emphasised, for better contrast
    terminal.scroll(F("Green radio button created"));
    terminal.showWidget(green);                                     //Make this button visible
    terminal.widgetShortcutKey(green,f8Pressed);
  }
  if(blue)
  {
    terminal.labelAttributes(blue, COLOUR_BLUE | ATTRIBUTE_BRIGHT);    //Make the label text more emphasised, for better contrast
    terminal.scroll(F("Blue radio button created"));
    terminal.showWidget(blue);                                       //Make this button visible
    terminal.widgetShortcutKey(blue,f9Pressed);
  }
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    if(terminal.widgetClicked(red))
    {
      if(terminal.widgetValue(red))
      {
        terminal.scroll(F("Red radio button selected"));
      }
    }
    if(terminal.widgetClicked(green))
    {
      if(terminal.widgetValue(green))
      {
        terminal.scroll(F("Green radio button selected"));
      }
    }
    if(terminal.widgetClicked(blue))
    {
      if(terminal.widgetValue(blue))
      {
        terminal.scroll(F("Blue radio button selected"));
      }
    }
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  endPage();
}
