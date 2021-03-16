# retroTerm

retroTerm is an Arduino library for creating user interfaces in a terminal emulator.

Now that many microcontrollers have Bluetooth or Wi-Fi it might seem superfluous but there are many examples that do not and occasions when you want to configure them without having a network available.

retroTerm allows you to create clickable 'buttons', 'checkboxes', 'option lists' and so on plus also simple line editing of text.

## Table of Contents

1. [About retoTerm](#about-the-project)
1. [Getting Started](#getting-started)
	1. [Dependencies](#dependencies)
	1. [Building](#building)
	1. [Installation](#installation)
	1. [Usage](#usage)
1. [Release Process](#release-process)
	1. [Versioning](#versioning)
	1. [Payload](#payload)
1. [How to Get Help](#how-to-get-help)
1. [Further Reading](#further-reading)
1. [Contributing](#contributing)
1. [License](#license)
1. [Authors](#authors)
1. [Acknowledgements](#acknowledgements)

# About retroTerm

retroTerm is an Arduino library for creating user interfaces in a terminal emulator.

By default this is expected to be the Serial interface, but it could also be any Stream. Testing is done with the popular terminal emulator [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) as it is open and supports many terminal features others do not. Please use PuTTY first before reporting issues. The Arduino IDE terminal is not useful, it supports almost no ANSI/VT terminal features.

* What features does your project provide?
* Short motivation for the project? (Don't be too long winded)
* Links to the project site

```
Show some example code to describe what your project does
Show some of your APIs
```

**[Back to top](#table-of-contents)**

# Getting Started

From the project page, select 'Code' -> 'Download ZIP' and save the file somewhere appropriate.

You can then install in the Arduino IDE by choosing 'Sketch' -> 'Include Library' -> 'Add .ZIP Library...'.

You should try the first two example sketches to check it works with your terminal emulator, then try the 'Alarm Clock' example which is a very simple application that will work on most microcontrollers.

Examples are found under After installation, the example will be available under "File" -> "Examples" -> "retroTerm" in the Arduino IDE.

[Capability Test](examples/Example01_terminalCapabilityTests/readme/md)

[Widget Test](examples/Example02_widgetTests/readme.md)

[Alarm Clock](examples/Example03_alarmClock/readme.md)

## Dependencies

This library has no specific dependencies, but you will need a fully featured terminal emulator that can connect to your target microcontroller over a serial or USB-serial interface. Testing is done with [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) and it is strongly recommend you start with using this.

## Getting the Source

This project is [hosted on GitHub](https://github.com/ncmreynolds/retroTerm).

## Building

Instructions for how to build your project

```
Examples should be included
```

## Running Tests

Describe how to run unit tests for your project.

```
Examples should be included
```

### Other Tests

If you have formatting checks, coding style checks, or static analysis tests that must pass before changes will be considered, add a section for those and provide instructions

## Installation

Instructions for how to install your project's build artifacts

```
Examples should be included
```

## Usage

Instructions for using your project. Ways to run the program, how to include it in another project, etc.

```
Examples should be included
```

If your project provides an API, either provide details for usage in this document or link to the appropriate API reference documents

**[Back to top](#table-of-contents)**

# Release Process

Talk about the release process. How are releases made? What cadence? How to get new releases?

## Versioning

This project uses [Semantic Versioning](http://semver.org/). For a list of available versions, see the [repository tag list](https://github.com/ncmreynolds/retroTerm/tags).

## Payload

**[Back to top](#table-of-contents)**

# How to Get Help

Provide any instructions or contact information for users who need to get further help with your project.

# Contributing

We encourage public contributions! Please review [CONTRIBUTING.md](docs/CONTRIBUTING.md) for details on our code of conduct and development process.

**[Back to top](#table-of-contents)**

# Further Reading

Serial 'dumb' terminals got quite smart over time, I took information from a variety of sources to help write this library including the ones linked below.

[VT100.net](https://www.vt100.net/)

[ascii-table.com](http://ascii-table.com/ansi-escape-sequences-vt-100.php)

[Paul Bourke's list of VT commands](http://braun-home.net/michael/info/misc/VT100_commands.htm)

[libvterm-ctrl](https://github.com/BryanHaley/libvterm-ctrl)

**[Back to top](#table-of-contents)**

# License

Copyright (c) 2021 Nick Reynolds

This project is licensed under the MIT License - see [LICENSE.md](LICENSE.md) file for details.

**[Back to top](#table-of-contents)**

# Authors

* **[Nick Reynolds](https://github.com/ncmreynolds)** 

**[Back to top](#table-of-contents)**

# Acknowledgments

This project wouldn't be useful without the excellent open source terminal emulator [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/), which I have used for what feels like decades.

**[Back to top](#table-of-contents)**