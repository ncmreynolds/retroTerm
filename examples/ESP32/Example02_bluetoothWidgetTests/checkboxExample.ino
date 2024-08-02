void checkboxExample()
{
  newPage(F("Checkbox test"));
  red = terminal.newCheckbox(terminal.columns()/3, terminal.lines()/2, 14, 1, F("Red"),   COLOUR_RED, SHORTCUT_INLINE);    //Create three new widgets and record the IDs
  green = terminal.newCheckbox(terminal.columns()/3, terminal.lines()/2 + 1, 14, 1, F("Green"), COLOUR_GREEN, SHORTCUT_INLINE);  //The method returns 0 if it fails
  blue = terminal.newCheckbox(terminal.columns()/3, terminal.lines()/2 + 2, 14, 1, F("Blue"),  COLOUR_BLUE, SHORTCUT_INLINE);   //Colour is the colour of the frame, lettering can be different
  if(red)
  {
    terminal.labelAttributes(red, COLOUR_RED | ATTRIBUTE_BRIGHT);     //Make the label text more emphasised, for better contrast
    terminal.scroll(F("Red widget created"));
    terminal.showWidget(red);                                         //Make this button visible
    terminal.widgetShortcutKey(red,f4Pressed);
  }
  if(green)
  {
    terminal.labelAttributes(green, COLOUR_GREEN | ATTRIBUTE_BRIGHT); //Make the label text more emphasised, for better contrast
    terminal.scroll(F("Green widget created"));
    terminal.showWidget(green);                               //Make this button visible
    terminal.widgetShortcutKey(green,f5Pressed);
  }
  if(blue)
  {
    terminal.labelAttributes(blue, COLOUR_BLUE | ATTRIBUTE_BRIGHT);    //Make the label text more emphasised, for better contrast
    terminal.scroll(F("Blue widget created"));
    terminal.showWidget(blue);                                       //Make this button visible
    terminal.widgetShortcutKey(blue,f6Pressed);
  }
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    if(terminal.widgetClicked(red))
    {
      if(terminal.widgetValue(red))
      {
        terminal.scroll(F("Red checked"));
      }
      else
      {
        terminal.scroll(F("Red unchecked"));
      }
    }
    if(terminal.widgetClicked(green))
    {
      if(terminal.widgetValue(green))
      {
        terminal.scroll(F("Green checked"));
      }
      else
      {
        terminal.scroll(F("Green unchecked"));
      }
    }
    if(terminal.widgetClicked(blue))
    {
      if(terminal.widgetValue(blue))
      {
        terminal.scroll(F("Blue checked"));
      }
      else
      {
        terminal.scroll(F("Blue unchecked"));
      }
    }
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  endPage();
}
