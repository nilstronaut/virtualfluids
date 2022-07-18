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
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//  for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with VirtualFluids (see COPYING.txt). If not, see <http://www.gnu.org/licenses/>.
//
//! \file LiggghtsCouplingCoProcessor.h
//! \ingroup LiggghtsCoupling
//! \author Konstantin Kutscher
//=======================================================================================

#ifndef LiggghtsCouplingCoProcessor_h
#define LiggghtsCouplingCoProcessor_h

#include "CoProcessor.h"

#include "lammps.h"
#include "input.h"
#include "atom.h"
#include "modify.h"

#include <memory>
#include <vector>


class CoProcessor;
class Communicator;
class LiggghtsCouplingWrapper;
class Grid3D;
class Block3D;
struct IBdynamicsParticleData;

class LiggghtsCouplingCoProcessor : public CoProcessor
{
public:
    LiggghtsCouplingCoProcessor(SPtr<Grid3D> grid, SPtr<UbScheduler> s, SPtr<Communicator> comm,
                                LiggghtsCouplingWrapper &wrapper, int demSteps);
    virtual ~LiggghtsCouplingCoProcessor();

    void process(double actualTimeStep) override;

    
protected:
    void setSpheresOnLattice();
    void getForcesFromLattice();
    void setSingleSphere3D(double *x, double *v, double *omega, /* double *com,*/ double r,
                           int id /*, bool initVelFlag*/);
    double calcSolidFraction(double const dx_, double const dy_, double const dz_, double const r_);

    void setValues(IBdynamicsParticleData &p, double const sf, double const dx, double const dy, double const dz,
                   double *omega, int id);

    void setToZero(IBdynamicsParticleData &p);

private:
    SPtr<Communicator> comm;
    LiggghtsCouplingWrapper &wrapper;
    int demSteps;
    std::vector<std::vector<SPtr<Block3D>>> blockVector;
    int minInitLevel;
    int maxInitLevel;
    int gridRank;
};

#endif

