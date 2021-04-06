#ifndef retroTerm_cpp
#define retroTerm_cpp
#include "retroTerm.h"


//Constructor function
retroTerm::retroTerm()
{
	//Do nothing
}
//Destructor function
retroTerm::~retroTerm()
{
	//Do nothing
}

//Overloaded function for starting the terminal
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::begin(Stream &terminalStream)
#else
void retroTerm::begin(Stream &terminalStream)
#endif
{
	_terminalStream = &terminalStream;		//Set the stream used for the terminal
	#if defined(ESP8266)
	if(&terminalStream == &Serial)
	{
		  _terminalStream->write(17);		//Send an XON to stop the hung terminal after reset on ESP8266
	}
	#endif
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::end()
#else
void retroTerm::end()
#endif
{
	for (uint8_t widgetIndex = 0 ; widgetIndex < _widgetObjectLimit ; widgetIndex++)
	{
		if(_widgetExists(widgetIndex))
		{
			deleteWidget(widgetIndex + 1);
		}
	}
	_terminalStream = nullptr;
}

//Run regularly to make sure things like line editing and mouse capture work. Not necessary for simple printing
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::houseKeeping()
#else
void retroTerm::houseKeeping()
#endif
{
	_readInput();				//Look for mouse and keyboard events
	if(_numberOfWidgets > 0)
	{
		_processInput();		//Pass mouse and keyboard events to widgets
		_displayChanges();		//Update the terminal with changes
	}
	#if defined(ESP8266) || defined(ESP32)
	yield();					//Necessary if the stream passed is WiFi on ESP8266/ESP32 where the usual worries about not causing a WDT reset exist
	#endif
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_processInput()
#else
void retroTerm::_processInput()
#endif
{
	bool inputEventCaught = false;																		//Use to bypass checks after an input is found
	for(uint8_t widgetId = _widgetObjectLimit ; widgetId-- > 0 && inputEventCaught == false ; )			//Search for clicks on visible objects and shortcut keys. There is no Z-Index but newest widgets take preference
	{
		if(_widgetExists(widgetId) && (_widgets[widgetId].currentState & 0x0103) == 0x0103)							//Only check visible, displayed widgets that are clickable
		{
			if((_mouseStatus & 0x04) && _mouseX >= _widgets[widgetId].x && _mouseX < _widgets[widgetId].x + _widgets[widgetId].w && _mouseY >= _widgets[widgetId].y && _mouseY < _widgets[widgetId].y + _widgets[widgetId].h)	//Primary button up
			{
				_clickWidget(widgetId);				//Do per-widget-type click handling, only if clickable
				_mouseStatus = _mouseStatus & 0xF9;	//Gobble the mouse event
				inputEventCaught = true;			//Stop looking for more events
			}
			else if((_mouseStatus & 0x20) && _mouseX >= _widgets[widgetId].x && _mouseX < _widgets[widgetId].x + _widgets[widgetId].w && _mouseY >= _widgets[widgetId].y && _mouseY < _widgets[widgetId].y + _widgets[widgetId].h)	//Mouse wheel up
			{
				_clickWidget(widgetId);				//Do per-widget-type click handling, only if clickable
				_mouseStatus = _mouseStatus & 0xDF;	//Gobble the mouse event
				inputEventCaught = true;			//Stop looking for more events
			}
			else if((_mouseStatus & 0x10) && _mouseX >= _widgets[widgetId].x && _mouseX < _widgets[widgetId].x + _widgets[widgetId].w && _mouseY >= _widgets[widgetId].y && _mouseY < _widgets[widgetId].y + _widgets[widgetId].h)	//Mouse wheel down
			{
				_clickWidget(widgetId);				//Do per-widget-type click handling, only if clickable
				_mouseStatus = _mouseStatus & 0xEF;	//Gobble the mouse event
				inputEventCaught = true;			//Stop looking for more events
			}
			if(_widgets[widgetId].shortcut != noKeyPressed && _widgets[widgetId].shortcut == _lastKeypress)	//Search for used keyboard shortcuts, which appear as 'clicks' of an object for simplicity. This gobbles up the keypress so the application doesn't see it
			{
				_clickWidget(widgetId);				//Do per-widget-type click handling, only if clickable
				_lastKeypress = noKeyPressed;		//Gobble the keypress and stop looking for more shortcuts
				inputEventCaught = true;			//Stop looking for more events
			}
		}
	}
	if(inputEventCaught == false && _selectedWidget < _widgetObjectLimit && _widgets[_selectedWidget].currentState & 0x0100)	//Process input from the keyboard to the currently selected widget
	{
		if(_widgets[_selectedWidget].type ==_widgetTypes::staticTextDisplay || _widgets[_selectedWidget].type ==_widgetTypes::scrollingTextDisplay)	//Handle keyboard input to a scrolling text display
		{
			if(_lastKeypress == downPressed)
			{
				_scrollDown(_selectedWidget);
				_lastKeypress = noKeyPressed;	//Gobble up the keypress
			}
			else if(_lastKeypress == upPressed)
			{
				_scrollUp(_selectedWidget);
				_lastKeypress = noKeyPressed;	//Gobble up the keypress
			}			
		}
		else if(_widgets[_selectedWidget].type == _widgetTypes::textInput)		//Handle keyboard input to a line editing widget
		{
			if(_lastKeypress == deletePressed)	//Delete
			{
				if(_contentSize(_selectedWidget)>0 && _widgets[_selectedWidget].contentOffset < _contentSize(_selectedWidget))
				{
					//Shuffle everything to the left
					for(uint8_t shuffle = _widgets[_selectedWidget].contentOffset; shuffle < _contentSize(_selectedWidget) + 1 ; shuffle++)
					{
						_widgets[_selectedWidget].content[shuffle] = _widgets[_selectedWidget].content[shuffle + 1];
					}
					//Reflect this on the screen
					moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
					_terminalStream->print(F("\033[001P"));		//Send the 'delete' ANSI control character, which shifts characters to the left as you delete
					//Add a space at the end to avoid things beyond the edited string scrolling left
					hideCursor();
					attributes(_widgets[_selectedWidget].attributes);
					_terminalStream->print(F("\033[4h"));			//Turn insert mode on
					printAt(_typingXposition(_selectedWidget) - _widgets[_selectedWidget].contentOffset + _contentSize(_selectedWidget),_typingYposition(_selectedWidget)," ");
					_terminalStream->print(F("\033[4l"));			//Turn insert mode off
					attributes(_defaultAttributes);
					moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
					showCursor();
					_widgets[_selectedWidget].value = 1;	//Mark the content as changed for the application
				}
				else if(_widgets[_selectedWidget].contentOffset == _contentSize(_selectedWidget) && _bellEnabled)
				{
					soundBell();	//Sound the bell as there's nothing to delete in the buffer
				}
				_lastKeypress = noKeyPressed;								//Gobble up the keypress as line editing is occuring
			}
			else if(_lastKeypress == leftPressed)	//Left arrow
			{
				if(_widgets[_selectedWidget].contentOffset>0)
				{
					_widgets[_selectedWidget].contentOffset--;
					moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
				}
				else
				{
					soundBell();		//Sound the bell as already at the beginning
				}
				_lastKeypress = noKeyPressed;								//Gobble up the keypress as line editing is occuring
			}
			else if(_lastKeypress == rightPressed)	//Right arrow
			{
				if(_widgets[_selectedWidget].contentOffset <= _contentSize(_selectedWidget))
				{
					_widgets[_selectedWidget].contentOffset++;
					moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
				}
				else if(_bellEnabled)
				{
					soundBell();		//Sound the bell as already at the end
				}
				_lastKeypress = noKeyPressed;								//Gobble up the keypress as line editing is occuring
			}
			else if(_lastKeypress == homePressed)	//Home
			{
				_widgets[_selectedWidget].contentOffset = 0;
				moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
				_lastKeypress = noKeyPressed;								//Gobble up the keypress as line editing is occuring
			}
			else if(_lastKeypress ==  endPressed)	//End
			{
				_widgets[_selectedWidget].contentOffset = _contentSize(_selectedWidget);
				moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
				_lastKeypress = noKeyPressed;								//Gobble up the keypress as line editing is occuring
			}
			else if(_lastKeypress ==  backspacePressed)	//Backspace
			{
				if(_widgets[_selectedWidget].contentOffset>0)
				{
					_widgets[_selectedWidget].contentOffset--;												//Move the index back one
					if(_widgets[_selectedWidget].contentOffset == _contentSize(_selectedWidget))
					{
						_widgets[_selectedWidget].content[_widgets[_selectedWidget].contentOffset] = 0;		//Overwrite the last character with a null
					}
					else
					{
						//Shuffle everything to the left
						for(uint8_t shuffle = _widgets[_selectedWidget].contentOffset; shuffle < _contentSize(_selectedWidget) + 1 ; shuffle++)
						{
							_widgets[_selectedWidget].content[shuffle] = _widgets[_selectedWidget].content[shuffle + 1];
						}
					}
					moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
					_terminalStream->print(F("\033[001P"));				//Send the 'delete' ANSI control character, which shifts characters to the left as you delete
					//Add a space at the end to avoid things beyond the edited string scrolling left
					hideCursor();
					attributes(_widgets[_selectedWidget].attributes);
					_terminalStream->print(F("\033[4h"));					//Turn insert mode on
					printAt(_typingXposition(_selectedWidget) - _widgets[_selectedWidget].contentOffset  + _contentSize(_selectedWidget),_typingYposition(_selectedWidget)," ");
					_terminalStream->print(F("\033[4l"));					//Turn insert mode off
					moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
					attributes(_defaultAttributes);
					showCursor();
					_widgets[_selectedWidget].value = 1;	//Mark the content as changed for the application
				}
				else if(_bellEnabled)
				{
					//Sound the bell when the buffer is empty
					soundBell();
				}
				_lastKeypress = noKeyPressed;								//Gobble up the keypress as line editing is occuring
			}
			else if(_lastKeypress>31 && _lastKeypress<127)				//Ordinary typing
			{
				moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
				if(_contentSize(_selectedWidget) < _typingBufferMaxLength(_selectedWidget))
				{
					if(_widgets[_selectedWidget].contentOffset == _contentSize(_selectedWidget))		//Typing at the end of the buffer
					{ 
						//Add the character to the end of the buffer
						_widgets[_selectedWidget].content[_widgets[_selectedWidget].contentOffset] = char(_lastKeypress);	//Add the character to the end of the buffer
						_widgets[_selectedWidget].contentOffset++;
						_widgets[_selectedWidget].content[_widgets[_selectedWidget].contentOffset] = 0; 					//Add a null termination
						attributes(_widgets[_selectedWidget].attributes);
						if(_widgets[_selectedWidget].style & PASSWORD_FIELD)
						{
							_printUnicodeCharacter(_widgets[_selectedWidget].style & 0x01, 18);
						}
						else
						{
							_terminalStream->print(char(_lastKeypress));
						}
						if(_widgets[_selectedWidget].contentOffset == _typingBufferMaxLength(_selectedWidget))
						{
							_terminalStream->print(F("\033[1D"));		//Move the cursor back one place so it doesn't overflow the field
						}
						attributes(_defaultAttributes);
					}
					else
					{
						//Shuffle everything to the right
						for(uint8_t shuffle = _contentSize(_selectedWidget) + 1; shuffle > _widgets[_selectedWidget].contentOffset ; shuffle--)
						{
							_widgets[_selectedWidget].content[shuffle] = _widgets[_selectedWidget].content[shuffle - 1];
						}
						_widgets[_selectedWidget].content[_widgets[_selectedWidget].contentOffset] = char(_lastKeypress);	//Add the character to the current position in the buffer
						//Clear space at the end of the string to prevent moving everything past it to the right
						attributes(_widgets[_selectedWidget].attributes);
						moveCursorTo(_typingXposition(_selectedWidget) - _widgets[_selectedWidget].contentOffset + _contentSize(_selectedWidget) - 1,_typingYposition(_selectedWidget));
						_terminalStream->print(F("\033[001P"));				//Send the 'delete' ANSI control character, which shifts characters to the left as you delete
						_terminalStream->print(F("\033[4h"));				//Turn on ANSI terminal insert
						moveCursorTo(_typingXposition(_selectedWidget),_typingYposition(_selectedWidget));
						if(_widgets[_selectedWidget].style & PASSWORD_FIELD)
						{
							_printUnicodeCharacter(_widgets[_selectedWidget].style & 0x01, 18);
						}
						else
						{
							_terminalStream->print(char(_lastKeypress));
						}
						_terminalStream->print(F("\033[4l"));				//Turn off ANSI terminal insert
						_widgets[_selectedWidget].contentOffset++;			//Advance the cursor
						attributes(_defaultAttributes);
					}
					_widgets[_selectedWidget].value = 1;	//Mark the content as changed for the application
				}
				else //Discard the character
				{
					 if(_bellEnabled)
					 {
						//Sound the bell when the buffer is full
						soundBell();
					 }
				}
				_lastKeypress = noKeyPressed;								//Gobble up the keypress as line editing is occuring
			}
		}
	}
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_typingXposition(const uint8_t widgetId)
#else
uint8_t retroTerm::_typingXposition(const uint8_t widgetId)
#endif
{
	uint8_t xOffset;
	if(_widgets[widgetId].style & OUTER_BOX)
	{
		xOffset = _widgets[widgetId].x + 1 + _widgets[widgetId].contentOffset;
		if(_widgets[widgetId].h < 4)
		{
			if(_widgets[widgetId].label != nullptr)
			{
				xOffset += _labelLength(widgetId);
			}
			if(_widgets[widgetId].shortcut != noKeyPressed)
			{
				xOffset += 2 + _shortcutLength(widgetId);
			}
		}
	}
	else
	{
		xOffset = _widgets[widgetId].x + _widgets[widgetId].contentOffset;
		if(_widgets[widgetId].h == 1)
		{
			if(_widgets[widgetId].label != nullptr)
			{
				xOffset += _labelLength(widgetId);
			}
			if(_widgets[widgetId].shortcut != noKeyPressed)
			{
				xOffset += 2 + _shortcutLength(widgetId);
			}
		}
	}
	if(_widgets[widgetId].contentOffset == _typingBufferMaxLength(widgetId))
	{
		xOffset--;		//Avoid overflowing the space for typing
	}
	return(xOffset);
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_typingYposition(const uint8_t widgetId)
#else
uint8_t retroTerm::_typingYposition(const uint8_t widgetId)
#endif
{
	return(_contentYorigin(widgetId));
}

/*#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_typingYposition(const uint8_t widgetId)
#else
uint8_t retroTerm::_typingYposition(const uint8_t widgetId)
#endif
{
	if(_widgets[widgetId].style & OUTER_BOX)
	{
		if(_widgets[widgetId].style & LABEL_IN_BOX)
		{
			return(_widgets[widgetId].y + 3);
		}
		else
		{
			if(_widgets[widgetId].label == nullptr)
			{
				return(_widgets[widgetId].y + 1);
			}
			else
			{
				return(_widgets[widgetId].y + 2);
			}
		}
	}
	else
	{
		if(_widgets[widgetId].h > 1)
		{
			if(_widgets[widgetId].label == nullptr)
			{
				return(_widgets[widgetId].y);
			}
			else
			{
				return(_widgets[widgetId].y + 1);
			}
		}
		else
		{
			return(_widgets[widgetId].y);
		}
	}
}*/


#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_typingBufferMaxLength(const uint8_t widgetId)
#else
uint8_t retroTerm::_typingBufferMaxLength(const uint8_t widgetId)
#endif
{
	uint8_t width = _widgets[widgetId].w;
	if(_widgets[widgetId].style & OUTER_BOX)
	{
		width -=2;
		if(_widgets[widgetId].h < 4)
		{
			if(_widgets[widgetId].label != nullptr)
			{
				width -= _labelLength(widgetId);
			}
			if(_widgets[widgetId].shortcut != noKeyPressed)
			{
				width -= 2 + _shortcutLength(widgetId);
			}
		}
	}
	else
	{
		if(_widgets[widgetId].h == 1)
		{
			if(_widgets[widgetId].label != nullptr)
			{
				width -= _labelLength(widgetId);
			}
			if(_widgets[widgetId].shortcut != noKeyPressed)
			{
				width -= 2 + _shortcutLength(widgetId);
			}
		}
	}
	return(width);
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_displayChanges()
#else
void retroTerm::_displayChanges()
#endif
{
	//Deal with whole widgets appearing and disappearing, order this happens in is IMPORTANT, everything to be cleared must be cleared FIRST before displaying new things
	for (uint8_t widgetIndex = 0 ; widgetIndex < _widgetObjectLimit ; widgetIndex++)
	{
		if(_widgetExists(widgetIndex))
		{
			if((_widgets[widgetIndex].currentState & 0x0003) == 0x0002)								//Widget is displayed but 'invisible' and needs clearing
			{
				clearBox(_widgets[widgetIndex].x, _widgets[widgetIndex].y,
					_widgets[widgetIndex].w, _widgets[widgetIndex].h);								//Clear the space and fill with default attributes
				_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState & 0xFFFC;	//Mark as not displayed and not visible
				//Now check for overlapping widgets that may need refreshing
				/*for (uint8_t overlapCheck = 0 ; overlapCheck < _widgetObjectLimit ; overlapCheck++)
				{
					if((_widgets[overlapCheck].currentState & 0x0003) == 0x0003 && _widgetsOverlap(widgetIndex,overlapCheck))	//Widget is visible, displayed and overlaps
					{
						_widgets[overlapCheck].currentState = _widgets[overlapCheck].currentState & 0xFFFD;						//Mark as visible but not displayed, forcing a refresh
					}
				}*/
			}
		}
	}
	for (uint8_t widgetIndex = 0 ; widgetIndex < _widgetObjectLimit ; widgetIndex++)
	{
		if(_widgetExists(widgetIndex))
		{
			if((_widgets[widgetIndex].currentState & 0x0003) == 0x0001)									//Widget is visible but not displayed
			{
				clearBox(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, _widgets[widgetIndex].attributes);	//Clear the space the widget will occupy
				_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x001F;		//Mark as displayed, but all the elements changed so they get displayed in the next step
			}
			if((_widgets[widgetIndex].currentState & 0x0003) == 0x0003)									//Handle display of separate parts of the widget only if the widget is currently visible
			{
				if((_widgets[widgetIndex].currentState & 0x0004) == 0x0004)								//Widget outer box has changed
				{
					_displayWidgetOuterBox(widgetIndex);												//Draw the outer box, if necessary
					_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState & 0xFFFB;	//Mark the widget as drawn
				}
				if((_widgets[widgetIndex].currentState & 0x0008) == 0x0008)								//Label has changed
				{
					if(_widgets[widgetIndex].style & SHORTCUT_INLINE)
					{
						_displayLabel(widgetIndex);														//Add the label, which will also add the shortcut
					}
					else
					{
						_displayLabel(widgetIndex);														//Add the label
						_displayKeyboardShortcut(widgetIndex);											//Add the keyboard shortcut
					}
					_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState & 0xFFF7;	//Mark the label as drawn
				}
				if((_widgets[widgetIndex].currentState & 0x0010) == 0x0010)								//Widget content/state has changed
				{
					if(_widgets[widgetIndex].type == _widgetTypes::checkbox)
					{
						saveCursorPosition();															//Save current cursor state
						hideCursor();																	//Hide the cursor to reduce flickery movement in the terminal
						if(_widgets[widgetIndex].style & OUTER_BOX)
						{
							//moveCursorTo(_widgets[widgetIndex].x + 1 + _labelLength(widgetIndex), _widgets[widgetIndex].y + 1);		//Move to widget position
							moveCursorTo(_widgets[widgetIndex].x + 1, _widgets[widgetIndex].y + 1);		//Move to widget position
						}
						else
						{
							//moveCursorTo(_widgets[widgetIndex].x + _labelLength(widgetIndex), _widgets[widgetIndex].y);				//Move to widget position
							moveCursorTo(_widgets[widgetIndex].x, _widgets[widgetIndex].y);				//Move to widget position
						}
						attributes(_widgets[widgetIndex].attributes);									//Apply widget attributes
						if(_widgets[widgetIndex].value)													//Boolean state is true
						{
							_printUnicodeCharacter(_widgets[widgetIndex].style & 0x01, 16);
						}
						else
						{
							_printUnicodeCharacter(_widgets[widgetIndex].style & 0x01, 15);
						}
						restoreCursorPosition();														//Restore the cursor
					}
					else if(_widgets[widgetIndex].type == _widgetTypes::radioButton)
					{
						saveCursorPosition();															//Save current cursor state
						hideCursor();																	//Hide the cursor to reduce flickery movement in the terminal
						if(_widgets[widgetIndex].style & OUTER_BOX)
						{
							moveCursorTo(_widgets[widgetIndex].x + 1, _widgets[widgetIndex].y + 1);		//Move to widget position
						}
						else
						{
							moveCursorTo(_widgets[widgetIndex].x, _widgets[widgetIndex].y);				//Move to widget position
						}
						attributes(_widgets[widgetIndex].attributes);									//Apply widget attributes
						if(_widgets[widgetIndex].value)													//Boolean state is true
						{
							_printUnicodeCharacter(_widgets[widgetIndex].style & 0x01, 18);
						}
						else
						{
							_printUnicodeCharacter(_widgets[widgetIndex].style & 0x01, 17);
						}
						restoreCursorPosition();														//Restore the cursor
					}
					else if(_widgets[widgetIndex].type == _widgetTypes::staticTextDisplay)
					{
						if(_widgets[widgetIndex].content == nullptr)	//Clear the content area
						{
							//clearBox(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex), _columnsAvailable(widgetIndex), _linesAvailable(widgetIndex), _widgets[widgetIndex].contentAttributes );
						}
						else	//Display the content
						{
							if(_scrollbarNeeded(widgetIndex))
							{
								_drawScrollbar(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, (_widgets[widgetIndex].label != nullptr) && (_widgets[widgetIndex].style & LABEL_IN_BOX), _widgets[widgetIndex].contentOffset, uint32_t(_contentSize(widgetIndex)), _widgets[widgetIndex].attributes, _widgets[widgetIndex].style);
							}
							_displayContent(widgetIndex);			//Show the content
						}
					}
					else if(_widgets[widgetIndex].type == _widgetTypes::scrollingTextDisplay)
					{
						if(_widgets[widgetIndex].content == nullptr)	//Clear the content area
						{
							//clearBox(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex), _columnsAvailable(widgetIndex), _linesAvailable(widgetIndex), _widgets[widgetIndex].contentAttributes );
						}
						else	//Display the content
						{
							if(_scrollbarNeeded(widgetIndex))
							{
								_drawScrollbar(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, (_widgets[widgetIndex].label != nullptr) && (_widgets[widgetIndex].style & LABEL_IN_BOX), _widgets[widgetIndex].contentOffset, uint32_t(_contentSize(widgetIndex)), _widgets[widgetIndex].attributes, _widgets[widgetIndex].style);
							}
							_displayContent(widgetIndex);			//Show the content
						}
					}
					else if(_widgets[widgetIndex].type == _widgetTypes::textInput)
					{
						//Content changes should usually be handled by the text editing routines, this will only happen if the application changes the content, rather than the user
						if(_widgets[widgetIndex].content == nullptr)	//Clear the content area
						{
							clearBox(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex), _columnsAvailable(widgetIndex), _linesAvailable(widgetIndex), _widgets[widgetIndex].contentAttributes );
						}
						else	//Display the content
						{
							if(_contentSize(widgetIndex) > 0)
							{
								if(_widgets[widgetIndex].style & PASSWORD_FIELD)						//Show blobs instead of the string
								{
									saveCursorPosition();
									moveCursorTo(_typingXposition(widgetIndex) - _contentSize(widgetIndex),_contentYorigin(widgetIndex));
									attributes(_widgets[widgetIndex].attributes);
									for(uint8_t blob = 0; blob < _contentSize(widgetIndex); blob++)
									{
										_printUnicodeCharacter(_widgets[widgetIndex].style & 0x01, 18);
									}
									restoreCursorPosition();
								}
								else
								{
									printAt(_typingXposition(widgetIndex) - _contentSize(widgetIndex),_contentYorigin(widgetIndex), _widgets[widgetIndex].content,  _widgets[widgetIndex].attributes);
								}
							}
							else
							{
								clearBox(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex), _columnsAvailable(widgetIndex), _linesAvailable(widgetIndex), _widgets[widgetIndex].contentAttributes);
							}
						}
					}
					else if(_widgets[widgetIndex].type == _widgetTypes::listBox)
					{
						if(_widgets[widgetIndex].content == nullptr)	//Clear the content area
						{
							clearBox(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex), _columnsAvailable(widgetIndex), _linesAvailable(widgetIndex), _widgets[widgetIndex].contentAttributes );
						}
						else	//Display the content
						{
							if(_scrollbarNeeded(widgetIndex))
							{
								_drawScrollbar(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, (_widgets[widgetIndex].label != nullptr) && (_widgets[widgetIndex].style & LABEL_IN_BOX), _widgets[widgetIndex].contentOffset, uint32_t(_widgets[widgetIndex].contentLength), _widgets[widgetIndex].attributes, _widgets[widgetIndex].style);
							}
							_displayContent(widgetIndex);			//Show the content
						}
					}
					_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState & 0xFFEF;		//Mark content as drawn
				}
			}
		}
	}
	if(_widgets[_selectedWidget].type == _widgetTypes::textInput &&
		(_cursorX != _typingXposition(_selectedWidget) || _cursorX != _contentYorigin(_selectedWidget)))	//Cursor needs moving back to typing position
	{
		moveCursorTo(_typingXposition(_selectedWidget), _contentYorigin(_selectedWidget));					//Move the cursor
	}
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::_widgetsOverlap(const uint8_t widgetIndexA, const uint8_t widgetIndexB)	//Checks if a widgets overlap each other
#else
bool retroTerm::_widgetsOverlap(const uint8_t widgetIndexA, const uint8_t widgetIndexB)						//Checks if a widgets overlap each other
#endif
{
	if(_widgets[widgetIndexA].x + _widgets[widgetIndexA].w < _widgets[widgetIndexB].x)	//Widget A is to the left of Widget B, so not overlapping
	{
		return(false);
	}
	else if(_widgets[widgetIndexB].x + _widgets[widgetIndexB].w < _widgets[widgetIndexA].x)	//Widget B is to the left of Widget A, so not overlapping
	{
		return(false);
	}
	else if(_widgets[widgetIndexA].y + _widgets[widgetIndexA].h < _widgets[widgetIndexB].y)	//Widget A is above Widget B, so not overlapping
	{
		return(false);
	}
	else if(_widgets[widgetIndexB].y + _widgets[widgetIndexB].h < _widgets[widgetIndexA].y)	//Widget B is above Widget A, so not overlapping
	{
		return(false);
	}
	return(true);	//In all other cases they overlap
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_displayWidgetOuterBox(const uint8_t widgetIndex)
#else
void retroTerm::_displayWidgetOuterBox(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].style & OUTER_BOX)							//Only draw a box if specified
	{
		if(_widgets[widgetIndex].label == nullptr)							//No label set
		{
			if(_scrollbarNeeded(widgetIndex))								//Scrollbar is needed
			{
				_drawBoxLines(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, false, true, _widgets[widgetIndex].contentOffset, uint32_t(_contentSize(widgetIndex)), _widgets[widgetIndex].attributes, _widgets[widgetIndex].style);
			}
			else
			{
				_drawBoxLines(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, false, false, 0, 0, _widgets[widgetIndex].attributes, _widgets[widgetIndex].style);
			}
		}
		else															//Label is set
		{
			if(_scrollbarNeeded(widgetIndex))								//Scrollbar is needed
			{
				if(_widgets[widgetIndex].style & LABEL_IN_BOX)				//Label is in its own box
				{
					_drawBoxLines(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, true, true, _widgets[widgetIndex].contentOffset, uint32_t(_contentSize(widgetIndex)), _widgets[widgetIndex].attributes, _widgets[widgetIndex].style);
				}
				else													//Label on first line of box
				{
					_drawBoxLines(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, false, true, _widgets[widgetIndex].contentOffset, uint32_t(_contentSize(widgetIndex)), _widgets[widgetIndex].attributes, _widgets[widgetIndex].style);
				}
			}
			else
			{
				if(_widgets[widgetIndex].style & LABEL_IN_BOX)				//Label is in its own box
				{
					_drawBoxLines(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, true, false, 0, 0, _widgets[widgetIndex].attributes, _widgets[widgetIndex].style);
				}
				else													//Label on first line of box
				{
					_drawBoxLines(_widgets[widgetIndex].x, _widgets[widgetIndex].y, _widgets[widgetIndex].w, _widgets[widgetIndex].h, false, false, 0, 0, _widgets[widgetIndex].attributes, _widgets[widgetIndex].style);
				}
			}
		}
	}
}

#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::_textCapacity(const uint8_t widgetIndex)
#else
uint16_t retroTerm::_textCapacity(const uint8_t widgetIndex)
#endif
{
	return(_linesAvailable(widgetIndex) * _columnsAvailable(widgetIndex));
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_linesAvailable(const uint8_t widgetIndex)
#else
uint8_t retroTerm::_linesAvailable(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].label == nullptr)
	{
		if(_widgets[widgetIndex].style & OUTER_BOX)
		{
			return(_widgets[widgetIndex].h - 2);
		}
		else
		{
			return(_widgets[widgetIndex].h);
		}
	}
	else
	{
		if(_widgets[widgetIndex].style & LABEL_IN_BOX)
		{
			return(_widgets[widgetIndex].h - 4);
		}
		else
		{
			return(_widgets[widgetIndex].h - 3);
		}
	}
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_columnsAvailable(const uint8_t widgetIndex)
#else
uint8_t retroTerm::_columnsAvailable(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].style & OUTER_BOX)
	{
		return(_widgets[widgetIndex].w - 2);
	}
	else
	{
		if(_scrollbarNeeded(widgetIndex))
		{
			return(_widgets[widgetIndex].w - 1);
		}
		else
		{
			return(_widgets[widgetIndex].w);
		}
	}
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::_scrollbarNeeded(const uint8_t widgetIndex)
#else
bool retroTerm::_scrollbarNeeded(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].type == _widgetTypes::staticTextDisplay && _widgets[widgetIndex].contentLength > _linesAvailable(widgetIndex))
	{
		return(true);
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::listBox && _widgets[widgetIndex].contentLength > _linesAvailable(widgetIndex))
	{
		return(true);
	}
	return(false);
}

#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::_contentSize(const uint8_t widgetIndex)
#else
uint16_t retroTerm::_contentSize(const uint8_t widgetIndex)
#endif
{
	#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
	if(_widgets[widgetIndex].content == nullptr)
	{
		return(0);
	}
	else if(_widgets[widgetIndex].currentState & 0x8000)	//Use the PROGMEM variant
	{
		return(strlen_P((PGM_P)_widgets[widgetIndex].content));
	}
	else
	{
		return(strlen(_widgets[widgetIndex].content));
	}
	#else
	if(_widgets[widgetIndex].content == nullptr)
	{
		return(0);
	}
	else
	{
		return(strlen(_widgets[widgetIndex].content));
	}
	#endif
}

#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::_shortcutLength(const uint8_t widgetIndex)
#else
uint16_t retroTerm::_shortcutLength(const uint8_t widgetIndex)
#endif
{
	#if defined(__AVR__)
	return(strlen_P((const char *) pgm_read_word (&keyLabels[_widgets[widgetIndex].shortcut])));
	#elif defined(ESP8266) || defined(ESP32)
	return(strlen_P(keyLabels[_widgets[widgetIndex].shortcut]));
	#else
	return(strlen(keyLabels[_widgets[widgetIndex].shortcut]));
	#endif
}

#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::_labelLength(const uint8_t widgetIndex)
#else
uint16_t retroTerm::_labelLength(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].currentState & 0x4000)
	{
		#if defined(__AVR__)
		return(strlen_P((PGM_P)_widgets[widgetIndex].label));
		#elif defined(ESP8266) || defined(ESP32)
		return(strlen_P(_widgets[widgetIndex].label));
		#else
		return(strlen(_widgets[widgetIndex].label));
		#endif
	}
	else
	{
		return(strlen(_widgets[widgetIndex].label));
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_displayLabel(const uint8_t widgetIndex)
#else
void retroTerm::_displayLabel(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].label != nullptr)
	{
		uint8_t xOffset = 0;
		uint8_t yOffset = 0;
		uint8_t availableWidth = _widgets[widgetIndex].w;
		saveCursorPosition();								//Save cursor state & position
		if(_widgets[widgetIndex].style & OUTER_BOX)
		{
			xOffset = 1;
			yOffset = 1;
			availableWidth -=2;
		}
		else
		{
			if(_widgets[widgetIndex].type == _widgetTypes::button && (_widgets[widgetIndex].style & LABEL_CENTRED) && _widgets[widgetIndex].h >= 3)
			{
				xOffset = 0;
				yOffset = _widgets[widgetIndex].h/2;		
			}
			else
			{
				xOffset = 0;
				yOffset = 0;
			}
		}
		if(_widgets[widgetIndex].type == _widgetTypes::checkbox || _widgets[widgetIndex].type == _widgetTypes::radioButton)
		{
			xOffset++;	//Step past the unicode character used for the checbox or radio button
			availableWidth--;
		}
		clearBox(_widgets[widgetIndex].x + xOffset, _widgets[widgetIndex].y + yOffset, availableWidth, 1,  _widgets[widgetIndex].labelAttributes);
		if(_widgets[widgetIndex].style & LABEL_CENTRED)
		{
			if(_widgets[widgetIndex].style & SHORTCUT_INLINE && _widgets[widgetIndex].shortcut!=noKeyPressed)
			{
				moveCursorTo(_widgets[widgetIndex].x + (_widgets[widgetIndex].w - (2 + _labelLength(widgetIndex) + _shortcutLength(widgetIndex)))/2, _widgets[widgetIndex].y + yOffset);
				_printKeyboardShortcut(widgetIndex);
				_printLabel(widgetIndex);
			}
			else
			{
				moveCursorTo(_widgets[widgetIndex].x + (_widgets[widgetIndex].w - _labelLength(widgetIndex))/2, _widgets[widgetIndex].y + yOffset);
				_printLabel(widgetIndex);
			}
		}
		else if(_widgets[widgetIndex].style & LABEL_RIGHT_JUSTIFIED)
		{
			if(_widgets[widgetIndex].style & SHORTCUT_INLINE && _widgets[widgetIndex].shortcut!=noKeyPressed)
			{
				moveCursorTo(_widgets[widgetIndex].x + _widgets[widgetIndex].w - (2 + _labelLength(widgetIndex) + _shortcutLength(widgetIndex)), _widgets[widgetIndex].y + yOffset);
				_printKeyboardShortcut(widgetIndex);
				_printLabel(widgetIndex);
			}
			else
			{
				moveCursorTo(_widgets[widgetIndex].x + _widgets[widgetIndex].w - _labelLength(widgetIndex), _widgets[widgetIndex].y + yOffset);
				_printLabel(widgetIndex);
			}
		}
		else
		{
			if(_widgets[widgetIndex].style & SHORTCUT_INLINE)
			{
				
				moveCursorTo(_widgets[widgetIndex].x + xOffset, _widgets[widgetIndex].y + yOffset);
				_printKeyboardShortcut(widgetIndex);
				_printLabel(widgetIndex);
			}
			else
			{
				moveCursorTo(_widgets[widgetIndex].x + xOffset, _widgets[widgetIndex].y + yOffset);
				_printLabel(widgetIndex);
			}
		}
		restoreCursorPosition();
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_printLabel(const uint8_t widgetIndex)
#else
void retroTerm::_printLabel(const uint8_t widgetIndex)
#endif
{
	attributes(_widgets[widgetIndex].labelAttributes);					//Set the right attributes
	if(_widgets[widgetIndex].currentState & 0x4000)
	{
		#if defined(__AVR__)
		_printProgStr((const char *) _widgets[widgetIndex].label);
		#elif defined(ESP8266) || defined(ESP32)
		_terminalStream->print(_widgets[widgetIndex].label);
		#else
		_terminalStream->print(_widgets[widgetIndex].label);
		#endif
	}
	else
	{
		_terminalStream->print(_widgets[widgetIndex].label);
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_displayKeyboardShortcut(const uint8_t widgetIndex)
#else
void retroTerm::_displayKeyboardShortcut(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].shortcut != noKeyPressed)
	{
		saveCursorPosition();
		moveCursorTo(_widgets[widgetIndex].x + 1,_widgets[widgetIndex].y);
		attributes(_widgets[widgetIndex].attributes);
		_printKeyboardShortcut(widgetIndex);
		restoreCursorPosition();
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_printKeyboardShortcut(const uint8_t widgetIndex)
#else
void retroTerm::_printKeyboardShortcut(const uint8_t widgetIndex)
#endif
{
	attributes(_widgets[widgetIndex].attributes);	//Set the right attributes
	_terminalStream->print(F("["));
	#if defined(__AVR__)
	_printProgStr((const char *) pgm_read_word (&keyLabels[_widgets[widgetIndex].shortcut]));
	#elif defined(ESP8266) || defined(ESP32)
	_terminalStream->print(keyLabels[_widgets[widgetIndex].shortcut]);
	#else
	_terminalStream->print(keyLabels[_widgets[widgetIndex].shortcut]);
	#endif
	_terminalStream->print(F("]"));
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_displayContent(const uint8_t widgetIndex)
#else
void retroTerm::_displayContent(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].type == _widgetTypes::staticTextDisplay)
	{
		attributes(_widgets[widgetIndex].contentAttributes);
		uint32_t contentPosition = _widgets[widgetIndex].contentOffset;
		uint8_t lineSize = 0;												//Store first line size for later scroll down use
		for(uint8_t line = 0 ; line < _linesAvailable(widgetIndex) ; line++)
		{
			moveCursorTo(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex) + line);	//Put the cursor at start of line
			lineSize = _displayLineOfContent(widgetIndex,contentPosition, true);			//Display one formatted line of content, including clearing the end of the line
			contentPosition += lineSize;													//Move down a line
			if(line == 0)
			{
				_widgets[widgetIndex].value = lineSize;									//Store the size of the first line of content for scrolling down later
			}
		}
		attributes(_defaultAttributes);
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::scrollingTextDisplay)
	{
		uint16_t textCapacity = _textCapacity(widgetIndex);							//Determine how much space is in the box
		moveCursorTo(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex));
		attributes(_widgets[widgetIndex].contentAttributes);
		uint8_t line = 0;
		for(uint16_t characterPosition = 0 ; characterPosition < textCapacity ; characterPosition++)
		{
			_terminalStream->print(_widgets[widgetIndex].content[characterPosition + _widgets[widgetIndex].contentOffset]);	//Print the content
			if((_widgets[widgetIndex].style & OUTER_BOX && (characterPosition % (_widgets[widgetIndex].w - 2) == (_widgets[widgetIndex].w - 3)))
				|| ((_widgets[widgetIndex].style & OUTER_BOX) == 0x00 && (characterPosition % (_widgets[widgetIndex].w - 1) == (_widgets[widgetIndex].w - 2))))//Move down and back, staying inside the box
			{
				line++;
				moveCursorTo(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex) + line);	//Put the cursor at start of line
			}
		}
		attributes(_defaultAttributes);
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::listBox)
	{
		uint8_t linesAvailable = _linesAvailable(widgetIndex);							//Determine how much space is in the box
		uint8_t columnsAvailable = _columnsAvailable(widgetIndex);
		uint8_t currentOption = 0;
		uint16_t contentIndex = 0;
		uint8_t currentColumn = 0;
		uint16_t contentLength = _contentSize(widgetIndex);
		if(_widgets[widgetIndex].contentOffset > 0)											//Move forward through the content, not using strtok as it modifies the content
		{
			while(currentOption<_widgets[widgetIndex].contentOffset)
			{
				#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
				if(_widgets[widgetIndex].currentState & 0x8000)										//Use PROGMEM variant
				{
					if(pgm_read_byte(_widgets[widgetIndex].content + contentIndex) == '\n' || pgm_read_byte(_widgets[widgetIndex].content + contentIndex) == '\r')
					{
						currentOption++;
					}
					contentIndex++;
				}
				else
				{
					if(_widgets[widgetIndex].content[contentIndex] == '\n' || _widgets[widgetIndex].content[contentIndex] == '\r')
					{
						currentOption++;
					}
					contentIndex++;
				}
				#else
				if(_widgets[widgetIndex].content[contentIndex] == '\n' || _widgets[widgetIndex].content[contentIndex] == '\r')
				{
					currentOption++;
				}
				contentIndex++;
				#endif
			}
		}
		attributes(_widgets[widgetIndex].contentAttributes);
		while(currentOption - _widgets[widgetIndex].contentOffset < linesAvailable && contentIndex < contentLength)
		{
			currentColumn = 0;
			moveCursorTo(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex) + currentOption - _widgets[widgetIndex].contentOffset);
			if(currentOption == _widgets[widgetIndex].value)
			{
				attributes(_widgets[widgetIndex].contentAttributes | ATTRIBUTE_INVERSE);	//Highlight the selected option
			}
			#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
			if(_widgets[widgetIndex].currentState & 0x8000)										//Use PROGMEM variant
			{
				while(pgm_read_byte(_widgets[widgetIndex].content + contentIndex) != '\n' && pgm_read_byte(_widgets[widgetIndex].content + contentIndex) != '\r' && contentIndex < contentLength)	//Print the option
				{
					_terminalStream->write(pgm_read_byte(_widgets[widgetIndex].content + contentIndex++));
					currentColumn++;
				}
			}
			else
			{
				while(_widgets[widgetIndex].content[contentIndex] != '\n' && _widgets[widgetIndex].content[contentIndex] != '\r' && contentIndex < contentLength)	//Print the option
				{
					_terminalStream->print(_widgets[widgetIndex].content[contentIndex++]);
					currentColumn++;
				}
			}
			#else
			while(_widgets[widgetIndex].content[contentIndex] != '\n' && _widgets[widgetIndex].content[contentIndex] != '\r' && contentIndex < contentLength)	//Print the option
			{
				_terminalStream->print(_widgets[widgetIndex].content[contentIndex++]);
				currentColumn++;
			}
			#endif
			while(currentColumn++ < columnsAvailable)	//Blank the space in the widget as options are most likely variable length
			{
				_terminalStream->print(' ');
			}
			if(currentOption == _widgets[widgetIndex].value)
			{
				attributes(_widgets[widgetIndex].contentAttributes);	//Remove highlight
			}
			contentIndex++;			//Step over the \n
			currentOption++;		//Increment the option counter
		}
		if(_widgets[widgetIndex].contentLength < linesAvailable) //Clear the rest of the box, if necessary
		{
			currentColumn = 0;
			clearBox(_contentXorigin(widgetIndex), _contentYorigin(widgetIndex) + _widgets[widgetIndex].contentLength, columnsAvailable, linesAvailable - _widgets[widgetIndex].contentLength);//, uint16_t specifiedAttributes)						//x y w h
		}
		attributes(_defaultAttributes);
	}
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_lineSize(const uint8_t widgetIndex, const uint32_t offset)
#else
uint8_t retroTerm::_lineSize(const uint8_t widgetIndex, const uint32_t offset)
#endif
{
	return(_displayLineOfContent(widgetIndex, offset, false));	//Use the content printing routine to tell how much content is on the line
}

/*#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_displayLineOfContent(const uint8_t widgetIndex, const uint32_t offset, bool printContent = true)
#else
uint8_t retroTerm::_displayLineOfContent(const uint8_t widgetIndex, const uint32_t offset, bool printContent = true)
#endif
{
	uint8_t column = 1;
	uint8_t contentProcessed = 0;
	char firstCharacter = _currentCharacter(widgetIndex, offset);
	char secondCharacter = _currentCharacter(widgetIndex, offset + 1);
	char thirdCharacter = _currentCharacter(widgetIndex, offset + 2);
	bool bold = false;
	bool italic = false;
	bool boldItalic = false;
	bool heading = false;
	bool potentialHorizontalLine = false;
	uint8_t wordLength = 0;
	uint8_t forwardWhitespaceLength = 0;
	while(column <= _columnsAvailable(widgetIndex) && offset + contentProcessed < _contentSize(widgetIndex))
	{
		#if defined(PROCESS_MARKDOWN)
		if(firstCharacter == '#' && secondCharacter == ' ')	//This is a heading, we only support one level
		{
			contentProcessed += 2;	//Gobble up the header formatting
			if(printContent)
			{
				attributes(attributes() | ATTRIBUTE_UNDERLINE);
			}
			heading = true;
		}
		else if(firstCharacter == '\r' && secondCharacter == '\n')	//Forced line break, windows style
		#else
		if(firstCharacter == '\r' && secondCharacter == '\n')	//Forced line break, windows style
		#endif
		{
			#if defined(PROCESS_MARKDOWN)
			if(heading)
			{
				if(printContent)
				{
					attributes(attributes() & (0xFFFF ^ ATTRIBUTE_UNDERLINE));	//Remove the underline
				}
				heading = false;
			} 
			#endif
			contentProcessed += 2;
			break;
		}
		else if(firstCharacter == '\n')	//Forced line break
		{
			#if defined(PROCESS_MARKDOWN)
			if(heading)
			{
				if(printContent)
				{
					attributes(attributes() & (0xFFFF ^ ATTRIBUTE_UNDERLINE));	//Remove the underline
				}
				heading = false;
			} 
			#endif
			contentProcessed++;
			break;
		}
		else if(firstCharacter == '\t')	//Tab stop
		{
			if(column + (tabStopWidth - (column-1)%tabStopWidth) < _columnsAvailable(widgetIndex))
			{
				for (uint8_t c = 0; c <= tabStopWidth - (column-1)%tabStopWidth; c++)
				{
					if(printContent)
					{
						_terminalStream->print(' ');
					}
					column++;
				}
				contentProcessed++;	//Gobble the tab
			}
			else
			{
				#if defined(PROCESS_MARKDOWN)
				if(heading)
				{
					if(printContent)
					{
						attributes(attributes() & (0xFFFF ^ ATTRIBUTE_UNDERLINE));	//Remove the underline
					}
					heading = false;
				} 
				#endif
				break;
			}
		}
		else if(isspace(firstCharacter))
		{
			forwardWhitespaceLength = _whitespaceLength(widgetIndex, offset + contentProcessed);
			if(column + forwardWhitespaceLength > _columnsAvailable(widgetIndex))	//Break the line and gobble excess whitespace so next line starts on a word
			{
				contentProcessed += forwardWhitespaceLength;
				break;
			}
			else
			{
				if(printContent)
				{
					_terminalStream->print(firstCharacter);
				}
				contentProcessed++;
				column++;
			}
		}
		else	//A word, which needs checking for wrap
		{
			wordLength = _wordLength(widgetIndex, offset + contentProcessed);
			if(column + wordLength <= _columnsAvailable(widgetIndex))	//Print the word, including markdown formatting
			{
				for (uint8_t c = 0; c < wordLength; c++)
				{
					#if defined(PROCESS_MARKDOWN)
					firstCharacter =  _currentCharacter(widgetIndex, offset + contentProcessed + c);
					if(c + 1 < wordLength)
					{
						secondCharacter = _currentCharacter(widgetIndex, offset + contentProcessed + c + 1);
					}
					else
					{
						secondCharacter = ' ';
					}
					if(c + 2 < wordLength)
					{
						thirdCharacter =  _currentCharacter(widgetIndex, offset + contentProcessed + c + 2);
					}
					else
					{
						thirdCharacter = ' ';
					}
					if((firstCharacter == '*' && secondCharacter == '*' && thirdCharacter == '*') || (firstCharacter == '_' && secondCharacter == '_' && thirdCharacter == '_'))	//Bold italic
					{
						if(boldItalic == false)
						{
							if(printContent)
							{
								attributes(attributes() | ATTRIBUTE_INVERSE);
							}
							boldItalic = true;
						}
						else
						{
							if(printContent)
							{
								attributes(attributes() & (0xFFFF ^ ATTRIBUTE_INVERSE));
							}
							boldItalic = false;
						}
						contentProcessed += 3;	//Gobble up the formatting
						c += 3;
						if(column == 1)
						{
							potentialHorizontalLine = true;
						}
					}
					else if(column == 1 && firstCharacter == '-' && secondCharacter == '-' && thirdCharacter == '-')	//Most likely a horizontal line
					{
						contentProcessed += 3;	//Gobble up the formatting
						potentialHorizontalLine = true;
					}
					else if((firstCharacter == '*' && secondCharacter == '*' && thirdCharacter != '*') || (firstCharacter == '_' && secondCharacter == '_' && thirdCharacter != '_'))	//Bold
					{
						if(bold == false)
						{
							if(printContent)
							{
								attributes(attributes() | ATTRIBUTE_BOLD);
							}
							bold = true;
							contentProcessed += 2;	//Gobble up the formatting
							c += 2;
						}
					}
					else if((firstCharacter == '*' && secondCharacter == '*' && (thirdCharacter == ' ' || ispunct(thirdCharacter))) || (firstCharacter == '_' && secondCharacter == '_' && (thirdCharacter == ' ' || ispunct(thirdCharacter))))	//Bold off
					{
						//if(bold == true)
						{
							if(printContent)
							{
								attributes(attributes() & (0xFFFF ^ ATTRIBUTE_BOLD));
							}
							bold = false;
							contentProcessed += 2;	//Gobble up the formatting
							c += 2;
						}
					}
					else if((firstCharacter == '*' || firstCharacter == '_') && isSpace(secondCharacter) == false)	//Italic on
					{
						if(italic == false)
						{
							if(printContent)
							{
								attributes(attributes() | ATTRIBUTE_BLINK);
							}
							italic = true;
						}
						if(printContent)
						{
							_terminalStream->print(firstCharacter);
						}
						c += 1;
						contentProcessed += 1;	//Gobble up the header formatting
					}
					else if(isSpace(firstCharacter) == false && ((secondCharacter == '*' && thirdCharacter != '*') || (secondCharacter == '_' && thirdCharacter != '_')))	//Italic off
					{
						if(italic == true)
						{
							if(printContent)
							{
								attributes(attributes() & (0xFFFF ^ ATTRIBUTE_BLINK));
							}
							italic = false;
						}
						if(printContent)
						{
							_terminalStream->print(firstCharacter);
						}
						column++;
						c += 2;
						contentProcessed += 2;	//Gobble up the header formatting
					}
					else
					{
						if(potentialHorizontalLine)	//Printing some content means it wasn't a horizontal line
						{
							potentialHorizontalLine = false;
						}
						if(printContent)
						{
							_terminalStream->print(_currentCharacter(widgetIndex, offset + contentProcessed));
						}
						contentProcessed++;
						column++;
					}
					#else
					if(printContent)
					{
						_terminalStream->print(_currentCharacter(widgetIndex, offset + contentProcessed));
					}
					contentProcessed++;
					column++;
					#endif
				}
			}
			else	//End the line
			{
				if(heading)
				{
					if(printContent)
					{
						attributes(attributes() & (0xFFFF ^ ATTRIBUTE_UNDERLINE));
					}
					heading = false;
				}
				break;
			}
		}
		firstCharacter =  _currentCharacter(widgetIndex, offset + contentProcessed);
		secondCharacter = _currentCharacter(widgetIndex, offset + contentProcessed + 1);
		thirdCharacter =  _currentCharacter(widgetIndex, offset + contentProcessed + 2);
	}
	if(heading && printContent)	//Clear the underline on a heading
	{
		attributes(attributes() ^ ATTRIBUTE_UNDERLINE);
	}
	if(printContent && potentialHorizontalLine && column == 1)	//It's been just *** or ___ on the line so it's a horizontal line
	{
		attributes(attributes() & (0xFFFF ^ ATTRIBUTE_INVERSE));
		_terminalStream->print(" ");
		column++;
		while(column++ < _columnsAvailable(widgetIndex))
		{
			_printUnicodeCharacter(0, 1);	//The horizontal line
		}
	}
	if(printContent)
	{
		while(column++ <= _columnsAvailable(widgetIndex))
		{
			_terminalStream->print(" ");
		}
	}
	return(contentProcessed);
}*/

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_displayLineOfContent(const uint8_t widgetIndex, const uint32_t offset, bool printContent = true)
#else
uint8_t retroTerm::_displayLineOfContent(const uint8_t widgetIndex, const uint32_t offset, bool printContent = true)
#endif
{
	uint8_t column = 1;
	uint8_t contentProcessed = 0;
	char currentCharacter = _currentCharacter(widgetIndex, offset);
	#if defined(PROCESS_MARKDOWN)
	bool bold = false;
	bool italic = false;
	bool boldItalic = false;
	bool blockquote = false;
	uint8_t headingLevel = 0;
	uint8_t wordLength = 0;
	uint8_t whitespaceLength = 0;
	uint8_t runLength = 0;
	bool preceededOnlyByWhiteSpace = true;
	#endif
	while(column <= _columnsAvailable(widgetIndex) && offset + contentProcessed < _contentSize(widgetIndex))
	{
		if(currentCharacter == '\r' && _currentCharacter(widgetIndex, offset + contentProcessed + 1) == '\n')	//Forced line break, windows style
		{
			#if defined(PROCESS_MARKDOWN)
			if(headingLevel)
			{
				if(printContent)
				{
					attributes(attributes() & (0xFFFF ^ ATTRIBUTE_UNDERLINE));	//Remove the underline
				}
				headingLevel = 0;
			} 
			#endif
			contentProcessed += 2;
			break;
		}
		else if(currentCharacter == '\n' || currentCharacter == '\r')	//Forced line break
		{
			#if defined(PROCESS_MARKDOWN)
			if(headingLevel)
			{
				if(printContent)
				{
					attributes(attributes() & (0xFFFF ^ ATTRIBUTE_UNDERLINE));	//Remove the underline
				}
				headingLevel = 0;
			} 
			#endif
			contentProcessed++;
			break;
		}
		else if(currentCharacter == '\t')	//Tab stop
		{
			if(column + (tabStopWidth - (column-1)%tabStopWidth) < _columnsAvailable(widgetIndex))
			{
				for (uint8_t c = 0; c <= tabStopWidth - (column-1)%tabStopWidth; c++)
				{
					if(printContent)
					{
						_terminalStream->print(' ');
					}
					column++;
				}
				contentProcessed++;	//Gobble the tab
			}
			else	//End the line
			{
				#if defined(PROCESS_MARKDOWN)
				if(headingLevel)
				{
					if(printContent)
					{
						attributes(attributes() & (0xFFFF ^ ATTRIBUTE_UNDERLINE));	//Remove the underline
					}
					headingLevel = 0;
				} 
				#endif
				break;
			}
		}
		else if(isspace(currentCharacter))
		{
			whitespaceLength = _whitespaceLength(widgetIndex, offset + contentProcessed);
			if(column + whitespaceLength > _columnsAvailable(widgetIndex))	//Break the line and gobble excess whitespace so next line starts on a word
			{
				contentProcessed += whitespaceLength;
				break;
			}
			else
			{
				if(printContent)
				{
					_terminalStream->print(currentCharacter);
				}
				contentProcessed++;
				column++;
			}
		}
		#if defined(PROCESS_MARKDOWN)
		else if(currentCharacter == '#')	//This is potentially a heading which is 1-6 # followed by a space
		{
			preceededOnlyByWhiteSpace = false;
			runLength = _runLength(widgetIndex, offset + contentProcessed, '#');
			headingLevel = runLength;
			if(_currentCharacter(widgetIndex, offset + contentProcessed + runLength) == ' ')
			{
				if(printContent)
				{
					if(headingLevel == 1)
					{
						attributes(attributes() | ATTRIBUTE_UNDERLINE | ATTRIBUTE_BOLD);
					}
					else if(headingLevel == 2)
					{
						attributes(attributes() | ATTRIBUTE_UNDERLINE);
					}
					else
					{
						attributes(attributes() | ATTRIBUTE_UNDERLINE | ATTRIBUTE_FAINT);
					}
				}
				contentProcessed += 1 + runLength;	//Gobble up the header formatting
			}
		}
		#endif
		else	//A word, which needs checking for wrap
		{
			wordLength = _wordLength(widgetIndex, offset + contentProcessed);
			if(column + wordLength - 1 <= _columnsAvailable(widgetIndex))	//Print the word, including markdown formatting
			{
				for (uint8_t c = 0; c < wordLength; c++)
				{
					#if defined(PROCESS_MARKDOWN)
					currentCharacter =  _currentCharacter(widgetIndex, offset + contentProcessed + c);
					if(currentCharacter == '*' || currentCharacter == '_')	//Various options for bold/italic
					{
						runLength = 0;
						if(currentCharacter == '*')
						{
							runLength = _runLength(widgetIndex, offset + contentProcessed + c, '*');
						}
						else if(currentCharacter == '_')
						{
							runLength = _runLength(widgetIndex, offset + contentProcessed + c, '_');
						}
						if(runLength == 1)	//Italic
						{
							if(italic == false && isalnum(_currentCharacter(widgetIndex, offset + contentProcessed + c + runLength)))	//Only switch on at start of word
							{
								if(printContent)
								{
									attributes(attributes() | ATTRIBUTE_FAINT);
								}
								italic = true;
								//c++;
							}
							else if(italic == true)
							{
								if(printContent)
								{
									attributes(attributes() & (0xFFFF ^ ATTRIBUTE_FAINT));
								}
								italic = false;
								//c++;
							}
							//else if(column == 1 && isspace(_currentCharacter(widgetIndex, offset + contentProcessed + c + runLength)))	//This is an unordered list with a *
							else if(preceededOnlyByWhiteSpace && isspace(_currentCharacter(widgetIndex, offset + contentProcessed + c + runLength)))	//This is an unordered list with a *
							{
								if(printContent)
								{
									_printUnicodeCharacter(_widgets[widgetIndex].style & 0x01, 18);
								}
								column++;
							}
							else
							{
								if(printContent)
								{
									_terminalStream->print(_currentCharacter(widgetIndex, offset + contentProcessed + c));
								}
								preceededOnlyByWhiteSpace = false;
								column++;
							}
						}
						else if(runLength == 2)	//Bold
						{
							if(bold == false && isalnum(_currentCharacter(widgetIndex, offset + contentProcessed + c + runLength)))	//Only switch on at start of word
							{
								if(printContent)
								{
									attributes(attributes() | ATTRIBUTE_BOLD);
								}
								bold = true;
								c += 1;
							}
							else if(bold == true)
							{
								if(printContent)
								{
									attributes(attributes() & (0xFFFF ^ ATTRIBUTE_BOLD));
								}
								bold = false;
								c += 1;
							}
						}
						else if(runLength >= 3)	//Bold italic, or potentially a horizontal line
						{
							if(column == 1 && (_currentCharacter(widgetIndex, offset + contentProcessed + c + runLength) == '\n' || _currentCharacter(widgetIndex, offset + contentProcessed + c + runLength) == '\r')) //Horizontal line
							{
								_terminalStream->print(" ");
								column++;
								while(column++ < _columnsAvailable(widgetIndex)-1)
								{
									_printUnicodeCharacter(0, 1);	//The horizontal line
								}
								c += runLength - 1;
								//break;	//End the line
							}
							else	//Bold & italic
							{
								if(boldItalic == false)
								{
									if(printContent)
									{
										attributes(attributes() | ATTRIBUTE_INVERSE);
									}
									boldItalic = true;
									c += runLength - 1;
								}
								else if(boldItalic == true)
								{
									if(printContent)
									{
										attributes(attributes() & (0xFFFF ^ ATTRIBUTE_INVERSE));
									}
									boldItalic = false;
									c += runLength - 1;
								}
							}
						}
					}
					else if(currentCharacter == '-')	//Horizontal line or unordered list
					{
						runLength = _runLength(widgetIndex, offset + contentProcessed + c, '-');
						if(column == 1 && runLength >= 3 && (_currentCharacter(widgetIndex, offset + contentProcessed + c + runLength) == '\n' || _currentCharacter(widgetIndex, offset + contentProcessed + c + runLength) == '\r')) //That's all that's on this line so it's a horizontal line
						{
							_terminalStream->print(" ");
							column++;
							while(column++ < _columnsAvailable(widgetIndex)-1)
							{
								_printUnicodeCharacter(0, 1);	//The horizontal line
							}
							c += runLength;
							//break;	//End the line
						}
						else if(preceededOnlyByWhiteSpace && runLength == 1 && isspace(_currentCharacter(widgetIndex, offset + contentProcessed + c + runLength)))	//This is an unordered list with a *
						{
							if(printContent)
							{
								_printUnicodeCharacter(_widgets[widgetIndex].style & 0x01, 18);
							}
							column++;
						}
						else	//Just a '-' on its own
						{
							if(printContent)
							{
								preceededOnlyByWhiteSpace = false;
								_terminalStream->print(_currentCharacter(widgetIndex, offset + contentProcessed + c));
							}
							column++;
						}
					}
					else if(currentCharacter == '>' && preceededOnlyByWhiteSpace)	//Blockquote
					{
						runLength = _runLength(widgetIndex, offset + contentProcessed + c, '>');
						column += runLength;
						c += runLength - 1;
						blockquote = true;
						if(printContent)
						{
							while(runLength > 0)
							{
								_printUnicodeCharacter(_widgets[widgetIndex].style & 0x01, 3);
								runLength--;
							}
						}
					}
					else	//Print the actual content
					#endif
					{
						if(printContent)
						{
							_terminalStream->print(_currentCharacter(widgetIndex, offset + contentProcessed + c));
						}
						preceededOnlyByWhiteSpace = false;
						column++;
					}
				}
				contentProcessed += wordLength;
			}
			else	//End the line
			{
				/*if(headingLevel > 0)
				{
					if(printContent)
					{
						if(headingLevel == 1)// && bold == false)
						{
							//attributes(attributes() & (0xFFFF ^ ATTRIBUTE_BOLD));
						}
						//attributes(attributes() & (0xFFFF ^ ATTRIBUTE_UNDERLINE));
					}
					headingLevel = 0;
				}*/
				break;
			}
		}
		currentCharacter =  _currentCharacter(widgetIndex, offset + contentProcessed);
	}
	if(printContent)	//Clear the underline on a heading
	{
		//if(headingLevel == 1)
		if(not bold)
		{
			attributes(attributes() & (0xFFFF ^ ATTRIBUTE_BOLD));
		}
		if(not italic)
		{
			attributes(attributes() & (0xFFFF ^ ATTRIBUTE_FAINT));
		}
		if(headingLevel > 0)
		{
			attributes(attributes() & (0xFFFF ^ ATTRIBUTE_UNDERLINE));
		}
	}
	if(printContent)
	{
		while(column++ <= _columnsAvailable(widgetIndex))
		{
			_terminalStream->print(" ");
		}
	}
	return(contentProcessed);
}


#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_wordLength(const uint8_t widgetIndex, uint32_t offset)
#else
uint8_t retroTerm::_wordLength(const uint8_t widgetIndex, uint32_t offset)
#endif
{
	uint8_t length = 0;
	//while(offset < _contentSize(widgetIndex) && (isalnum(_currentCharacter(widgetIndex, offset++)) || ispunct(_currentCharacter(widgetIndex, offset++))))
	while(offset < _contentSize(widgetIndex) && isspace(_currentCharacter(widgetIndex, offset++)) == false)
	{
		length++;
	}
	return(length);
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_runLength(const uint8_t widgetIndex, uint32_t offset, const char character)
#else
uint8_t retroTerm::_runLength(const uint8_t widgetIndex, uint32_t offset, const char character)
#endif
{
	uint8_t length = 0;
	while(offset < _contentSize(widgetIndex) && _currentCharacter(widgetIndex, offset++) == character)
	{
		length++;
	}
	return(length);
}


#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_whitespaceLength(const uint8_t widgetIndex, uint32_t offset)
#else
uint8_t retroTerm::_whitespaceLength(const uint8_t widgetIndex, uint32_t offset)
#endif
{
	uint8_t length = 0;
	while(offset < _contentSize(widgetIndex) && isspace(_currentCharacter(widgetIndex, offset++)) == true)
	{
		length++;
	}
	return(length);
}

#if defined(ESP8266) || defined(ESP32)
uint32_t ICACHE_FLASH_ATTR retroTerm::_previousLineOffset(const uint8_t widgetIndex,const uint32_t offset)
#else
uint32_t retroTerm::_previousLineOffset(const uint8_t widgetIndex,const uint32_t offset)
#endif
{
	/*if(offset > _columnsAvailable(widgetIndex))
	{
		return(offset - _columnsAvailable(widgetIndex));
	}
	else
	{
		return(0);
	}*/
	uint32_t tempOffset;
	if(offset > _columnsAvailable(widgetIndex))
	{
		tempOffset = offset - _columnsAvailable(widgetIndex);
	}
	else
	{
		tempOffset = 0;
	}
	//uint8_t lineSize = _lineSize(widgetIndex, tempOffset);	//Go fishing for where the previous line starts
	uint8_t lineSize = _displayLineOfContent(widgetIndex,tempOffset, true);
	if(tempOffset + lineSize == offset)								//Good luck with first guess
	{
		printAt(1,1,F("HIT"));
		return(tempOffset);
	}
	else if(tempOffset + lineSize < offset)							//Too far back
	{
		printAt(1,1,"LOW by " + String(offset - (tempOffset + lineSize)));
		tempOffset++;
		/*while(tempOffset + _lineSize(widgetIndex, tempOffset) < offset)
		{
			tempOffset++;
		}*/
		return(tempOffset);
	}
	else if(tempOffset + lineSize > offset)							//Overshot
	{
		printAt(1,1,"HIGH by " + String((tempOffset + lineSize) - offset));
		tempOffset--;
		/*while(tempOffset + _lineSize(widgetIndex, tempOffset) > offset)
		{
			tempOffset--;
		}*/
		return(tempOffset);
	}
	else
	{
		return(tempOffset);
	}
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_contentXorigin(const uint8_t widgetIndex)
#else
uint8_t retroTerm::_contentXorigin(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].style & OUTER_BOX)
	{
		return(_widgets[widgetIndex].x + 1);
	}
	else
	{
		return(_widgets[widgetIndex].x);
	}
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_contentYorigin(const uint8_t widgetIndex)
#else
uint8_t retroTerm::_contentYorigin(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].label == nullptr)
	{
		if(_widgets[widgetIndex].style & OUTER_BOX)
		{
			return(_widgets[widgetIndex].y + 1);
		}
		else
		{
			return(_widgets[widgetIndex].y);
		}
	}
	else
	{
		if(_widgets[widgetIndex].style & LABEL_IN_BOX)
		{
			return(_widgets[widgetIndex].y + 3);
		}
		else
		{
			if(_widgets[widgetIndex].style & OUTER_BOX)
			{
				if(_widgets[widgetIndex].h >= 4)
				{
					return(_widgets[widgetIndex].y + 2);
				}
				else
				{
					return(_widgets[widgetIndex].y + 1);
				}
			}
			else
			{
				if(_widgets[widgetIndex].h >= 2)
				{
					return(_widgets[widgetIndex].y + 1);
				}
				else
				{
					return(_widgets[widgetIndex].y);
				}
			}				
		}
	}
}


#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::numberOfOptions(const uint8_t widgetId)
#else
uint8_t retroTerm::numberOfOptions(const uint8_t widgetId)
#endif
{
	if(_widgetExists(widgetId - 1) && _widgets[widgetId - 1].type == _widgetTypes::listBox)
	{
		return(_widgets[widgetId - 1].contentLength);
	}
	else
	{
		return(0);
	}
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_calculateNumberOfOptions(const uint8_t widgetIndex)
#else
uint8_t retroTerm::_calculateNumberOfOptions(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].content == nullptr)
	{
		return(0);
	}
	else
	{
		uint8_t count = 1;
		#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
		if(_widgets[widgetIndex].currentState & 0x8000)
		{
			for(uint16_t arrayIterator = 0 ; arrayIterator < strlen_P((PGM_P)_widgets[widgetIndex].content) ; arrayIterator++)
			{
				if(pgm_read_byte(_widgets[widgetIndex].content + arrayIterator) == '\n' || pgm_read_byte(_widgets[widgetIndex].content + arrayIterator) == '\r')
				{
					count++;
				}
			}
		}
		else
		{
			for(uint16_t arrayIterator = 0 ; arrayIterator < strlen(_widgets[widgetIndex].content) ; arrayIterator++)
			{
				if(_widgets[widgetIndex].content[arrayIterator] == '\n' || _widgets[widgetIndex].content[arrayIterator] == '\r')
				{
					count++;
				}
			}
		}
		#else
		for(uint16_t arrayIterator = 0 ; arrayIterator < strlen(_widgets[widgetIndex].content) ; arrayIterator++)
		{
			if(_widgets[widgetIndex].content[arrayIterator] == '\n' || _widgets[widgetIndex].content[arrayIterator] == '\r')
			{
				count++;
			}
		}
		#endif
		return(count);
	}
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::_calculateNumberOfLines(const uint8_t widgetIndex)
#else
uint8_t retroTerm::_calculateNumberOfLines(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].content == nullptr)
	{
		return(0);
	}
	else
	{
		uint8_t count = 1;
		uint8_t columnsAvailable = _columnsAvailable(widgetIndex);
		#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
		if(_widgets[widgetIndex].currentState & 0x8000)
		{
			for(uint16_t arrayIterator = 0 ; arrayIterator < strlen_P((PGM_P)_widgets[widgetIndex].content) ; arrayIterator++)
			{
				if(pgm_read_byte(_widgets[widgetIndex].content + arrayIterator) == '\n')
				{
					count++;
				}
				else if(arrayIterator % columnsAvailable == columnsAvailable - 1)
				{
					count++;
				}
			}
		}
		else
		{
			for(uint16_t arrayIterator = 0 ; arrayIterator < strlen(_widgets[widgetIndex].content) ; arrayIterator++)
			{
				if(_widgets[widgetIndex].content[arrayIterator] == '\n')
				{
					count++;
				}
				else if(arrayIterator % columnsAvailable == columnsAvailable - 1)
				{
					count++;
				}
			}
		}
		#else
		for(uint16_t arrayIterator = 0 ; arrayIterator < strlen(_widgets[widgetIndex].content) ; arrayIterator++)
		{
			if(_widgets[widgetIndex].content[arrayIterator] == '\n')
			{
				count++;
			}
			else if(arrayIterator % columnsAvailable == columnsAvailable - 1)
			{
				count++;
			}
		}
		#endif
		return(count);
	}
}


//Processes a click of shortcut when it hits a widget
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_clickWidget(const uint8_t widgetIndex)
#else
void retroTerm::_clickWidget(const uint8_t widgetIndex)
#endif
{
	if(widgetIndex != _selectedWidget && _widgets[_selectedWidget].type == _widgetTypes::textInput)	//Current widget is a text input
	{
		hideCursor();																		//Hide the cursor, otherwise it'll visibly jump away from the text input
	}
	if(_widgets[widgetIndex].type == _widgetTypes::button)
	{
		_widgets[widgetIndex].value = true;													//Register a click to inform the application
		_selectedWidget = widgetIndex;														//Select this widget, but nothing actually changes
		_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0200;	//Mark the widget clicked
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::checkbox)
	{
		_widgets[widgetIndex].value = not _widgets[widgetIndex].value;						//Invert the state
		_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0210;	//Mark the widget clicked and content as changed
		_selectedWidget = widgetIndex;														//Select this widget, but nothing actually changes
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::radioButton)
	{
		if(_widgets[widgetIndex].value == false)											//Only act on clicks to unselected radio buttons
		{
			for(uint8_t id = 0 ; id < _widgetObjectLimit ; id++)
			{
				if(_widgets[id].type == _widgetTypes::radioButton && (_widgets[id].currentState & 0x0003) == 0x0003 && _widgets[id].value) //Widget is visible and true
				{
					_widgets[id].value = false;													//Unselect the current selected button
					_widgets[id].currentState = _widgets[id].currentState | 0x0010;				//Mark the widget state as changed
				}
			}
			_widgets[widgetIndex].value = true;													//Mark the widget as checked
			_selectedWidget = widgetIndex;														//Select this widget, but nothing actually changes
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0210;	//Mark the widget clicked
		}
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::staticTextDisplay)
	{
		if(_selectedWidget != widgetIndex)														//Only select if not previously selected
		{
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0200;	//Mark the widget clicked
			_selectedWidget = widgetIndex;														//Select this widget, but nothing actually changes
		}
		_handleScrollbarClicks(widgetIndex);
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::scrollingTextDisplay)
	{
		if(_selectedWidget != widgetIndex)														//Only select if not previously selected
		{
			_selectedWidget = widgetIndex;														//Select this widget, but nothing actually changes
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0200;	//Mark the widget clicked
		}
		_handleScrollbarClicks(widgetIndex);
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::textInput)
	{
		if(_selectedWidget != widgetIndex)														//Only select if not previously selected
		{
			moveCursorTo(_typingXposition(widgetIndex),_contentYorigin(widgetIndex));			//Move the cursor to the typing position
			showCursor();																		//Show the cursor
			_selectedWidget = widgetIndex;														//Select this widget, but nothing actually changes
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0200;	//Mark the widget clicked
		}
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::listBox)
	{
		if(_selectedWidget != widgetIndex)														//Only select if not previously selected
		{
			_selectedWidget = widgetIndex;														//Select this widget, but nothing actually changes
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0200;	//Mark the widget clicked
		}
		if((_mouseStatus & 0x04) && _mouseX >= _contentXorigin(widgetIndex) && _mouseX < _contentXorigin(widgetIndex) + _columnsAvailable(widgetIndex) && _mouseY >= _contentYorigin(widgetIndex) && _mouseY < _contentYorigin(widgetIndex) + _linesAvailable(widgetIndex) && _mouseY - _contentYorigin(widgetIndex) < _widgets[widgetIndex].contentLength
			&& _widgets[widgetIndex].value != _mouseY + _widgets[widgetIndex].contentOffset - _contentYorigin(widgetIndex))	//Option has changed
		{
			_widgets[widgetIndex].value = _mouseY + _widgets[widgetIndex].contentOffset - _contentYorigin(widgetIndex);		//Choose the option
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0210;								//Mark the widget content as changed and widget as clicked
		}
		else
		{
			_handleScrollbarClicks(widgetIndex);
		}
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_handleScrollbarClicks(const uint8_t widgetIndex)
#else
void retroTerm::_handleScrollbarClicks(const uint8_t widgetIndex)
#endif
{
	if(_scrollbarNeeded(widgetIndex))
	{
		if((_mouseStatus & 0x04) && _mouseX == _widgets[widgetIndex].x + _widgets[widgetIndex].w - 1)	//Right hand border clicked, possible scroll
		{
			if(_mouseY == _contentYorigin(widgetIndex) + _linesAvailable(widgetIndex) - 1)				//Down arrow clicked
			{
				_scrollDown(widgetIndex);
			}
			else if(_mouseY == _contentYorigin(widgetIndex)) 											//No title, up arrow clicked
			{
				_scrollUp(widgetIndex);
			}
		}
		else if(_mouseStatus & 0x20)
		{
			_scrollDown(widgetIndex);
		}
		else if(_mouseStatus & 0x10)
		{
			_scrollUp(widgetIndex);
		}
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_scrollDown(const uint8_t widgetIndex)
#else
void retroTerm::_scrollDown(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].type == _widgetTypes::staticTextDisplay)
	{
		if(_widgets[widgetIndex].contentOffset + _widgets[widgetIndex].value < _contentSize(widgetIndex))	//Don't overflow
		{
			_widgets[widgetIndex].contentOffset += _widgets[widgetIndex].value;						//Move down one line, using the stored line size from when it was printed
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0010;		//Mark the widget state as changed
		}
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::listBox && _widgets[widgetIndex].contentOffset + _linesAvailable(widgetIndex) < _widgets[widgetIndex].contentLength)
	{
		_widgets[widgetIndex].contentOffset++;													//Move down one option
		_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0010;		//Mark the widget state as changed
	}
}

#if defined(ESP8266) || defined(ESP32)
char ICACHE_FLASH_ATTR retroTerm::_currentCharacter(const uint8_t widgetIndex, const uint32_t offset)
#else
char retroTerm::_currentCharacter(const uint8_t widgetIndex, const uint32_t offset)
#endif
{
	if(offset < _contentSize(widgetIndex))
	{
		#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
		if(_widgets[widgetIndex].currentState & 0x8000)	//Use the PROGMEM variant
		{
			return(pgm_read_byte(_widgets[widgetIndex].content + offset));
		}
		else
		{
			return(_widgets[widgetIndex].content[offset]);
		}
		#else
		return(_widgets[widgetIndex].content[offset]);
		#endif
	}
	else
	{
		return(' ');	//Any invalid content is a space
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_scrollUp(const uint8_t widgetIndex)
#else
void retroTerm::_scrollUp(const uint8_t widgetIndex)
#endif
{
	if(_widgets[widgetIndex].type == _widgetTypes::staticTextDisplay)
	{
		/*if(_widgets[widgetIndex].contentOffset > _previousLineOffset(widgetIndex, _widgets[widgetIndex].contentOffset))
		{
			_widgets[widgetIndex].contentOffset = _previousLineOffset(widgetIndex, _widgets[widgetIndex].contentOffset);	//Move up one line
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0010;								//Mark the widget state as changed
		}
		else
		{
			_widgets[widgetIndex].contentOffset = 0;
		}*/
		if(_widgets[widgetIndex].contentOffset > _columnsAvailable(widgetIndex))
		{
			_widgets[widgetIndex].contentOffset -= _columnsAvailable(widgetIndex);
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0010;	//Mark the widget state as changed
		}
		else if(_widgets[widgetIndex].contentOffset > 0)
		{
			_widgets[widgetIndex].contentOffset = 0;
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0010;	//Mark the widget state as changed
		}
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::listBox && _widgets[widgetIndex].contentOffset > 0)
	{
		_widgets[widgetIndex].contentOffset--;
		_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0010;	//Mark the widget state as changed
	}
}

//Probing commands that interrogate the terminal
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::probeSize()	//Warning, this is blocking for up to 1s so do it sparingly
#else
bool retroTerm::probeSize()	//Warning, this is blocking for up to 1s so do it sparingly
#endif
{
	saveCursorPosition();
	hideCursor();
	moveCursorTo(255,255);
	requestCursorPosition();
	uint32_t timeout = millis();
	while(_cursorPositionReceived == false && millis() - timeout < 1000ul)
	{
		_readInput();
	}
	if(_cursorPositionReceived)
	{
		_columns = _cursorX;
		_lines = _cursorY;
		restoreCursorPosition();
		return(true);
	}
	else
	{
		_columns = 80;
		_lines = 24;
		restoreCursorPosition();
		return(false);
	}
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::columns()
#else
uint8_t retroTerm::columns()
#endif
{
	return(_columns);
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::lines()
#else
uint8_t retroTerm::lines()
#endif
{
	return(_lines);
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::probeType()	//Warning, this is blocking for up to 3s so do it sparingly
#else
bool retroTerm::probeType()	//Warning, this is blocking for up to 3s so do it sparingly
#endif
{
	//_terminalStream->print(F("\033[c"));
	_terminalStream->print(F("\033Z"));
	_terminalTypeReceived = false;
	for(uint8_t attempt = 0; attempt < 3;attempt++)
	{
		uint32_t timeout = millis();
		while(_terminalTypeReceived == false && millis() - timeout < 1000ul)
		{
			_readInput();
		}
		if(_terminalTypeReceived)
		{
			return(true);
		}
	}
	return(false);
}
#if defined(ESP8266) || defined(ESP32)
char * ICACHE_FLASH_ATTR retroTerm::type()
#else
char * retroTerm::type()
#endif
{
	return(_terminalType);
}

//Terminal 'bell'
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::enableBell()
#else
void retroTerm::enableBell()
#endif
{
	_bellEnabled = true;
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::disableBell()
#else
void retroTerm::disableBell()
#endif
{
	_bellEnabled = false;
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::soundBell()
#else
void retroTerm::soundBell()
#endif
{
	if(_bellEnabled)
	{
		_terminalStream->print(char(7));
	}
}

//Cursor control
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::moveCursorTo(uint8_t x, uint8_t y)
#else
void retroTerm::moveCursorTo(uint8_t x, uint8_t y)
#endif
{
	if(y == _cursorY)
	{
		//Move just X
		_terminalStream->write(0x1B);		//Send the VT Escape sequence
		_terminalStream->write(0x5B);		//Send the VT Escape sequence
		_terminalStream->print(x, DEC);
		_terminalStream->write('G');
		
	}
	if(x == _cursorX)
	{
		//Move just Y
		_terminalStream->write(0x1B);		//Send the VT Escape sequence
		_terminalStream->write(0x5B);		//Send the VT Escape sequence
		_terminalStream->print(y, DEC);
		_terminalStream->write('d');
		
	}
	else
	{
		//Move both X and Y
		_terminalStream->write(0x1B);		//Send the VT Escape sequence
		_terminalStream->write(0x5B);		//Send the VT Escape sequence
		_terminalStream->print(y,DEC);
		_terminalStream->write(';');
		_terminalStream->print(x, DEC);
		_terminalStream->write('H');
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::hideCursor()
#else
void retroTerm::hideCursor()
#endif
{
	//if(_cursorVisible == true)
	{
		_terminalStream->print(F("\033[?25l"));
		_cursorVisible = false;
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::showCursor()
#else
void retroTerm::showCursor()
#endif
{
	//if(_cursorVisible == false)
	{
		_terminalStream->print(F("\033[?25h"));
		_cursorVisible = true;
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::saveCursorPosition()
#else
void retroTerm::saveCursorPosition()
#endif
{
	_savedAttributes = _currentAttributes;
	_savedCursorX = _cursorX;
	_savedCursorY = _cursorY;
	_savedCursorState = _cursorVisible;
	_saved256colourSet = _256colourSet;
	_saved256colour = _256colour;
	_terminalStream->write(0x1B);		//Send the VT Escape sequence
	_terminalStream->print('7');
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::restoreCursorPosition()
#else
void retroTerm::restoreCursorPosition()
#endif
{
	_currentAttributes = _savedAttributes;
	_cursorX = _savedCursorX;
	_cursorY = _savedCursorY;
	//_cursorVisible = _savedCursorState;
	if(_savedCursorState)	//The terminal does not save the visibiity of the cursor correctly
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
	_256colourSet = _saved256colourSet;
	_256colour = _saved256colour;
	_terminalStream->write(0x1B);		//Send the VT Escape sequence
	_terminalStream->print('8');
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::requestCursorPosition()
#else
void retroTerm::requestCursorPosition()
#endif
{
	_terminalStream->print(F("\033[6n"));
	_cursorPositionReceived = false;
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::currentCursorColumn()		//Current cursor position
#else
uint8_t retroTerm::currentCursorColumn()		//Current cursor position
#endif
{
	return(_cursorX);
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::currentCursorRow()			//Current cursor position
#else
uint8_t retroTerm::currentCursorRow()			//Current cursor position
#endif
{
	return(_cursorY);
}

//Screen/line control
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::reset()
#else
void retroTerm::reset()
#endif
{
	_terminalStream->write(0x1B);				//Send the VT Escape sequence
	_terminalStream->print('c');				//Reset the terminal
	_terminalStream->print(F("\033[64\"p"));	//Set VT4xx mode
	_currentAttributes = _defaultAttributes;
	attributes(_currentAttributes);
	if(_mouseStatus & 0x01)
	{
		_mouseStatus = _mouseStatus & 0xFE;		//Force a resend of the mouse enable sequence
		enableMouse();
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::eraseScreen()
#else
void retroTerm::eraseScreen()
#endif
{
	_terminalStream->print(F("\033[2J"));
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::eraseLine()
#else
void retroTerm::eraseLine()
#endif
{
	_terminalStream->print(F("\033[2K"));
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::insertLine()
#else
void retroTerm::insertLine()
#endif
{
	_terminalStream->print(F("\033[001L"));
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::deleteLine()
#else
void retroTerm::deleteLine()
#endif
{
	_terminalStream->print(F("\033[001M"));
}

//Get/set printing attributes
#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::attributes()
#else
uint16_t retroTerm::attributes()
#endif
{
	return(_currentAttributes);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::attributes(const uint16_t attributes)
#else
void retroTerm::attributes(const uint16_t attributes)
#endif
{
	if(attributes != _currentAttributes)
	{
		//Clear attributes before specifically setting each flag
		_terminalStream->print(F("\033[0m"));
		//Foreground colours and brightness
		if(attributes & COLOUR_BLACK)	//A foreground colour has been sent, due to the marker bit in the constant
		{
			_terminalStream->print(F("\033["));
			_terminalStream->print((attributes & 0x0007) + 30,DEC);
			if(attributes & ATTRIBUTE_BRIGHT)
			{
				_terminalStream->print(F(";1"));
			}
			_terminalStream->print('m');
		}
		//Background colours
		if(attributes & BACKGROUND_COLOUR_BLACK)	//A background colour has been sent, due to the marker bit in the constant
		{
			_terminalStream->print(F("\033["));
			_terminalStream->print(((attributes & 0x0070)>>4) + 40,DEC);
			_terminalStream->print('m');
		}
		//Bold
		if(attributes & ATTRIBUTE_BOLD)
		{
			_terminalStream->print(F("\033[1m"));
		}
		//Faint
		if(attributes & ATTRIBUTE_FAINT)
		{
			_terminalStream->print(F("\033[2m"));
		}
		//Underline
		if(attributes & ATTRIBUTE_UNDERLINE)
		{
			_terminalStream->print(F("\033[4m"));
		}
		//Blink
		if(attributes & ATTRIBUTE_BLINK)
		{
			_terminalStream->print(F("\033[5m"));
		}
		//Inverse
		if(attributes & ATTRIBUTE_INVERSE)
		{
			_terminalStream->print(F("\033[7m"));
		}
		//Double Width
		if(attributes & ATTRIBUTE_DOUBLE_WIDTH)
		{
			_terminalStream->print(F("\033#6"));
		}
		//Double Height and Width needs kludgey extra handling, can't simply be set and forgotten
		_currentAttributes = attributes;
	}
}
#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::defaultAttributes()
#else
uint16_t retroTerm::defaultAttributes()
#endif
{
	return(_defaultAttributes);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::defaultAttributes(const uint16_t attributes)
#else
void retroTerm::defaultAttributes(const uint16_t attributes)
#endif
{
	_defaultAttributes = attributes;
}
#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::defaultWidgetAttributes()
#else
uint16_t retroTerm::defaultWidgetAttributes()
#endif
{
	return(_defaultWidgetAttributes);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::defaultWidgetAttributes(const uint16_t attributes)
#else
void retroTerm::defaultWidgetAttributes(const uint16_t attributes)
#endif
{
	_defaultWidgetAttributes = attributes;
}

#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::defaultLabelAttributes()
#else
uint16_t retroTerm::defaultLabelAttributes()
#endif
{
	return(_defaultLabelAttributes);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::defaultLabelAttributes(const uint16_t attributes)
#else
void retroTerm::defaultLabelAttributes(const uint16_t attributes)
#endif
{
	_defaultLabelAttributes = attributes;
}
#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::defaultContentAttributes()
#else
uint16_t retroTerm::defaultContentAttributes()
#endif
{
	return(_defaultContentAttributes);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::defaultContentAttributes(const uint16_t attributes)
#else
void retroTerm::defaultContentAttributes(const uint16_t attributes)
#endif
{
	_defaultContentAttributes = attributes;
}


#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::resetAttributes()
#else
void retroTerm::resetAttributes()
#endif
{
	_terminalStream->print(F("\033[0m"));
	_currentAttributes = _defaultAttributes;
	attributes(_currentAttributes);
}

#if defined(ESP8266) || defined(ESP32)
uint16_t ICACHE_FLASH_ATTR retroTerm::defaultWidgetStyle()
#else
uint16_t retroTerm::defaultWidgetStyle()
#endif
{
	return(_defaultStyle);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::defaultWidgetStyle(uint16_t style)
#else
void retroTerm::defaultWidgetStyle(uint16_t style)
#endif
{
	_defaultStyle = style;
}


//Foreground colour in 256-colour mode
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::foregroundColor(uint8_t newColour)
#else
void retroTerm::foregroundColor(uint8_t newColour)
#endif
{
	foregroundColour(newColour);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::foregroundColour(uint8_t newColour)
#else
void retroTerm::foregroundColour(uint8_t newColour)
#endif
{
	if(_256colourSet && newColour != _256colour)
	{
		_terminalStream->print(F("\033[38;5;"));
		_terminalStream->print(newColour,DEC);
		_terminalStream->print('m');
		_256colour = newColour;
	}
	else if(_256colourSet != true)
	{
		_terminalStream->print(F("\033[38;5;"));
		_terminalStream->print(newColour,DEC);
		_terminalStream->print('m');
		_256colour = newColour;
		_256colourSet = true;
		_currentAttributes = _currentAttributes & 0xfff0;	//Strip the foreground colour from the current attributes so it is not applied when printing
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::clearForegroundColour()
#else
void retroTerm::clearForegroundColour()
#endif
{
	_256colourSet = false;
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::clearForegroundColor()
#else
void retroTerm::clearForegroundColor()
#endif
{
	clearForegroundColour();
}

//24-bit colour
//printf("\033[48;2;RED;GREEN;BLUEm ",r,g,b);

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_printDouble(char *content)
#else
void retroTerm::_printDouble(char *content)
#endif
{
	_terminalStream->print(F("\033#3"));					//Set double size top
	_terminalStream->print(content);						//Print
	_terminalStream->print(F("\033[1B"));					//Move down 1 line
	_terminalStream->write(0x1B);							//Send the VT Escape sequence
	_terminalStream->write(0x5B);							//Send the VT Escape sequence
	_terminalStream->print(strlen(content),DEC);			//Move left the length of the printed item
	_terminalStream->print('D');							//Move left the length of the printed item
	_terminalStream->print(F("\033#4"));					//Set double size bottom
	_terminalStream->print(content);						//Print
	_terminalStream->print(F("\033[1B"));					//Move down 1 line
	_terminalStream->print(F("\033#5"));					//Set normal size
}


#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_printDouble(String content)
#else
void retroTerm::_printDouble(String content)
#endif
{
	_terminalStream->print(F("\033#3"));					//Set double size top
	_terminalStream->print(content);						//Print
	_terminalStream->print(F("\033[1B"));					//Move down 1 line
	_terminalStream->write(0x1B);							//Send the VT Escape sequence
	_terminalStream->write(0x5B);							//Send the VT Escape sequence
	_terminalStream->print(content.length(),DEC);			//Move left the length of the printed item
	_terminalStream->print('D');							//Move left the length of the printed item
	_terminalStream->print(F("\033#4"));					//Set double size bottom
	_terminalStream->print(content);						//Print
	_terminalStream->print(F("\033[1B"));					//Move down 1 line
	_terminalStream->print(F("\033#5"));					//Set normal size
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)														//x y w h
#else
void retroTerm::drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)														//x y w h
#endif
{
	_drawBoxLines(x, y, w, h, false, false, 0ul, 0ul, _currentAttributes, _currentStyle);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint16_t specificAttributes)							//x y w h attributes
#else
void retroTerm::drawBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint16_t specificAttributes)							//x y w h attributes
#endif
{
	_drawBoxLines(x, y, w, h, false, false, 0ul, 0ul, specificAttributes, _currentStyle);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint16_t specificAttributes, uint8_t specificStyle)	//x y w h attributes style
#else
void retroTerm::drawBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint16_t specificAttributes, uint8_t specificStyle)	//x y w h attributes style
#endif
{
	_drawBoxLines(x, y, w, h, false, false, 0ul, 0ul, specificAttributes, specificStyle);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint32_t currentPosition, uint32_t totalLength)		//x y w h scrollbarPosition scrollbarLength
#else
void retroTerm::drawBoxWithScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint32_t currentPosition, uint32_t totalLength)		//x y w h scrollbarPosition scrollbarLength
#endif
{
	_drawBoxLines(x, y, w, h, false, true, currentPosition, totalLength, _currentAttributes, _currentAttributes);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes)					//x y w h scrollbarPosition scrollbarLength attributes
#else
void retroTerm::drawBoxWithScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes)					//x y w h scrollbarPosition scrollbarLength attributes
#endif
{
	_drawBoxLines(x, y, w, h, false, true, currentPosition, totalLength, specificAttributes, _currentAttributes);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes, uint8_t specificStyle)			//x y w h scrollbarPosition scrollbarLength attributes style
#else
void retroTerm::drawBoxWithScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes, uint8_t specificStyle)			//x y w h scrollbarPosition scrollbarLength attributes style
#endif
{
	_drawBoxLines(x, y, w, h, false, true, currentPosition, totalLength, specificAttributes, specificStyle);
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label)																										//x y w h
#else
void retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label)																										//x y w h
#endif
{
	_drawBoxLines(x, y, w, h, true, false, 0ul, 0ul, _currentAttributes, _currentStyle);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint16_t specificAttributes)																			//x y w h attributes
#else
void retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint16_t specificAttributes)																			//x y w h attributes
#endif
{
	_drawBoxLines(x, y, w, h, true, false, 0ul, 0ul, specificAttributes, _currentStyle);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint16_t specificAttributes, uint8_t specificStyle)														//x y w h attributes style
#else
void retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint16_t specificAttributes, uint8_t specificStyle)														//x y w h attributes style
#endif
{
	_drawBoxLines(x, y, w, h, true, false, 0ul, 0ul, specificAttributes, specificStyle);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint32_t currentPosition, uint32_t totalLength)														//x y w h scrollbarPosition scrollbarLength
#else
void retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint32_t currentPosition, uint32_t totalLength)														//x y w h scrollbarPosition scrollbarLength
#endif
{
	_drawBoxLines(x, y, w, h, true, true, currentPosition, totalLength, _currentAttributes, _currentAttributes);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes)							//x y w h scrollbarPosition scrollbarLength attributes
#else
void retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes)							//x y w h scrollbarPosition scrollbarLength attributes
#endif
{
	_drawBoxLines(x, y, w, h, true, true, currentPosition, totalLength, specificAttributes, _currentAttributes);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes, uint8_t specificStyle)	//x y w h scrollbarPosition scrollbarLength attributes style
#else
void retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const char *label, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes, uint8_t specificStyle)	//x y w h scrollbarPosition scrollbarLength attributes style
#endif
{
	_drawBoxLines(x, y, w, h, true, true, currentPosition, totalLength, specificAttributes, specificStyle);
	printAt(x + 1,y + 1, label);
}
//PROGMEM variants
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)																										//x y w h
#else
void retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)																										//x y w h
#endif
{
	_drawBoxLines(x, y, w, h, true, false, 0ul, 0ul, _currentAttributes, _currentStyle);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint16_t specificAttributes)																			//x y w h attributes
#else
void retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint16_t specificAttributes)																			//x y w h attributes
#endif
{
	_drawBoxLines(x, y, w, h, true, false, 0ul, 0ul, specificAttributes, _currentStyle);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint16_t specificAttributes, uint8_t specificStyle)														//x y w h attributes style
#else
void retroTerm::drawBoxWithTitle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint16_t specificAttributes, uint8_t specificStyle)														//x y w h attributes style
#endif
{
	_drawBoxLines(x, y, w, h, true, false, 0ul, 0ul, specificAttributes, specificStyle);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint32_t currentPosition, uint32_t totalLength)														//x y w h scrollbarPosition scrollbarLength
#else
void retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint32_t currentPosition, uint32_t totalLength)														//x y w h scrollbarPosition scrollbarLength
#endif
{
	_drawBoxLines(x, y, w, h, true, true, currentPosition, totalLength, _currentAttributes, _currentAttributes);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes)							//x y w h scrollbarPosition scrollbarLength attributes
#else
void retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes)							//x y w h scrollbarPosition scrollbarLength attributes
#endif
{
	_drawBoxLines(x, y, w, h, true, true, currentPosition, totalLength, specificAttributes, _currentAttributes);
	printAt(x + 1,y + 1, label);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes, uint8_t specificStyle)	//x y w h scrollbarPosition scrollbarLength attributes style
#else
void retroTerm::drawBoxWithTitleAndScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, uint32_t currentPosition, uint32_t totalLength, uint16_t specificAttributes, uint8_t specificStyle)	//x y w h scrollbarPosition scrollbarLength attributes style
#endif
{
	_drawBoxLines(x, y, w, h, true, true, currentPosition, totalLength, specificAttributes, specificStyle);
	printAt(x + 1,y + 1, label);
}



#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::clearWidget(uint8_t widgetId)						//Widget ID
#else
void retroTerm::clearWidget(uint8_t widgetId)										//Widget ID
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		clearBox(_widgets[widgetId].x, _widgets[widgetId].y, _widgets[widgetId].w, _widgets[widgetId].h,_widgets[widgetId].attributes);	//Clear the space for the widget
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::clearBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)														//x y w h
#else
void retroTerm::clearBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)														//x y w h
#endif
{
	clearBox(x, y, w, h, _currentAttributes);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::clearBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint16_t specifiedAttributes)						//x y w h
#else
void retroTerm::clearBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, uint16_t specifiedAttributes)						//x y w h
#endif
{
	saveCursorPosition();				//Save the cursor and attributes for later
	hideCursor();						//Hide the cursor for all the drawing to come
	attributes(specifiedAttributes);	//Set the attributes for the box
	for(uint8_t row = 0 ; row < h; row++)
	{
		moveCursorTo(x,y + row);
		for(uint8_t column = 0 ; column < w; column++)
		{
			_terminalStream->print(' ');
		}
	}
	restoreCursorPosition();
	/*saveCursorPosition();				//Save the cursor and attributes for later
	attributes(specifiedAttributes);	//Set the attributes for the box
	_terminalStream->print(F("\033"));
	_terminalStream->print(y,DEC);
	_terminalStream->print(';');
	_terminalStream->print(x,DEC);
	_terminalStream->print(';');
	_terminalStream->print(y + h -1,DEC);
	_terminalStream->print(';');
	_terminalStream->print(x + w -1,DEC);
	_terminalStream->print(F("$z"));
	restoreCursorPosition();*/
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_printUnicodeCharacter(uint8_t row, uint8_t col)
#else
void retroTerm::_printUnicodeCharacter(uint8_t row, uint8_t col)
#endif
{
	#if defined(__AVR__)
	_printProgStr((const char *) pgm_read_word (&retroTermCharacterTable[col + row * 19]));
	#elif defined(ESP8266) || defined(ESP32)
	_terminalStream->print(retroTermCharacterTable[col + row * 19]);
	#else
	_terminalStream->print(retroTermCharacterTable[row][col]);
	#endif
}

#if defined(__AVR__)
// Print a string from Program Memory directly to save RAM 
// taken from http://www.gammon.com.au/progmem
void retroTerm::_printProgStr (const char * str)
{
  char c;
  if (!str) 
    return;
  while ((c = pgm_read_byte(str++)))
    _terminalStream->print(c);
}
#endif

//Draw a scrollbar for a box
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_drawScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, bool titleBar, uint32_t currentPosition, uint32_t totalLength, uint16_t specifiedAttributes, uint8_t specifiedStyle)
#else
void retroTerm::_drawScrollbar(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, bool titleBar, uint32_t currentPosition, uint32_t totalLength, uint16_t specifiedAttributes, uint8_t specifiedStyle)
#endif
{
	float scrollbarPosition = 0;
	uint8_t startingLine;
	uint8_t endingLine;
	if(titleBar)
	{
		startingLine = 3;
	}
	else
	{
		if(specifiedStyle & OUTER_BOX)
		{
			startingLine = 1;
		}
		else
		{
			startingLine = 0;
		}
	}
	if(specifiedStyle & OUTER_BOX)
	{
		endingLine = h - 2;
	}
	else
	{
		endingLine = h - 1;
	}
	if(totalLength > 0)
	{
		//Scroll bar position marker
		scrollbarPosition = startingLine + 1 + ((endingLine - (startingLine + 1)) * currentPosition / totalLength);
	}
	saveCursorPosition();				//Save the cursor and attributes for later
	hideCursor();						//Hide the cursor for all the drawing to come
	moveCursorTo(x,y);					//Move to the top left corner
	attributes(specifiedAttributes);	//Set the attributes for the box
	//Sides
	for(uint8_t row = startingLine ; row <= endingLine; row++)
	{
		//Right side
		moveCursorTo(x+w-1,y+row);
		if(row == startingLine)
		{
			//Up arrow
			_printUnicodeCharacter(specifiedStyle & 0x03, 8);
		}
		else if(row == endingLine)
		{
			//Down arrow
			_printUnicodeCharacter(specifiedStyle & 0x03, 9);
		}
		else if(row == scrollbarPosition)
		{
			//Marker block
			_printUnicodeCharacter(specifiedStyle & 0x03, 14);
		}
		else
		{
			//Side
			_printUnicodeCharacter(specifiedStyle & 0x03, 3);
		}
	}
	restoreCursorPosition();
}

//Draw a box with optional title bar or scrollbar
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_drawBoxLines(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, bool titleBar, bool scrollbar, uint32_t currentPosition, uint32_t totalLength, uint16_t specifiedAttributes, uint8_t specifiedStyle)
#else
void retroTerm::_drawBoxLines(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, bool titleBar, bool scrollbar, uint32_t currentPosition, uint32_t totalLength, uint16_t specifiedAttributes, uint8_t specifiedStyle)
#endif
{
	float scrollbarPosition = 0;
	uint8_t startingRow;
	if(titleBar)
	{
		startingRow = 3;
	}
	else
	{
		startingRow = 1;
	}
	if(scrollbar == true && totalLength > 0)
	{
		//Scroll bar position marker
		scrollbarPosition = startingRow + 1 + ((h - 3 - startingRow) * currentPosition / totalLength);
		if(scrollbarPosition > startingRow + h - 6)
		{
			scrollbarPosition = startingRow + h - 6;
		}
	}
	saveCursorPosition();				//Save the cursor and attributes for later
	hideCursor();						//Hide the cursor for all the drawing to come
	moveCursorTo(x,y);					//Move to the top left corner
	attributes(specifiedAttributes);	//Set the attributes for the box
	//Top left corner
	_printUnicodeCharacter(specifiedStyle & 0x03, 0);
	//Top
	for(uint8_t column = 1 ; column < w-1; column++)
	{
		_printUnicodeCharacter(specifiedStyle & 0x03, 1);
	}
	//Top right corner
	_printUnicodeCharacter(specifiedStyle & 0x03, 2);
	//Bottom left corner
	moveCursorTo(x,y+h-1);
	_printUnicodeCharacter(specifiedStyle & 0x03, 4);
	//Bottom
	for(uint8_t column = 1 ; column < w-1; column++)
	{
		_printUnicodeCharacter(specifiedStyle & 0x03, 1);
	}
	//Bottom right corner
	_printUnicodeCharacter(specifiedStyle & 0x03, 5);
	if(titleBar)
	{
		//Left side, title bar
		moveCursorTo(x,y+1);
		_printUnicodeCharacter(specifiedStyle & 0x03, 3);
		//Right side, title bar
		moveCursorTo(x+w-1,y+1);
		_printUnicodeCharacter(specifiedStyle & 0x03, 3);
		//Left side
		moveCursorTo(x,y+2);
		_printUnicodeCharacter(specifiedStyle & 0x03, 6);
		//Bar separator
		for(uint8_t column = 1 ; column < w-1; column++)
		{
			_printUnicodeCharacter(specifiedStyle & 0x03, 1);
		}
		//Right side
		_printUnicodeCharacter(specifiedStyle & 0x03, 7);
	}
	//Sides
	for(uint8_t row = startingRow ; row < h-1; row++)
	{
		//Left side
		moveCursorTo(x,y+row);
		_printUnicodeCharacter(specifiedStyle & 0x03, 3);
		//Right side
		moveCursorTo(x+w-1,y+row);
		if(scrollbar == true)
		{
			if(titleBar)
			{
				if(row == 3)
				{
					//Up arrow
					_printUnicodeCharacter(specifiedStyle & 0x03, 8);
				}
				else if(row == h - 2)
				{
					//Down arrow
					_printUnicodeCharacter(specifiedStyle & 0x03, 9);
				}
				else if(row == scrollbarPosition)
				{
					//Marker block
					_printUnicodeCharacter(specifiedStyle & 0x03, 14);
				}
				else if(row > 3 && row < h-2)
				{
					//Shaded block
					_printUnicodeCharacter(specifiedStyle & 0x03, 3);
				}
				else
				{
					//Side
					_printUnicodeCharacter(specifiedStyle & 0x03, 3);
				}
			}
			else
			{
				if(row == 1)
				{
					//Up arrow
					_printUnicodeCharacter(specifiedStyle & 0x03, 8);
				}
				else if(row == h - 2)
				{
					//Down arrow
					_printUnicodeCharacter(specifiedStyle & 0x03, 9);
				}
				else if(row == scrollbarPosition)
				{
					//Marker block
					_printUnicodeCharacter(specifiedStyle & 0x03, 14);
				}
				else if(row > 1 && row < h-2)
				{
					//Shaded block
					_printUnicodeCharacter(specifiedStyle & 0x03, 3);
				}
				else
				{
					//Side
					_printUnicodeCharacter(specifiedStyle & 0x03, 3);
				}
			}
		}
		else
		{
			//Side
			_printUnicodeCharacter(specifiedStyle & 0x03, 3);
		}
	}
	restoreCursorPosition();
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::centredTextBox(String textToPrint)
#else
void retroTerm::centredTextBox(String textToPrint)
#endif
{
	centredTextBox(textToPrint, _currentAttributes, _currentStyle);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::centredTextBox(String textToPrint, uint16_t specifiedAttributes)
#else
void retroTerm::centredTextBox(String textToPrint, uint16_t specifiedAttributes)
#endif
{
	centredTextBox(textToPrint, specifiedAttributes, _currentStyle);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::centredTextBox(String textToPrint, uint8_t specifiedStyle)
#else
void retroTerm::centredTextBox(String textToPrint, uint8_t specifiedStyle)
#endif
{
	centredTextBox(textToPrint, _currentAttributes, specifiedStyle);
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::centredTextBox(String textToPrint, uint16_t specifiedAttributes, uint8_t specifiedStyle)
#else
void retroTerm::centredTextBox(String textToPrint, uint16_t specifiedAttributes, uint8_t specifiedStyle)
#endif
{
	uint8_t y = _lines/2;
	uint8_t x = (_columns-textToPrint.length())/2;
	drawBox(x-1,y-1,textToPrint.length()+2,3,specifiedAttributes,specifiedStyle);
	printAt(x,y,textToPrint);
}


/*void ICACHE_FLASH_ATTR retroTerm::setOriginMode(uint8_t top,uint8_t bottom)
{
	terminalStream->printf("\033[?6h");
}
void ICACHE_FLASH_ATTR retroTerm::clearOriginMode(uint8_t top,uint8_t bottom)
{
	terminalStream->printf("\033[?6l");
}*/

//Scrolling window control
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::setScrollWindow(uint8_t top,uint8_t bottom)
#else
void retroTerm::setScrollWindow(uint8_t top,uint8_t bottom)
#endif
{
	_terminalStream->write(0x1B);							//Send the VT Escape sequence
	_terminalStream->write(0x5B);							//Send the [
	_terminalStream->print(top,DEC);
	_terminalStream->print(';');
	_terminalStream->print(bottom,DEC);
	_terminalStream->print('r');
	//_terminalStream->printf("\033[%03d;%03dr",top,bottom);
	_scrollWindowSet = true;
	_scrollWindowTop = top;
	_scrollWindowBottom = bottom;
}

/*#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::setScrollWindow(uint8_t x,uint8_t y, uint8_t w,uint8_t h)
#else
void retroTerm::setScrollWindow(uint8_t x,uint8_t y, uint8_t w,uint8_t h)
#endif
{
	_terminalStream->write(0x1B);							//Send the VT Escape sequence
	_terminalStream->write(0x5B);							//Send the [
	_terminalStream->print(y,DEC);							//Send the top row
	_terminalStream->print(';');
	_terminalStream->print(y + h - 1,DEC);					//Send the bottom row
	_terminalStream->print('r');

	_terminalStream->write(0x1B);							//Send the VT Escape sequence
	_terminalStream->write(0x5B);							//Send the [
	_terminalStream->print("?69h");							//Enable Left/Right margin mode

	_terminalStream->write(0x1B);							//Send the VT Escape sequence
	_terminalStream->write(0x5B);							//Send the [
	_terminalStream->print(x,DEC);							//Send the left margin
	_terminalStream->print(';');
	_terminalStream->print(x + w - 1,DEC);					//Send the right margin
	_terminalStream->print('s');

	_scrollWindowSet = true;
	_scrollWindowTop = y;
	_scrollWindowBottom = y + h - 1;
}*/


//Single key inputs
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::userIsTyping()
#else
bool retroTerm::userIsTyping()
#endif
{
	if(millis() - _lastInputActivity < _typingTimeout)
	{
		return(true);
	}
	return(false);
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::keyPressed()
#else
bool retroTerm::keyPressed()
#endif
{
	if(_lastKeypress != noKeyPressed)
	{
		return(true);
	}
	return(false);
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::readKeypress()
#else
uint8_t retroTerm::readKeypress()
#endif
{
	if(_lastKeypress != noKeyPressed)
	{
		uint8_t pressedKey = _lastKeypress;
		_lastKeypress = noKeyPressed;
		return(pressedKey);
	}
	else
	{
		return(noKeyPressed);
	}
}


#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_readInput()
#else
void retroTerm::_readInput()
#endif
{
	if(_escapeReceived == true && millis() - _escapeReceivedAt > 100ul)	//Potentially time out checking for escape character sequences
	{
		//We've timed out reading an escape sequence so assume it's a user keypress
		//if(strlen(_escapeBuffer) > 0)
		if(escapeBufferPosition > 0)
		{
			//Show what was sent
			_terminalStream->print(F("\r\fUnknown Control character ESC + "));
			//for(uint8_t bufferIndex = 0; bufferIndex < strlen(_escapeBuffer) ; bufferIndex++)
			for(uint8_t bufferIndex = 0; bufferIndex < escapeBufferPosition ; bufferIndex++)
			{
				if(_escapeBuffer[bufferIndex] > 31 && _escapeBuffer[bufferIndex] < 127)
				{
					_terminalStream->print(F("\r\fChar "));
					_terminalStream->print(bufferIndex, DEC);
					_terminalStream->print(' ');
					_terminalStream->print(char(_escapeBuffer[bufferIndex]));
					_terminalStream->print(F(" Int "));
					_terminalStream->print(_escapeBuffer[bufferIndex], DEC);
					
				}
				else
				{
					_terminalStream->print(F("\r\fChar "));
					_terminalStream->print(bufferIndex, DEC);
					_terminalStream->print(F(" ? Int "));
					_terminalStream->print(_escapeBuffer[bufferIndex], DEC);
				}
			}
			_resetEscapeBuffer();
		}
		else
		{
			_lastKeypress = escapePressed;
		}
		_resetEscapeBuffer();
	}
	while(_terminalStream->available())
	{
		//Update the inactivity timer
		_lastInputActivity = millis();
		uint8_t typedCharacter = _terminalStream->read();
		//if(_escapeReceived && strlen(_escapeBuffer) < 11)
		if(_escapeReceived && escapeBufferPosition < 24)
		{
			//_escapeBuffer[strlen(_escapeBuffer)] = char(typedCharacter);
			_escapeBuffer[escapeBufferPosition++] = char(typedCharacter);
			//Now see if there's an escape sequence we understand
			if(_escapeBuffer[0] == '?')
			{
				//Alt pressed
			}
			if(_escapeBuffer[0] == '[')
			{
				if(_escapeBuffer[1] == 'A')			//Up arrow
				{
					_resetEscapeBuffer();
					_lastKeypress = upPressed;
				}
				else if(_escapeBuffer[1] == 'B')		//Down arrow
				{
					_resetEscapeBuffer();
					_lastKeypress = downPressed;
				}
				else if(_escapeBuffer[1] == 'C')		//Right arrow
				{
					_resetEscapeBuffer();
					_lastKeypress = rightPressed;
				}
				else if(_escapeBuffer[1] == 'D')		//Left arrow
				{
					_resetEscapeBuffer();
					_lastKeypress = leftPressed;
				}
				else if(_escapeBuffer[1] == 'Z')		//Shift+tab / backtab
				{
					_resetEscapeBuffer();
					_lastKeypress = backTabPressed;
				}
				else if(_escapeBuffer[2] == '~')
				{
					if(_escapeBuffer[1] == '1')				//Home
					{
						_resetEscapeBuffer();
						_lastKeypress = homePressed;
					}
					else if(_escapeBuffer[1] == '2')		//Insert
					{
						_resetEscapeBuffer();
						_lastKeypress = insertPressed;
					}
					else if(_escapeBuffer[1] == '3')		//Delete
					{
						_resetEscapeBuffer();
						_lastKeypress = deletePressed;
					}
					else if(_escapeBuffer[1] == '4')		//End
					{
						_resetEscapeBuffer();
						_lastKeypress = endPressed;
					}
					else if(_escapeBuffer[1] == '5')		//Page Up
					{
						_resetEscapeBuffer();
						_lastKeypress = pageUpPressed;
					}
					else if(_escapeBuffer[1] == '6')		//Page Down
					{
						_resetEscapeBuffer();
						_lastKeypress = pageDownPressed;
					}
				}
				else if(_escapeBuffer[3] == '~')
				{
					if(_escapeBuffer[1] == '1')
					{
						if(_escapeBuffer[2] == '1')			//F1
						{
							_resetEscapeBuffer();
							_lastKeypress = f1Pressed;
						}
						else if(_escapeBuffer[2] == '2')	//F2
						{
							_resetEscapeBuffer();
							_lastKeypress = f2Pressed;
						}
						else if(_escapeBuffer[2] == '3')	//F3
						{
							_resetEscapeBuffer();
							_lastKeypress = f3Pressed;
						}
						else if(_escapeBuffer[2] == '4')	//F4
						{
							_resetEscapeBuffer();
							_lastKeypress = f4Pressed;
						}
						else if(_escapeBuffer[2] == '5')	//F5
						{
							_resetEscapeBuffer();
							_lastKeypress = f5Pressed;
						}
						else if(_escapeBuffer[2] == '7')	//F6
						{
							_resetEscapeBuffer();
							_lastKeypress = f6Pressed;
						}
						else if(_escapeBuffer[2] == '8')	//F7
						{
							_resetEscapeBuffer();
							_lastKeypress = f7Pressed;
						}
						else if(_escapeBuffer[2] == '9')	//F8
						{
							_resetEscapeBuffer();
							_lastKeypress = f8Pressed;
						}
					}
					else if(_escapeBuffer[1] == '2')
					{
						if(_escapeBuffer[2] == '0')			//F9
						{
							_resetEscapeBuffer();
							_lastKeypress = f9Pressed;
						}
						else if(_escapeBuffer[2] == '1')	//F10
						{
							_resetEscapeBuffer();
							_lastKeypress = f10Pressed;
						}
						else if(_escapeBuffer[2] == '3')	//F11
						{
							_resetEscapeBuffer();
							_lastKeypress = f11Pressed;
						}
						else if(_escapeBuffer[2] == '4')	//F12
						{
							_resetEscapeBuffer();
							_lastKeypress = f12Pressed;
						}
					}
				}
				//else if(_escapeBuffer[strlen(_escapeBuffer) - 1] == 'R')	//Cursor position response ESC [ pl;pc R
				else if(escapeBufferPosition > 4 && _escapeBuffer[escapeBufferPosition - 1] == 'R')	//Cursor position response ESC [ pl;pc R
				{
					uint8_t separatorPosition = 0;
					//while(_escapeBuffer[separatorPosition] != ';' && separatorPosition < strlen(_escapeBuffer))
					while(_escapeBuffer[separatorPosition] != ';' && separatorPosition < escapeBufferPosition)
					{
						separatorPosition++;
					}
					if(separatorPosition > 1 && separatorPosition < escapeBufferPosition - 1)
					{
						char *lineText = new char[separatorPosition];
						//char *columnText = new char[strlen(_escapeBuffer) - separatorPosition];
						char *columnText = new char[escapeBufferPosition - separatorPosition];
						memcpy(lineText, &_escapeBuffer[1], separatorPosition - 1);
						//memcpy(columnText, &_escapeBuffer[separatorPosition + 1], strlen(_escapeBuffer) - separatorPosition);
						memcpy(columnText, &_escapeBuffer[separatorPosition + 1], escapeBufferPosition - separatorPosition);
						_cursorX = atoi(columnText);
						_cursorY = atoi(lineText);
						delete[] lineText;
						delete[] columnText;
						_cursorPositionReceived = true;
					}
					_resetEscapeBuffer();
				}
				//else if(_escapeBuffer[strlen(_escapeBuffer) - 1] == 'c')	//Terminal type response ESC [ ? 1;0 c
				else if(_escapeBuffer[escapeBufferPosition - 1] == 'c')	//Terminal type response ESC [ ? 1;0 c
				{
					if(_terminalType != nullptr)
					{
						delete[] _terminalType;
					}
					_terminalType = new char[strlen(_escapeBuffer) - 1];
					memcpy(_terminalType,&_escapeBuffer[2],strlen(_escapeBuffer) - 3);
					_resetEscapeBuffer();
					_terminalTypeReceived = true;
				}
				//else if(_escapeBuffer[1] == 'M' && strlen(_escapeBuffer) == 5)
				else if(_escapeBuffer[1] == 'M' && escapeBufferPosition == 5)
				{
					if(_escapeBuffer[2] == ' ')	//Left mouse button down
					{
						if(_mouseStatus & 0x01)	//Only log mouse actions when it's enabled
						{
							_mouseX = uint8_t(_escapeBuffer[3]-32);
							_mouseY = uint8_t(_escapeBuffer[4]-32);
							_mouseStatus = _mouseStatus | 0x02;
						}
						_resetEscapeBuffer();
						_lastKeypress = noKeyPressed;
					}
					else if(_escapeBuffer[2] == '#')	//Left mouse button up
					{
						if(_mouseStatus & 0x01)	//Only log mouse actions when it's enabled
						{
							_mouseX = uint8_t(_escapeBuffer[3]-32);
							_mouseY = uint8_t(_escapeBuffer[4]-32);
							_mouseStatus = _mouseStatus | 0x04;
						}
						_resetEscapeBuffer();
						_lastKeypress = noKeyPressed;
					}
					else if(_escapeBuffer[2] == '!')	//Middle mouse button down
					{
						if(_mouseStatus & 0x01)	//Only log mouse actions when it's enabled
						{
							_mouseX = uint8_t(_escapeBuffer[3]-32);
							_mouseY = uint8_t(_escapeBuffer[4]-32);
							_mouseStatus = _mouseStatus | 0x08;
						}
						_resetEscapeBuffer();
						_lastKeypress = noKeyPressed;
					}
					else if(_escapeBuffer[2] == 'a')	//Mouse wheel roll up
					{
						if(_mouseStatus & 0x01)	//Only log mouse actions when it's enabled
						{
							_mouseX = uint8_t(_escapeBuffer[3]-32);
							_mouseY = uint8_t(_escapeBuffer[4]-32);
							_mouseStatus = _mouseStatus | 0x20;
						}
						_resetEscapeBuffer();
						_lastKeypress = noKeyPressed;
					}
					else if(_escapeBuffer[2] == '`')	//Mouse wheel roll down
					{
						if(_mouseStatus & 0x01)	//Only log mouse actions when it's enabled
						{
							_mouseX = uint8_t(_escapeBuffer[3]-32);
							_mouseY = uint8_t(_escapeBuffer[4]-32);
							_mouseStatus = _mouseStatus | 0x10;
						}
						_resetEscapeBuffer();
						_lastKeypress = noKeyPressed;
					}
					else if(_escapeBuffer[2] == '"')	//Mouse right button down
					{
						if(_mouseStatus & 0x01)	//Only log mouse actions when it's enabled
						{
							_mouseX = uint8_t(_escapeBuffer[3]-32);
							_mouseY = uint8_t(_escapeBuffer[4]-32);
							_mouseStatus = _mouseStatus | 0x40;
						}
						_resetEscapeBuffer();
						_lastKeypress = noKeyPressed;
					}
				}
			}
		}
		else
		{
			if(typedCharacter==127 || typedCharacter==8)		//Backspace
			{
				_lastKeypress = backspacePressed;
			}
			else if(typedCharacter==9)							//Tab
			{
				_lastKeypress = tabPressed;
			}
			else if(typedCharacter==26 || typedCharacter==0)	//Break
			{
				_lastKeypress = breakPressed;
			}
			else if(typedCharacter==10)							//An enter
			{
				_lastKeypress = enterPressed;
			}
			else if(typedCharacter==13)							//A return
			{
				_lastKeypress = returnPressed;
			}
			else if(typedCharacter==28)							//A file space (` + CTRL)
			{
				_lastKeypress = fsPressed;
			}
			else if(typedCharacter>31 && typedCharacter<127)	//Normal typing
			{
				_lastKeypress = typedCharacter;
			}
			else if(typedCharacter==27)							//Escape
			{
				//ESC - mostly used for control characters
				_escapeReceived = true;
				_escapeReceivedAt = millis();
			}
			else												//Unknown
			{
				//_terminalStream->print(F("Unknown character - "));
				//_terminalStream->print(typedCharacter, HEX);
				//_lastKeypress = unknownKeyPressed;
			}
		}
	}
}


#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::contentChanged(uint8_t widgetId)
#else
bool retroTerm::contentChanged(const uint8_t widgetId)
#endif
{
	if(_widgetExists(widgetId-1) && _widgets[widgetId - 1].value == 1)
	{
		_widgets[widgetId - 1].value = 0;	//Clear the flag
		return(true);
	}
	else
	{
		return(false);
	}
}

#if defined(ESP8266) || defined(ESP32)
char* ICACHE_FLASH_ATTR retroTerm::retrieveContent(const uint8_t widgetId)
#else
char* retroTerm::retrieveContent(uint8_t widgetId)
#endif
{
	if(_widgetExists(widgetId - 1))
	{
		return(_widgets[widgetId - 1].content);
	}
	else
	{
		return(nullptr);
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::clearContent(uint8_t widgetId)
#else
void retroTerm::clearContent(uint8_t widgetId)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		_widgets[widgetId].content[0] = 0;																	//Stick a null at the start of the buffer
		_widgets[widgetId].contentOffset = 0;																//Move back to the start of the buffer
		clearBox(_typingXposition(widgetId), _contentYorigin(widgetId), _typingBufferMaxLength(widgetId), 1, _widgets[widgetId].contentAttributes);	//Clear the screen region
		moveCursorTo(_typingXposition(widgetId),_contentYorigin(widgetId));								//Move the cursor to reflect this
	}
}


/*#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_resetEscapeBuffer()
#else
void retroTerm::_resetEscapeBuffer()
#endif
{
	_escapeBuffer[0] = 0x00;
	_escapeBuffer[1] = 0x00;
	_escapeBuffer[2] = 0x00;
	_escapeBuffer[3] = 0x00;
	_escapeBuffer[4] = 0x00;
	_escapeBuffer[5] = 0x00;
	_escapeBuffer[6] = 0x00;
	_escapeBuffer[7] = 0x00;
	_escapeBuffer[8] = 0x00;
	_escapeBuffer[9] = 0x00;
	_escapeBuffer[10] = 0x00;
	_escapeBuffer[11] = 0x00;
	_escapeReceived = false;
	_escapeReceivedAt = millis();
}*/

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_resetEscapeBuffer()
#else
void retroTerm::_resetEscapeBuffer()
#endif
{
	memset(_escapeBuffer, 0, 24);
	escapeBufferPosition = 0;
	_escapeReceived = false;
	_escapeReceivedAt = millis();
}

//Mouse support
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::enableMouse()
#else
void retroTerm::enableMouse()
#endif
{
	if((_mouseStatus & 0x01) == 0x00)
	{
		_terminalStream->print(F("\033[?1000h"));	//Enable mouse click reporting
		//_terminalStream->print(F("[?1001h"));	//Enable mouse movement reporting
		//_terminalStream->print(F("[?1002h"));	//Enable mouse movement reporting
		//_terminalStream->print(F("[?1003h"));	//Enable mouse movement reporting
		//_terminalStream->print(F("[?1005h"));
		//_terminalStream->print(F("[?1006h"));
		//_terminalStream->print(F("[?1015h"));
		//_terminalStream->print(F("[?9h"));
		_mouseStatus = _mouseStatus | 0x01;
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::disableMouse()
#else
void retroTerm::disableMouse()
#endif
{
	if(_mouseStatus & 0x01)
	{
		_terminalStream->print(F("\033[?1000l"));
		_mouseStatus = _mouseStatus & 0xFE;
	}
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::mouseColumn()
#else
uint8_t retroTerm::mouseColumn()
#endif
{
	return(_mouseX);
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::mouseRow()
#else
uint8_t retroTerm::mouseRow()
#endif
{
	return(_mouseY);
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::mouseButtonDown()
#else
bool retroTerm::mouseButtonDown()
#endif
{
	if(_mouseStatus & 0x02)
	{
		_mouseStatus = _mouseStatus & 0xFD;
		return(true);
	}
	return(false);
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::mouseButtonUp()
#else
bool retroTerm::mouseButtonUp()
#endif
{
	if(_mouseStatus & 0x04)
	{
		_mouseStatus = _mouseStatus & 0xFB;
		return(true);
	}
	return(false);
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::mouseWheelDown()
#else
bool retroTerm::mouseWheelDown()
#endif
{
	if(_mouseStatus & 0x10)
	{
		_mouseStatus = _mouseStatus & 0xEF;
		return(true);
	}
	return(false);
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::mouseWheelUp()
#else
bool retroTerm::mouseWheelUp()
#endif
{
	if(_mouseStatus & 0x20)
	{
		_mouseStatus = _mouseStatus & 0xDF;
		return(true);
	}
	return(false);
}


#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::widgetVisible(const uint8_t widgetId)		//Is this widget visible
#else
bool retroTerm::widgetVisible(const uint8_t widgetId)					//Is this widget visible
#endif
{
	if(_widgetExists(widgetId - 1) && _widgets[widgetId - 1].currentState & 0x0001)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::showWidget(uint8_t widgetId, bool newState)	//Set visibility of this widget
#else
void retroTerm::showWidget(uint8_t widgetId, bool newState)	//Set visibility of this widget
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId) && bool(_widgets[widgetId].currentState & 0x0001) != newState)
	{
		_widgets[widgetId].currentState = _widgets[widgetId].currentState ^ 0x0001;	//Change visibility
		/*if(_widgets[widgetId].shortcut != noKeyPressed) //Keep a track of the number of widgets with shortcuts, to cut down loop processing of inputs
		{
			if(_widgets[widgetId].currentState & 0x0001)	//This widget is visible
			{
				_numberOfWidgetShortcuts++;
			}
			else
			{
				_numberOfWidgetShortcuts--;
			}
		}*/
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::hideAllWidgets()	//Make all widgets invisible
#else
void retroTerm::hideAllWidgets()					//Make all widgets invisible
#endif
{
	for (uint8_t widgetIndex = 0 ; widgetIndex < _widgetObjectLimit ; widgetIndex++)
	{
		if(_widgetExists(widgetIndex))
		{
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState & 0xFFFE;	//Change visibility
		}
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::showAllWidgets()	//Make all widgets visible
#else
void retroTerm::showAllWidgets()					//Make all widgets visible
#endif
{
	for (uint8_t widgetIndex = 0 ; widgetIndex < _widgetObjectLimit ; widgetIndex++)
	{
		if(_widgetExists(widgetIndex))
		{
			_widgets[widgetIndex].currentState = _widgets[widgetIndex].currentState | 0x0001;	//Change visibility
		}
	}
}
/*#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::widgetActive(uint8_t widgetId)	//Is this widget active
#else
bool retroTerm::widgetActive(uint8_t widgetId)						//Is this widget active
#endif
{
	if(_widgetExists(widgetId - 1) && _widgets[widgetId - 1].currentState & 0x0100)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::widgetActive(uint8_t widgetId, bool newState)	//Control if this widget is active
#else
void retroTerm::widgetActive(uint8_t widgetId, bool newState)	//Control if this widget is active
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(bool(_widgets[widgetId].currentState & 0x0100) != newState)
	{
		_widgets[widgetId].currentState = _widgets[widgetId].currentState ^ 0x0100;	//Change active state
	}
	if(widgetId == _selectedWidget && _widgets[widgetId].type == _widgetTypes::textInput)
	{
		if(newState == false)
		{
			hideCursor();	//Hide cursor when an input field goes inactive
		}
		else
		{
			showCursor();
		}
	}
}*/

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::widgetActive(const uint8_t widgetId)	//Make this widget active
#else
void retroTerm::widgetActive(const uint8_t widgetId)	//Make this widget active
#endif
{
	if(_widgetExists(widgetId - 1))
	{
		_widgets[widgetId - 1].currentState = _widgets[widgetId - 1].currentState | 0x0100;	//Change active state
		if(widgetId == _selectedWidget && _widgets[widgetId - 1].type == _widgetTypes::textInput)
		{
			showCursor();
		}
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::widgetPassive(const uint8_t widgetId)		//Make this widget passive, ie. it cannot be clicked on
#else
void retroTerm::widgetPassive(const uint8_t widgetId)		//Make this widget passive, ie. it cannot be clicked on
#endif
{
	if(_widgetExists(widgetId - 1))
	{
		_widgets[widgetId - 1].currentState = _widgets[widgetId - 1].currentState & 0xFEFF;	//Mark widget as passive
		if(widgetId == _selectedWidget && _widgets[widgetId - 1].type == _widgetTypes::textInput)
		{
			hideCursor();
		}
	}
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::widgetClicked(uint8_t widgetId)					//Is this widget clicked, resets on read
#else
bool retroTerm::widgetClicked(uint8_t widgetId)					//Is this widget clicked, resets on read
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId) && _widgets[widgetId].currentState & 0x0200)
	{
		_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0xFDFF;
		return(true);
	}
	return(false);
}


#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::widgetAttributes(uint8_t widgetId, uint16_t newAttributes)
#else
void retroTerm::widgetAttributes(uint8_t widgetId, uint16_t newAttributes)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId) && _widgets[widgetId].attributes != newAttributes)
	{
		_widgets[widgetId].attributes = newAttributes;
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0004;		//Mark widget as changed
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::labelAttributes(uint8_t widgetId, uint16_t newAttributes)
#else
void retroTerm::labelAttributes(uint8_t widgetId, uint16_t newAttributes)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId) && _widgets[widgetId].labelAttributes != newAttributes)
	{
		_widgets[widgetId].labelAttributes = newAttributes;
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0008;	//Mark label as changed
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::contentAttributes(uint8_t widgetId, uint16_t newAttributes)
#else
void retroTerm::contentAttributes(uint8_t widgetId, uint16_t newAttributes)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgets[widgetId].contentAttributes != newAttributes)
	{
		_widgets[widgetId].contentAttributes = newAttributes;
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content changed
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::widgetStyle(uint8_t widgetId, uint8_t newStyle)
#else
void retroTerm::widgetStyle(uint8_t widgetId, uint8_t newStyle)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId) && _widgets[widgetId].style != newStyle)
	{
		_widgets[widgetId].style = newStyle;
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x001c;		//Mark widget as completely changed
	}
}


//Label methods
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::setWidgetLabel(uint8_t widgetId, char* label) //Add or change label for existing widget
#else
bool retroTerm::setWidgetLabel(uint8_t widgetId, char* label)
#endif
{
	deleteWidgetLabel(widgetId);	//Delete any existing label
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(label != nullptr)
		{
			_widgets[widgetId].label = new char[strlen(label) + 1];		//Assign some memory for the label
			memcpy(_widgets[widgetId].label, label, strlen(label) + 1);	//Copy in the label
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0008;	//Mark label as changed
		}
		else
		{
			_widgets[widgetId].label = nullptr;
		}
		return(true);
	}
	else
	{
		return(false);
	}
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::setWidgetLabel(uint8_t widgetId, String label) //Add or change label for existing widget
#else
bool retroTerm::setWidgetLabel(uint8_t widgetId, String label)
#endif
{
	deleteWidgetLabel(widgetId);	//Delete any existing label
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		_widgets[widgetId].label = new char[label.length() + 1];		//Assign some memory for the label
		memcpy(_widgets[widgetId].label, (label).c_str(), label.length());		//Copy in the label
		_widgets[widgetId].label[label.length()] = 0;
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0008;	//Mark label as changed
		return(true);
	}
	else
	{
		return(false);
	}
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::setWidgetLabel(uint8_t widgetId, const __FlashStringHelper* label) //Add or change label for existing widget
#else
bool retroTerm::setWidgetLabel(uint8_t widgetId, const __FlashStringHelper* label)
#endif
{
	deleteWidgetLabel(widgetId);	//Delete any existing label
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(label != nullptr)
		{
			_widgets[widgetId].label = (char *) label;									//Cast the PROGMEM as a char * to store it
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x4008;	//Mark label as changed and label stored in FLASH
			return(true);
		}
		else
		{
			return(false);
		}
	}
	else
	{
		return(false);
	}
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::deleteWidgetLabel(uint8_t widgetId)	//Delete a label, returns true if there was one to delete
#else
bool retroTerm::deleteWidgetLabel(uint8_t widgetId)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].currentState & 0x4000)				//label is stored in FLASH
		{
			_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0xBFFF; //Clear the 'label stored in flash' marker
			if(_widgets[widgetId].label == nullptr)
			{
				return(false);
			}
			else
			{
				_widgets[widgetId].label = nullptr;
				_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0008;	//Mark label as changed
				return(true);
			}
		}
		else	//Widget is stored in the heap, so release the memory if needed
		{
			if(_widgets[widgetId].label == nullptr)
			{
				return(false);
			}
			else
			{
				delete[] _widgets[widgetId].label;
				_widgets[widgetId].label = nullptr;
				_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0008;	//Mark label as changed
				return(true);
			}
		}
		
	}
	else
	{
		return(false);
	}
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::numberOfWidgets()	//How many widgets are defined, useful for choosing if to destroy things to save memory
#else
uint8_t retroTerm::numberOfWidgets()					//How many widgets are defined, useful for choosing if to destroy things to save memory
#endif
{
	uint8_t widgetCount = 0;
	for (uint8_t widgetIndex = 0 ; widgetIndex < _widgetObjectLimit ; widgetIndex++)
	{
		if(_widgetExists(widgetIndex))
		{
			widgetCount++;
		}
	}
	return(widgetCount);
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::maximumNumberOfWidgets()	//Maximum number of widgets
#else
uint8_t retroTerm::maximumNumberOfWidgets()					//Maximum number of widgets
#endif
{
	return(_widgetObjectLimit);
}


#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::setWidgetContent(uint8_t widgetId, char *newContent)
#else
bool retroTerm::setWidgetContent(uint8_t widgetId, char *newContent)
#endif
{
	deleteWidgetContent(widgetId);	//Delete content if it exists already, freeing heap as necessary
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::textInput)
		{
			if(strlen(newContent) > _typingBufferMaxLength(widgetId)) //Text inputs need to check the length
			{
				return(false);
			}
			else
			{
				_widgets[widgetId].content = new char[_typingBufferMaxLength(widgetId) + 1];		//Allocate the memory in heap
				memcpy(_widgets[widgetId].content, newContent, strlen(newContent) + 1);				//Copy in the content
				_widgets[widgetId].contentOffset = strlen(_widgets[widgetId].content);				//Place the cursor at the end of the content
				_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;			//Mark as content changed
				_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0x7fff;			//Mark as stored in heap
				_calculateContentLength(widgetId);													//Calculate the number of lines of content, for scrolling
				return(true);
			}
		}
		else if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			return(false);
		}
		else
		{
			_widgets[widgetId].content = new char[strlen(newContent) + 1];					//Allocate the memory in heap
			memcpy(_widgets[widgetId].content, newContent, strlen(newContent) + 1);			//Copy in the content
			_calculateContentLength(widgetId);												//Calculate the number of lines of content, for scrolling
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;		//Mark content as changed
			_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0x7fff;		//Mark as stored in heap
			return(true);
		}
	}
	else
	{
		return(false);
	}
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::setWidgetContent(uint8_t widgetId, String newContent)
#else
bool retroTerm::setWidgetContent(uint8_t widgetId, String newContent)
#endif
{
	deleteWidgetContent(widgetId);	//Delete content if it exists already, freeing heap as necessary
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::textInput) //Text inputs need to check the length
		{
			if(newContent.length() > _typingBufferMaxLength(widgetId))
			{
				return(false);
			}
			else
			{
				_widgets[widgetId].content = new char[_typingBufferMaxLength(widgetId) + 1];		//Allocate the memory in heap
				memcpy(_widgets[widgetId].content, (newContent).c_str(), newContent.length() + 1);	//Copy in the content
				_widgets[widgetId].contentOffset = strlen(_widgets[widgetId].content);				//Place the cursor at the end of the content
				_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;			//Mark as content changed
				_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0x7fff;			//Mark as stored in heap
				_calculateContentLength(widgetId);													//Calculate the number of lines of content, for scrolling
				return(true);
			}
		}
		else if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			return(false);
		}
		else
		{
			_widgets[widgetId].content = new char[newContent.length() + 1];						//Allocate the memory in heap
			memcpy(_widgets[widgetId].content, (newContent).c_str(), newContent.length() + 1);	//Copy in the content
			_calculateContentLength(widgetId);													//Calculate the number of lines of content, for scrolling
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;			//Mark content as changed
			_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0x7fff;			//Mark as stored in heap
			return(true);
		}
	}
	else
	{
		return(false);
	}
}
//AVR PROGMEM variants
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::setWidgetContent(uint8_t widgetId, const __FlashStringHelper* newContent)
#else
bool retroTerm::setWidgetContent(uint8_t widgetId, const __FlashStringHelper* newContent)
#endif
{
	deleteWidgetContent(widgetId);	//Delete content if it exists already, freeing heap as necessary
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::textInput)
		{
			if(strlen_P((PGM_P) newContent) > _typingBufferMaxLength(widgetId))
			{
				return(false);
			}
			else
			{
				_widgets[widgetId].content = new char[_typingBufferMaxLength(widgetId) + 1];		//Allocate the memory in heap
				memcpy_P(_widgets[widgetId].content, newContent, strlen_P((PGM_P) newContent) + 1);	//Copy in the content
				_widgets[widgetId].contentOffset = strlen(_widgets[widgetId].content);				//Place the cursor at the end of the content
				_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;			//Mark as content changed
				_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0x7fff;			//Mark as stored in heap
				_calculateContentLength(widgetId);													//Calculate the number of lines of content, for scrolling
				return(true);
			}
		}
		else if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			return(false);
		}
		else	//Most widgets just store the pointer to the content as the widget CANNOT change the content
		{
			_widgets[widgetId].content = (char *) newContent;										//Point at the copy in flash
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x8010;				//Mark as content changed AND that the content is stored in FLASH. Used on AVR implementation
			_calculateContentLength(widgetId);														//Calculate the number of lines of content, for scrolling
			return(true);
		}
	}
	else
	{
		return(false);
	}
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::setWidgetContent(uint8_t widgetId, const char* newContent)
#else
bool retroTerm::setWidgetContent(uint8_t widgetId, const char* newContent)
#endif
{
	deleteWidgetContent(widgetId);	//Delete content if it exists already, freeing heap as necessary
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::textInput)
		{
			if(strlen_P((PGM_P) newContent) > _typingBufferMaxLength(widgetId))
			{
				return(false);
			}
			else
			{
				_widgets[widgetId].content = new char[_typingBufferMaxLength(widgetId) + 1];		//Allocate the memory in heap
				memcpy_P(_widgets[widgetId].content, newContent, strlen_P((PGM_P) newContent) + 1);	//Copy in the content
				_widgets[widgetId].contentOffset = strlen(_widgets[widgetId].content);				//Move the cursor to the end
				_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;			//Mark as content changed
				_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0x7fff;			//Mark as stored in heap
				_calculateContentLength(widgetId);													//Calculate the number of lines of content, for scrolling
				return(true);
			}
		}
		else if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			return(false);
		}
		else	//Most widgets just store the pointer to the content as the widget CANNOT change the content
		{
			_widgets[widgetId].content = (char *) newContent;										//Point at the copy in flash
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x8010;				//Mark as content changed AND that the content is stored in FLASH. Used on AVR implementation
			_calculateContentLength(widgetId);														//Calculate the number of lines of content, for scrolling
			return(true);
		}
	}
	else
	{
		return(false);
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::_calculateContentLength(uint8_t widgetIndex)	//Calculate content length, which varies by type of widget
#else
void retroTerm::_calculateContentLength(uint8_t widgetIndex)						//Calculate content length, which varies by type of widget
#endif
{
	if(_widgets[widgetIndex].type == _widgetTypes::listBox)						//Recalculate the content 'length' which is the number of lines/options and helps with scrolling
	{
		_widgets[widgetIndex].contentLength = _calculateNumberOfOptions(widgetIndex);	//Calculate the number of options
	}
	else if(_widgets[widgetIndex].type == _widgetTypes::staticTextDisplay)
	{
		_widgets[widgetIndex].contentLength = _calculateNumberOfLines(widgetIndex);		//Calculate the number of lines of FORMATTED text. This is time consuming!
	}
	else if(_columnsAvailable(widgetIndex) > 0)
	{
		_widgets[widgetIndex].contentLength = _contentSize(widgetIndex)/_columnsAvailable(widgetIndex);	//By default it's size/lines
	}
	else
	{
		_widgets[widgetIndex].contentLength = 0;
	}
}

//Append widget content, mostly for scrolling text windows

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::appendWidgetContent(uint8_t widgetId, char* newContent)		//Add/change widget content char array
#else
bool retroTerm::appendWidgetContent(uint8_t widgetId, char* newContent)		//Add/change widget content char array
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint8_t columns = _columnsAvailable(widgetId);
			uint8_t lines = _linesAvailable(widgetId);
			uint8_t newContentLength = strlen(newContent);
			memmove (_widgets[widgetId].content, _widgets[widgetId].content + columns, columns * (lines - 1) );		//Scroll the existing content up
			memcpy(_widgets[widgetId].content + columns * (lines - 1), newContent, strnlen(newContent,columns));	//Copy in the new content at the end, note strnlen to limit length that can be added
			if(newContentLength < columns)	//If necessary, pad the end of the line with spaces
			{
				memset(_widgets[widgetId].content + columns * (lines - 1) + strlen(newContent), ' ', columns - newContentLength);
			}
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed
			return(true);
		}
		else
		{
			return(false);
		}
	}
	return(false);
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::appendWidgetContent(uint8_t widgetId, String newContent)		//Add/change widget content String version
#else
bool retroTerm::appendWidgetContent(uint8_t widgetId, String newContent)		//Add/change widget content String version
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint8_t columns = _columnsAvailable(widgetId);
			uint8_t lines = _linesAvailable(widgetId);
			uint8_t newContentLength = newContent.length();
			if(newContentLength > columns)	//Limit the length of what can be added
			{
				newContentLength = columns;
			}
			memmove (_widgets[widgetId].content, _widgets[widgetId].content + columns, columns * (lines - 1) );	//Scroll the existing content up
			memcpy(_widgets[widgetId].content + columns * (lines - 1), (newContent).c_str(), newContentLength);			//Copy in the new content at the end
			if(newContentLength < columns)	//If necessary, pad the end of the line with spaces
			{
				memset(_widgets[widgetId].content + columns * (lines - 1) + newContentLength, ' ', columns - newContentLength);
			}
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed
			return(true);
		}
		else
		{
			return(false);
		}
	}
	return(false);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::appendWidgetContent(uint8_t widgetId, const char* newContent)	//Add/change widget content string literal version
#else
bool retroTerm::appendWidgetContent(uint8_t widgetId, const char* newContent)	//Add/change widget content string literal version
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint8_t columns = _columnsAvailable(widgetId);
			uint8_t lines = _linesAvailable(widgetId);
			uint8_t newContentLength = strlen(newContent);
			memmove (_widgets[widgetId].content, _widgets[widgetId].content + columns, columns * (lines - 1) );		//Scroll the existing content up
			memcpy(_widgets[widgetId].content + columns * (lines - 1), newContent, strnlen(newContent,columns));	//Copy in the new content at the end, note strnlen to limit length that can be added
			if(newContentLength < columns)	//If necessary, pad the end of the line with spaces
			{
				memset(_widgets[widgetId].content + columns * (lines - 1) + strlen(newContent), ' ', columns - newContentLength);
			}
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed
			return(true);
		}
		else
		{
			return(false);
		}
	}
	return(false);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::appendWidgetContent(uint8_t widgetId, const __FlashStringHelper* newContent)	//Add/change widget content PROGMEM version
#else
bool retroTerm::appendWidgetContent(uint8_t widgetId, const __FlashStringHelper* newContent)	//Add/change widget content PROGMEM version
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint8_t columns = _columnsAvailable(widgetId);
			uint8_t lines = _linesAvailable(widgetId);
			#if defined(__AVR__)
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#elif defined(ESP8266) || defined(ESP32)
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#elif defined(CORE_TEENSY)
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#elif defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_GENERIC_RP2040) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040)
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#else
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#endif
			if(newContentLength > columns)	//Limit the length of what can be added
			{
				newContentLength = columns;
			}
			memmove (_widgets[widgetId].content, _widgets[widgetId].content + columns, columns * (lines - 1) );	//Scroll the existing content up
			#if defined(__AVR__)
			memcpy_P(_widgets[widgetId].content + columns * (lines - 1), newContent, newContentLength);	//Copy the content to the widget, limited by the width
			#elif defined(ESP8266) || defined(ESP32)
			memcpy_P(_widgets[widgetId].content + columns * (lines - 1), newContent, newContentLength);	//Copy the content to the widget, limited by the width
			#else
			memcpy_P(_widgets[widgetId].content + columns * (lines - 1), newContent, newContentLength);	//Copy the content to the widget, limited by the width
			#endif
			if(newContentLength < columns)	//If necessary, pad the end of the line with spaces
			{
				memset(_widgets[widgetId].content + columns * (lines - 1) + newContentLength, ' ', columns - newContentLength);
			}
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed
			return(true);
		}
		else
		{
			return(false);
		}
	}
	//_terminalStream->print("__FlashStringHelper*");
	return(false);
}

//Prepend widget content, adding it to the top of a scrolling window

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::prependWidgetContent(uint8_t widgetId, char* newContent)		//Add/change widget content char array
#else
bool retroTerm::prependWidgetContent(uint8_t widgetId, char* newContent)		//Add/change widget content char array
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			_terminalStream->print("char*");
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed
			return(true);
		}
		else
		{
			return(false);
		}
	}
	return(false);
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::prependWidgetContent(uint8_t widgetId, String newContent)		//Add/change widget content String version
#else
bool retroTerm::prependWidgetContent(uint8_t widgetId, String newContent)		//Add/change widget content String version
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint8_t columns = _columnsAvailable(widgetId);
			uint8_t lines = _linesAvailable(widgetId);
			uint8_t newContentLength = newContent.length();
			if(newContentLength > columns)	//Limit the length of what can be added
			{
				newContentLength = columns;
			}
			memmove (_widgets[widgetId].content + columns, _widgets[widgetId].content , columns * (lines - 1) );	//Scroll the existing content up
			memcpy(_widgets[widgetId].content, &newContent, newContentLength);			//Copy in the new content at the end
			if(newContentLength < columns)
			{
				memset(_widgets[widgetId].content + newContentLength, ' ', columns - newContentLength);
			}
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed
			return(true);
		}
		else
		{
			return(false);
		}
	}
	return(false);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::prependWidgetContent(uint8_t widgetId, const char* newContent)	//Add/change widget content string literal version
#else
bool retroTerm::prependWidgetContent(uint8_t widgetId, const char* newContent)	//Add/change widget content string literal version
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint8_t columns = _columnsAvailable(widgetId);
			uint8_t lines = _linesAvailable(widgetId);
			uint8_t newContentLength = strlen(newContent);
			if(newContentLength > columns)	//Limit the length of what can be added
			{
				newContentLength = columns;
			}
			memmove (_widgets[widgetId].content + columns, _widgets[widgetId].content, columns * (lines - 1) );		//Scroll the existing content up
			memcpy(_widgets[widgetId].content, newContent, newContentLength);	//Copy in the new content at the end
			if(newContentLength < columns)
			{
				memset(_widgets[widgetId].content + strlen(newContent), ' ', columns - newContentLength);
			}
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed
			return(true);
		}
		else
		{
			return(false);
		}
	}
	return(false);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::prependWidgetContent(uint8_t widgetId, const __FlashStringHelper* newContent)	//Add/change widget content PROGMEM version
#else
bool retroTerm::prependWidgetContent(uint8_t widgetId, const __FlashStringHelper* newContent)	//Add/change widget content PROGMEM version
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint8_t columns = _columnsAvailable(widgetId);
			uint8_t lines = _linesAvailable(widgetId);
			#if defined(__AVR__)
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#elif defined(ESP8266) || defined(ESP32)
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#elif defined(CORE_TEENSY)
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#elif defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_GENERIC_RP2040) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040)
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#else
			uint8_t newContentLength = strlen_P((PGM_P)newContent);
			#endif
			if(newContentLength > columns)	//Limit the length of what can be added
			{
				newContentLength = columns;
			}
			memmove (_widgets[widgetId].content + columns, _widgets[widgetId].content, columns * (lines - 1) );	//Scroll the existing content up
			#if defined(__AVR__)
			memcpy_P(_widgets[widgetId].content, newContent, newContentLength);	//Copy the content to the widget, limited by the width
			#elif defined(ESP8266) || defined(ESP32)
			memcpy_P(_widgets[widgetId].content, newContent, newContentLength);	//Copy the content to the widget, limited by the width
			#else
			memcpy_P(_widgets[widgetId].content, newContent, newContentLength);	//Copy the content to the widget, limited by the width
			#endif
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed
			return(true);
		}
		else
		{
			return(false);
		}
	}
	//_terminalStream->print("__FlashStringHelper*");
	return(false);
}

//Delete widget content

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::deleteWidgetContent(uint8_t widgetId)
#else
bool retroTerm::deleteWidgetContent(uint8_t widgetId)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		_widgets[widgetId].contentOffset = 0;											//Reset the content offset
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			memset(_widgets[widgetId].content, ' ',_textCapacity(widgetId));			//Clear the scrolling content
		}
		else
		{
			if(_widgets[widgetId].currentState & 0x8000)									//Content is a pointer to flash
			{
				_widgets[widgetId].content = nullptr;										//Clear the pointer
				_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0x7FFF;	//Clear the flag showing content is stored in flash
			}
			else																			//Content is stored in heap
			{
				delete[] _widgets[widgetId].content;										//Free the space in heap
				_widgets[widgetId].content = nullptr;
			}
		}
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;			//Mark as content changed
		return(true);
	}
	else
	{
		return(false);
	}
}

#if defined(ESP8266) || defined(ESP32)
uint32_t ICACHE_FLASH_ATTR retroTerm::contentOffset(uint8_t widgetId)
#else
uint32_t retroTerm::contentOffset(uint8_t widgetId)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		return(_widgets[widgetId].contentOffset);	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	}
	else
	{
		return(0);
	}
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::contentOffset(uint8_t widgetId, uint32_t newContentOffset)
#else
bool retroTerm::contentOffset(uint8_t widgetId, uint32_t newContentOffset)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_widgets[widgetId].contentOffset != newContentOffset)
		{
			_widgets[widgetId].contentOffset = newContentOffset;
			_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed (it will have moved onscreen
			return(true);
		}
		else
		{
			return(false);
		}
	}
	else
	{
		return(false);
	}			
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::state(const uint8_t widgetId)	//What is the boolean state of this widget (used for checkboxes/radio buttons)
#else
bool retroTerm::state(const uint8_t widgetId)					//What is the boolean state of this widget (used for checkboxes/radio buttons)
#endif
{
	if(_widgetExists(widgetId - 1) && _widgets[widgetId - 1].value)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::state(uint8_t widgetId, bool newState)			//Set the boolean state of this widget (used for checkboxes/radio buttons)
#else
void retroTerm::state(uint8_t widgetId, bool newState)								//Set the boolean state of this widget (used for checkboxes/radio buttons)
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId) && bool(_widgets[widgetId].value) != newState)
	{
		_widgets[widgetId].currentState = not _widgets[widgetId].value;				//Change state
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0010;	//Mark content as changed
	}
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::widgetValue(const uint8_t widgetId)				//Return the current 'value' of the widget
#else
uint8_t retroTerm::widgetValue(const uint8_t widgetId)									//Return the current 'value' of the widget
#endif
{
	if(_widgetExists(widgetId - 1))
	{
		return(_widgets[widgetId - 1].value);
	}
	else
	{
		return(0);
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::widgetValue(const uint8_t widgetId, uint8_t newValue)	//Set the current 'value' of the widget
#else
void retroTerm::widgetValue(const uint8_t widgetId, uint8_t newValue)					//Set the current 'value' of the widget
#endif
{
	if(_widgets[widgetId - 1].value != newValue)
	{
		_widgets[widgetId - 1].value = newValue;
		_widgets[widgetId - 1].currentState = _widgets[widgetId - 1].currentState | 0x0010;	//Mark state as changed
	}
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::selectWidget(const uint8_t widgetId)				//Select a widget
#else
bool retroTerm::selectWidget(const uint8_t widgetId)									//Select a widget
#endif
{
	if(_widgetExists(widgetId - 1) && _selectedWidget != widgetId - 1)	//Only select if not already selected
	{
		if(_widgets[_selectedWidget].type == _widgetTypes::textInput &&
			_widgets[widgetId - 1].type != _widgetTypes::textInput)		//Current widget is a text input, new one isn't
		{
			hideCursor();												//Hide the cursor, otherwise it'll visibly jump away from the text input
		}
		else if(_widgets[_selectedWidget].type != _widgetTypes::textInput &&
			_widgets[widgetId - 1].type == _widgetTypes::textInput)		//Current widget is not a text input, new one is
		{
			moveCursorTo(_typingXposition(widgetId - 1),_typingYposition(widgetId - 1));	//Move back to the text input
			showCursor();																	//Show the cursor for typing
		}
		_selectedWidget = widgetId - 1;
		return(true);
	}
	else
	{
		return(false);
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::deselectWidget()				//Deselect all widgets
#else
void retroTerm::deselectWidget()								//Deselect all widgets
#endif
{
	if(_widgets[_selectedWidget].type == _widgetTypes::textInput)
	{
		hideCursor();
	}
	_selectedWidget = _widgetObjectLimit;
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::widgetShortcutKey(uint8_t widgetId, uint8_t shortcut)		//Set a keyboard shortcut on a widget
#else
void retroTerm::widgetShortcutKey(uint8_t widgetId, uint8_t shortcut)		//Set a keyboard shortcut on a widget
#endif
{
	widgetId--;	//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgets[widgetId].shortcut != shortcut)							
	{
		/*if(_widgets[widgetId].currentState & 0x0001 && _widgets[widgetId].shortcut == noKeyPressed) //Keep a track of the number of widgets with shortcuts, to cut down loop processing of inputs
		{
			_numberOfWidgetShortcuts++;
		}*/
		_widgets[widgetId].shortcut = shortcut;
		if(_widgets[widgetId].h == 1)
		{
			_widgets[widgetId].style = _widgets[widgetId].style | SHORTCUT_INLINE;	//Shortcuts on a height 1 widget are always inline
		}
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x0004;	//Mark the widget as changed
	}
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::widgetExists(const uint8_t widgetId)				//Does a particular widget exist?
#else
bool retroTerm::widgetExists(const uint8_t widgetId)								//Does a particular widget exist?
#endif
{
	return(_widgetExists(widgetId - 1));
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::_widgetExists(const uint8_t widgetIndex)				//Does a particular widget exist?
#else
bool retroTerm::_widgetExists(const uint8_t widgetIndex)								//Does a particular widget exist?
#endif
{
	#ifdef retroTerm_DYNAMIC_OBJECT_ALLOCATION
	if(_widgets[widgetIndex] == nullptr)
	{
		return(false);
	}
	else
	{
		return(true);
	}
	#else
	if(_widgets[widgetIndex].x == 0 && _widgets[widgetIndex].y == 0 && _widgets[widgetIndex].w == 0 && _widgets[widgetIndex].h == 0)
	{
		return(false);
	}
	else if(_numberOfWidgets > 0 && widgetIndex < _widgetObjectLimit)
	{
		return(true);
	}
	else
	{
		return(false);
	}
	#endif
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR retroTerm::deleteWidget(uint8_t widgetId)
#else
bool retroTerm::deleteWidget(uint8_t widgetId)
#endif
{
	widgetId--;																		//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId))
	{
		if(_selectedWidget == widgetId)													//Unselect widget if it was selected
		{
			_selectedWidget = _widgetObjectLimit;										//The widget ID just outside the acceptable range is considered 'none'
		}
		/*if(_widgets[widgetId].currentState & 0x0001 && _widgets[widgetId].shortcut != noKeyPressed) 	//Keep a track of the number of widgets with shortcuts, to cut down loop processing of inputs
		{
			_numberOfWidgetShortcuts--;
		}*/
		#ifdef retroTerm_DYNAMIC_OBJECT_ALLOCATION
		delete[] _widgets[widgetId];									//Delete the widget data
		#else
		_widgets[widgetId].x = 0;
		_widgets[widgetId].y = 0;
		_widgets[widgetId].w = 0;
		_widgets[widgetId].h = 0;
		_widgets[widgetId].value = 0;
		_widgets[widgetId].shortcut = noKeyPressed;						//Shortcut ket, start as nothing, causes a 'click' when used
		//Handle removal of the label from the widget
		if(_widgets[widgetId].label != nullptr)							//Delete the label, if it is set
		{
			if((_widgets[widgetId].currentState & 0x4000) != 0x4000)	//Widget label is stored in the heap, so release the memory
			{
				delete[] _widgets[widgetId].label;
			}
			_widgets[widgetId].label = nullptr;
		}
		//Handle removal of content from the widget
		if(_widgets[widgetId].content != nullptr)						//Delete the content, if it is set
		{
			if((_widgets[widgetId].currentState & 0x8000) != 0x8000)	//Widget content is stored in the heap, so release the memory
			{
				delete[] _widgets[widgetId].content;
			}
			_widgets[widgetId].content = nullptr;
			_widgets[widgetId].contentOffset = 0;						//Clear content offset
			_widgets[widgetId].contentLength = 0;						//Clear content length
		}
		/*if(_widgets[widgetId].type == _widgetTypes::staticTextDisplay)
		{
			_widgets[widgetId].content = nullptr;						//Remove address of any content
		}
		else if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			delete[] _widgets[widgetId].content;						//De-allocate scrolling buffer memory
			_widgets[widgetId].content = nullptr;						//Remove address of any content
		}
		else if(_widgets[widgetId].type == _widgetTypes::textInput)
		{
			delete[] _widgets[widgetId].content;						//De-allocate typing buffer memory
			_widgets[widgetId].content = nullptr;
		}*/
		_widgets[widgetId].currentState = 0x0000;						//Set to no usable status
		#endif
		_numberOfWidgets--;
		return(true);
	}
	else
	{
		return(false);
	}
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newWidget(_widgetTypes type, const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newWidget(_widgetTypes type, const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
#endif
{
	if(_numberOfWidgets < _widgetObjectLimit)
	{
		uint8_t widgetId = 0;
		#ifdef retroTerm_DYNAMIC_OBJECT_ALLOCATION
		while(&_widgets[widgetId] != nullptr) //Find the first free widget slot, they are not necessarily deleted in order of creation
		{
			widgetId++;
		}
		_widgets[widgetId] = new widget;
		#else
		while(_widgetExists(widgetId%_widgetObjectLimit)) //Find the first free widget slot
		{
			widgetId++;
		}
		#endif
		_widgets[widgetId].type = type;
		_widgets[widgetId].x = x;
		_widgets[widgetId].y = y;
		_widgets[widgetId].w = w;
		_widgets[widgetId].h = h;
		_widgets[widgetId].currentState = 0x011C;						//New widgets are considered changed and active
		_widgets[widgetId].value = 0;									//New widgets have a value of zero or false
		_widgets[widgetId].label = nullptr;								//There is no label yet
		_widgets[widgetId].attributes = attributes;						//Set the attributes
		_widgets[widgetId].labelAttributes = attributes;				//Set the label attributes the same as main ones
		_widgets[widgetId].contentAttributes = attributes;				//Set the label attributes the same as main ones
		_widgets[widgetId].style = style;								//Set the style
		_widgets[widgetId].shortcut = noKeyPressed;						//Delete the shortcut
		if(_widgets[widgetId].type == _widgetTypes::textInput)
		{
			_widgets[widgetId].content = new char[w-1];					//Allocate space for the typing buffer
			_widgets[widgetId].content[0] = 0;							//Null terminate the string
		}
		else if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint16_t bufferSize = _textCapacity(widgetId);
			_widgets[widgetId].content = new char[bufferSize];			//Allocate space for the scrolling text buffer
			memset(_widgets[widgetId].content, ' ',bufferSize);			//Fill this with spaces
		}
		_widgets[widgetId].contentOffset = 0;							//Clear content offset
		_numberOfWidgets++;
		return(widgetId + 1);
	}
	else
	{
		return(0);	//Return 0 indicates failure
	}
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newWidget(_widgetTypes type, const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newWidget(_widgetTypes type, const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#endif
{
	if(_numberOfWidgets < _widgetObjectLimit)
	{
		uint8_t widgetId = 0;
		#ifdef retroTerm_DYNAMIC_OBJECT_ALLOCATION
		while(&_widgets[widgetId] != nullptr) //Find the first free widget slot, they are not necessarily deleted in order of creation
		{
			widgetId++;
		}
		_widgets[widgetId] = new widget;
		#else
		while(_widgetExists(widgetId%_widgetObjectLimit)) //Find the first free widget slot
		{
			widgetId++;
		}
		#endif
		_widgets[widgetId].type = type;
		_widgets[widgetId].x = x;
		_widgets[widgetId].y = y;
		_widgets[widgetId].w = w;
		_widgets[widgetId].h = h;
		_widgets[widgetId].currentState = 0x011C;						//New widgets and the label are considered changed and active
		_widgets[widgetId].value = 0;									//New widgets have a value of zero or false
		if(strlen(label) > 0)
		{
			_widgets[widgetId].label = new char[strlen(label) + 1];		//Assign some memory for the label
			memcpy(_widgets[widgetId].label, label, strlen(label) + 1);	//Copy in the label
		}
		else
		{
			_widgets[widgetId].label = nullptr;
		}
		_widgets[widgetId].attributes = attributes;						//Set the attributes
		_widgets[widgetId].labelAttributes = attributes;				//Set the label attributes the same as main ones
		_widgets[widgetId].contentAttributes = attributes;				//Set the label attributes the same as main ones
		_widgets[widgetId].style = style;								//Set the style
		_widgets[widgetId].shortcut = noKeyPressed;						//Delete the shortcut
		if(_widgets[widgetId].type == _widgetTypes::textInput)
		{
			_widgets[widgetId].content = new char[w-1];					//Allocate space for the typing buffer
			_widgets[widgetId].content[0] = 0;							//Null terminate the string
		}
		else if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint16_t bufferSize = _textCapacity(widgetId);
			_widgets[widgetId].content = new char[bufferSize];			//Allocate space for the scrolling text buffer
			memset(_widgets[widgetId].content, ' ',bufferSize);			//Fill this with spaces
		}
		_widgets[widgetId].contentOffset = 0;							//Clear content offset
		_numberOfWidgets++;
		return(widgetId + 1);
	}
	else
	{
		return(0);	//Return 0 indicates failure
	}
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newWidget(_widgetTypes type, const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newWidget(_widgetTypes type, const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#endif
{
	if(_numberOfWidgets < _widgetObjectLimit)
	{
		uint8_t widgetId = 0;
		#ifdef retroTerm_DYNAMIC_OBJECT_ALLOCATION
		while(&_widgets[widgetId] != nullptr) //Find the first free widget slot, they are not necessarily deleted in order of creation
		{
			widgetId++;
		}
		_widgets[widgetId] = new widget;
		#else
		while(_widgetExists(widgetId%_widgetObjectLimit)) //Find the first free widget slot
		{
			widgetId++;
		}
		#endif
		_widgets[widgetId].type = type;
		_widgets[widgetId].x = x;
		_widgets[widgetId].y = y;
		_widgets[widgetId].w = w;
		_widgets[widgetId].h = h;
		_widgets[widgetId].currentState = 0x411C;						//New widgets and the label are considered changed and active, it is also marked that the lable is in PROGMEM
		_widgets[widgetId].value = 0;									//New widgets have a value of zero or false
		if(label != nullptr)
		{
			_widgets[widgetId].label = (char *) label;					//Cast the PROGMEM as a char * to store it
		}
		else
		{
			_widgets[widgetId].label = nullptr;
		}
		_widgets[widgetId].attributes = attributes;						//Set the attributes
		_widgets[widgetId].labelAttributes = attributes;				//Set the label attributes the same as main ones
		_widgets[widgetId].contentAttributes = attributes;				//Set the label attributes the same as main ones
		_widgets[widgetId].style = style;								//Set the style
		_widgets[widgetId].shortcut = noKeyPressed;						//Delete the shortcut
		if(_widgets[widgetId].type == _widgetTypes::textInput)
		{
			_widgets[widgetId].content = new char[w-1];					//Allocate space for the typing buffer
			_widgets[widgetId].content[0] = 0;							//Null terminate the string
		}
		else if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)
		{
			uint16_t bufferSize = _textCapacity(widgetId);
			_widgets[widgetId].content = new char[bufferSize];			//Allocate space for the scrolling text buffer
			memset(_widgets[widgetId].content, ' ',bufferSize);			//Fill this with spaces
		}
		_widgets[widgetId].contentOffset = 0;							//Set the content offset to the start of the content
		_numberOfWidgets++;
		return(widgetId + 1);
	}
	else
	{
		return(0);	//Return 0 indicates failure
	}
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#else
uint8_t retroTerm::newListBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#endif
{
	return(newWidget(_widgetTypes::listBox, x, y, w, h, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
#else
uint8_t retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::listBox, x, y, w, h, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#else
uint8_t retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#endif
{
	return(newWidget(_widgetTypes::listBox, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#else
uint8_t retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::listBox, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::listBox, x, y, w, h, label, attributes, style));
}
//AVR PROGMEM variants
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#else
uint8_t retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#endif
{
	return(newWidget(_widgetTypes::listBox, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#else
uint8_t retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::listBox, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newListBox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::listBox, x, y, w, h, label, attributes, style));
}

//'Scrolling' text windows, which have a scrolling buffer of text in them, you can add to the top or bottom to 'scroll' it

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#else
uint8_t retroTerm::newTextLog(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#endif
{
	return(newWidget(_widgetTypes::scrollingTextDisplay, x, y, w, h, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
#else
uint8_t retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::scrollingTextDisplay, x, y, w, h, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::scrollingTextDisplay, x, y, w, h, attributes, style));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#else
uint8_t retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#endif
{
	return(newWidget(_widgetTypes::scrollingTextDisplay, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#else
uint8_t retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::scrollingTextDisplay, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::scrollingTextDisplay, x, y, w, h, label, attributes, style));
}
//AVR PROGMEM variants
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#else
uint8_t retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#endif
{
	return(newWidget(_widgetTypes::scrollingTextDisplay, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#else
uint8_t retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::scrollingTextDisplay, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newTextLog(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::scrollingTextDisplay, x, y, w, h, label, attributes, style));
}

//Text displays, which store text for showing. Changing the content frequently in large blocks will cause heap fragmentation.

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#else
uint8_t retroTerm::newTextDisplay(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#endif
{
	return(newWidget(_widgetTypes::staticTextDisplay, x, y, w, h, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
#else
uint8_t retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::staticTextDisplay, x, y, w, h, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::staticTextDisplay, x, y, w, h, attributes, style));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#else
uint8_t retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#endif
{
	return(newWidget(_widgetTypes::staticTextDisplay, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#else
uint8_t retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::staticTextDisplay, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::staticTextDisplay, x, y, w, h, label, attributes, style));
}
//AVR PROGMEM variants
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#else
uint8_t retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#endif
{
	return(newWidget(_widgetTypes::staticTextDisplay, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#else
uint8_t retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::staticTextDisplay, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newTextDisplay(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::staticTextDisplay, x, y, w, h, label, attributes, style));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextInput(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#else
uint8_t retroTerm::newTextInput(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#endif
{
	return(newWidget(_widgetTypes::textInput, x, y, w, h, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
#else
uint8_t retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::textInput, x, y, w, h, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::textInput, x, y, w, h, attributes, style));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#else
uint8_t retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#endif
{
	return(newWidget(_widgetTypes::textInput, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#else
uint8_t retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::textInput, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::textInput, x, y, w, h, label, attributes, style));
}
//PROGMEM variants
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#else
uint8_t retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#endif
{
	return(newWidget(_widgetTypes::textInput, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#else
uint8_t retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::textInput, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newTextInput(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::textInput, x, y, w, h, label, attributes, style));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#else
uint8_t retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#endif
{
	return(newWidget(_widgetTypes::radioButton, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#else
uint8_t retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::radioButton, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::radioButton, x, y, w, h, label, attributes, style));
}
//AVR PROGMEM variants
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#else
uint8_t retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#endif
{
	return(newWidget(_widgetTypes::radioButton, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#else
uint8_t retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::radioButton, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newRadioButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::radioButton, x, y, w, h, label, attributes, style));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#else
uint8_t retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#endif
{
	return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#else
uint8_t retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, attributes, style));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#else
uint8_t retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#endif
{
	return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#else
uint8_t retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newCheckbox(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::checkbox, x, y, w, h, label, attributes, style));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#else
uint8_t retroTerm::newButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
#endif
{
	return(newWidget(_widgetTypes::button, x, y, w, h, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#else
uint8_t retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label)
#endif
{
	return(newWidget(_widgetTypes::button, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#else
uint8_t retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::button, x, y, w, h, label, attributes, _defaultStyle));
}		
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, char *label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::button, x, y, w, h, label, attributes, style));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#else
uint8_t retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label)
#endif
{
	return(newWidget(_widgetTypes::button, x, y, w, h, label, _defaultAttributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#else
uint8_t retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes)
#endif
{
	return(newWidget(_widgetTypes::button, x, y, w, h, label, attributes, _defaultStyle));
}
#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#else
uint8_t retroTerm::newButton(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const __FlashStringHelper* label, const uint16_t attributes, const uint8_t style)
#endif
{
	return(newWidget(_widgetTypes::button, x, y, w, h, label, attributes, style));
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::showWidget(const uint8_t widgetId)	//Show a widget
#else
void retroTerm::showWidget(const uint8_t widgetId)	//Show a widget
#endif
{
	if(_widgetExists(widgetId - 1) && (_widgets[widgetId - 1].currentState & 0x0001) == 0x0000)
	{
		_widgets[widgetId - 1].currentState = _widgets[widgetId - 1].currentState | 0x0001;	//Change visibility
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::hideWidget(const uint8_t widgetId)	//Hide a widget
#else
void retroTerm::hideWidget(const uint8_t widgetId)	//Hide a widget
#endif
{
	if(_widgetExists(widgetId - 1) && (_widgets[widgetId - 1].currentState & 0x0001) == 0x0001)
	{
		_widgets[widgetId - 1].currentState = _widgets[widgetId - 1].currentState & 0xFFFE;	//Change visibility
		if(widgetId - 1 == _selectedWidget && _widgets[widgetId - 1].type == _widgetTypes::textInput)
		{
			hideCursor();	//Stop the cursor hanging around
		}
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::refreshAllWidgets()
#else
void retroTerm::refreshAllWidgets()
#endif
{
	for (uint8_t widgetId = 0 ; widgetId < _widgetObjectLimit ; widgetId++)
	{
		if(_widgetExists(widgetId) && _widgets[widgetId].currentState & 0x0001)	//Widget is visible
		{
			_widgets[widgetId].currentState = _widgets[widgetId].currentState & 0xFFFD;	//Change to not displayed
		}
	}
}
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::moveWidget(uint8_t widgetId, const uint8_t x, const uint8_t y)		//Move a widget
#else
void retroTerm::moveWidget(uint8_t widgetId, const uint8_t x, const uint8_t y)		//Move a widget
#endif
{
	widgetId--;																		//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId) && (_widgets[widgetId].x != x || _widgets[widgetId].y != y))
	{
		_widgets[widgetId].x = x;
		_widgets[widgetId].y = y;
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x000C;	//Mark widget and content as changed
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR retroTerm::resizeWidget(uint8_t widgetId, const uint8_t w, const uint8_t h)	//Resize a widget
#else
void retroTerm::resizeWidget(uint8_t widgetId, const uint8_t w, const uint8_t h)	//Resize a widget
#endif
{
	widgetId--;																		//Using ID 0 as 'unallocated/fail' when feeding back to the application so adjust it
	if(_widgetExists(widgetId) && (_widgets[widgetId].w != w || _widgets[widgetId].h != h))
	{
		if(_widgets[widgetId].type == _widgetTypes::scrollingTextDisplay)//A resize of the content needs to occur
		{
			uint8_t oldWidth = _columnsAvailable(widgetId);				//Record info about the previous content
			uint8_t oldHeight = _linesAvailable(widgetId);
			char* oldContent = _widgets[widgetId].content;
			_widgets[widgetId].w = w;									//Set the new dimensions
			_widgets[widgetId].h = h;
			uint16_t bufferSize = _textCapacity(widgetId);
			uint8_t newWidth = _columnsAvailable(widgetId);
			uint8_t newHeight = _linesAvailable(widgetId);
			_widgets[widgetId].content = new char[bufferSize];			//Allocate space for the new scrolling text buffer
			if(newWidth > oldWidth)										//There will be whitespace at the end of each line
			{
				memset(_widgets[widgetId].content, ' ',bufferSize);		//Fill this with spaces to avoid doing it line by line
			}
			uint8_t lineOffset = 0;
			if(newHeight < oldHeight)
			{
				lineOffset = oldHeight - newHeight;
			}
			for(uint8_t line = 0; line < newHeight && line < oldHeight; line++)	//Line by line copy in the content
			{
				if(newWidth > oldWidth)
				{
					memcpy(_widgets[widgetId].content + (line * newWidth), oldContent + ((line + lineOffset) * oldWidth), oldWidth);
				}
				else
				{
					memcpy(_widgets[widgetId].content + (line * newWidth), oldContent + ((line + lineOffset) * oldWidth), newWidth);
				}
			}
			delete[] oldContent;						//Delete the previous content
		}
		else
		{
			_widgets[widgetId].w = w;
			_widgets[widgetId].h = h;
		}
		_widgets[widgetId].currentState = _widgets[widgetId].currentState | 0x000C;	//Mark widget and content as changed
	}
}

//Trailing ifdef from top of file
#endif