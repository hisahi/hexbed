# HexBed
A graphical hex editor, still a work in progress.

## Supported features
* Viewing and editing hex and text data
* Custom color schemes for data viewer and editor
* Localization/internationalization
* Search and replace
* Support for very large files
* Setting for viewing hex only, text only or both
* Opening files as read-only
* Full undo & redo
* Backup system (backs up files before overwriting)

## Planned features
* Stability
* Performance
* Not losing data
* Not having bugs
* Inserting blocks, bitwise operations, etc.
* Bit editing
* Search for text
* Search for integers, floats, etc.
* Data inspector and editor (integers, etc.), with plugin support
* Import/export, with plugin support
* Support for more text encodings
* Multiple windows/views
* Bookmarks
* File compare
* Macros
* Bells and whistles

## Platforms
Tested and developed for Linux, but theoretically cross-platform.

## Implementation
In modern C++ (C++17 with some C++20). The UI uses wxWidgets.

## License
GPL v3. See `COPYING`.
