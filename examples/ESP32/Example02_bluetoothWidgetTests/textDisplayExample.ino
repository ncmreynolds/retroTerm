void textDisplayExample()
{
  newPage(F("Text display window test"));
  red = terminal.newTextDisplay(1,terminal.lines()/2,terminal.columns()/3, (terminal.lines()-2)/2, F("Window 1"),   COLOUR_RED, OUTER_BOX);    //Create a window to display the text
  green = terminal.newTextDisplay(terminal.columns()/3 + 1,terminal.lines()/2,terminal.columns()/3, (terminal.lines()-2)/2, F("Window 2"), COLOUR_GREEN | ATTRIBUTE_FAINT, OUTER_BOX | LABEL_IN_BOX | LABEL_CENTRED | SHORTCUT_INLINE | BOX_DOUBLE_LINE);    //Create a window to display the text
  blue = terminal.newTextDisplay(terminal.columns()*2/3 + 1,terminal.lines()/2,terminal.columns()/3, (terminal.lines()-2)/2, F("Window 3"), COLOUR_BLUE, OUTER_BOX | BOX_ASCII);    //Create a window to display the text
  if(red)
  {
    terminal.labelAttributes(red, COLOUR_RED);
    terminal.contentAttributes(red, COLOUR_RED | ATTRIBUTE_BRIGHT);
    terminal.setWidgetContent(red, textToShow); //The address of the content to show in the window. It MUST remain in scope all the time you want it to be available to show.
    terminal.widgetShortcutKey(red,f10Pressed);
    terminal.showWidget(red);        //All widgets starts as invisible, so change this to show it
  }
  if(green)
  {
    terminal.setWidgetLabel(green,F("Window 2")); //Another way to add the label
    terminal.labelAttributes(green, COLOUR_GREEN);
    terminal.setWidgetContent(green,textToShow); //The address of the content to show in the window. It MUST remain in scope all the time you want it to be available to show.
    terminal.contentAttributes(green, COLOUR_GREEN | ATTRIBUTE_BRIGHT);
    terminal.widgetShortcutKey(green,f11Pressed);
    terminal.showWidget(green);        //All widgets starts as invisible, so change this to show it
  }
  if(blue)
  {
    terminal.contentAttributes(blue, COLOUR_BLUE | ATTRIBUTE_BRIGHT);
    terminal.setWidgetContent(blue,textToShow); //The address of the content to show in the window. It MUST remain in scope all the time you want it to be available to show.
    terminal.showWidget(blue);        //All widgets starts as invisible, so change this to show it
  }
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    if(terminal.widgetClicked(red))
    {
      terminal.scroll(F("Red window selected"));
    }
    if(terminal.widgetClicked(green))
    {
      terminal.scroll(F("Green window selected"));
    }
    if(terminal.widgetClicked(blue))
    {
      terminal.scroll(F("Blue window selected"));
    }
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  endPage();
}
