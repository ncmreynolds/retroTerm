/*
 * retroTerm widget test sketch
 * 
 * This sketch tests all the widget types, one at a time. Code for each type of widget is in a separate tab.
 * 
 * Probing terminal size, to see what space is available. You can change terminal size between tests to check this works.
 * Button test. The buttons of different style appear and and can be clicked with mouse or keyboard shortcut.
 * Checkbox test. Three differet checkboxes appear and can be checked with the mouse or keyboard shortcut.
 * Radio button test. Three different radio buttons can be appear and one can be selected with the mouse or keyboard shortcut.
 * Fixed text display test. Three windows of wordwrapped text, they can be clicked on and will scroll with the mouse wheel or arrow keys on the keyboard.
 * Scrolling text display test. Three windows of text, which scroll up as more is added.
 * List box test. Three list boxes with different options that can be selected in each.
 * Text input field test. Three text input fields, which can be selected with the mouse or keyboard shortcuts. You can type in these with normal editing keys.
 * 
 */
 #include <retroTerm.h>    //Include terminal library

retroTerm terminal;       //Create a terminal instance

#if defined(__AVR__)
const char textToShow[] PROGMEM = {"HAMLET: To be, or not to be--that is the question: \
Whether 'tis nobler in the mind to suffer \
The slings and arrows of outrageous fortune \
Or to take arms against a sea of troubles \
And by opposing end them. To die, to sleep-- \
No more--and by a sleep to say we end \
The heartache, and the thousand natural shocks \
That flesh is heir to. 'Tis a consummation \
Devoutly to be wished. To die, to sleep \
No more--and by a sleep to say we end \
The heartache, and the thousand natural shocks \
That flesh is heir to. 'Tis a consummation \
Devoutly to be wished. To die, to sleep-- \
To sleep--perchance to dream: ay, there's the rub, \
For in that sleep of death what dreams may come \
When we have shuffled off this mortal coil, \
Must give us pause. There's the respect \
That makes calamity of so long life."};
#elif defined(ESP8266) || defined(ESP32)
const char textToShow[] PROGMEM = {"HAMLET: To be, or not to be--that is the question: \
Whether 'tis nobler in the mind to suffer \
The slings and arrows of outrageous fortune \
Or to take arms against a sea of troubles \
And by opposing end them. To die, to sleep-- \
No more--and by a sleep to say we end \
The heartache, and the thousand natural shocks \
That flesh is heir to. 'Tis a consummation \
Devoutly to be wished. To die, to sleep \
No more--and by a sleep to say we end \
The heartache, and the thousand natural shocks \
That flesh is heir to. 'Tis a consummation \
Devoutly to be wished. To die, to sleep-- \
To sleep--perchance to dream: ay, there's the rub, \
For in that sleep of death what dreams may come \
When we have shuffled off this mortal coil, \
Must give us pause. There's the respect \
That makes calamity of so long life. \
For who would bear the whips and scorns of time, \
Th' oppressor's wrong, the proud man's contumely \
The pangs of despised love, the law's delay, \
The insolence of office, and the spurns \
That patient merit of th' unworthy takes, \
When he himself might his quietus make \
With a bare bodkin? Who would fardels bear, \
To grunt and sweat under a weary life, \
But that the dread of something after death, \
The undiscovered country, from whose bourn \
No traveller returns, puzzles the will, \
And makes us rather bear those ills we have \
Than fly to others that we know not of? \
Thus conscience does make cowards of us all, \
And thus the native hue of resolution \
Is sicklied o'er with the pale cast of thought, \
And enterprise of great pitch and moment \
With this regard their currents turn awry \
And lose the name of action. -- Soft you now, \
The fair Ophelia! -- Nymph, in thy orisons \
Be all my sins remembered."};
#else
char textToShow[] = "HAMLET: To be, or not to be--that is the question: \
Whether 'tis nobler in the mind to suffer \
The slings and arrows of outrageous fortune \
Or to take arms against a sea of troubles \
And by opposing end them. To die, to sleep-- \
No more--and by a sleep to say we end \
The heartache, and the thousand natural shocks \
That flesh is heir to. 'Tis a consummation \
Devoutly to be wished. To die, to sleep-- \
To sleep--perchance to dream: ay, there's the rub, \
For in that sleep of death what dreams may come \
When we have shuffled off this mortal coil, \
Must give us pause. There's the respect \
That makes calamity of so long life. \
For who would bear the whips and scorns of time, \
Th' oppressor's wrong, the proud man's contumely \
The pangs of despised love, the law's delay, \
The insolence of office, and the spurns \
That patient merit of th' unworthy takes, \
When he himself might his quietus make \
With a bare bodkin? Who would fardels bear, \
To grunt and sweat under a weary life, \
But that the dread of something after death, \
The undiscovered country, from whose bourn \
No traveller returns, puzzles the will, \
And makes us rather bear those ills we have \
Than fly to others that we know not of? \
Thus conscience does make cowards of us all, \
And thus the native hue of resolution \
Is sicklied o'er with the pale cast of thought, \
And enterprise of great pitch and moment \
With this regard their currents turn awry \
And lose the name of action. -- Soft you now, \
The fair Ophelia! -- Nymph, in thy orisons \
Be all my sins remembered.";
#endif

//Global variables for the widgets

uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 0;

void setup() {
  Serial.begin(115200);   //Initialise the Serial stream
  terminal.begin(Serial); //Initialise the library
  terminal.defaultAttributes(BACKGROUND_COLOR_BLACK | COLOUR_WHITE);  //Set some initial attributes for printing
  terminal.probeSize();   //Attempts to determine how many lines and columns the terminal has
}

void loop() {
  probingExample();             //Demo methods that 'probe' the terminal, see the tab 'probingDemo' for code
  buttonExample();              //Demo methods for creating and checking the state of button widgets, see the tab 'buttonDemo' for code
  checkboxExample();            //Demo methods for creating and checking the state of checkbox widgets, see the tab 'checkboxDemo' for code
  radioButtonExample();         //Demo methods for creating and checking the state of radio button widgets, see the tab 'radiobuttonDemo' for code
  textDisplayExample();         //Demo methods for creating static content text box widgets, see the tab 'textDisplayDemo' for code
  textLogExample();             //Demo methods for creating 'logging' text box widgets, see the tab 'textLogDemo' for code. This widget does NOT store content long term, it's intended to be a logging/chat window etc.
  listBoxExample();             //Demo methods for creating list box widgets, see the tab 'listBoxExample' for code
  textInputExample();           //Demo methods for creating and retrieving input from text input fields, see the tab 'textInputDemo' for code
}

/*
 * The 'newPage' function uses some 'primitves' from the library
 * to clear the screen and show a title at the top of the page before every test
 */

void newPage(String title) {
  terminal.reset();             //Resets the terminal, clearing colours etc.
  terminal.hideCursor();        //Hide the cursor
  terminal.printAt(1,1,title + ", press a key or wait 30s to finish");  //Print the title at the top of the page
  terminal.println();           //Print a blank line
  terminal.setScrollWindow(3,terminal.lines()/2 - 4);     //Set up somewhere to show the events without disrupting the widgets
  terminal.enableMouse();       //Enable the mouse
}

/*
 * The 'endPage' function tidies up after the test
 */
void endPage() {
  terminal.houseKeeping();
  if(terminal.keyPressed())
  {
    terminal.readKeypress();
  }
  terminal.deleteWidget(red);     //It is safe to try and delete a non-existent widget, the method will check before attempting to de-allocate memory etc.
  terminal.deleteWidget(green);
  terminal.deleteWidget(blue);
  terminal.houseKeeping();
}
