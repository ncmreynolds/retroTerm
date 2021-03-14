void largeFontDemo()
{
  newPage(F("Large font test"));
  
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

  endPage();
  
}
