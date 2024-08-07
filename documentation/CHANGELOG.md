# Change Log

## Version 0.1.6

- New example
  - Added a Widgets example using Classic Bluetooth Serial as the connecting Stream, as this may be useful

- Bugfixes
  - Fixed non-scrolling of listboxes when they overflow the widget. I had thought this was working but meh, perhaps it never did.
  - Made option selection in listboxes using arrow keys scroll the content up/down when going off the top/bottom of the widget. Previously it selected the  correct option but did not scroll the options to keep it visible inside the widget.

## Version 0.1.5

- Bugfixes
  - Fix compilation error on AVR, had only been tested on ESP32.

## Version 0.1.4

- New methods
  - `void setScrollUpCallback(void (*scrollUpCallback)(uint8_t))`, which allows you to set a callback function for scrolling up the content of widgets. The argument supplied to the callback function is the widget ID of the scrolled widget. This is for when the application needs to feed new content to the bottom of the widget as it's not in memory. Normally this would be used for a `textLog` and the content added with `appendWidgetContent`.
  - `void setScrollDownCallback(void (*scrollDownCallback)(uint8_t))`, similar to the callback for scrolling widget content up. Normally this would be used for a `textLog` and the content added with `prependWidgetContent`.
  - Experimental: Putting a tab `\t` at the start of an option in a `listBox` makes that row unclickable. These unclickable rows can be blank, just `\t\n`. Useful for formatting the `listBox` with a heading/dividers of sorts. Ugly code needs fixing.
  - Experimental: You can add options to a `listBox` with `appendWidgetContent`. This is inefficient but it _does_ work.
  
- Bugfixes
  - Widgets created without attributes specified were created with 'default attributes', not 'default **widget** attributes'.
  - Improve redraw of `textLog` widgets, which previously failed to hide the cursor.
  - Improved rendering of text in `textDisplay` so that it should no longer end up with redundant spaces at the beginning of lines.

## Version 0.1.3

- New methods
  - `void setClickCallback(void (*clickCallback)(uint8_t))`, which allows you to set a callback function for clicks on widgets. The argument supplied to the callback function is the widget ID of the clicked widget. This avoids polling widgets and is something I've been meaning to implement from the start. As per usual 'clicks' includes the use of shortcut keys on widgets. If there are multiple clicks then the callback function will be called repeatedly until there are none left, without doing any widget updates.
  - `void setTypingCallback(void (*typingCallback)(uint8_t))`, which allows you to set a callback function for typing.  The argument supplied to the callback function is the ID of the pressed key, as used elsewhere in the library. This avoids polling to see if any typing has occurred. Note that like other functions for reading the keyboard, any navigation/shortcut keys intercepted by a widget are **not** reported.
  - `uint8_t widgetClicked()`, which returns the ID of any widget has been clicked, 0 if not. Resets on read to the next clicked widget, or 0 if there are no more. Avoids polling all widgets individually.
  - `void scrollDownWidgetContent(uint8_t widgetId)`, which scrolls down content in a widget
  - `uint8_t lines(uint8_t widgetId)`, number of line available for displaying content in a widget
  - `uint8_t columns(uint8_t widgetId)`, number of columns available for displaying content in a widget
- Improvements to `void houseKeeping()` so it tracks changes better and only iterates through widgets looking for changes/clicks when necessary. This has massively improved 'idle' performance of the library.

## Version 0.1.2

- Markdown formatting changes
  - H1 is now inverse
  - H2 is now bold + underline
  - H3 is now underline
  - H4+ is now underline + faint
- Fixed a bug where omitting the space after # character for a Markdown heading would cause an infinite loop, watchdog error and exception
- Improvements to overloading/templating of widget functions
- Scrollbar added to textLog widget
- `newWidget` and `setWidgetContent` now record the size of the content and ONLY reallocate memory on heap if the new content is larger. This should reduce heap fragmentation if your application makes small changes to content frequently.
- Internal refactoring

## Version 0.1.1

- Single letter shortcuts (e.g. 'A') would cause a crash. They are now also case-insensitive when typed, but display as set.

## Version 0.1.0

- First public release.