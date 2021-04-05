void eightColourDemo()
{
  newPage(F("8-colour test"));
  
  terminal.moveCursorTo(1,3);
  terminal.print(F("Testing black text - "));
  terminal.attributes(COLOUR_BLACK);
  terminal.print(F("Black Text"));
  terminal.resetAttributes();
  terminal.println();

  terminal.print(F("Testing red text - "));
  terminal.attributes(COLOUR_RED);
  terminal.print(F("Red Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing green text - "));
  terminal.attributes(COLOUR_GREEN);
  terminal.print(F("Green Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing yellow text - "));
  terminal.attributes(COLOUR_YELLOW);
  terminal.print(F("Yellow Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing blue text - "));
  terminal.attributes(COLOUR_BLUE);
  terminal.print(F("Blue Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing magenta text - "));
  terminal.attributes(COLOUR_MAGENTA);
  terminal.print(F("Magenta Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing cyan text - "));
  terminal.attributes(COLOUR_CYAN);
  terminal.print(F("Cyan Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing white text - "));
  terminal.attributes(COLOUR_WHITE);
  terminal.print(F("White Text"));
  terminal.resetAttributes();
  terminal.println();

  terminal.print(F("Testing black background - "));
  terminal.attributes(BACKGROUND_COLOUR_BLACK);
  terminal.print(F("Black Background"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing red background - "));
  terminal.attributes(BACKGROUND_COLOUR_RED);
  terminal.print(F("Red Background"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing green background - "));
  terminal.attributes(COLOUR_BLACK | BACKGROUND_COLOUR_GREEN);
  terminal.print(F("Green Background"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing yellow background - "));
  terminal.attributes(COLOUR_BLACK | BACKGROUND_COLOUR_YELLOW);
  terminal.print(F("Yellow Background"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing blue background - "));
  terminal.attributes(BACKGROUND_COLOUR_BLUE);
  terminal.print(F("Blue Background"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing magenta background - "));
  terminal.attributes(BACKGROUND_COLOUR_MAGENTA);
  terminal.print(F("Magenta Background"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing cyan background - "));
  terminal.attributes(COLOUR_BLACK | BACKGROUND_COLOUR_CYAN);
  terminal.print(F("Cyan Background"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing white background - "));
  terminal.attributes(COLOUR_BLACK | BACKGROUND_COLOUR_WHITE);
  terminal.print(F("White Background"));
  terminal.resetAttributes();
  terminal.println();
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();
  }  
  endPage();  
}
