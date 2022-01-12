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

## Planned features
* Stability
* Performance
* Not losing data
* Not having bugs
* Import/export data
* Support for more text encodings
* Bookmarks
* Multiple windows/views
* File compare
* Plugin system for data inspector and import/export
* Macros
* Bells and whistles

## Platforms
Tested and developed for Linux, but theoretically cross-platform.

## Implementation
In modern C++ (C++17 with some C++20). The UI uses wxWidgets.

## License
GPL version 3. See `COPYING`.
