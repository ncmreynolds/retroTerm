void sixteenColourDemo()
{
  newPage(F("16-colour test"));
  
  terminal.moveCursorTo(1,3);
  terminal.print(F("Testing bright black text - "));
  terminal.attributes(ATTRIBUTE_BRIGHT | COLOUR_BLACK);
  terminal.print(F("Bright Black Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing bright red text - "));
  terminal.attributes(ATTRIBUTE_BRIGHT | COLOUR_RED);
  terminal.print(F("Bright Red Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing bright green text - "));
  terminal.attributes(ATTRIBUTE_BRIGHT | COLOUR_GREEN);
  terminal.print(F("Bright Green Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing bright yellow text - "));
  terminal.attributes(ATTRIBUTE_BRIGHT | COLOUR_YELLOW);
  terminal.print(F("Bright Yellow Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing bright blue text - "));
  terminal.attributes(ATTRIBUTE_BRIGHT | COLOUR_BLUE);
  terminal.print(F("Bright Blue Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing bright magenta text - "));
  terminal.attributes(ATTRIBUTE_BRIGHT | COLOUR_MAGENTA);
  terminal.print(F("Bright Magenta Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing bright cyan text - "));
  terminal.attributes(ATTRIBUTE_BRIGHT | COLOUR_CYAN);
  terminal.print(F("Bright Cyan Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing bright white text - "));
  terminal.attributes(ATTRIBUTE_BRIGHT | COLOUR_WHITE);
  terminal.print(F("Bright White Text"));
  terminal.resetAttributes();
  terminal.println();
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();
  }  
  endPage();
}
