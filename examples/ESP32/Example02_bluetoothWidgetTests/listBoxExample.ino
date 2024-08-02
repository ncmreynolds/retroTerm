void listBoxExample()
{
  newPage(F("List box test"));
  red = terminal.newListBox(1,terminal.lines()/2,terminal.columns()/3, (terminal.lines()-2)/2, F("Red"),   COLOUR_RED, OUTER_BOX);    //Create a window to display the text
  green = terminal.newListBox(terminal.columns()/3 + 1,terminal.lines()/2,terminal.columns()/3, (terminal.lines()-2)/2, F("Green"),   COLOUR_GREEN | ATTRIBUTE_FAINT, OUTER_BOX | LABEL_IN_BOX | LABEL_CENTRED | SHORTCUT_INLINE | BOX_DOUBLE_LINE);    //Create a window to display the text
  blue = terminal.newListBox(terminal.columns()*2/3 + 1,terminal.lines()/2,terminal.columns()/3, (terminal.lines()-2)/2, COLOUR_BLUE);    //Create a window to display the text
  if(red) //The widget ID is zero if it failed to create. This is most likely because the max number of widgets was reached.
  {
    terminal.labelAttributes(red, COLOUR_RED);
    terminal.contentAttributes(red, COLOUR_RED | ATTRIBUTE_BRIGHT);
    #if defined(__AVR__)
    terminal.setWidgetContent(red,F("Red Zero\rRed One\rRed Two\rRed Three\rRed Four\rRed Five\rRed Six\rRed Seven\rRed Eight")); //The address of the content to show in the window. It MUST remain in scope all the time you want it to be available to show.
    #else
    terminal.setWidgetContent(red,F("Red Zero\rRed One\rRed Two\rRed Three\rRed Four\rRed Five\rRed Six\rRed Seven\rRed Eight\rRed Nine\rRed Ten\rRed Eleven\rRed Twelve\rRed Thirteen\rRed Fourteen\rRed Fifteen\rRed Sixteen\rRed Seventeen\rRed Eighteen\rRed Nineteen\rRed Twenty")); //The address of the content to show in the window. It MUST remain in scope all the time you want it to be available to show.
    #endif
    terminal.widgetShortcutKey(red,f10Pressed);
    terminal.widgetValue(red, 1);            //Select option 1
    terminal.showWidget(red);              //All widgets starts as invivisble, so change this to show it
  }
  if(green)
  {
    terminal.labelAttributes(green, COLOUR_GREEN);
    terminal.contentAttributes(green, COLOUR_GREEN | ATTRIBUTE_BRIGHT);
    #if defined(__AVR__)
    terminal.setWidgetContent(green,F("Green Zero\rGreen One\rGreen Two\rGreen Three\rGreen Four")); //The address of the content to show in the window. It MUST remain in scope all the time you want it to be available to show.
    #else
    terminal.setWidgetContent(green,F("Green Zero\rGreen One\rGreen Two\rGreen Three\rGreen Four\rGreen Five\rGreen Six\rGreen Seven\rGreen Eight\rGreen Nine\rGreen Ten\rGreen Eleven\rGreen Twelve\rGreen Thirteen\rGreen Fourteen\rGreen Fifteen\rGreen Sixteen\rGreen Seventeen\rGreen Eighteen\rGreen Nineteen\rGreen Twenty")); //The address of the content to show in the window. It MUST remain in scope all the time you want it to be available to show.
    #endif
    terminal.widgetShortcutKey(green,f11Pressed);
    terminal.widgetValue(green, 2);            //Select option 2
    terminal.showWidget(green);              //All widgets starts as invisible, so change this to show it
  }
  if(blue)
  {
    terminal.contentAttributes(blue, COLOUR_BLUE | ATTRIBUTE_BRIGHT);
    #if defined(__AVR__)
    terminal.setWidgetContent(blue,F("Blue Zero\rBlue One\rBlue Two")); //The address of the content to show in the window. It MUST remain in scope all the time you want it to be available to show.
    #else
    terminal.setWidgetContent(blue,F("Blue Zero\rBlue One\rBlue Two\rBlue Three\rBlue Four")); //The address of the content to show in the window. It MUST remain in scope all the time you want it to be available to show.
    #endif
    terminal.widgetValue(blue, 3);             //Select option 3
    terminal.showWidget(blue);               //All widgets starts as invivisble, so change this to show it
  }
  terminal.scroll("You can scroll through the items and select one. There is no multi-select in this version.");
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    if(terminal.widgetClicked(red))
    {
      terminal.scroll("Red listbox selected, option " + String(terminal.widgetValue(red)));
    }
    if(terminal.widgetClicked(green))
    {
      terminal.scroll("Green listbox selected, option " + String(terminal.widgetValue(green)));
    }
    if(terminal.widgetClicked(blue))
    {
      terminal.scroll("Blue listbox selected, option " + String(terminal.widgetValue(blue)));
    }
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  endPage();
}
