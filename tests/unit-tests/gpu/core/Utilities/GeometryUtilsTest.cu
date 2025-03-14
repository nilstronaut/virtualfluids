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
//! \addtogroup gpu_Utilities_tests Utilities
//! \ingroup gpu_core_tests core
//! \{
//! \author Martin Schoenherr
//=======================================================================================
#include <gpu/core/Utilities/GeometryUtils.h>

#include <basics/constants/NumericConstants.h>
#include <basics/tests/testUtilities.h>

TEST(GeometryUtilsTest, translate2D)
{
    real newPositionX;
    real newPositionY;

    translate2D(0., 0., newPositionX, newPositionY, 0., 0.);
    EXPECT_THAT(newPositionX, RealEq(0.));
    EXPECT_THAT(newPositionY, RealEq(0.));

    translate2D(0.5, 0.5, newPositionX, newPositionY, 1., 1.);
    EXPECT_THAT(newPositionX, RealEq(1.5));
    EXPECT_THAT(newPositionY, RealEq(1.5));

    translate2D(0.5, 0.5, newPositionX, newPositionY, -1., -1.);
    EXPECT_THAT(newPositionX, RealEq(-0.5));
    EXPECT_THAT(newPositionY, RealEq(-0.5));
}

TEST(GeometryUtilsTest, inverseTranslate2D)
{
    real newPositionX;
    real newPositionY;

    invTranslate2D(0., 0., newPositionX, newPositionY, 0., 0.);
    EXPECT_THAT(newPositionX, RealEq(0.));
    EXPECT_THAT(newPositionY, RealEq(0.));

    invTranslate2D(0.5, 0.5, newPositionX, newPositionY, 1., 1.);
    EXPECT_THAT(newPositionX, RealEq(-0.5));
    EXPECT_THAT(newPositionY, RealEq(-0.5));

    invTranslate2D(0.5, 0.5, newPositionX, newPositionY, -1., -1.);
    EXPECT_THAT(newPositionX, RealEq(1.5));
    EXPECT_THAT(newPositionY, RealEq(1.5));
}

TEST(GeometryUtilsTest, rotate2dAround0)
{
    auto posX = 2.0;
    auto posY = 0.0;
    real newPosX;
    real newPosY;

    auto angle = 0.0;
    rotate2D(angle, posX, posY, newPosX, newPosY);
    EXPECT_THAT(newPosX, RealNear(2.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(0.0, 10e-5));

    angle = 0.5 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY);
    EXPECT_THAT(newPosX, RealNear(0.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(2.0, 10e-5));

    angle = 1.0 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY);
    EXPECT_THAT(newPosX, RealNear(-2.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(0.0, 10e-5));

    angle = 1.5 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY);
    EXPECT_THAT(newPosX, RealNear(0.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-2.0, 10e-5));

    angle = 2.0 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY);
    EXPECT_THAT(newPosX, RealNear(2.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(0.0, 10e-5));

    angle = -0.5 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY);
    EXPECT_THAT(newPosX, RealNear(0.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-2.0, 10e-5));
}

TEST(GeometryUtilsTest, rotate2dWithOrigin)
{
    auto posX = 3.0;
    auto posY = -1.0;
    auto originX = 1.0;
    auto originY = -1.0;
    real newPosX;
    real newPosY;

    auto angle = 0.0;
    rotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(3.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-1.0, 10e-5));

    angle = 0.5 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(1.0, 10e-5));

    angle = 1.0 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(-1.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-1.0, 10e-5));

    angle = 1.5 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-3.0, 10e-5));

    angle = 2.0 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(3.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-1.0, 10e-5));

    angle = -0.5 * vf::basics::constant::cPi;
    rotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-3.0, 10e-5));
}

TEST(GeometryUtilsTest, inverseRotate2DWithOrigin)
{
    auto posX = 3.0;
    auto posY = -1.0;
    auto originX = 1.0;
    auto originY = -1.0;
    real newPosX;
    real newPosY;

    auto angle = 0.0;
    invRotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(3.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-1.0, 10e-5));

    angle = 0.5 * vf::basics::constant::cPi;
    invRotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-3.0, 10e-5));

    angle = 1.0 * vf::basics::constant::cPi;
    invRotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(-1.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-1.0, 10e-5));

    angle = 1.5 * vf::basics::constant::cPi;
    invRotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(1.0, 10e-5));

    angle = 2.0 * vf::basics::constant::cPi;
    invRotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(3.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-1.0, 10e-5));

    angle = -1.5 * vf::basics::constant::cPi;
    invRotate2D(angle, posX, posY, newPosX, newPosY, originX, originY);
    EXPECT_THAT(newPosX, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-3.0, 10e-5));
}

TEST(GeometryUtilsTest, rotateAboutX3dAround0)
{
    auto posX = 0.5;
    auto posY = 2.0;
    auto posZ = 0.0;
    real newPosX;
    real newPosY;
    real newPosZ;

    auto angle = 0.0;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(2.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(0.0, 10e-5));

    angle = 0.5 * vf::basics::constant::cPi;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(0.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(2.0, 10e-5));

    angle = 1.0 * vf::basics::constant::cPi;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-2.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(0.0, 10e-5));

    angle = 1.5 * vf::basics::constant::cPi;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(0.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-2.0, 10e-5));

    angle = 2.0 * vf::basics::constant::cPi;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(2.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(0.0, 10e-5));

    angle = -0.5 * vf::basics::constant::cPi;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(0.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-2.0, 10e-5));
}

TEST(GeometryUtilsTest, rotateAboutX3dWithOrigin)
{
    auto posX = 0.5;
    auto posY = 3.0;
    auto posZ = -1.0;
    auto originX = -0.75;
    auto originY = 1.0;
    auto originZ = -1.0;
    real newPosX;
    real newPosY;
    real newPosZ;

    auto angle = 0.0;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(3.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-1.0, 10e-5));

    angle = 0.5 * vf::basics::constant::cPi;    
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(1.0, 10e-5));

    angle = 1.0 * vf::basics::constant::cPi;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-1.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-1.0, 10e-5));

    angle = 1.5 * vf::basics::constant::cPi;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-3.0, 10e-5));

    angle = 2.0 * vf::basics::constant::cPi;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(3.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-1.0, 10e-5));

    angle = -0.5 * vf::basics::constant::cPi;
    rotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-3.0, 10e-5));
}

TEST(GeometryUtilsTest, inverseRotateAboutX3dWithOrigin)
{
    auto posX = 0.5;
    auto posY = 3.0;
    auto posZ = -1.0;
    auto originX = -0.75;
    auto originY = 1.0;
    auto originZ = -1.0;
    real newPosX;
    real newPosY;
    real newPosZ;

    auto angle = 0.0;
    invRotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(3.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-1.0, 10e-5));

    angle = 0.5 * vf::basics::constant::cPi;
    invRotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-3.0, 10e-5));

    angle = 1.0 * vf::basics::constant::cPi;
    invRotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(-1.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-1.0, 10e-5));

    angle = 1.5 * vf::basics::constant::cPi;    
    invRotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(1.0, 10e-5));

    angle = 2.0 * vf::basics::constant::cPi;
    invRotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(3.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(-1.0, 10e-5));

    angle = -0.5 * vf::basics::constant::cPi;
    invRotateAboutX3D(angle, posX, posY, posZ, newPosX, newPosY, newPosZ, originX, originY, originZ);
    EXPECT_THAT(newPosX, RealNear(0.5, 10e-5));
    EXPECT_THAT(newPosY, RealNear(1.0, 10e-5));
    EXPECT_THAT(newPosZ, RealNear(1.0, 10e-5));
}

TEST(GeometryUtilsTest, trilinearInterpolationX)
{
    const real dx = 0.5;
    const real dy = 0.0;
    const real dz = 0.0;
    const int kMMM = 0;
    const int kMMP = 0;
    const int kMPM = 0;
    const int kMPP = 0;
    const int kPMM = 1;
    const int kPMP = 1;
    const int kPPM = 1;
    const int kPPP = 1;
    const real quantity[2] = {0.0, 1.0};
    const real result = trilinearInterpolation(dx, dy, dz, kMMM, kPMM, kMPM, kMMP, kPPM, kPMP, kMPP, kPPP, quantity);
    EXPECT_THAT(result, RealNear(0.5, 10e-5));
}

TEST(GeometryUtilsTest, trilinearInterpolationY)
{
    const real dx = 0.0;
    const real dy = 0.3;
    const real dz = 0.0;
    const int kMMM = 0;
    const int kMMP = 0;
    const int kMPM = 1;
    const int kMPP = 1;
    const int kPMM = 0;
    const int kPMP = 0;
    const int kPPM = 1;
    const int kPPP = 1;
    const real quantity[2] = {0.0, 1.0};
    const real result = trilinearInterpolation(dx, dy, dz, kMMM, kPMM, kMPM, kMMP, kPPM, kPMP, kMPP, kPPP, quantity);
    EXPECT_THAT(result, RealNear(0.3, 10e-5));
}

TEST(GeometryUtilsTest, trilinearInterpolationZ)
{
    const real dx = 0.0;
    const real dy = 0.0;
    const real dz = 0.7;
    const int kMMM = 0;
    const int kMMP = 1;
    const int kMPM = 0;
    const int kMPP = 1;
    const int kPMM = 0;
    const int kPMP = 1;
    const int kPPM = 0;
    const int kPPP = 1;
    const real quantity[2] = {0.0, 2.0};
    const real result = trilinearInterpolation(dx, dy, dz, kMMM, kPMM, kMPM, kMMP, kPPM, kPMP, kMPP, kPPP, quantity);
    EXPECT_THAT(result, RealNear(1.4, 10e-5));
}

TEST(GeometryUtilsTest, trilinearInterpolationXYZ)
{
    const real dx = 0.5;
    const real dy = 0.5;
    const real dz = 0.5;
    const int kMMM = 0;
    const int kMMP = 1;
    const int kMPM = 0;
    const int kMPP = 1;
    const int kPMM = 0;
    const int kPMP = 1;
    const int kPPM = 0;
    const int kPPP = 1;
    const real quantity[2] = {0.0, 1.0};
    const real result = trilinearInterpolation(dx, dy, dz, kMMM, kPMM, kMPM, kMMP, kPPM, kPMP, kMPP, kPPP, quantity);
    EXPECT_THAT(result, RealNear(0.5, 10e-5));
}

TEST(GeometryUtilsTest, getNeighborIndices)
{
    const uint kMMM = 0;
    const uint neighborX[8] = {1, 100, 3, 100, 5, 100, 7, 100};
    const uint neighborY[8] = {2, 3, 200, 200, 6, 7, 200, 200};
    const uint neighborZ[8] = {4, 5, 6, 7, 300, 300, 300, 300};
    uint kPMM, kMPM, kMMP, kPPM, kPMP, kMPP, kPPP;
    getNeighborIndicesOfBSW(kMMM, kPMM, kMPM, kMMP, kPPM, kPMP, kMPP, kPPP, neighborX, neighborY, neighborZ);
}

TEST(GeometryUtilsTest, findNearestCell)
{
    const uint startIndex = 0;
    const real coordsX[8] = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
    const real coordsY[8] = {0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0};
    const real coordsZ[8] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};
    const real posX = 0.5, posY = 0.5, posZ = 0.5;
    const uint neighborX[8] = {1, 100, 3, 100, 5, 100, 7, 100};
    const uint neighborY[8] = {2, 3, 200, 200, 6, 7, 200, 200};
    const uint neighborZ[8] = {4, 5, 6, 7, 300, 300, 300, 300};
    const uint neighborBSW[8] = {400, 400, 400, 400, 400, 400, 400, 0};
    const uint result = findNearestCellBSW(startIndex, coordsX, coordsY, coordsZ, posX, posY, posZ, neighborX, neighborY, neighborZ, neighborBSW);
    EXPECT_THAT(result, 0);
}
//! \}
