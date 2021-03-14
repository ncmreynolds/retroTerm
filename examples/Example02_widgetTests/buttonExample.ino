void buttonExample()
{
  newPage(F("Button demo"));
  terminal.setScrollWindow(4,12);     //Set up somewhere to show the events without disrupting the buttons
  terminal.hideCursor();
  terminal.enableMouse();
  terminal.scroll("Click on the buttons once they appear, or use the shortcut keys");
  //          ID                                      X                  Y   W  H  Label           Attributes
  uint8_t red =   terminal.newButton(1                         , 16, terminal.columns()/3, 3, F("Red button"),   COLOUR_WHITE | ATTRIBUTE_BRIGHT | BACKGROUND_COLOUR_RED, SHORTCUT_INLINE | LABEL_CENTRED);    //Create three new buttons and record the IDs
  uint8_t green = terminal.newButton(terminal.columns()/3 +1   , 16, terminal.columns()/3, 3, F("Green button"), COLOUR_GREEN, OUTER_BOX | BOX_SINGLE_LINE | SHORTCUT_INLINE);  //The method returns 0 if it fails
  uint8_t blue =  terminal.newButton(terminal.columns()*2/3 + 1, 16, terminal.columns()/3, 3, F("Blue button"),  COLOUR_BLUE,  OUTER_BOX | BOX_DOUBLE_LINE | LABEL_CENTRED);   //Colour is the colour of the frame, lettering can be different
  if(red)
  {
    terminal.labelAttributes(red, COLOUR_WHITE | ATTRIBUTE_BRIGHT | BACKGROUND_COLOUR_RED);    //Make the button text more emphasised, for better contrast
    terminal.widgetShortcutKey(red,f1Pressed);
    terminal.scroll("Red button created ID=" + String(red) +" in background, displaying in 1s");
    terminal.scroll("Red button keyboard shortcut F1");
  }
  if(green)
  {
    terminal.labelAttributes(green, COLOUR_GREEN | ATTRIBUTE_BRIGHT);        //Make the button text more emphasised, for better contrast
    terminal.widgetShortcutKey(green,f2Pressed);
    terminal.scroll("Green button created ID=" + String(green) +" in background, displaying in 5s");
    terminal.scroll("Green button keyboard shortcut F2");
  }
  if(blue)
  {
    terminal.labelAttributes(blue, COLOUR_BLUE | ATTRIBUTE_BRIGHT);          //Make the button text more emphasised, for better contrast
    terminal.widgetShortcutKey(blue,f3Pressed);
    terminal.scroll("Blue button created ID=" + String(blue) +" in background, displaying in 10s");
    terminal.scroll("Blue button keyboard shortcut F3");
  }
  bool redDemoStarted = false;
  bool greenDemoStarted = false;
  bool blueDemoStarted = false;
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    if(red) //The button was successfully created
    {
      if(redDemoStarted == false && millis() - timeout > 1000ul && not terminal.widgetVisible(red))    //Show the button near the start of the demo
      {
        terminal.scroll("Showing red button");
        redDemoStarted = true;
        terminal.showWidget(red);   //Make this button visible
      }
      else if(terminal.widgetVisible(red) && millis() - timeout > 20000ul)  //Hide the button again near the end of the demo
      {
        terminal.scroll("Hiding red button");
        terminal.hideWidget(red);  //Make this button invisible
      }
    }
    if(green) //The button was successfully created
    {
      if(greenDemoStarted == false && millis() - timeout > 5000ul && not terminal.widgetVisible(green))       //Show the button near the start of the demo
      {
        terminal.scroll("Showing green button");
        greenDemoStarted = true;
        terminal.showWidget(green); //Make this button visible
      }
      else if(terminal.widgetVisible(green) && millis() - timeout > 22000ul)    //Hide the button again near the end of the demo
      {
        terminal.scroll("Hiding green button");
        terminal.hideWidget(green);  //Make this button invisible
      }
    }
    if(blue) //The button was successfully created
    {
      if(blueDemoStarted == false && millis() - timeout > 10000ul && not terminal.widgetVisible(blue))      //Show the button near the start of the demo
      {
        terminal.scroll("Showing blue button");
        blueDemoStarted = true;
        terminal.showWidget(blue);  //Make this button visible
      }
      else if(terminal.widgetVisible(blue) && millis() - timeout > 24000ul)   //Hide the button again near the end of the demo
      {
        terminal.scroll("Hiding blue button");
        terminal.hideWidget(blue); //Make this button invisible
      }
    }
    if(terminal.widgetClicked(red))
    {
      terminal.scroll("Red clicked");
    }
    if(terminal.widgetClicked(green))
    {
      terminal.scroll("Green clicked");
    }
    if(terminal.widgetClicked(blue))
    {
      terminal.scroll("Blue clicked");
    }
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  terminal.deleteWidget(red);     //It is safe to try and delete a non-existent widget, the method will check before attempting to de-allocate memory etc.
  terminal.deleteWidget(green);
  terminal.deleteWidget(blue);
  terminal.readKeypress();
  terminal.disableMouse();
}
