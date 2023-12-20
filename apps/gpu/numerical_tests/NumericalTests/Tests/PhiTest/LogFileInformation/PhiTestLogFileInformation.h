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
//! \addtogroup NumericalTests
//! \ingroup numerical_tests
//! \{
//=======================================================================================
#ifndef PHI_TEST_LOGFILE_INFORMATION_H
#define PHI_TEST_LOGFILE_INFORMATION_H

#include "Utilities/LogFileInformation/TestLogFileInformation/TestLogFileInformation.h"
#include "Utilities/Test/TestStatus.h"

#include <memory>
#include <vector>

class PhiTest;
struct PhiTestParameterStruct;

class PhiTestLogFileInformation : public TestLogFileInformation
{
public:
    static std::shared_ptr<PhiTestLogFileInformation> getNewInstance(std::shared_ptr<PhiTestParameterStruct> testPara);

    std::string getOutput();
    void addTestGroup(std::vector<std::shared_ptr<PhiTest> > tests);

private:
    PhiTestLogFileInformation() {};
    PhiTestLogFileInformation(std::shared_ptr<PhiTestParameterStruct> testPara);

    void fillMyData(std::vector<std::shared_ptr<PhiTest> > testGroup);

    std::vector<std::vector<std::shared_ptr<PhiTest> > > testGroups;
    unsigned int startTimeStepCalculation, endTimeStepCalculation;
    std::vector<int> lx;
    std::vector<int> lxForErase;
    std::vector<double> phiDiff;
    std::vector<double> orderOfAccuracy;
    std::vector<std::string> dataToCalc;
    std::vector<TestStatus> status;
};
#endif
//! \}
