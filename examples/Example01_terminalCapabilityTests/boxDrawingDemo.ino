void boxDrawingDemo()
{
  newPage(F("Box drawing test"));
  uint8_t boxWidth = (terminal.columns())/4;
  uint8_t boxHeight = (terminal.lines() - 2 )/2;
  
  terminal.drawBox(1, 2 , boxWidth, boxHeight);                                                                    //Default style box with no title
  terminal.drawBoxWithTitle(boxWidth + 1, 2, boxWidth, boxHeight, "Single");                                       //Default style box with title
  terminal.drawBoxWithScrollbar(boxWidth * 2 + 1, 2, boxWidth, boxHeight, 0, 256, COLOUR_GREEN, BOX_SINGLE_LINE);                               //Scrollbar is shown at the top (0/256)
  terminal.drawBoxWithTitleAndScrollbar(boxWidth * 3 + 1, 2, boxWidth, boxHeight, "Single w/scroll", 64, 256, COLOUR_GREEN, BOX_SINGLE_LINE);  //Scrollbar is shown at one quarter (64/256)

  terminal.drawBox(1, 2 + boxHeight, boxWidth, boxHeight, COLOUR_RED, BOX_DOUBLE_LINE);                                                                      //Specific style and colour boxes
  terminal.drawBoxWithTitle(boxWidth + 1, 2 + boxHeight, boxWidth, boxHeight, "Double", COLOUR_GREEN, BOX_DOUBLE_LINE);                                      //Title, colour and style
  terminal.drawBoxWithScrollbar(boxWidth * 2 + 1, 2 + boxHeight, boxWidth, boxHeight, 128, 256, COLOUR_BLUE, BOX_DOUBLE_LINE);                               //Scrollbar at middle (128/256)
  terminal.drawBoxWithTitleAndScrollbar(boxWidth * 3 + 1, 2 + boxHeight, boxWidth, boxHeight, "Double w/scroll", 256, 256, COLOUR_MAGENTA, BOX_DOUBLE_LINE); //Scrollbar at end (256/256)
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();
  }  
  endPage();
}
