void twoHundredAndFiftySixColourDemo()
{
  newPage(F("256-colour test"));
  for(uint8_t column = 0 ; column < 16 ; column++)
  {
    for(uint8_t row = 0 ; row < 16 ; row++)
    {
      terminal.foregroundColour(column + 16*row);                  //Set 256-colour mode
      terminal.printAt(column * 4 + 3, row + 3, column + 16*row);  //Print the colour
    }
  }
  endPage();
}
