void attributeDemo()
{
  newPage(F("Character attributes test"));
  
  terminal.print(F("Testing bold text - "));
  terminal.attributes(ATTRIBUTE_BOLD);
  terminal.print(F("Bold Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing faint text - "));
  terminal.attributes(ATTRIBUTE_FAINT);
  terminal.print(F("Faint Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing underlined text - "));
  terminal.attributes(ATTRIBUTE_UNDERLINE);
  terminal.print(F("Underlined Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.print(F("Testing blinking text - "));
  terminal.attributes(ATTRIBUTE_BLINK);
  terminal.print(F("Blinking Text"));
  terminal.resetAttributes();
  terminal.println();

  terminal.print(F("Testing inverse text - "));
  terminal.attributes(ATTRIBUTE_INVERSE);
  terminal.print(F("Inverse Text"));
  terminal.resetAttributes();
  terminal.println();
  
  endPage();
}
