<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder -->
# Source Files

## C++ file extensions must be .h, .cpp, .cu or .cuh
- C++ header files must have the extension .h. Source files must have the extension .cpp. In Addition, we use .cu for cuda definition and .cuh for cuda header files.

Example:
```cpp
MyClass.cpp, MyClass.h
```

## A class should be declared in a header file and defined in a source file where the name of the files matches the name of the class.

Example:
```cpp
MyClass.h, MyClass.cpp
```

Makes it easy to find the associated files of a given class. An obvious exception are template classes that must be both declared and defined inside a .h file.

## All definitions should reside in source files.

Example:
```cpp
// MyClass.h
class MyClass
{
public :
    int getValue() {return value;} // NO!
    ...
private:
    int value;
}

// MyClass.cpp
int MyClass::getValue() {return value;} // YES!
```

The header files should declare an interface, the source file should implement it. When looking for an implementation, the programmer should always know that it is found in the source file.

## Header files must contain an include guard.

Example:
```cpp
#ifndef MODULE_CLASSNAME_H
#define MODULE_CLASSNAME_H
...
#endif // MODULE_CLASSNAME_H
```

The include guard avoids compilation errors. The name convention resembles the location of the file inside the source tree and prevents naming conflicts. The include guard should be named after the file name in uppercase letters with the extension replaced by _H. The name should be prefixed by the name of the module in uppercase letters.

## Include statements should be sorted and grouped in the following form
- they should be sorted by their hierarchical position in the system with low level files included first. Leave an empty line between groups of include statements.

We want to include self implemented files always starting with the library name. e.g.: `#include <lbm/constants/D3Q27.h>`

We distinguish between quotation marks ( "..." ) for self-implemented libraries and angle brackets ( \<...\> ) for external libraries.

Includes have to be grouped according to their abstraction (standard library / external libraries / internal libraries / same library)

Includes per group have to be in alphabetical order.

Example:
```cpp
// standard library
#include <iostream>
#include <string>

// external libraries
#include <cuda.h>
#include <cuda-runtime.h>

// internal libraries
#include <lbm/ChimeraTransformation.h>
#include <lbm/constants/D3Q27.h>

// same library
#include "cpu/core/BoundaryConditions/BCArray.h"
#include "cpu/core/BoundaryConditions/BCFunction.h"
```


## If possible include statements must be located in the cpp file and forward declarated in the header file.

Example:
```cpp
// Header file:
class B;

class A
{
public:
    A(B& b);
}

// Source file:
#include B.h

A::A(B& b) 
{
    //...
}
```