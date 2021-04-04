void mouseDemo()
{
  newPage(F("Mouse capture test - prints D at mouse button down, U at mouse button up, N at wheel up, S at wheel down"));
  terminal.moveCursorTo(1,terminal.lines());
  terminal.print(F("Press any key to continue, or wait 30s"));
  terminal.hideCursor();
  terminal.enableMouse();
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();
    if(terminal.mouseButtonDown())
    {
      terminal.printAt(terminal.mouseColumn(),terminal.mouseRow(),"D");
    }
    if(terminal.mouseButtonUp())
    {
      terminal.printAt(terminal.mouseColumn(),terminal.mouseRow(),"U");
    }
    if(terminal.mouseWheelDown())
    {
      terminal.printAt(terminal.mouseColumn(),terminal.mouseRow(),"N");
    }
    if(terminal.mouseWheelUp())
    {
      terminal.printAt(terminal.mouseColumn(),terminal.mouseRow(),"S");
    }
  }
  terminal.readKeypress();
  terminal.disableMouse();
}
