//=======================================================================================
// ____          ____    __    ______     __________   __      __       __        __
// \    \       |    |  |  |  |   _   \  |___    ___| |  |    |  |     /  \      |  |
//  \    \      |    |  |  |  |  |_)   |     |  |     |  |    |  |    /    \     |  |
//   \    \     |    |  |  |  |   _   /      |  |     |  |    |  |   /  /\  \    |  |
//    \    \    |    |  |  |  |  | \  \      |  |     |   \__/   |  /  ____  \   |  |____
//     \    \   |    |  |__|  |__|  \__\     |__|      \________/  /__/    \__\  |_______|
//      \    \  |    |   ________________________________________________________________
//       \    \ |    |  |  ______________________________________________________________|
//        \    \|    |  |  |         __          __     __     __     ______      _______
//         \         |  |  |_____   |  |        |  |   |  |   |  |   |   _  \    /  _____)
//          \        |  |   _____|  |  |        |  |   |  |   |  |   |  | \  \   \_______
//           \       |  |  |        |  |_____   |   \_/   |   |  |   |  |_/  /    _____  |
//            \ _____|  |__|        |________|   \_______/    |__|   |______/    (_______/
//
//  This file is part of VirtualFluids. VirtualFluids is free software: you can
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  VirtualFluids is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
//  for more details.
//
//  SPDX-License-Identifier: GPL-3.0-or-later
//  SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder
//
//! \addtogroup gpu_utilities utilities
//! \ingroup gpu_GridGenerator GridGenerator
//! \{
//! \author Soeren Peters, Stephan Lenz
//=======================================================================================
#ifndef Transformator_h
#define Transformator_h

#include <memory>

#include "global.h"

class BoundingBox;
struct Triangle;
class TriangularMesh;
struct Vertex;


class Transformator
{
public:
    static std::shared_ptr<Transformator> makeTransformator(real delta, real dx, real dy, real dz);
    virtual ~Transformator() {}

protected:
    Transformator(){}

public:
    virtual void transformWorldToGrid(Triangle &value) const = 0;
    virtual void transformWorldToGrid(TriangularMesh &geom) const = 0;
    virtual void transformWorldToGrid(Vertex &value) const = 0;

    virtual void transformGridToWorld(Triangle &t) const = 0;
    virtual void transformGridToWorld(Vertex &value) const = 0;
    
    virtual void transformGridToWorld(BoundingBox &box) const = 0;
    virtual void transformWorldToGrid(BoundingBox &box) const = 0;

};


#endif

//! \}
