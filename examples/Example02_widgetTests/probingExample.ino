void probingExample()
{
  newPage(F("Probing demo"));
  
  terminal.print(F("Probing terminal type"));
  if(terminal.probeType())
  {
    terminal.print(F(", success - "));
    terminal.println(terminal.type());
  }
  else
  {
    terminal.println(F(", failure assuming VT100"));
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
  endPage();  
}
