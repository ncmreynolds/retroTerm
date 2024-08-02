void textLogExample()
{
  newPage(F("Text log window test"));
  red = terminal.newTextLog(1,terminal.lines()/2,terminal.columns()/3, (terminal.lines()-2)/2, F("Window 1"),   COLOUR_RED, OUTER_BOX);    //Create a window to display the text
  green = terminal.newTextLog(terminal.columns()/3 + 1,terminal.lines()/2,terminal.columns()/3, (terminal.lines()-2)/2, F("Window 2"),   COLOUR_GREEN | ATTRIBUTE_FAINT, OUTER_BOX | LABEL_IN_BOX | LABEL_CENTRED | SHORTCUT_INLINE | BOX_DOUBLE_LINE);    //Create a window to display the text
  blue = terminal.newTextLog(terminal.columns()*2/3 + 1,terminal.lines()/2,terminal.columns()/3, (terminal.lines()-2)/2, F("Window 3"), COLOUR_BLUE | ATTRIBUTE_FAINT, OUTER_BOX | BOX_ASCII);    //Create a window to display the text
  if(red)
  {
    terminal.labelAttributes(red, COLOUR_RED);
    terminal.contentAttributes(red, COLOUR_RED | ATTRIBUTE_BRIGHT);
    terminal.widgetShortcutKey(red,f10Pressed);
    terminal.showWidget(red);        //All widgets starts as invisible, so change this to show it
  }
  if(green)
  {
    terminal.labelAttributes(green, COLOUR_GREEN);
    terminal.contentAttributes(green, COLOUR_GREEN | ATTRIBUTE_BRIGHT);
    terminal.widgetShortcutKey(green,f11Pressed);
    terminal.showWidget(green);        //All widgets starts as invisible, so change this to show it
  }
  if(blue)
  {
    terminal.contentAttributes(blue, COLOUR_BLUE | ATTRIBUTE_BRIGHT);
    terminal.showWidget(blue);        //All widgets starts as invisible, so change this to show it
  }
  uint32_t timeout = millis();
  uint32_t randomContentTimer = millis();
  //We will use clicks to show content can be added dynamically to each window
  uint8_t redClicks = 0;
  uint8_t greenClicks = 0;
  uint8_t blueClicks = 0;
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    if(terminal.widgetClicked(red))
    {
      terminal.scroll(F("Red window selected"));
      terminal.appendWidgetContent(red, ("Red click " + String(redClicks++)).c_str()); //By default appendWidgetContent adds it to the 'end' of the circular buffer, the library manages memory allocation for this.
      //terminal.appendWidgetContent(red, F("Red click"));
    }
    if(terminal.widgetClicked(green))
    {
      terminal.scroll(F("Green window selected"));
      terminal.prependWidgetContent(green, ("Green click " + String(greenClicks++)).c_str()); //By default appendWidgetContent adds it to the 'end' of the circular buffer, the library manages memory allocation for this.
    }
    if(terminal.widgetClicked(blue))
    {
      terminal.scroll(F("Blue window selected"));
      terminal.appendWidgetContent(blue, ("Blue click " + String(blueClicks++)).c_str()); //By default appendWidgetContent adds it to the 'end' of the circular buffer, the library manages memory allocation for this.
    }
    //Generate some random scrolling content
    if(millis() - randomContentTimer > 500)
    {
      randomContentTimer = millis();
      switch (random(4))
      {
        case 1:
          terminal.appendWidgetContent(red,("Random red " + String(random(1,100))).c_str());
          break;
        case 2:
          terminal.prependWidgetContent(green,("Random green " + String(random(1,100))).c_str());
          break;
        case 3:
          terminal.appendWidgetContent(blue,("Random blue " + String(random(1,100))).c_str());
          break;
      }
    }
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  endPage();
}
