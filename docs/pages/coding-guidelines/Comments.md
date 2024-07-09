<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder -->
# Comments

## Tricky code should not be commented on but rewritten!

Example:
```cpp
// NOT:
Point () {} // constructor
int nc; // number of cars
f = m * a; // force = mass * acceleration

// BETTER:
force = mass * acceleration;
```

In general, the use of comments should be minimized by making the code self-documenting through appropriate name choices and an explicit logical structure.

## All comments should be written in English.

Example: NA

In an international environment, English is the preferred language.

## Use // for all comments, including multi-line comments.

Example:
```cpp
// Comment spanning
// more than one line.
```

Since multilevel C-commenting is not supported, using // comments ensures that it is always possible to comment out entire sections of a file using /\* \*/ for debugging purposes etc.

There should be a space between the "//" and the actual comment, and comments should always start with an uppercase letter and end with a period.
