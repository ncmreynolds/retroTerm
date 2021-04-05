void probingExample()
{
  newPage(F("Probing test"));
  terminal.moveCursorTo(1,3);
  terminal.print(F("Probing terminal type"));
  if(terminal.probeType())
  {
    terminal.print(F(", success - '"));
    terminal.print(terminal.type());
    terminal.println('\'');
  }
  else
  {
    terminal.println((", failure assuming VT100"));
  }
  terminal.print(F("Probing terminal size"));
  if(terminal.probeSize())
  {
    terminal.print(F(", success - "));
    terminal.print(terminal.columns());
    terminal.print('x');
    terminal.println(terminal.lines());
  }
  else
  {
    terminal.println(F(", failure assuming 80x24"));
  }
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();  //You MUST run housekeeping to show any changes!
  }
  endPage();
}
