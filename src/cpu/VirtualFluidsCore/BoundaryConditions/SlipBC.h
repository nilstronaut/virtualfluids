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
//! \file SlipBC.h
//! \ingroup BoundarConditions
//! \author Soeren Freudiger
//=======================================================================================
#ifndef SlipBC_H
#define SlipBC_H

#include "BC.h"

/*=======================================================*/
// D3Q27SlipBCCreator
// class D3Q27SlipBCCreator : public ObObjectCreator
//{
// public:
//   static D3Q27SlipBCCreator* getInstance()
//   {
//      static D3Q27SlipBCCreator instance;
//      return &instance;
//   }
//
//   ObObject* createObObject();
//
//   std::string getTypeID() { return "D3Q27SlipBC";}
//   std::string toString()  { return "D3Q27SlipBCCreator"; }
//
// private:
//   D3Q27SlipBCCreator( const D3Q27SlipBCCreator& );                  //no copy allowed
//   const D3Q27SlipBCCreator& operator=( const D3Q27SlipBCCreator& ); //no copy allowed
//   D3Q27SlipBCCreator() : ObObjectCreator() {}
//};
//

class SlipBC : public BC
{
public:
    SlipBC() : BC() {}
    SlipBC(const short &secondaryBcOption) : BC(secondaryBcOption) {}

    //------------- implements D3Q27BoundaryConditionAdapter ----- start

    void init(const D3Q27Interactor *const &interactor, const real &timestep = 0) override {}
    void update(const D3Q27Interactor *const &interactor, const real &timestep = 0) override {}

    void adaptBCForDirection(const D3Q27Interactor & /*interactor*/, SPtr<BoundaryConditions> bc,
                             const real & /*worldX1*/, const real & /*worldX2*/, const real & /*worldX3*/,
                             const real &q, const int &fdirection, const real & /*time*/ = 0) override
    {
        bc->setSlipBoundaryFlag(D3Q27System::INVDIR[fdirection], secondaryBcOption);
        bc->setQ((real)q, fdirection);
    }
    void adaptBC(const D3Q27Interactor &interactor, SPtr<BoundaryConditions> bc, const real &worldX1,
                 const real &worldX2, const real &worldX3, const real &time = 0) override;

    //------------- implements D3Q27BoundaryConditionAdapter ----- end

private:
};

#endif
