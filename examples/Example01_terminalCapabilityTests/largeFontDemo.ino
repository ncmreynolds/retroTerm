void largeFontDemo()
{
  newPage(F("Large font test"));
  
  terminal.moveCursorTo(1,3);
  terminal.println(F("Testing double width text"));
  terminal.attributes(ATTRIBUTE_DOUBLE_WIDTH);
  terminal.print(F("Double Width Text"));
  terminal.resetAttributes();
  terminal.println();
  
  terminal.println(F("Testing double width & height text"));
  terminal.attributes(ATTRIBUTE_DOUBLE_SIZE);
  terminal.print(F("Double Width & Height Text"));
  terminal.resetAttributes();
  terminal.println();
  uint32_t timeout = millis();
  while(millis() - timeout < 30000ul && not terminal.keyPressed())
  {
    terminal.houseKeeping();
  }  
  endPage();
  
}
