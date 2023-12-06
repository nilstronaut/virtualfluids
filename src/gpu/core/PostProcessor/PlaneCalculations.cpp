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
//=======================================================================================
#include "PlaneCalculations.h"

#include <cuda_runtime.h>
#include <helper_cuda.h>

#include <cstdio>
#include <fstream>
#include <sstream>

#include <basics/StringUtilities/StringUtil.h>

#include "Cuda/CudaMemoryManager.h"
#include "Parameter/Parameter.h"

//////////////////////////////////////////////////////////////////////////
//advection + diffusion
//////////////////////////////////////////////////////////////////////////

void calcPlaneConc(Parameter* para, CudaMemoryManager* cudaMemoryManager, int lev)
{
    //////////////////////////////////////////////////////////////////////////
    //copy to host
    //coarsest Grid ... with the pressure nodes
    //please test -> Copy == Alloc ??
    ////////////////////////////////////////////
    //Version Press neighbor
    unsigned int NoNin   = para->getParH(lev)->numberOfPointsCpTop;
    unsigned int NoNout1 = para->getParH(lev)->numberOfPointsCpBottom;
    unsigned int NoNout2 = para->getParH(lev)->pressureBC.numberOfBCnodes;
    ////////////////////////////////////////////
    ////Version cp top
    //unsigned int NoN = para->getParH(lev)->numberOfPointsCpTop;
    ////////////////////////////////////////////
    ////Version cp bottom
    //unsigned int NoN = para->getParH(lev)->numberOfPointsCpBottom;

    cudaMemoryManager->cudaCopyPlaneConcIn(lev, NoNin);
    cudaMemoryManager->cudaCopyPlaneConcOut1(lev, NoNout1);
    cudaMemoryManager->cudaCopyPlaneConcOut2(lev, NoNout2);
    ////////////////////////////////////////////
    //calculate concentration
    double concPlaneIn = 0.;
    double concPlaneOut1 = 0.;
    double concPlaneOut2 = 0.;
    ////////////////////////////////////////////
    double counter1 = 0.;
    for (unsigned int it = 0; it < NoNin; it++)
    {
        if (para->getParH(lev)->typeOfGridNode[it] == GEO_FLUID)
        {
            concPlaneIn   += (double) (para->getParH(lev)->ConcPlaneIn[it]);
            counter1 += 1.;
        }
    }
    concPlaneIn /= (double)(counter1);
    ////////////////////////////////////////////
    counter1 = 0.;
    for (unsigned int it = 0; it < NoNout1; it++)
    {
        if (para->getParH(lev)->typeOfGridNode[it] == GEO_FLUID)
        {
            concPlaneOut1 += (double) (para->getParH(lev)->ConcPlaneOut1[it]);
            counter1 += 1.;
        }
    }
    concPlaneOut1 /= (double)(counter1);
    ////////////////////////////////////////////
    counter1 = 0.;
    for (unsigned int it = 0; it < NoNout2; it++)
    {
        if (para->getParH(lev)->typeOfGridNode[it] == GEO_FLUID)
        {
            concPlaneOut2 += (double) (para->getParH(lev)->ConcPlaneOut2[it]);
            counter1 += 1.;
        }
    }
    concPlaneOut2 /= (double)(counter1);
    ////////////////////////////////////////////
    //concPlaneIn /= (double)(NoN);
    //concPlaneOut1 /= (double)(NoN);
    //concPlaneOut2 /= (double)(NoN);
    //////////////////////////////////////////////////////////////////////////
    //Copy to vector x,y,z
    para->getParH(lev)->PlaneConcVectorIn.push_back(concPlaneIn);
    para->getParH(lev)->PlaneConcVectorOut1.push_back(concPlaneOut1);
    para->getParH(lev)->PlaneConcVectorOut2.push_back(concPlaneOut2);
    //////////////////////////////////////////////////////////////////////////
}



void allocPlaneConc(Parameter* para, CudaMemoryManager* cudaMemoryManager)
{
    //////////////////////////////////////////////////////////////////////////
    //set level   ---> maybe we need a loop
    int lev = para->getCoarse();
    //////////////////////////////////////////////////////////////////////////
    //allocation
    //coarsest Grid ... with the pressure nodes
    //please test -> Copy == Alloc ??
    ////////////////////////////////////////////
    //Version Press neighbor
    cudaMemoryManager->cudaAllocPlaneConcIn(lev, para->getParH(lev)->numberOfPointsCpTop);
    cudaMemoryManager->cudaAllocPlaneConcOut1(lev, para->getParH(lev)->numberOfPointsCpBottom);
    cudaMemoryManager->cudaAllocPlaneConcOut2(lev, para->getParH(lev)->pressureBC.numberOfBCnodes);
    printf("\n Number of elements plane concentration = %d + %d + %d \n", para->getParH(lev)->numberOfPointsCpTop, para->getParH(lev)->numberOfPointsCpBottom, para->getParH(lev)->pressureBC.numberOfBCnodes);
    ////////////////////////////////////////////
    ////Version cp top
    //para->cudaAllocPlaneConc(lev, para->getParH(lev)->numberOfPointsCpTop);
    //printf("\n Number of elements plane concentration = %d \n", para->getParH(lev)->numberOfPointsCpTop);
    ////////////////////////////////////////////
    ////Version cp bottom
    //para->cudaAllocPlaneConc(lev, para->getParH(lev)->numberOfPointsCpBottom);
    //printf("\n Number of elements plane concentration = %d \n", para->getParH(lev)->numberOfPointsCpBottom);
    //////////////////////////////////////////////////////////////////////////
}



void printPlaneConc(Parameter* para, CudaMemoryManager* cudaMemoryManager)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //set level   ---> maybe we need a loop
    int lev = para->getCoarse();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //set filename
    std::string ffnameIn = para->getFName() + std::to_string(para->getMyProcessID()) + "_" + "In" + "_PlaneConc.txt";
    const char* fnameIn = ffnameIn.c_str();
    //////////////////////////////////////////////////////////////////////////
    //set ofstream
    std::ofstream ostrIn;
    //////////////////////////////////////////////////////////////////////////
    //open file
    ostrIn.open(fnameIn);
    //////////////////////////////////////////////////////////////////////////
    //fill file with data
    for (size_t i = 0; i < para->getParH(lev)->PlaneConcVectorIn.size(); i++)
    {
        ostrIn << para->getParH(lev)->PlaneConcVectorIn[i]  << std::endl ;
    }
    //////////////////////////////////////////////////////////////////////////
    //close file
    ostrIn.close();
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //set filename
    std::string ffnameOut1 = para->getFName() + std::to_string(para->getMyProcessID()) + "_" + "Out1" + "_PlaneConc.txt";
    const char* fnameOut1 = ffnameOut1.c_str();
    //////////////////////////////////////////////////////////////////////////
    //set ofstream
    std::ofstream ostrOut1;
    //////////////////////////////////////////////////////////////////////////
    //open file
    ostrOut1.open(fnameOut1);
    //////////////////////////////////////////////////////////////////////////
    //fill file with data
    for (size_t i = 0; i < para->getParH(lev)->PlaneConcVectorOut1.size(); i++)
    {
        ostrOut1 << para->getParH(lev)->PlaneConcVectorOut1[i]  << std::endl ;
    }
    //////////////////////////////////////////////////////////////////////////
    //close file
    ostrOut1.close();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //set filename
    std::string ffnameOut2 = para->getFName() + std::to_string(para->getMyProcessID()) + "_" + "Out2" + "_PlaneConc.txt";
    const char* fnameOut2 = ffnameOut2.c_str();
    //////////////////////////////////////////////////////////////////////////
    //set ofstream
    std::ofstream ostrOut2;
    //////////////////////////////////////////////////////////////////////////
    //open file
    ostrOut2.open(fnameOut2);
    //////////////////////////////////////////////////////////////////////////
    //fill file with data
    for (size_t i = 0; i < para->getParH(lev)->PlaneConcVectorOut2.size(); i++)
    {
        ostrOut2 << para->getParH(lev)->PlaneConcVectorOut2[i]  << std::endl ;
    }
    //////////////////////////////////////////////////////////////////////////
    //close file
    ostrOut2.close();
    //////////////////////////////////////////////////////////////////////////
    cudaMemoryManager->cudaFreePlaneConc(lev);
    //////////////////////////////////////////////////////////////////////////
}





//////////////////////////////////////////////////////////////////////////
//Print Test round of Error
void printRE(Parameter* para, CudaMemoryManager* cudaMemoryManager, int timestep)
{
    //////////////////////////////////////////////////////////////////////////
    //set level
    int lev = 0;
    //////////////////////////////////////////////////////////////////////////
    //set filename
    std::string ffname = para->getFName()+StringUtil::toString<int>(para->getMyProcessID())+"_"+StringUtil::toString<int>(timestep)+"_RE.txt";
    const char* fname = ffname.c_str();
    //////////////////////////////////////////////////////////////////////////
    //set ofstream
    std::ofstream ostr;
    //////////////////////////////////////////////////////////////////////////
    //open file
    ostr.open(fname);
    //////////////////////////////////////////////////////////////////////////
    //fill file with data
    bool doNothing = false;
    for (unsigned int i = 0; i < para->getParH(lev)->pressureBC.numberOfBCnodes; i++)
    {
        doNothing = false;
        for (std::size_t j = 0; j < 27; j++)
        {
            if (para->getParH(lev)->kDistTestRE.f[0][j*para->getParH(lev)->pressureBC.numberOfBCnodes + i]==0)
            {
                doNothing = true;
                continue;
            }
            ostr << para->getParH(lev)->kDistTestRE.f[0][j*para->getParH(lev)->pressureBC.numberOfBCnodes + i]  << "\t";
        }
        if (doNothing==true)
        {
            continue;
        }
        ostr << std::endl;
    }
    //////////////////////////////////////////////////////////////////////////
    //close file
    ostr.close();
    //////////////////////////////////////////////////////////////////////////
    if (timestep == (int)para->getTimestepEnd())
    {
        cudaMemoryManager->cudaFreeTestRE(lev);
    }
    //////////////////////////////////////////////////////////////////////////
}
