<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder -->
# Layout

 VirtualFluids project contains a [clang-format file](https://git.rz.tu-bs.de/irmb/VirtualFluids/-/blob/main/.clang-format). Clang-format is a tool to automatically format C++ code. The .clang-format file includes all formatting rules of the project. The most important rules a listed below.


## Spaces

### Basic indentation should be 4 spaces (not tabs!!).

Example:
```cpp
for (i = 0; i < nElements; i++) {
    a[i] = 0;
}
```

- Conventional operators should be surrounded by a space character.
- C++ reserved words should be followed by a white space.
- Commas should be followed by a white space.
- Semicolons in for statements should be followed by a space character.

Example:
```cpp
a = (b + c) * d; // NOT: a=(b+c) * d
while (true) { // NOT: while(true)
// . . .
doSomething (a, b, c, d) ; // NOT: doSomething ( a, b, c, d) ;
for (i = 0; i < 10; i++) { // NOT: for( i=0;i<10;i++){
//...
```

Makes the individual components of the statements stand out. Enhances readability. It is difficult to give a complete list of the suggested use of whitespace in C++ code. The examples above however should give a general idea of the intentions.


## The class declarations should have the following form
- curly braces should be on a separate line

Example:
```cpp
class SomeClass : public BaseClass
{
public:
//...
protected:
//...
private:
//...
}
```

## Method definitions should have the following form
- curly braces should be on a separate line

Example:
```cpp
void someMethod()
{
...
}
```


## The if-else class of statements should have the following form

Example:
```cpp
if (condition) {
    statements;
}
if (condition) {
    statements;
} else {
    statements;
}
if (condition) {
    statements;
} else if (condition) {
    statements;
} else {
    statements;
}
```

## A for statement should have the following form

Example:
```cpp
for (initialization; condition; update) {
    statements;
}
```

This follows from the general block rule above.

## An empty for statement should have the following form

Example:
```cpp
for (initialization; condition; update)
;
```

This emphasizes the fact that the for statement is empty and it makes it obvious for the reader that this is intentional. Empty loops should be avoided, however.

## A while statement should have the following form

Example:
```cpp
while (condition) {
    statements;
}
```

## A do-while statement should have the following form

Example:
```cpp
do {
    statements;
} while (condition);
```

## A switch statement should have the following form

Example:
```cpp
switch (condition) {
    case ABC:
        statements;
        // Fallthrough
    case DEF:
        statements;
        break;
    case XYZ:
        statements;
        break;
    default:
        statements;
        break;
}
```

Note that each case keyword is indented relative to the switch statement as a whole. This makes the entire switch statement stand out. The explicit Fallthrough comment should be included whenever there is a case statement without a break statement. Leaving the break out is a common error, and it must be made clear that it is intentional when it is not there.

## A try-catch statement should have the following form:

Example:
```cpp
try {
    statements;
}
catch (Exception& exception) {
    statements;
}
```

## Single statement if-else, for or while statements can be written without brackets.

Example:
```cpp
if (condition)
    statement;
while (condition)
    statement;
for (initialization; condition; update)
    statement;
```

It is a common recommendation that brackets should always be used in all these cases. However, brackets are in general a language construct that groups several statements. Brackets are per definition superfluous on a single statement. A common argument against this syntax is that the code will break if an additional statement is added without also adding the brackets.

## Logical units within a block should be separated by one blank line.

Example:
```cpp
Matrix4x4 matrix = new Matrix4x4();

double cosAngle = Math.cos(angle);
double sinAngle = Math.sin(angle);

matrix.setElement( 1, 1, cosAngle);
matrix.setElement( 1, 2, sinAngle);
matrix.setElement( 2, 1, sinAngle);
matrix.setElement( 2, 2, cosAngle);

multiply (matrix);
```

Enhance readability by introducing white space between logical units of a block.

## Methods should be separated by one blank line.

Example: NA

## Alignment can be used wherever it enhances readability.

Example:
```cpp
if      (a == lowValue)    compueSomething ();
else if (a == mediumValue) computeSomethingElse ();
else if (a == highValue)   computeSomethingElseYet ();

value = (potential * oilDensity) / constant1 +
        (depth * waterDensity) / constant2 +
        (zCoordinateValue * gasDensity) / constant3;

minPosition     = computeDistance (min, x, y, z);
averagePosition = computeDistance (average, x, y, z);

switch (value) {
    case PHASE_OIL   : strcpy (phase, "Oil");   break;
    case PHASE_WATER : strcpy (phase, "Water"); break;
    case PHASE_GAS   : strcpy (phase, "Gas");   break;
}
```

There are a number of places in the code where white space can be included to enhance readability even if this violates common guidelines. Many of these cases have to do with code alignment. General guidelines on code alignment are difficult to give, but the examples above should give a general clue.
