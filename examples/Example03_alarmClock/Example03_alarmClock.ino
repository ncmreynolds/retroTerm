/*
 * retroTerm alarm clock application example
 * 
 * 
 * 
 * 
 * 
 * 
 */
#include <retroTerm.h>    //Include serial terminal library

retroTerm terminal;       //Create a terminal instance and set up all the widgets
const uint16_t clockAttributes = BACKGROUND_COLOR_BLACK | COLOR_GREEN | ATTRIBUTE_FAINT;
const uint8_t clockStyle = OUTER_BOX | BOX_DOUBLE_LINE | LABEL_IN_BOX | LABEL_CENTRED;
const uint16_t clockLabelAttributes = BACKGROUND_COLOR_BLACK | COLOR_GREEN  | ATTRIBUTE_FAINT;
const uint16_t clockDisplayAttributes = BACKGROUND_COLOR_BLACK | COLOR_WHITE;
const uint16_t buttonAttributes = BACKGROUND_COLOR_BLACK | COLOUR_GREEN;
const uint16_t alarmAttributes =  BACKGROUND_COLOR_RED | COLOUR_WHITE | ATTRIBUTE_FAINT;
const uint8_t buttonStyle = NO_BOX;
const uint8_t clockWidth = 10;
const uint8_t clockHeight = 5;
const uint8_t clockY = (terminal.lines() - clockHeight*2)/2;
uint8_t clockDisplay;
uint8_t clockHourUp;
uint8_t clockHourDown;
uint8_t clockMinuteUp;
uint8_t clockMinuteDown;
uint8_t clockSecondUp;
uint8_t clockSecondDown;
uint8_t alarmDisplay;
uint8_t alarmHourUp;
uint8_t alarmHourDown;
uint8_t alarmMinuteUp;
uint8_t alarmMinuteDown;
uint8_t alarmSecondUp;
uint8_t alarmSecondDown;
uint8_t alarmCancel;

uint32_t clockOffset = 0;
bool clockOffsetChanged = true;
uint32_t alarmOffset = 60000ul;
bool alarmOffsetChanged = true;
bool alarmTriggered = false;

uint32_t lastTimeDisplay = 0;
uint32_t timeDisplayInterval = 1000ul;

uint32_t lastBell = 0;
uint32_t bellInterval = 3000ul;

void setup() {
  Serial.begin(115200);
  terminal.begin(Serial);   //Attach the terminal instance to Serial, but it could go elsewhere
  terminal.reset();         //Clear the screen
  terminal.setTitle(F("Alarm clock")); //Set the terminal title bar to "Alarm clock"
  terminal.hideCursor();    //Hide the cursor as only widgets are used
  clockDisplay =    terminal.newTextDisplay((terminal.columns() - clockWidth*2)/2, clockY, clockWidth, clockHeight, F("Time"),clockAttributes,clockStyle);
  terminal.labelAttributes(clockDisplay,clockLabelAttributes);
  terminal.contentAttributes(clockDisplay,clockDisplayAttributes);
  clockHourUp =     terminal.newButton((terminal.columns() - clockWidth*2)/2+1, clockY+5, 1, 1, F("+"), buttonAttributes, buttonStyle);
  clockHourDown =   terminal.newButton((terminal.columns() - clockWidth*2)/2+2, clockY+5, 1, 1, F("-"), buttonAttributes, buttonStyle);
  clockMinuteUp =   terminal.newButton((terminal.columns() - clockWidth*2)/2+4, clockY+5, 1, 1, F("+"), buttonAttributes, buttonStyle);
  clockMinuteDown = terminal.newButton((terminal.columns() - clockWidth*2)/2+5, clockY+5, 1, 1, F("-"), buttonAttributes, buttonStyle);
  clockSecondUp =   terminal.newButton((terminal.columns() - clockWidth*2)/2+7, clockY+5, 1, 1, F("+"), buttonAttributes, buttonStyle);
  clockSecondDown = terminal.newButton((terminal.columns() - clockWidth*2)/2+8, clockY+5, 1, 1, F("-"), buttonAttributes, buttonStyle);
  alarmDisplay =    terminal.newTextDisplay((terminal.columns() - clockWidth*2)/2 + clockWidth, clockY, clockWidth, clockHeight, F("Alarm"),clockAttributes,clockStyle);
  terminal.contentAttributes(alarmDisplay,clockDisplayAttributes);
  alarmHourUp =     terminal.newButton((terminal.columns() - clockWidth*2)/2 + clockWidth+1, clockY+5, 1, 1, F("+"), buttonAttributes, buttonStyle);
  alarmHourDown =   terminal.newButton((terminal.columns() - clockWidth*2)/2 + clockWidth+2, clockY+5, 1, 1, F("-"), buttonAttributes, buttonStyle);
  alarmMinuteUp =   terminal.newButton((terminal.columns() - clockWidth*2)/2 + clockWidth+4, clockY+5, 1, 1, F("+"), buttonAttributes, buttonStyle);
  alarmMinuteDown = terminal.newButton((terminal.columns() - clockWidth*2)/2 + clockWidth+5, clockY+5, 1, 1, F("-"), buttonAttributes, buttonStyle);
  alarmSecondUp =   terminal.newButton((terminal.columns() - clockWidth*2)/2 + clockWidth+7, clockY+5, 1, 1, F("+"), buttonAttributes, buttonStyle);
  alarmSecondDown = terminal.newButton((terminal.columns() - clockWidth*2)/2 + clockWidth+8, clockY+5, 1, 1, F("-"), buttonAttributes, buttonStyle);
  terminal.setWidgetContent(clockDisplay,F("00:00:00"));  //Stick an initial reading on the clock
  terminal.setWidgetContent(alarmDisplay,F("00:01:00"));  //Stick an initial reading on the alarm
  terminal.showAllWidgets();                              //Make all the widgets created so far visible, by default they are not
  alarmCancel =     terminal.newButton((terminal.columns() - clockWidth*2)/2, clockY - 1, clockWidth*2, 1, F("Cancel Alarm"),alarmAttributes, LABEL_CENTRED);
  terminal.enableBell();                                  //Enable the 'bell' which is used for the alarm sound
  terminal.enableMouse();                                 //Capture the mouse for clicking the +/- buttons
}

void loop() {
  if(terminal.widgetClicked(clockHourUp))
  {
    clockOffset += 3600000ul;
    if(clockOffset > 86400000)
    {
      clockOffset -= 86400000;
    }
    clockOffsetChanged = true;
  }
  else if(terminal.widgetClicked(clockHourDown))
  {
    if(clockOffset >= 3600000)
    {
      clockOffset -= 3600000;
    }
    else
    {
      clockOffset += 82800000;
    }
    clockOffsetChanged = true;
  }
  else if(terminal.widgetClicked(clockMinuteUp))
  {
    clockOffset += 60000;
    if(clockOffset > 86400000)
    {
      clockOffset -= 86400000;
    }
    clockOffsetChanged = true;
  }
  else if(terminal.widgetClicked(clockMinuteDown))
  {
    if(clockOffset >= 60000)
    {
      clockOffset -= 60000;
    }
    else
    {
      clockOffset += 82800000;
    }
    clockOffsetChanged = true;
  }
  else if(terminal.widgetClicked(clockSecondUp))
  {
    clockOffset += 1000;
    if(clockOffset > 86400000)
    {
      clockOffset -= 86400000;
    }
    clockOffsetChanged = true;
  }
  else if(terminal.widgetClicked(clockSecondDown) && clockOffset >= 1000)
  {
    clockOffset -= 1000;
    clockOffsetChanged = true;
  }
  //Alarm buttons
  if(terminal.widgetClicked(alarmHourUp))
  {
    alarmOffset += 3600000;
    alarmOffsetChanged = true;
  }
  else if(terminal.widgetClicked(alarmHourDown) && alarmOffset >= 3600000)
  {
    alarmOffset -= 3600000;
    alarmOffsetChanged = true;
  }
  else if(terminal.widgetClicked(alarmMinuteUp))
  {
    alarmOffset += 60000;
    alarmOffsetChanged = true;
  }
  else if(terminal.widgetClicked(alarmMinuteDown) && alarmOffset >= 60000)
  {
    alarmOffset -= 60000;
    alarmOffsetChanged = true;
  }
  else if(terminal.widgetClicked(alarmSecondUp))
  {
    alarmOffsetChanged = true;
    alarmOffset += 1000;
  }
  else if(terminal.widgetClicked(alarmSecondDown) && alarmOffset >= 1000)
  {
    alarmOffset -= 1000;
    alarmOffsetChanged = true;
  }
  //Alarm cancel button
  if(terminal.widgetClicked(alarmCancel))
  {
    terminal.hideWidget(alarmCancel);
    terminal.widgetAttributes(alarmDisplay,clockAttributes);
    terminal.widgetAttributes(clockDisplay,clockAttributes);
    alarmTriggered = false;
  }
  //Update the clock display
  if(clockOffsetChanged == true || millis() - lastTimeDisplay > timeDisplayInterval)
  {
    terminal.moveCursorTo(5,5);
    showTime(clockDisplay,millis() + clockOffset);
    clockOffsetChanged = false;
    lastTimeDisplay = millis();
  }
  //Update the alarm display
  if(alarmOffsetChanged == true)
  {
    terminal.moveCursorTo(5,6);
    showTime(alarmDisplay,alarmOffset);
    alarmOffsetChanged = false;
  }
  if(hour(millis() + clockOffset) == hour(alarmOffset) && minute(millis() + clockOffset) == minute(alarmOffset) && second(millis() + clockOffset) == second(alarmOffset))
  {
    alarmTriggered = true;
    terminal.showWidget(alarmCancel);
    terminal.widgetAttributes(alarmDisplay,alarmAttributes);
    terminal.widgetAttributes(clockDisplay,alarmAttributes);
  }
  if(alarmTriggered && millis() - lastBell > bellInterval)  //Ring the bell, if necessary
  {
    terminal.soundBell(); //Ring the terminal bell
    lastBell = millis();
  }
  terminal.houseKeeping();  //Read inputs and update the display
}
void showTime(uint8_t widgetId, uint32_t time)
{
  char tempString[9];
  sprintf(tempString,"%02d:%02d:%02d",hour(time),minute(time),second(time));
  terminal.setWidgetContent(widgetId,tempString);
}
uint8_t hour(uint32_t time)
{
  return((time/3600000ul)%24);
}

uint8_t minute(uint32_t time)
{
  return((time/60000ul)%60);
}

uint8_t second(uint32_t time)
{
  return((time/1000ul)%60);
}
