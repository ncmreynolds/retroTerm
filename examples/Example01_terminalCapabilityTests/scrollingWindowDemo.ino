void scrollingWindowDemo()
{
  newPage(F("Scroll windowing test, this line should not move"));
  terminal.setScrollWindow(3,terminal.lines()-2);
  //terminal.drawBox(1, 3 , terminal.columns()/2, terminal.lines()-4);
  //terminal.setScrollWindow(2,4,terminal.columns()/2-2,terminal.lines()-6);
  for(uint8_t scroll = 0 ; scroll < terminal.lines()-6 ; scroll++)
  {
    terminal.scroll("Scrolling whole line " + String(scroll+1) + " of " + String(terminal.lines()-6));
    delay(100);
  }
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();
  }  
  endPage();
}
