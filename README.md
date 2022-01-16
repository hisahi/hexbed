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
* Bit editing
* Inserting blocks
* Bitwise operations on blocks
* Data inspector and editor (integers, etc.)
* Search for text (including with case insensitivity)
* Search for integers, floats, etc.
* Import data (Intel HEX, Motorola SREC)
* Export data (Intel HEX, Motorola SREC)
* Export into programming languages (C, C#, Java)
* Export into Text, HTML
* Plugin system for loading extra charset definitions
* UTF-16/UTF-32 character viewer
* Text<->binary converter

## Planned features
* Stability
* Performance
* Not losing data
* Not having bugs
* Bookmarks
* Multiple windows/views
* Plugin system for data inspector
* Plugin system for import/export
* Plugin system for text converter
* File compare
* Scripting system
* Bells and whistles

## Platforms
Tested and developed for Linux, but theoretically cross-platform.

## Implementation
In modern C++ (C++17 with some C++20). The UI uses wxWidgets.

ICU is used, but not technically necessary (it can be disabled by changing
the Makefile).

## License
GPL version 3. See `COPYING`.
