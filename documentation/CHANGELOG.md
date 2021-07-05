# Change Log

## Version 0.1.3

- New methods
  - widgetClicked(), which returns the ID of any widget has been clicked, 0 if not. Resets on read.
  - scrollDownWidgetContent(uint8_t widgetId), which scrolls down content in a widget
  - lines(uint8_t widgetId), number of line available for content in a widget
  - columns(uint8_t widgetId), number of columns available for content in a widget

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

- Single key shortcuts would cause a crash. They are now also case-insensitive when typed, but display as set.

## Version 0.1.0

- First public release.