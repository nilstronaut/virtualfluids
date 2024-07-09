<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder -->
# Statements

## Variables should be initialized where they are declared.

Example: NA

This ensures that variables are valid at any time. Sometimes it is impossible to initialize a variable to a valid value where it is declared:

```cpp
int x, y, z;
getCenter(&x, &y, &z);
```

In these cases, it should be left uninitialized rather than initialized to some phony value.

## C++ pointers and references should have their reference symbol next to the type rather than to the name.

Example:
```cpp
float* x; // NOT: float *x;
int& y; // NOT: int &y;
```

The pointer-ness or reference-ness of a variable is a property of the type rather than the name. C-programmers often use the alternative approach, while in C++ it has become more common to follow this recommendation.

## Implicit test for 0 should not be used other than for boolean variables and pointers.

Example:
```cpp
if (nLines != 0) // NOT: if (nLines)
if (value != 0.0) // NOT: if (value)
```

It is not necessarily defined by the C++ standard that ints and floats 0 are implemented as binary 0. Also, by using an explicit test the statement gives an immediate clue of the type being tested.

It is common also to suggest that pointers shouldn't test implicitly for 0 either, i.e. if (line == 0) instead of if (line). The latter is regarded so common in C/C++ however that it can be used.

## Variables should be declared in the smallest scope possible.

Example: NA

Keeping the operations on a variable within a small scope, it is easier to control the effects and side effects of the variable.

## Do-while loops can be avoided.

Example: NA

_do-while_ loops are less readable than ordinary while loops and for loops since the conditional is at the bottom of the loop. The reader must scan the entire loop in order to understand the scope of the loop. In addition, _do-while_ loops are not needed. Any _do-while_ loop can easily be rewritten into a _while_ loop or a _for_ loop. Reducing the number of constructs enhances readability.

## The form while (true) should be used for infinite loops.

Example:
```cpp
while (true) {
//...
}
for (;;) { // NO!
//...
}
while (1) { // NO!
//...
}
```

Testing against 1 is neither necessary nor meaningful. The form for (;;) is not very readable, and it is not apparent that this is an infinite loop.

## Complex conditional expressions must be avoided. Introduce temporary boolean variables instead

Example:
```cpp
bool isFinished = (elementNo < 0) || (elementNo > maxElement);
bool isRepeatedEntry = elementNo == lastElement;
if ( isFinished || isRepeatedEntry) {
    //...
}

// NOT:
if ((elementNo < 0) || (elementNo > maxElement) ||
    elementNo == lastElement) {
    //...
}
```

By assigning boolean variables to expressions, the program gets automatic documentation. The construction will be easier to read, debug and maintain.

## The nominal case should be put in the if-part and the exception in the else-part of an if statement.

Example:
```cpp
bool isOk = readFile(fileName);
if (isOk) {
    //...
}
else {
    //...
}
```

Makes sure that the exceptions don't obscure the normal path of execution. This is important for both the readability and performance.

## The conditional should be put on a separate line.

Example:
```cpp
if (isDone) // NOT: if (isDone) doCleanup();
    doCleanup();
```

This is for debugging purposes. When writing on a single line, it is not apparent whether the test is true or not.

## Executable statements in conditionals must be avoided.

Example:
```cpp
File* fileHandle = open(fileName,"w");
if (!fileHandle) {
    //...
}
// NOT:
if (!(fileHandle = open(fileName,"w"))) {
    //...
}
```

Conditionals with executable statements are just very difficult to read. This is especially true for programmers new to C/C++.

## The use of magic numbers in the code should be avoided. Numbers other than 0 and 1 should be considered and declared as named constants instead.

Example: NA

If the number does not have an obvious meaning by itself, the readability is enhanced by introducing a named constant instead. A different approach is to introduce a method from which the constant can be accessed.

## Floating point constants should always be written with a digit before the decimal point.

Example: 
```cpp
double total = 0.5; // NOT: double total = .5;
```

The number and expression system in C++ is borrowed from mathematics and one should adhere to mathematical conventions for syntax wherever possible. Also, 0.5 is a lot more readable than .5; There is no way it can be mixed with the integer 5.

## Goto must not be used.

Example: NA

Goto statements violate the idea of structured code. Only in some very few cases (for instance breaking out of deeply nested structures) should goto be considered, and only if the alternative structured counterpart is proven to be less readable.

## nullptr should be used instead of NULL.

Example: NA

NULL is part of the standard C library, but is made obsolete in C++.