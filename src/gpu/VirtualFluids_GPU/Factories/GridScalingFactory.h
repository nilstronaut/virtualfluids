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
//  You should have received a copy of the GNU General Public License along
//  with VirtualFluids (see COPYING.txt). If not, see <http://www.gnu.org/licenses/>.
//
//! \file GridScalingFactory.h
//! \ingroup Factories
//! \author Anna Wellmann, Martin Schönherr
//=======================================================================================
#ifndef GS_FACTORY
#define GS_FACTORY

#include <functional>

#include "LBM/LB.h"
#include "Parameter/Parameter.h"

struct LBMSimulationParameter;
class Parameter;
struct CUstream_st;

using gridScalingFC = std::function<void(LBMSimulationParameter *, LBMSimulationParameter *, ICellFC *, CUstream_st *stream)>;
using gridScalingCF = std::function<void(LBMSimulationParameter *, LBMSimulationParameter *, ICellCF *, OffCF, CUstream_st *stream)>;

class GridScalingFactory
{
public:
    //! \brief An enumeration for selecting a scaling function
    enum class GridScaling {
        //! - ScaleK17 = scaling for cumulant K17 kernel
        ScaleK17,
        //! - DEPRECATED: ScaleRhoSq = scaling for cumulant kernel rho squared
        ScaleRhoSq,
        NotSpecified
    };

    void setScalingFactory(const GridScalingFactory::GridScaling gridScalingType);

    [[nodiscard]] gridScalingFC getGridScalingFC() const;
    [[nodiscard]] gridScalingCF getGridScalingCF() const;

private:
    GridScaling gridScaling = GridScaling::NotSpecified;
};

#endif
