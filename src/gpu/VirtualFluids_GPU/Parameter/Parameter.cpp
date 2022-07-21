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
//! \file Parameter.h
//! \ingroup Parameter
//! \author Martin Schoenherr
//=======================================================================================
#include "Parameter.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <curand_kernel.h>

#include "Core/StringUtilities/StringUtil.h"

#include <basics/config/ConfigurationFile.h>

#include "Parameter/CudaStreamManager.h"

Parameter::Parameter(int numberOfProcesses, int myId)
{
    this->ic.numprocs = numberOfProcesses; 
    this->ic.myid = myId;
    
    initGridPaths();
    initGridBasePoints();
    initDefaultLBMkernelAllLevels();
    this->setPathAndFilename(this->getOutputPath() + this->getOutputPrefix());
    this->setQuadricLimiters(0.01, 0.01, 0.01);
    this->setForcing(0.0, 0.0, 0.0);

    // initLBMSimulationParameter();
}

Parameter::Parameter(const vf::basics::ConfigurationFile &configData, int numberOfProcesses, int myId)
{
    this->ic.numprocs = numberOfProcesses; 
    this->ic.myid = myId;

    readConfigData(configData);

    initGridPaths();
    initGridBasePoints();
    initDefaultLBMkernelAllLevels();
    this->setPathAndFilename(this->getOutputPath() + this->getOutputPrefix());

    // initLBMSimulationParameter();
}

Parameter::~Parameter() = default;

void Parameter::readConfigData(const vf::basics::ConfigurationFile &configData)
{
    if (configData.contains("NumberOfDevices"))
        this->setMaxDev(configData.getValue<int>("NumberOfDevices"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Devices"))
        this->setDevices(configData.getVector<uint>("Devices"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Path"))
        this->setOutputPath(configData.getValue<std::string>("Path"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Prefix"))
        this->setOutputPrefix(configData.getValue<std::string>("Prefix"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("WriteGrid"))
        this->setPrintFiles(configData.getValue<bool>("WriteGrid"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("GeometryValues"))
        this->setUseGeometryValues(configData.getValue<bool>("GeometryValues"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("calc2ndOrderMoments"))
        this->setCalc2ndOrderMoments(configData.getValue<bool>("calc2ndOrderMoments"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("calc3rdOrderMoments"))
        this->setCalc3rdOrderMoments(configData.getValue<bool>("calc3rdOrderMoments"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("calcHigherOrderMoments"))
        this->setCalcHighOrderMoments(configData.getValue<bool>("calcHigherOrderMoments"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("calcMedian"))
        this->setCalcMedian(configData.getValue<bool>("calcMedian"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("calcCp"))
        this->calcCp = configData.getValue<bool>("calcCp");
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("calcDrafLift"))
        this->calcDragLift = configData.getValue<bool>("calcDrafLift");
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("writeVeloASCIIfiles"))
        this->writeVeloASCII = configData.getValue<bool>("writeVeloASCIIfiles");
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("calcPlaneConc"))
        this->calcPlaneConc = configData.getValue<bool>("calcPlaneConc");
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("UseConcFile"))
        this->setConcFile(configData.getValue<bool>("UseConcFile"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("UseStreetVelocityFile"))
        this->setStreetVelocityFile(configData.getValue<bool>("UseStreetVelocityFile"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("UseMeasurePoints"))
        this->setUseMeasurePoints(configData.getValue<bool>("UseMeasurePoints"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("UseWale"))
        this->setUseWale(configData.getValue<bool>("UseWale"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("UseAMD"))
        this->setUseAMD(configData.getValue<bool>("UseAMD"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("SGSconstant"))
        this->setSGSConstant(configData.getValue<real>("SGSconstant"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("UseInitNeq"))
        this->setUseInitNeq(configData.getValue<bool>("UseInitNeq"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("SimulatePorousMedia"))
        this->setSimulatePorousMedia(configData.getValue<bool>("SimulatePorousMedia"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("D3Qxx"))
        this->setD3Qxx(configData.getValue<int>("D3Qxx"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("TimeEnd"))
        this->setTimestepEnd(configData.getValue<int>("TimeEnd"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("TimeOut"))
        this->setTimestepOut(configData.getValue<int>("TimeOut"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("TimeStartOut"))
        this->setTimestepStartOut(configData.getValue<int>("TimeStartOut"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("TimeStartCalcMedian"))
        this->setTimeCalcMedStart(configData.getValue<int>("TimeStartCalcMedian"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("TimeEndCalcMedian"))
        this->setTimeCalcMedEnd(configData.getValue<int>("TimeEndCalcMedian"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("PressInID"))
        this->setPressInID(configData.getValue<int>("PressInID"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("PressOutID"))
        this->setPressOutID(configData.getValue<int>("PressOutID"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("PressInZ"))
        this->setPressInZ(configData.getValue<int>("PressInZ"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("PressOutZ"))
        this->setPressOutZ(configData.getValue<int>("PressOutZ"));

    //////////////////////////////////////////////////////////////////////////
    // second component
    if (configData.contains("DiffOn"))
        this->setDiffOn(configData.getValue<bool>("DiffOn"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("DiffMod"))
        this->setDiffMod(configData.getValue<int>("DiffMod"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Diffusivity"))
        this->setDiffusivity(configData.getValue<real>("Diffusivity"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Temp"))
        this->setTemperatureInit(configData.getValue<real>("Temp"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("TempBC"))
        this->setTemperatureBC(configData.getValue<real>("TempBC"));

    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Viscosity_LB"))
        this->setViscosityLB(configData.getValue<real>("Viscosity_LB"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Velocity_LB"))
        this->setVelocityLB(configData.getValue<real>("Velocity_LB"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Viscosity_Ratio_World_to_LB"))
        this->setViscosityRatio(configData.getValue<real>("Viscosity_Ratio_World_to_LB"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Velocity_Ratio_World_to_LB"))
        this->setVelocityRatio(configData.getValue<real>("Velocity_Ratio_World_to_LB"));
    // //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Density_Ratio_World_to_LB"))
        this->setDensityRatio(configData.getValue<real>("Density_Ratio_World_to_LB"));

    if (configData.contains("Delta_Press"))
        this->setPressRatio(configData.getValue<real>("Delta_Press"));

    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("SliceRealX"))
        this->setRealX(configData.getValue<real>("SliceRealX"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("SliceRealY"))
        this->setRealY(configData.getValue<real>("SliceRealY"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("FactorPressBC"))
        this->setFactorPressBC(configData.getValue<real>("FactorPressBC"));

    //////////////////////////////////////////////////////////////////////////
    // CUDA streams and optimized communication
    if (this->getNumprocs() > 1) {
        if (configData.contains("useStreams")) {
            if (configData.getValue<bool>("useStreams"))
                this->setUseStreams(true);
        }

        if (configData.contains("useReducedCommunicationInInterpolation")) {
            this->useReducedCommunicationAfterFtoC =
                configData.getValue<bool>("useReducedCommunicationInInterpolation");
        }
    }
    //////////////////////////////////////////////////////////////////////////

    // read Geometry (STL)
    if (configData.contains("ReadGeometry"))
        this->setReadGeo(configData.getValue<bool>("ReadGeometry"));

    if (configData.contains("GeometryC"))
        this->setGeometryFileC(configData.getValue<std::string>("GeometryC"));
    else if (this->getReadGeo())
        throw std::runtime_error("readGeo is true, GeometryC has to be set as well!");

    if (configData.contains("GeometryM"))
        this->setGeometryFileM(configData.getValue<std::string>("GeometryM"));
    else if (this->getReadGeo())
        throw std::runtime_error("readGeo is true, GeometryM has to be set as well!");

    if (configData.contains("GeometryF"))
        this->setGeometryFileF(configData.getValue<std::string>("GeometryF"));
    else if (this->getReadGeo())
        throw std::runtime_error("readGeo is true, GeometryF has to be set as well!");

    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("measureClockCycle"))
        this->setclockCycleForMP(configData.getValue<real>("measureClockCycle"));

    if (configData.contains("measureTimestep"))
        this->settimestepForMP(configData.getValue<uint>("measureTimestep"));

    //////////////////////////////////////////////////////////////////////////

    if (configData.contains("GridPath"))
        this->setGridPath(configData.getValue<std::string>("GridPath"));

    // Forcing
    real forcingX = 0.0;
    real forcingY = 0.0;
    real forcingZ = 0.0;

    if (configData.contains("ForcingX"))
        forcingX = configData.getValue<real>("ForcingX");
    if (configData.contains("ForcingY"))
        forcingY = configData.getValue<real>("ForcingY");
    if (configData.contains("ForcingZ"))
        forcingZ = configData.getValue<real>("ForcingZ");

    this->setForcing(forcingX, forcingY, forcingZ);
    //////////////////////////////////////////////////////////////////////////
    // quadricLimiters
    real quadricLimiterP = (real)0.01;
    real quadricLimiterM = (real)0.01;
    real quadricLimiterD = (real)0.01;

    if (configData.contains("QuadricLimiterP"))
        quadricLimiterP = configData.getValue<real>("QuadricLimiterP");
    if (configData.contains("QuadricLimiterM"))
        quadricLimiterM = configData.getValue<real>("QuadricLimiterM");
    if (configData.contains("QuadricLimiterD"))
        quadricLimiterD = configData.getValue<real>("QuadricLimiterD");

    this->setQuadricLimiters(quadricLimiterP, quadricLimiterM, quadricLimiterD);
    //////////////////////////////////////////////////////////////////////////
    // Particles
    if (configData.contains("calcParticles"))
        this->setCalcParticles(configData.getValue<bool>("calcParticles"));

    if (configData.contains("baseLevel"))
        this->setParticleBasicLevel(configData.getValue<int>("baseLevel"));

    if (configData.contains("initLevel"))
        this->setParticleInitLevel(configData.getValue<int>("initLevel"));

    if (configData.contains("numberOfParticles"))
        this->setNumberOfParticles(configData.getValue<int>("numberOfParticles"));

    if (configData.contains("startXHotWall"))
        this->setStartXHotWall(configData.getValue<real>("startXHotWall"));

    if (configData.contains("endXHotWall"))
        this->setEndXHotWall(configData.getValue<real>("endXHotWall"));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Restart
    if (configData.contains("TimeDoCheckPoint"))
        this->setTimeDoCheckPoint(configData.getValue<uint>("TimeDoCheckPoint"));

    if (configData.contains("TimeDoRestart"))
        this->setTimeDoRestart(configData.getValue<uint>("TimeDoRestart"));

    if (configData.contains("DoCheckPoint"))
        this->setDoCheckPoint(configData.getValue<bool>("DoCheckPoint"));

    if (configData.contains("DoRestart"))
        this->setDoRestart(configData.getValue<bool>("DoRestart"));
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (configData.contains("NOGL"))
        setMaxLevel(configData.getValue<int>("NOGL"));

    if (configData.contains("GridX"))
        this->setGridX(configData.getVector<int>("GridX"));

    if (configData.contains("GridY"))
        this->setGridY(configData.getVector<int>("GridY"));

    if (configData.contains("GridZ"))
        this->setGridZ(configData.getVector<int>("GridZ"));

    if (configData.contains("DistX"))
        this->setDistX(configData.getVector<int>("DistX"));

    if (configData.contains("DistY"))
        this->setDistY(configData.getVector<int>("DistY"));

    if (configData.contains("DistZ"))
        this->setDistZ(configData.getVector<int>("DistZ"));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Kernel
    if (configData.contains("MainKernelName"))
        this->setMainKernel(configData.getValue<std::string>("MainKernelName"));

    if (configData.contains("MultiKernelOn"))
        this->setMultiKernelOn(configData.getValue<bool>("MultiKernelOn"));

    if (configData.contains("MultiKernelLevel"))
        this->setMultiKernelLevel(configData.getVector<int>("MultiKernelLevel"));

    if (configData.contains("MultiKernelName"))
        this->setMultiKernel(configData.getVector<std::string>("MultiKernelName"));
}

void Parameter::initGridPaths(){
    std::string gridPath = this->getGridPath();

    // add missing slash to gridPath
    if (gridPath.back() != '/') {
        gridPath += "/";
        ic.gridPath = gridPath;
    }

    // for multi-gpu add process id (if not already there)
    if (this->getNumprocs() > 1) {
        gridPath += StringUtil::toString(this->getMyID()) + "/";
        ic.gridPath = gridPath;
    }

    //////////////////////////////////////////////////////////////////////////
        
    this->setgeoVec(gridPath + "geoVec.dat");
    this->setcoordX(gridPath + "coordX.dat");
    this->setcoordY(gridPath + "coordY.dat");
    this->setcoordZ(gridPath + "coordZ.dat");
    this->setneighborX(gridPath + "neighborX.dat");
    this->setneighborY(gridPath + "neighborY.dat");
    this->setneighborZ(gridPath + "neighborZ.dat");
    this->setneighborWSB(gridPath + "neighborWSB.dat");
    this->setscaleCFC(gridPath + "scaleCFC.dat");
    this->setscaleCFF(gridPath + "scaleCFF.dat");
    this->setscaleFCC(gridPath + "scaleFCC.dat");
    this->setscaleFCF(gridPath + "scaleFCF.dat");
    this->setscaleOffsetCF(gridPath + "offsetVecCF.dat");
    this->setscaleOffsetFC(gridPath + "offsetVecFC.dat");
    this->setgeomBoundaryBcQs(gridPath + "geomBoundaryQs.dat");
    this->setgeomBoundaryBcValues(gridPath + "geomBoundaryValues.dat");
    this->setinletBcQs(gridPath + "inletBoundaryQs.dat");
    this->setinletBcValues(gridPath + "inletBoundaryValues.dat");
    this->setoutletBcQs(gridPath + "outletBoundaryQs.dat");
    this->setoutletBcValues(gridPath + "outletBoundaryValues.dat");
    this->settopBcQs(gridPath + "topBoundaryQs.dat");
    this->settopBcValues(gridPath + "topBoundaryValues.dat");
    this->setbottomBcQs(gridPath + "bottomBoundaryQs.dat");
    this->setbottomBcValues(gridPath + "bottomBoundaryValues.dat");
    this->setfrontBcQs(gridPath + "frontBoundaryQs.dat");
    this->setfrontBcValues(gridPath + "frontBoundaryValues.dat");
    this->setbackBcQs(gridPath + "backBoundaryQs.dat");
    this->setbackBcValues(gridPath + "backBoundaryValues.dat");
    this->setnumberNodes(gridPath + "numberNodes.dat");
    this->setLBMvsSI(gridPath + "LBMvsSI.dat");
    this->setmeasurePoints(gridPath + "measurePoints.dat");
    this->setpropellerValues(gridPath + "propellerValues.dat");
    this->setcpTop(gridPath + "cpTop.dat");
    this->setcpBottom(gridPath + "cpBottom.dat");
    this->setcpBottom2(gridPath + "cpBottom2.dat");
    this->setConcentration(gridPath + "conc.dat");
    this->setStreetVelocity(gridPath + "streetVector.dat");
    
    //////////////////////////////////////////////////////////////////////////
    // Normals - Geometry
    this->setgeomBoundaryNormalX(gridPath + "geomBoundaryNormalX.dat");
    this->setgeomBoundaryNormalY(gridPath + "geomBoundaryNormalY.dat");
    this->setgeomBoundaryNormalZ(gridPath + "geomBoundaryNormalZ.dat");
    // Normals - Inlet
    this->setInflowBoundaryNormalX(gridPath + "inletBoundaryNormalX.dat");
    this->setInflowBoundaryNormalY(gridPath + "inletBoundaryNormalY.dat");
    this->setInflowBoundaryNormalZ(gridPath + "inletBoundaryNormalZ.dat");
    // Normals - Outlet
    this->setOutflowBoundaryNormalX(gridPath + "outletBoundaryNormalX.dat");
    this->setOutflowBoundaryNormalY(gridPath + "outletBoundaryNormalY.dat");
    this->setOutflowBoundaryNormalZ(gridPath + "outletBoundaryNormalZ.dat");
    //////////////////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////////////////
    // for Multi GPU
    if (this->getNumprocs() > 1) {
        
        // 3D domain decomposition
        std::vector<std::string> sendProcNeighborsX, sendProcNeighborsY, sendProcNeighborsZ;
        std::vector<std::string> recvProcNeighborsX, recvProcNeighborsY, recvProcNeighborsZ;
        for (int i = 0; i < this->getNumprocs(); i++) {
            sendProcNeighborsX.push_back(gridPath + StringUtil::toString(i) + "Xs.dat");
            sendProcNeighborsY.push_back(gridPath + StringUtil::toString(i) + "Ys.dat");
            sendProcNeighborsZ.push_back(gridPath + StringUtil::toString(i) + "Zs.dat");
            recvProcNeighborsX.push_back(gridPath + StringUtil::toString(i) + "Xr.dat");
            recvProcNeighborsY.push_back(gridPath + StringUtil::toString(i) + "Yr.dat");
            recvProcNeighborsZ.push_back(gridPath + StringUtil::toString(i) + "Zr.dat");
        }
        this->setPossNeighborFilesX(sendProcNeighborsX, "send");
        this->setPossNeighborFilesY(sendProcNeighborsY, "send");
        this->setPossNeighborFilesZ(sendProcNeighborsZ, "send");
        this->setPossNeighborFilesX(recvProcNeighborsX, "recv");
        this->setPossNeighborFilesY(recvProcNeighborsY, "recv");
        this->setPossNeighborFilesZ(recvProcNeighborsZ, "recv");
    
    //////////////////////////////////////////////////////////////////////////
    }
}

void Parameter::initGridBasePoints()
{
    if (this->getGridX().empty())
        this->setGridX(std::vector<int>(this->getMaxLevel() + 1, 32));
    if (this->getGridY().empty())
        this->setGridY(std::vector<int>(this->getMaxLevel() + 1, 32));
    if (this->getGridZ().empty())
        this->setGridZ(std::vector<int>(this->getMaxLevel() + 1, 32));

    if (this->getDistX().empty())
        this->setDistX(std::vector<int>(this->getMaxLevel() + 1, 32));
    if (this->getDistY().empty())
        this->setDistY(std::vector<int>(this->getMaxLevel() + 1, 32));
    if (this->getDistZ().empty())
        this->setDistZ(std::vector<int>(this->getMaxLevel() + 1, 32));
}

void Parameter::initDefaultLBMkernelAllLevels(){
    if (this->getMultiKernelOn() && this->getMultiKernelLevel().empty()) {
        std::vector<int> tmp;
        for (int i = 0; i < this->getMaxLevel() + 1; i++) {
            tmp.push_back(i);
        }
        this->setMultiKernelLevel(tmp);
    }
    
    if (this->getMultiKernelOn() && this->getMultiKernel().empty()) {
        std::vector<std::string> tmp;
        for (int i = 0; i < this->getMaxLevel() + 1; i++) {
            tmp.push_back("CumulantK17Comp");
        }
        this->setMultiKernel(tmp);
    }
}

void Parameter::initLBMSimulationParameter()
{
    // host
    for (int i = coarse; i <= fine; i++) {
        parH[i]                   = std::make_shared<LBMSimulationParameter>();
        parH[i]->numberofthreads  = 64; // 128;
        parH[i]->gridNX           = getGridX().at(i);
        parH[i]->gridNY           = getGridY().at(i);
        parH[i]->gridNZ           = getGridZ().at(i);
        parH[i]->vis              = ic.vis * pow(2.f, i);
        parH[i]->diffusivity      = ic.Diffusivity * pow(2.f, i);
        parH[i]->omega            = 1.0f / (3.0f * parH[i]->vis + 0.5f); // omega :-) not s9 = -1.0f/(3.0f*parH[i]->vis+0.5f);//
        parH[i]->nx               = parH[i]->gridNX + 2 * STARTOFFX;
        parH[i]->ny               = parH[i]->gridNY + 2 * STARTOFFY;
        parH[i]->nz               = parH[i]->gridNZ + 2 * STARTOFFZ;
        parH[i]->size_Mat         = parH[i]->nx * parH[i]->ny * parH[i]->nz;
        parH[i]->sizePlaneXY      = parH[i]->nx * parH[i]->ny;
        parH[i]->sizePlaneYZ      = parH[i]->ny * parH[i]->nz;
        parH[i]->sizePlaneXZ      = parH[i]->nx * parH[i]->nz;
        parH[i]->mem_size_real    = sizeof(real) * parH[i]->size_Mat;
        parH[i]->mem_size_int     = sizeof(unsigned int) * parH[i]->size_Mat;
        parH[i]->mem_size_bool    = sizeof(bool) * parH[i]->size_Mat;
        parH[i]->mem_size_real_yz = sizeof(real) * parH[i]->ny * parH[i]->nz;
        parH[i]->isEvenTimestep        = true;
        parH[i]->startz           = parH[i]->gridNZ * ic.myid;
        parH[i]->endz             = parH[i]->gridNZ * ic.myid + parH[i]->gridNZ;
        parH[i]->Lx               = (real)((1.f * parH[i]->gridNX - 1.f) / (pow(2.f, i)));
        parH[i]->Ly               = (real)((1.f * parH[i]->gridNY - 1.f) / (pow(2.f, i)));
        parH[i]->Lz               = (real)((1.f * parH[i]->gridNZ - 1.f) / (pow(2.f, i)));
        parH[i]->dx               = (real)(1.f / (pow(2.f, i)));
        parH[i]->XdistKn          = getDistX().at(i);
        parH[i]->YdistKn          = getDistY().at(i);
        parH[i]->ZdistKn          = getDistZ().at(i);
        if (i == coarse) {
            parH[i]->distX  = (real)getDistX().at(i);
            parH[i]->distY  = (real)getDistY().at(i);
            parH[i]->distZ  = (real)getDistZ().at(i);
            parH[i]->mTtoWx = (real)1.0f;
            parH[i]->mTtoWy = (real)1.0f;
            parH[i]->mTtoWz = (real)1.0f;
            parH[i]->cTtoWx = (real)0.0f;
            parH[i]->cTtoWy = (real)0.0f;
            parH[i]->cTtoWz = (real)0.0f;
            ////MGs Trafo///////////////////////////////////////////////////////////////
            // parH[i]->cStartx               = (real)parH[i]->XdistKn;
            // parH[i]->cStarty               = (real)parH[i]->XdistKn;
            // parH[i]->cStartz               = (real)parH[i]->XdistKn;
            ////////////////////////////////////////////////////////////////////////////
        } else {
            // Geller
            parH[i]->distX = ((real)getDistX().at(i) + 0.25f) * parH[i - 1]->dx;
            parH[i]->distY = ((real)getDistY().at(i) + 0.25f) * parH[i - 1]->dx;
            parH[i]->distZ = ((real)getDistZ().at(i) + 0.25f) * parH[i - 1]->dx;
            // parH[i]->distX                 = ((real)getDistX().at(i) + 0.25f) * parH[i-1]->dx + parH[i-1]->distX;
            // parH[i]->distY                 = ((real)getDistY().at(i) + 0.25f) * parH[i-1]->dx + parH[i-1]->distY;
            // parH[i]->distZ                 = ((real)getDistZ().at(i) + 0.25f) * parH[i-1]->dx + parH[i-1]->distZ;
            parH[i]->mTtoWx = (real)pow(0.5f, i);
            parH[i]->mTtoWy = (real)pow(0.5f, i);
            parH[i]->mTtoWz = (real)pow(0.5f, i);
            parH[i]->cTtoWx = (real)(STARTOFFX / 2.f + (parH[i]->gridNX + 1.f) / 4.f); // funzt nur fuer zwei level
            parH[i]->cTtoWy = (real)(STARTOFFY / 2.f + (parH[i]->gridNY + 1.f) / 4.f); // funzt nur fuer zwei level
            parH[i]->cTtoWz = (real)(STARTOFFZ / 2.f + (parH[i]->gridNZ + 1.f) / 4.f); // funzt nur fuer zwei level
            ////MGs Trafo///////////////////////////////////////////////////////////////
            // parH[i]->cStartx               = (real)parH[i]->XdistKn;
            // parH[i]->cStarty               = (real)parH[i]->XdistKn;
            // parH[i]->cStartz               = (real)parH[i]->XdistKn;
            ////////////////////////////////////////////////////////////////////////////
        }
    }

    // device
    for (int i = coarse; i <= fine; i++) {
        parD[i]                   = std::make_shared<LBMSimulationParameter>();
        parD[i]->numberofthreads  = parH[i]->numberofthreads;
        parD[i]->gridNX           = parH[i]->gridNX;
        parD[i]->gridNY           = parH[i]->gridNY;
        parD[i]->gridNZ           = parH[i]->gridNZ;
        parD[i]->vis              = parH[i]->vis;
        parD[i]->diffusivity      = parH[i]->diffusivity;
        parD[i]->omega            = parH[i]->omega;
        parD[i]->nx               = parH[i]->nx;
        parD[i]->ny               = parH[i]->ny;
        parD[i]->nz               = parH[i]->nz;
        parD[i]->size_Mat         = parH[i]->size_Mat;
        parD[i]->sizePlaneXY      = parH[i]->sizePlaneXY;
        parD[i]->sizePlaneYZ      = parH[i]->sizePlaneYZ;
        parD[i]->sizePlaneXZ      = parH[i]->sizePlaneXZ;
        parD[i]->mem_size_real    = sizeof(real) * parD[i]->size_Mat;
        parD[i]->mem_size_int     = sizeof(unsigned int) * parD[i]->size_Mat;
        parD[i]->mem_size_bool    = sizeof(bool) * parD[i]->size_Mat;
        parD[i]->mem_size_real_yz = sizeof(real) * parD[i]->ny * parD[i]->nz;
        parD[i]->isEvenTimestep        = parH[i]->isEvenTimestep;
        parD[i]->startz           = parH[i]->startz;
        parD[i]->endz             = parH[i]->endz;
        parD[i]->Lx               = parH[i]->Lx;
        parD[i]->Ly               = parH[i]->Ly;
        parD[i]->Lz               = parH[i]->Lz;
        parD[i]->dx               = parH[i]->dx;
        parD[i]->XdistKn          = parH[i]->XdistKn;
        parD[i]->YdistKn          = parH[i]->YdistKn;
        parD[i]->ZdistKn          = parH[i]->ZdistKn;
        parD[i]->distX            = parH[i]->distX;
        parD[i]->distY            = parH[i]->distY;
        parD[i]->distZ            = parH[i]->distZ;
    }
}

void Parameter::copyMeasurePointsArrayToVector(int lev)
{
    int valuesPerClockCycle = (int)(getclockCycleForMP() / getTimestepForMP());
    for (int i = 0; i < (int)parH[lev]->MP.size(); i++) {
        for (int j = 0; j < valuesPerClockCycle; j++) {
            int index = i * valuesPerClockCycle + j;
            parH[lev]->MP[i].Vx.push_back(parH[lev]->VxMP[index]);
            parH[lev]->MP[i].Vy.push_back(parH[lev]->VyMP[index]);
            parH[lev]->MP[i].Vz.push_back(parH[lev]->VzMP[index]);
            parH[lev]->MP[i].Rho.push_back(parH[lev]->RhoMP[index]);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set-methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Parameter::setForcing(real forcingX, real forcingY, real forcingZ)
{
    this->hostForcing[0] = forcingX;
    this->hostForcing[1] = forcingY;
    this->hostForcing[2] = forcingZ;
}
void Parameter::setQuadricLimiters(real quadricLimiterP, real quadricLimiterM, real quadricLimiterD)
{
    this->hostQuadricLimiters[0] = quadricLimiterP;
    this->hostQuadricLimiters[1] = quadricLimiterM;
    this->hostQuadricLimiters[2] = quadricLimiterD;
}
void Parameter::setPhi(real inPhi)
{
    Phi = inPhi;
}
void Parameter::setAngularVelocity(real inAngVel)
{
    angularVelocity = inAngVel;
}
void Parameter::setStepEnsight(unsigned int step)
{
    this->stepEnsight = step;
}
void Parameter::setOutputCount(unsigned int outputCount)
{
    this->outputCount = outputCount;
}
void Parameter::setlimitOfNodesForVTK(unsigned int limitOfNodesForVTK)
{
    this->limitOfNodesForVTK = limitOfNodesForVTK;
}
void Parameter::setStartTurn(unsigned int inStartTurn)
{
    startTurn = inStartTurn;
}
void Parameter::setDiffOn(bool isDiff)
{
    diffOn = isDiff;
}
void Parameter::setCompOn(bool isComp)
{
    compOn = isComp;
}
void Parameter::setDiffMod(int DiffMod)
{
    diffMod = DiffMod;
}
void Parameter::setD3Qxx(int d3qxx)
{
    this->D3Qxx = d3qxx;
}
void Parameter::setMaxLevel(int maxlevel)
{
    this->maxlevel = maxlevel - 1;
    this->fine = this->maxlevel;
    parH.resize(this->maxlevel + 1);
    parD.resize(this->maxlevel + 1);
}
void Parameter::setParticleBasicLevel(int pbl)
{
    this->particleBasicLevel = pbl;
}
void Parameter::setParticleInitLevel(int pil)
{
    this->particleInitLevel = pil;
}
void Parameter::setNumberOfParticles(int nop)
{
    this->numberOfParticles = nop;
}
void Parameter::setCalcParticles(bool calcParticles)
{
    this->calcParticles = calcParticles;
}
void Parameter::setStartXHotWall(real startXHotWall)
{
    this->startXHotWall = startXHotWall;
}
void Parameter::setEndXHotWall(real endXHotWall)
{
    this->endXHotWall = endXHotWall;
}
void Parameter::setTimestepEnd(unsigned int tend)
{
    ic.tend = tend;
}
void Parameter::setTimestepOut(unsigned int tout)
{
    ic.tout = tout;
}
void Parameter::setTimestepStartOut(unsigned int tStartOut)
{
    ic.tStartOut = tStartOut;
}
void Parameter::setTimestepOfCoarseLevel(unsigned int timestep)
{
    this->timestep = timestep;
}
void Parameter::setCalcTurbulenceIntensity(bool calcVelocityAndFluctuations)
{
    this->calcVelocityAndFluctuations = calcVelocityAndFluctuations;
}
void Parameter::setCalcMedian(bool calcMedian)
{
    ic.calcMedian = calcMedian;
}
void Parameter::setCalcDragLift(bool calcDragLift)
{
    this->calcDragLift = calcDragLift;
}
void Parameter::setCalcCp(bool calcCp)
{
    this->calcCp = calcCp;
}
void Parameter::setWriteVeloASCIIfiles(bool writeVeloASCII)
{
    this->writeVeloASCII = writeVeloASCII;
}
void Parameter::setCalcPlaneConc(bool calcPlaneConc)
{
    this->calcPlaneConc = calcPlaneConc;
}
void Parameter::setTimeCalcMedStart(int CalcMedStart)
{
    ic.tCalcMedStart = CalcMedStart;
}
void Parameter::setTimeCalcMedEnd(int CalcMedEnd)
{
    ic.tCalcMedEnd = CalcMedEnd;
}
void Parameter::setOutputPath(std::string oPath)
{
    // add missing slash to outputPath
    if (oPath.back() != '/')
        oPath += "/";

    ic.oPath = oPath;
}
void Parameter::setOutputPrefix(std::string oPrefix)
{
    ic.oPrefix = oPrefix;
}
void Parameter::setPathAndFilename(std::string fname)
{
    ic.fname = fname;
}
void Parameter::setGridPath(std::string gridPath)
{
    ic.gridPath = gridPath;
    this->initGridPaths();
}
void Parameter::setPrintFiles(bool printfiles)
{
    ic.printFiles = printfiles;
}
void Parameter::setReadGeo(bool readGeo)
{
    ic.readGeo = readGeo;
}
void Parameter::setDiffusivity(real Diffusivity)
{
    ic.Diffusivity = Diffusivity;
}
void Parameter::setTemperatureInit(real Temp)
{
    ic.Temp = Temp;
}
void Parameter::setTemperatureBC(real TempBC)
{
    ic.TempBC = TempBC;
}
void Parameter::setViscosityLB(real Viscosity)
{
    ic.vis = Viscosity;
}
void Parameter::setVelocityLB(real Velocity)
{
    ic.u0 = Velocity;
}
void Parameter::setViscosityRatio(real ViscosityRatio)
{
    ic.vis_ratio = ViscosityRatio;
}
void Parameter::setVelocityRatio(real VelocityRatio)
{
    ic.u0_ratio = VelocityRatio;
}
void Parameter::setDensityRatio(real DensityRatio)
{
    ic.delta_rho = DensityRatio;
}
void Parameter::setPressRatio(real PressRatio)
{
    ic.delta_press = PressRatio;
}
real Parameter::getTimeRatio()
{
    return this->getViscosityRatio() * pow(this->getVelocityRatio(), -2);
}
real Parameter::getForceRatio()
{
    return this->getDensityRatio() * pow(this->getViscosityRatio(), 2);
}
real Parameter::getLengthRatio()
{
    return this->getViscosityRatio() / this->getVelocityRatio();
}
void Parameter::setRealX(real RealX)
{
    ic.RealX = RealX;
}
void Parameter::setRealY(real RealY)
{
    ic.RealY = RealY;
}
void Parameter::setPressInID(unsigned int PressInID)
{
    ic.PressInID = PressInID;
}
void Parameter::setPressOutID(unsigned int PressOutID)
{
    ic.PressOutID = PressOutID;
}
void Parameter::setPressInZ(unsigned int PressInZ)
{
    ic.PressInZ = PressInZ;
}
void Parameter::setPressOutZ(unsigned int PressOutZ)
{
    ic.PressOutZ = PressOutZ;
}
void Parameter::setMaxDev(int maxdev)
{
    ic.maxdev = maxdev;
}
void Parameter::setMyID(int myid)
{
    ic.myid = myid;
}
void Parameter::setNumprocs(int numprocs)
{
    ic.numprocs = numprocs;
}
void Parameter::setDevices(std::vector<uint> devices)
{
    ic.devices = devices;
}
void Parameter::setGeometryFileC(std::string GeometryFileC)
{
    ic.geometryFileC = GeometryFileC;
}
void Parameter::setGeometryFileM(std::string GeometryFileM)
{
    ic.geometryFileM = GeometryFileM;
}
void Parameter::setGeometryFileF(std::string GeometryFileF)
{
    ic.geometryFileF = GeometryFileF;
}
void Parameter::setRe(real Re)
{
    ic.Re = Re;
}
void Parameter::setFactorPressBC(real factorPressBC)
{
    ic.factorPressBC = factorPressBC;
}
void Parameter::setIsGeo(bool isGeo)
{
    ic.isGeo = isGeo;
}
void Parameter::setIsGeoNormal(bool isGeoNormal)
{
    ic.isGeoNormal = isGeoNormal;
}
void Parameter::setIsInflowNormal(bool isInflowNormal)
{
    ic.isInflowNormal = isInflowNormal;
}
void Parameter::setIsOutflowNormal(bool isOutflowNormal)
{
    ic.isOutflowNormal = isOutflowNormal;
}
void Parameter::setIsProp(bool isProp)
{
    ic.isProp = isProp;
}
void Parameter::setIsCp(bool isCp)
{
    ic.isCp = isCp;
}
void Parameter::setConcFile(bool concFile)
{
    ic.isConc = concFile;
}
void Parameter::setStreetVelocityFile(bool streetVelocityFile)
{
    ic.streetVelocityFile = streetVelocityFile;
}
void Parameter::setUseMeasurePoints(bool useMeasurePoints)
{
    ic.isMeasurePoints = useMeasurePoints;
}
void Parameter::setUseInitNeq(bool useInitNeq)
{
    ic.isInitNeq = useInitNeq;
}
void Parameter::setSimulatePorousMedia(bool simulatePorousMedia)
{
    ic.simulatePorousMedia = simulatePorousMedia;
}
void Parameter::setUseTurbulentViscosity(bool useTurbulentViscosity)
{
    ic.isTurbulentViscosity = useTurbulentViscosity;
}
void Parameter::setUseWale(bool useWale)
{
    ic.isWale = useWale;
    if (useWale)
        setUseTurbulentViscosity(true);
}
void Parameter::setUseAMD(bool useAMD)
{
    ic.isAMD = useAMD;
    if (useAMD)
        setUseTurbulentViscosity(true);
}
void Parameter::setSGSConstant(real SGSConstant)
{
    ic.SGSConstant = SGSConstant;
}
void Parameter::setHasWallModelMonitor(bool hasWallModelMonitor)
{
    ic.hasWallModelMonitor = hasWallModelMonitor;
}

void Parameter::setIsF3(bool isF3)
{
    this->isF3 = isF3;
}

void Parameter::setIsBodyForce(bool isBodyForce)
{
    this->isBodyForce = isBodyForce;
}

void Parameter::setGridX(std::vector<int> GridX)
{
    ic.GridX = GridX;
}
void Parameter::setGridY(std::vector<int> GridY)
{
    ic.GridY = GridY;
}
void Parameter::setGridZ(std::vector<int> GridZ)
{
    ic.GridZ = GridZ;
}
void Parameter::setDistX(std::vector<int> DistX)
{
    ic.DistX = DistX;
}
void Parameter::setDistY(std::vector<int> DistY)
{
    ic.DistY = DistY;
}
void Parameter::setDistZ(std::vector<int> DistZ)
{
    ic.DistZ = DistZ;
}
void Parameter::setScaleLBMtoSI(std::vector<real> scaleLBMtoSI)
{
    ic.scaleLBMtoSI = scaleLBMtoSI;
}
void Parameter::setTranslateLBMtoSI(std::vector<real> translateLBMtoSI)
{
    ic.translateLBMtoSI = translateLBMtoSI;
}
void Parameter::setMinCoordX(std::vector<real> MinCoordX)
{
    ic.minCoordX = MinCoordX;
}
void Parameter::setMinCoordY(std::vector<real> MinCoordY)
{
    ic.minCoordY = MinCoordY;
}
void Parameter::setMinCoordZ(std::vector<real> MinCoordZ)
{
    ic.minCoordZ = MinCoordZ;
}
void Parameter::setMaxCoordX(std::vector<real> MaxCoordX)
{
    ic.maxCoordX = MaxCoordX;
}
void Parameter::setMaxCoordY(std::vector<real> MaxCoordY)
{
    ic.maxCoordY = MaxCoordY;
}
void Parameter::setMaxCoordZ(std::vector<real> MaxCoordZ)
{
    ic.maxCoordZ = MaxCoordZ;
}
void Parameter::setTempH(TempforBoundaryConditions *TempH)
{
    this->TempH = TempH;
}
void Parameter::setTempD(TempforBoundaryConditions *TempD)
{
    this->TempD = TempD;
}
void Parameter::setTempVelH(TempVelforBoundaryConditions *TempVelH)
{
    this->TempVelH = TempVelH;
}
void Parameter::setTempVelD(TempVelforBoundaryConditions *TempVelD)
{
    this->TempVelD = TempVelD;
}
void Parameter::setTempPressH(TempPressforBoundaryConditions *TempPressH)
{
    this->TempPressH = TempPressH;
}
void Parameter::setTempPressD(TempPressforBoundaryConditions *TempPressD)
{
    this->TempPressD = TempPressD;
}
// void Parameter::setQinflowH(QforBoundaryConditions* QinflowH)
//{
//   this->QinflowH = QinflowH;
//}
// void Parameter::setQinflowD(QforBoundaryConditions* QinflowD)
//{
//   this->QinflowD = QinflowD;
//}
// void Parameter::setQoutflowH(QforBoundaryConditions* QoutflowH)
//{
//   this->QoutflowH = QoutflowH;
//}
// void Parameter::setQoutflowD(QforBoundaryConditions* QoutflowD)
//{
//   this->QoutflowD = QoutflowD;
//}
void Parameter::setkFull(std::string kFull)
{
    ic.kFull = kFull;
}
void Parameter::setgeoFull(std::string geoFull)
{
    ic.geoFull = geoFull;
}
void Parameter::setgeoVec(std::string geoVec)
{
    ic.geoVec = geoVec;
}
void Parameter::setcoordX(std::string coordX)
{
    ic.coordX = coordX;
}
void Parameter::setcoordY(std::string coordY)
{
    ic.coordY = coordY;
}
void Parameter::setcoordZ(std::string coordZ)
{
    ic.coordZ = coordZ;
}
void Parameter::setneighborX(std::string neighborX)
{
    ic.neighborX = neighborX;
}
void Parameter::setneighborY(std::string neighborY)
{
    ic.neighborY = neighborY;
}
void Parameter::setneighborZ(std::string neighborZ)
{
    ic.neighborZ = neighborZ;
}
void Parameter::setneighborWSB(std::string neighborWSB)
{
    ic.neighborWSB = neighborWSB;
}
void Parameter::setscaleCFC(std::string scaleCFC)
{
    ic.scaleCFC = scaleCFC;
}
void Parameter::setscaleCFF(std::string scaleCFF)
{
    ic.scaleCFF = scaleCFF;
}
void Parameter::setscaleFCC(std::string scaleFCC)
{
    ic.scaleFCC = scaleFCC;
}
void Parameter::setscaleFCF(std::string scaleFCF)
{
    ic.scaleFCF = scaleFCF;
}
void Parameter::setscaleOffsetCF(std::string scaleOffsetCF)
{
    ic.scaleOffsetCF = scaleOffsetCF;
}
void Parameter::setscaleOffsetFC(std::string scaleOffsetFC)
{
    ic.scaleOffsetFC = scaleOffsetFC;
}
void Parameter::setgeomBoundaryBcQs(std::string geomBoundaryBcQs)
{
    ic.geomBoundaryBcQs = geomBoundaryBcQs;
}
void Parameter::setgeomBoundaryBcValues(std::string geomBoundaryBcValues)
{
    ic.geomBoundaryBcValues = geomBoundaryBcValues;
}
void Parameter::setnoSlipBcPos(std::string noSlipBcPos)
{
    ic.noSlipBcPos = noSlipBcPos;
}
void Parameter::setnoSlipBcQs(std::string noSlipBcQs)
{
    ic.noSlipBcQs = noSlipBcQs;
}
void Parameter::setnoSlipBcValue(std::string noSlipBcValue)
{
    ic.noSlipBcValue = noSlipBcValue;
}
void Parameter::setnoSlipBcValues(std::string noSlipBcValues)
{
    ic.noSlipBcValues = noSlipBcValues;
}
void Parameter::setslipBcPos(std::string slipBcPos)
{
    ic.slipBcPos = slipBcPos;
}
void Parameter::setslipBcQs(std::string slipBcQs)
{
    ic.slipBcQs = slipBcQs;
}
void Parameter::setslipBcValue(std::string slipBcValue)
{
    ic.slipBcValue = slipBcValue;
}
void Parameter::setpressBcPos(std::string pressBcPos)
{
    ic.pressBcPos = pressBcPos;
}
void Parameter::setpressBcQs(std::string pressBcQs)
{
    ic.pressBcQs = pressBcQs;
}
void Parameter::setpressBcValue(std::string pressBcValue)
{
    ic.pressBcValue = pressBcValue;
}
void Parameter::setpressBcValues(std::string pressBcValues)
{
    ic.pressBcValues = pressBcValues;
}
void Parameter::setvelBcQs(std::string velBcQs)
{
    ic.velBcQs = velBcQs;
}
void Parameter::setvelBcValues(std::string velBcValues)
{
    ic.velBcValues = velBcValues;
}
void Parameter::setinletBcQs(std::string inletBcQs)
{
    ic.inletBcQs = inletBcQs;
}
void Parameter::setinletBcValues(std::string inletBcValues)
{
    ic.inletBcValues = inletBcValues;
}
void Parameter::setoutletBcQs(std::string outletBcQs)
{
    ic.outletBcQs = outletBcQs;
}
void Parameter::setoutletBcValues(std::string outletBcValues)
{
    ic.outletBcValues = outletBcValues;
}
void Parameter::settopBcQs(std::string topBcQs)
{
    ic.topBcQs = topBcQs;
}
void Parameter::settopBcValues(std::string topBcValues)
{
    ic.topBcValues = topBcValues;
}
void Parameter::setbottomBcQs(std::string bottomBcQs)
{
    ic.bottomBcQs = bottomBcQs;
}
void Parameter::setbottomBcValues(std::string bottomBcValues)
{
    ic.bottomBcValues = bottomBcValues;
}
void Parameter::setfrontBcQs(std::string frontBcQs)
{
    ic.frontBcQs = frontBcQs;
}
void Parameter::setfrontBcValues(std::string frontBcValues)
{
    ic.frontBcValues = frontBcValues;
}
void Parameter::setbackBcQs(std::string backBcQs)
{
    ic.backBcQs = backBcQs;
}
void Parameter::setbackBcValues(std::string backBcValues)
{
    ic.backBcValues = backBcValues;
}
void Parameter::setwallBcQs(std::string wallBcQs)
{
    ic.wallBcQs = wallBcQs;
}
void Parameter::setwallBcValues(std::string wallBcValues)
{
    ic.wallBcValues = wallBcValues;
}
void Parameter::setperiodicBcQs(std::string periodicBcQs)
{
    ic.periodicBcQs = periodicBcQs;
}
void Parameter::setperiodicBcValues(std::string periodicBcValues)
{
    ic.periodicBcValues = periodicBcValues;
}
void Parameter::setpropellerQs(std::string propellerQs)
{
    ic.propellerQs = propellerQs;
}
void Parameter::setpropellerValues(std::string propellerValues)
{
    ic.propellerValues = propellerValues;
}
void Parameter::setpropellerCylinder(std::string propellerCylinder)
{
    ic.propellerCylinder = propellerCylinder;
}
void Parameter::setmeasurePoints(std::string measurePoints)
{
    ic.measurePoints = measurePoints;
}
void Parameter::setnumberNodes(std::string numberNodes)
{
    ic.numberNodes = numberNodes;
}
void Parameter::setLBMvsSI(std::string LBMvsSI)
{
    ic.LBMvsSI = LBMvsSI;
}
void Parameter::setcpTop(std::string cpTop)
{
    ic.cpTop = cpTop;
}
void Parameter::setcpBottom(std::string cpBottom)
{
    ic.cpBottom = cpBottom;
}
void Parameter::setcpBottom2(std::string cpBottom2)
{
    ic.cpBottom2 = cpBottom2;
}
void Parameter::setConcentration(std::string concFile)
{
    ic.concentration = concFile;
}
void Parameter::setStreetVelocity(std::string streetVelocity)
{
    ic.streetVelocity = streetVelocity;
}
void Parameter::setclockCycleForMP(real clockCycleForMP)
{
    ic.clockCycleForMP = clockCycleForMP;
}
void Parameter::setTimeDoCheckPoint(unsigned int tDoCheckPoint)
{
    ic.tDoCheckPoint = tDoCheckPoint;
}
void Parameter::setTimeDoRestart(unsigned int tDoRestart)
{
    ic.tDoRestart = tDoRestart;
}
void Parameter::setDoCheckPoint(bool doCheckPoint)
{
    ic.doCheckPoint = doCheckPoint;
}
void Parameter::setDoRestart(bool doRestart)
{
    ic.doRestart = doRestart;
}
void Parameter::settimestepForMP(unsigned int timestepForMP)
{
    ic.timeStepForMP = timestepForMP;
}
void Parameter::setObj(std::string str, bool isObj)
{
    if (str == "geo") {
        this->setIsGeo(isObj);
    } else if (str == "prop") {
        this->setIsProp(isObj);
    } else if (str == "cp") {
        this->setIsCp(isObj);
    } else if (str == "geoNormal") {
        this->setIsGeoNormal(isObj);
    } else if (str == "inflowNormal") {
        this->setIsInflowNormal(isObj);
    } else if (str == "outflowNormal") {
        this->setIsOutflowNormal(isObj);
    }
}
void Parameter::setUseGeometryValues(bool useGeometryValues)
{
    ic.GeometryValues = useGeometryValues;
}
void Parameter::setCalc2ndOrderMoments(bool is2ndOrderMoments)
{
    ic.is2ndOrderMoments = is2ndOrderMoments;
}
void Parameter::setCalc3rdOrderMoments(bool is3rdOrderMoments)
{
    ic.is3rdOrderMoments = is3rdOrderMoments;
}
void Parameter::setCalcHighOrderMoments(bool isHighOrderMoments)
{
    ic.isHighOrderMoments = isHighOrderMoments;
}
void Parameter::setMemsizeGPU(double admem, bool reset)
{
    if (reset == true) {
        this->memsizeGPU = 0.;
    } else {
        this->memsizeGPU += admem;
    }
}
// 1D domain decomposition
void Parameter::setPossNeighborFiles(std::vector<std::string> possNeighborFiles, std::string sor)
{
    if (sor == "send") {
        this->possNeighborFilesSend = possNeighborFiles;
    } else if (sor == "recv") {
        this->possNeighborFilesRecv = possNeighborFiles;
    }
}
void Parameter::setNumberOfProcessNeighbors(unsigned int numberOfProcessNeighbors, int level, std::string sor)
{
    if (sor == "send") {
        parH[level]->sendProcessNeighbor.resize(numberOfProcessNeighbors);
        parD[level]->sendProcessNeighbor.resize(numberOfProcessNeighbors);
    } else if (sor == "recv") {
        parH[level]->recvProcessNeighbor.resize(numberOfProcessNeighbors);
        parD[level]->recvProcessNeighbor.resize(numberOfProcessNeighbors);
    }
}
void Parameter::setIsNeighbor(bool isNeigbor)
{
    this->isNeigbor = isNeigbor;
}
// 3D domain decomposition
void Parameter::setPossNeighborFilesX(std::vector<std::string> possNeighborFiles, std::string sor)
{
    if (sor == "send") {
        this->possNeighborFilesSendX = possNeighborFiles;
    } else if (sor == "recv") {
        this->possNeighborFilesRecvX = possNeighborFiles;
    }
}
void Parameter::setPossNeighborFilesY(std::vector<std::string> possNeighborFiles, std::string sor)
{
    if (sor == "send") {
        this->possNeighborFilesSendY = possNeighborFiles;
    } else if (sor == "recv") {
        this->possNeighborFilesRecvY = possNeighborFiles;
    }
}
void Parameter::setPossNeighborFilesZ(std::vector<std::string> possNeighborFiles, std::string sor)
{
    if (sor == "send") {
        this->possNeighborFilesSendZ = possNeighborFiles;
    } else if (sor == "recv") {
        this->possNeighborFilesRecvZ = possNeighborFiles;
    }
}
void Parameter::setNumberOfProcessNeighborsX(unsigned int numberOfProcessNeighbors, int level, std::string sor)
{
    if (sor == "send") {
        parH[level]->sendProcessNeighborX.resize(numberOfProcessNeighbors);
        parD[level]->sendProcessNeighborX.resize(numberOfProcessNeighbors);
        //////////////////////////////////////////////////////////////////////////
        if (getDiffOn() == true) {
            parH[level]->sendProcessNeighborADX.resize(numberOfProcessNeighbors);
            parD[level]->sendProcessNeighborADX.resize(numberOfProcessNeighbors);
        }
        //////////////////////////////////////////////////////////////////////////
    } else if (sor == "recv") {
        parH[level]->recvProcessNeighborX.resize(numberOfProcessNeighbors);
        parD[level]->recvProcessNeighborX.resize(numberOfProcessNeighbors);
        //////////////////////////////////////////////////////////////////////////
        if (getDiffOn() == true) {
            parH[level]->recvProcessNeighborADX.resize(numberOfProcessNeighbors);
            parD[level]->recvProcessNeighborADX.resize(numberOfProcessNeighbors);
        }
        //////////////////////////////////////////////////////////////////////////
    }
}
void Parameter::setNumberOfProcessNeighborsY(unsigned int numberOfProcessNeighbors, int level, std::string sor)
{
    if (sor == "send") {
        parH[level]->sendProcessNeighborY.resize(numberOfProcessNeighbors);
        parD[level]->sendProcessNeighborY.resize(numberOfProcessNeighbors);
        //////////////////////////////////////////////////////////////////////////
        if (getDiffOn() == true) {
            parH[level]->sendProcessNeighborADY.resize(numberOfProcessNeighbors);
            parD[level]->sendProcessNeighborADY.resize(numberOfProcessNeighbors);
        }
        //////////////////////////////////////////////////////////////////////////
    } else if (sor == "recv") {
        parH[level]->recvProcessNeighborY.resize(numberOfProcessNeighbors);
        parD[level]->recvProcessNeighborY.resize(numberOfProcessNeighbors);
        //////////////////////////////////////////////////////////////////////////
        if (getDiffOn() == true) {
            parH[level]->recvProcessNeighborADY.resize(numberOfProcessNeighbors);
            parD[level]->recvProcessNeighborADY.resize(numberOfProcessNeighbors);
        }
        //////////////////////////////////////////////////////////////////////////
    }
}
void Parameter::setNumberOfProcessNeighborsZ(unsigned int numberOfProcessNeighbors, int level, std::string sor)
{
    if (sor == "send") {
        parH[level]->sendProcessNeighborZ.resize(numberOfProcessNeighbors);
        parD[level]->sendProcessNeighborZ.resize(numberOfProcessNeighbors);
        //////////////////////////////////////////////////////////////////////////
        if (getDiffOn() == true) {
            parH[level]->sendProcessNeighborADZ.resize(numberOfProcessNeighbors);
            parD[level]->sendProcessNeighborADZ.resize(numberOfProcessNeighbors);
        }
        //////////////////////////////////////////////////////////////////////////
    } else if (sor == "recv") {
        parH[level]->recvProcessNeighborZ.resize(numberOfProcessNeighbors);
        parD[level]->recvProcessNeighborZ.resize(numberOfProcessNeighbors);
        //////////////////////////////////////////////////////////////////////////
        if (getDiffOn() == true) {
            parH[level]->recvProcessNeighborADZ.resize(numberOfProcessNeighbors);
            parD[level]->recvProcessNeighborADZ.resize(numberOfProcessNeighbors);
        }
        //////////////////////////////////////////////////////////////////////////
    }
}
void Parameter::setIsNeighborX(bool isNeigbor)
{
    this->isNeigborX = isNeigbor;
}
void Parameter::setIsNeighborY(bool isNeigbor)
{
    this->isNeigborY = isNeigbor;
}
void Parameter::setIsNeighborZ(bool isNeigbor)
{
    this->isNeigborZ = isNeigbor;
}
void Parameter::setSendProcessNeighborsAfterFtoCX(int numberOfNodes, int level, int arrayIndex)
{
    this->getParH(level)->sendProcessNeighborsAfterFtoCX[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParD(level)->sendProcessNeighborsAfterFtoCX[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParH(level)->sendProcessNeighborsAfterFtoCX[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParD(level)->sendProcessNeighborsAfterFtoCX[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParH(level)->sendProcessNeighborsAfterFtoCX[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
    this->getParD(level)->sendProcessNeighborsAfterFtoCX[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
}
void Parameter::setSendProcessNeighborsAfterFtoCY(int numberOfNodes, int level, int arrayIndex)
{
    this->getParH(level)->sendProcessNeighborsAfterFtoCY[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParD(level)->sendProcessNeighborsAfterFtoCY[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParH(level)->sendProcessNeighborsAfterFtoCY[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParD(level)->sendProcessNeighborsAfterFtoCY[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParH(level)->sendProcessNeighborsAfterFtoCY[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
    this->getParD(level)->sendProcessNeighborsAfterFtoCY[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
}
void Parameter::setSendProcessNeighborsAfterFtoCZ(int numberOfNodes, int level, int arrayIndex)
{
    this->getParH(level)->sendProcessNeighborsAfterFtoCZ[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParD(level)->sendProcessNeighborsAfterFtoCZ[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParH(level)->sendProcessNeighborsAfterFtoCZ[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParD(level)->sendProcessNeighborsAfterFtoCZ[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParH(level)->sendProcessNeighborsAfterFtoCZ[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
    this->getParD(level)->sendProcessNeighborsAfterFtoCZ[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
}
void Parameter::setRecvProcessNeighborsAfterFtoCX(int numberOfNodes, int level, int arrayIndex)
{
    this->getParH(level)->recvProcessNeighborsAfterFtoCX[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParD(level)->recvProcessNeighborsAfterFtoCX[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParH(level)->recvProcessNeighborsAfterFtoCX[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParD(level)->recvProcessNeighborsAfterFtoCX[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParH(level)->recvProcessNeighborsAfterFtoCX[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
    this->getParD(level)->recvProcessNeighborsAfterFtoCX[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
}
void Parameter::setRecvProcessNeighborsAfterFtoCY(int numberOfNodes, int level, int arrayIndex)
{
    this->getParH(level)->recvProcessNeighborsAfterFtoCY[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParD(level)->recvProcessNeighborsAfterFtoCY[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParH(level)->recvProcessNeighborsAfterFtoCY[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParD(level)->recvProcessNeighborsAfterFtoCY[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParH(level)->recvProcessNeighborsAfterFtoCY[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
    this->getParD(level)->recvProcessNeighborsAfterFtoCY[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
}
void Parameter::setRecvProcessNeighborsAfterFtoCZ(int numberOfNodes, int level, int arrayIndex)
{
    this->getParH(level)->recvProcessNeighborsAfterFtoCZ[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParD(level)->recvProcessNeighborsAfterFtoCZ[arrayIndex].numberOfNodes = numberOfNodes;
    this->getParH(level)->recvProcessNeighborsAfterFtoCZ[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParD(level)->recvProcessNeighborsAfterFtoCZ[arrayIndex].memsizeFs     = sizeof(real) * numberOfNodes;
    this->getParH(level)->recvProcessNeighborsAfterFtoCZ[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
    this->getParD(level)->recvProcessNeighborsAfterFtoCZ[arrayIndex].numberOfFs    = this->D3Qxx * numberOfNodes;
}
void Parameter::setgeomBoundaryNormalX(std::string geomNormalX)
{
    ic.geomNormalX = geomNormalX;
}
void Parameter::setgeomBoundaryNormalY(std::string geomNormalY)
{
    ic.geomNormalY = geomNormalY;
}
void Parameter::setgeomBoundaryNormalZ(std::string geomNormalZ)
{
    ic.geomNormalZ = geomNormalZ;
}
void Parameter::setInflowBoundaryNormalX(std::string inflowNormalX)
{
    ic.inflowNormalX = inflowNormalX;
}
void Parameter::setInflowBoundaryNormalY(std::string inflowNormalY)
{
    ic.inflowNormalY = inflowNormalY;
}
void Parameter::setInflowBoundaryNormalZ(std::string inflowNormalZ)
{
    ic.inflowNormalZ = inflowNormalZ;
}
void Parameter::setOutflowBoundaryNormalX(std::string outflowNormalX)
{
    ic.outflowNormalX = outflowNormalX;
}
void Parameter::setOutflowBoundaryNormalY(std::string outflowNormalY)
{
    ic.outflowNormalY = outflowNormalY;
}
void Parameter::setOutflowBoundaryNormalZ(std::string outflowNormalZ)
{
    ic.outflowNormalZ = outflowNormalZ;
}
void Parameter::setMainKernel(std::string kernel)
{
    this->mainKernel = kernel;
    if (kernel.find("Stream") != std::string::npos || kernel.find("Redesigned") != std::string::npos)
        this->kernelNeedsFluidNodeIndicesToRun = true;
}
void Parameter::setMultiKernelOn(bool isOn)
{
    this->multiKernelOn = isOn;
}
void Parameter::setMultiKernelLevel(std::vector<int> kernelLevel)
{
    this->multiKernelLevel = kernelLevel;
}
void Parameter::setMultiKernel(std::vector<std::string> kernel)
{
    this->multiKernel = kernel;
}
void Parameter::setADKernel(std::string adKernel)
{
    this->adKernel = adKernel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add-methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Parameter::addActuator(SPtr<PreCollisionInteractor> actuator)
{
    actuators.push_back(actuator);
}
void Parameter::addProbe(SPtr<PreCollisionInteractor> probe)
{
    probes.push_back(probe);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get-methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double *Parameter::getForcesDouble()
{
    return this->hostForcing;
}
real *Parameter::getForcesHost()
{
    return this->forcingH;
}
real *Parameter::getForcesDev()
{
    return this->forcingD;
}
double *Parameter::getQuadricLimitersDouble()
{
    return this->hostQuadricLimiters;
}
real *Parameter::getQuadricLimitersHost()
{
    return this->quadricLimitersH;
}
real *Parameter::getQuadricLimitersDev()
{
    return this->quadricLimitersD;
}
real Parameter::getPhi()
{
    return Phi;
}
real Parameter::getAngularVelocity()
{
    return angularVelocity;
}
real Parameter::getStartXHotWall()
{
    return this->startXHotWall;
}
real Parameter::getEndXHotWall()
{
    return this->endXHotWall;
}
unsigned int Parameter::getStepEnsight()
{
    return this->stepEnsight;
}
unsigned int Parameter::getOutputCount()
{
    return this->outputCount;
}
unsigned int Parameter::getlimitOfNodesForVTK()
{
    return this->limitOfNodesForVTK;
}
unsigned int Parameter::getStartTurn()
{
    return startTurn;
}
std::shared_ptr<LBMSimulationParameter> Parameter::getParD(int level)
{
    return parD[level];
}
std::shared_ptr<LBMSimulationParameter> Parameter::getParH(int level)
{
    return parH[level];
}
unsigned int Parameter::getSizeMat(int level)
{
    return parH[level]->size_Mat;
}
unsigned int Parameter::getMemSizereal(int level)
{
    return parH[level]->mem_size_real;
}
unsigned int Parameter::getMemSizeInt(int level)
{
    return parH[level]->mem_size_int;
}
unsigned int Parameter::getMemSizeBool(int level)
{
    return parH[level]->mem_size_bool;
}
unsigned int Parameter::getMemSizerealYZ(int level)
{
    return parH[level]->mem_size_real_yz;
}
int Parameter::getFine()
{
    return fine;
}
int Parameter::getCoarse()
{
    return coarse;
}
int Parameter::getParticleBasicLevel()
{
    return this->particleBasicLevel;
}
int Parameter::getParticleInitLevel()
{
    return this->particleInitLevel;
}
int Parameter::getNumberOfParticles()
{
    return this->numberOfParticles;
}
bool Parameter::getEvenOrOdd(int level)
{
    return parH[level]->isEvenTimestep;
}
bool Parameter::getDiffOn()
{
    return diffOn;
}
bool Parameter::getCompOn()
{
    return compOn;
}
int Parameter::getDiffMod()
{
    return diffMod;
}
int Parameter::getFactorNZ()
{
    return factor_gridNZ;
}
int Parameter::getD3Qxx()
{
    return this->D3Qxx;
}
int Parameter::getMaxLevel()
{
    return this->maxlevel;
}
unsigned int Parameter::getTStart()
{
    if (getDoRestart()) {
        return getTimeDoRestart() + 1;
    } else {
        return 1;
    }
}
unsigned int Parameter::getTInit()
{
    if (getDoRestart()) {
        return getTimeDoRestart();
    } else {
        return 0;
    }
}
unsigned int Parameter::getTEnd()
{
    return ic.tend;
}
unsigned int Parameter::getTOut()
{
    return ic.tout;
}
unsigned int Parameter::getTStartOut()
{
    return ic.tStartOut;
}
bool Parameter::getCalcMedian()
{
    return ic.calcMedian;
}
bool Parameter::getCalcDragLift()
{
    return this->calcDragLift;
}
bool Parameter::getCalcCp()
{
    return this->calcCp;
}
bool Parameter::getCalcParticles()
{
    return this->calcParticles;
}
bool Parameter::getWriteVeloASCIIfiles()
{
    return this->writeVeloASCII;
}
bool Parameter::getCalcPlaneConc()
{
    return this->calcPlaneConc;
}
int Parameter::getTimeCalcMedStart()
{
    return ic.tCalcMedStart;
}
int Parameter::getTimeCalcMedEnd()
{
    return ic.tCalcMedEnd;
}
std::string Parameter::getOutputPath()
{
    return ic.oPath;
}
std::string Parameter::getOutputPrefix()
{
    return ic.oPrefix;
}
std::string Parameter::getFName()
{
    return ic.fname;
}
std::string Parameter::getGridPath()
{
    return ic.gridPath;
}
bool Parameter::getPrintFiles()
{
    return ic.printFiles;
}
bool Parameter::getReadGeo()
{
    return ic.readGeo;
}
bool Parameter::getCalcTurbulenceIntensity()
{
    return this->calcVelocityAndFluctuations;
}
real Parameter::getDiffusivity()
{
    return ic.Diffusivity;
}
real Parameter::getTemperatureInit()
{
    return ic.Temp;
}
real Parameter::getTemperatureBC()
{
    return ic.TempBC;
}
real Parameter::getViscosity()
{
    return ic.vis;
}
real Parameter::getVelocity()
{
    return ic.u0;
}
real Parameter::getViscosityRatio()
{
    return ic.vis_ratio;
}
real Parameter::getVelocityRatio()
{
    return ic.u0_ratio;
}
real Parameter::getDensityRatio()
{
    return ic.delta_rho;
}
real Parameter::getPressRatio()
{
    return ic.delta_press;
}
real Parameter::getRealX()
{
    return ic.RealX;
}
real Parameter::getRealY()
{
    return ic.RealY;
}
unsigned int Parameter::getPressInID()
{
    return ic.PressInID;
}
unsigned int Parameter::getPressOutID()
{
    return ic.PressOutID;
}
unsigned int Parameter::getPressInZ()
{
    return ic.PressInZ;
}
unsigned int Parameter::getPressOutZ()
{
    return ic.PressOutZ;
}
int Parameter::getMaxDev()
{
    return ic.maxdev;
}
int Parameter::getMyID()
{
    return ic.myid;
}
int Parameter::getNumprocs()
{
    return ic.numprocs;
}
std::vector<uint> Parameter::getDevices()
{
    return ic.devices;
}
std::string Parameter::getGeometryFileC()
{
    return ic.geometryFileC;
}
std::string Parameter::getGeometryFileM()
{
    return ic.geometryFileM;
}
std::string Parameter::getGeometryFileF()
{
    return ic.geometryFileF;
}
real Parameter::getRe()
{
    return ic.Re;
}
real Parameter::getFactorPressBC()
{
    return ic.factorPressBC;
}
std::vector<int> Parameter::getGridX()
{
    return ic.GridX;
}
std::vector<int> Parameter::getGridY()
{
    return ic.GridY;
}
std::vector<int> Parameter::getGridZ()
{
    return ic.GridZ;
}
std::vector<int> Parameter::getDistX()
{
    return ic.DistX;
}
std::vector<int> Parameter::getDistY()
{
    return ic.DistY;
}
std::vector<int> Parameter::getDistZ()
{
    return ic.DistZ;
}
std::vector<real> Parameter::getScaleLBMtoSI()
{
    return ic.scaleLBMtoSI;
}
std::vector<real> Parameter::getTranslateLBMtoSI()
{
    return ic.translateLBMtoSI;
}
std::vector<real> Parameter::getMinCoordX()
{
    return ic.minCoordX;
}
std::vector<real> Parameter::getMinCoordY()
{
    return ic.minCoordY;
}
std::vector<real> Parameter::getMinCoordZ()
{
    return ic.minCoordZ;
}
std::vector<real> Parameter::getMaxCoordX()
{
    return ic.maxCoordX;
}
std::vector<real> Parameter::getMaxCoordY()
{
    return ic.maxCoordY;
}
std::vector<real> Parameter::getMaxCoordZ()
{
    return ic.maxCoordZ;
}
TempforBoundaryConditions *Parameter::getTempH()
{
    return this->TempH;
}
TempforBoundaryConditions *Parameter::getTempD()
{
    return this->TempD;
}
TempVelforBoundaryConditions *Parameter::getTempVelH()
{
    return this->TempVelH;
}
TempVelforBoundaryConditions *Parameter::getTempVelD()
{
    return this->TempVelD;
}
TempPressforBoundaryConditions *Parameter::getTempPressH()
{
    return this->TempPressH;
}
TempPressforBoundaryConditions *Parameter::getTempPressD()
{
    return this->TempPressD;
}
// QforBoundaryConditions* Parameter::getQinflowH()
//{
//   return this->QinflowH;
//}
// QforBoundaryConditions* Parameter::getQinflowD()
//{
//   return this->QinflowD;
//}
// QforBoundaryConditions* Parameter::getQoutflowH()
//{
//   return this->QoutflowH;
//}
// QforBoundaryConditions* Parameter::getQoutflowD()
//{
//   return this->QoutflowD;
//}
std::string Parameter::getkFull()
{
    return ic.kFull;
}
std::string Parameter::getgeoFull()
{
    return ic.geoFull;
}
std::string Parameter::getgeoVec()
{
    return ic.geoVec;
}
std::string Parameter::getcoordX()
{
    return ic.coordX;
}
std::string Parameter::getcoordY()
{
    return ic.coordY;
}
std::string Parameter::getcoordZ()
{
    return ic.coordZ;
}
std::string Parameter::getneighborX()
{
    return ic.neighborX;
}
std::string Parameter::getneighborY()
{
    return ic.neighborY;
}
std::string Parameter::getneighborZ()
{
    return ic.neighborZ;
}
std::string Parameter::getneighborWSB()
{
    return ic.neighborWSB;
}
std::string Parameter::getscaleCFC()
{
    return ic.scaleCFC;
}
std::string Parameter::getscaleCFF()
{
    return ic.scaleCFF;
}
std::string Parameter::getscaleFCC()
{
    return ic.scaleFCC;
}
std::string Parameter::getscaleFCF()
{
    return ic.scaleFCF;
}
std::string Parameter::getscaleOffsetCF()
{
    return ic.scaleOffsetCF;
}
std::string Parameter::getscaleOffsetFC()
{
    return ic.scaleOffsetFC;
}
std::string Parameter::getgeomBoundaryBcQs()
{
    return ic.geomBoundaryBcQs;
}
std::string Parameter::getgeomBoundaryBcValues()
{
    return ic.geomBoundaryBcValues;
}
std::string Parameter::getnoSlipBcPos()
{
    return ic.noSlipBcPos;
}
std::string Parameter::getnoSlipBcQs()
{
    return ic.noSlipBcQs;
}
std::string Parameter::getnoSlipBcValue()
{
    return ic.noSlipBcValue;
}
std::string Parameter::getnoSlipBcValues()
{
    return ic.noSlipBcValues;
}
std::string Parameter::getslipBcPos()
{
    return ic.slipBcPos;
}
std::string Parameter::getslipBcQs()
{
    return ic.slipBcQs;
}
std::string Parameter::getslipBcValue()
{
    return ic.slipBcValue;
}
std::string Parameter::getpressBcPos()
{
    return ic.pressBcPos;
}
std::string Parameter::getpressBcQs()
{
    return ic.pressBcQs;
}
std::string Parameter::getpressBcValue()
{
    return ic.pressBcValue;
}
std::string Parameter::getpressBcValues()
{
    return ic.pressBcValues;
}
std::string Parameter::getvelBcQs()
{
    return ic.velBcQs;
}
std::string Parameter::getvelBcValues()
{
    return ic.velBcValues;
}
std::string Parameter::getinletBcQs()
{
    return ic.inletBcQs;
}
std::string Parameter::getinletBcValues()
{
    return ic.inletBcValues;
}
std::string Parameter::getoutletBcQs()
{
    return ic.outletBcQs;
}
std::string Parameter::getoutletBcValues()
{
    return ic.outletBcValues;
}
std::string Parameter::gettopBcQs()
{
    return ic.topBcQs;
}
std::string Parameter::gettopBcValues()
{
    return ic.topBcValues;
}
std::string Parameter::getbottomBcQs()
{
    return ic.bottomBcQs;
}
std::string Parameter::getbottomBcValues()
{
    return ic.bottomBcValues;
}
std::string Parameter::getfrontBcQs()
{
    return ic.frontBcQs;
}
std::string Parameter::getfrontBcValues()
{
    return ic.frontBcValues;
}
std::string Parameter::getbackBcQs()
{
    return ic.backBcQs;
}
std::string Parameter::getbackBcValues()
{
    return ic.backBcValues;
}
std::string Parameter::getwallBcQs()
{
    return ic.wallBcQs;
}
std::string Parameter::getwallBcValues()
{
    return ic.wallBcValues;
}
std::string Parameter::getperiodicBcQs()
{
    return ic.periodicBcQs;
}
std::string Parameter::getperiodicBcValues()
{
    return ic.periodicBcValues;
}
std::string Parameter::getpropellerQs()
{
    return ic.propellerQs;
}
std::string Parameter::getpropellerValues()
{
    return ic.propellerValues;
}
std::string Parameter::getpropellerCylinder()
{
    return ic.propellerCylinder;
}
std::string Parameter::getmeasurePoints()
{
    return ic.measurePoints;
}
std::string Parameter::getLBMvsSI()
{
    return ic.LBMvsSI;
}
std::string Parameter::getnumberNodes()
{
    return ic.numberNodes;
}
std::string Parameter::getcpTop()
{
    return ic.cpTop;
}
std::string Parameter::getcpBottom()
{
    return ic.cpBottom;
}
std::string Parameter::getcpBottom2()
{
    return ic.cpBottom2;
}
std::string Parameter::getConcentration()
{
    return ic.concentration;
}
std::string Parameter::getStreetVelocityFilePath()
{
    return ic.streetVelocity;
}
real Parameter::getclockCycleForMP()
{
    return ic.clockCycleForMP;
}
unsigned int Parameter::getTimeDoCheckPoint()
{
    return ic.tDoCheckPoint;
}
unsigned int Parameter::getTimeDoRestart()
{
    return ic.tDoRestart;
}
bool Parameter::getDoCheckPoint()
{
    return ic.doCheckPoint;
}
bool Parameter::getDoRestart()
{
    return ic.doRestart;
}
bool Parameter::getIsGeo()
{
    return ic.isGeo;
}
bool Parameter::getIsGeoNormal()
{
    return ic.isGeoNormal;
}
bool Parameter::getIsInflowNormal()
{
    return ic.isInflowNormal;
}
bool Parameter::getIsOutflowNormal()
{
    return ic.isOutflowNormal;
}
bool Parameter::getIsCp()
{
    return ic.isCp;
}
bool Parameter::getConcFile()
{
    return ic.isConc;
}
bool Parameter::isStreetVelocityFile()
{
    return ic.streetVelocityFile;
}
bool Parameter::getUseMeasurePoints()
{
    return ic.isMeasurePoints;
}
bool Parameter::getUseWale()
{
    return ic.isWale;
}
bool Parameter::getUseAMD()
{
    return ic.isAMD;
}
bool Parameter::getUseTurbulentViscosity()
{
    return ic.isTurbulentViscosity;
}
real Parameter::getSGSConstant()
{
    return ic.SGSConstant;
}
bool Parameter::getHasWallModelMonitor()
{
    return ic.hasWallModelMonitor;
}
std::vector<SPtr<PreCollisionInteractor>> Parameter::getActuators()
{
    return actuators;
}
std::vector<SPtr<PreCollisionInteractor>> Parameter::getProbes()
{
    return probes;
}
bool Parameter::getUseInitNeq()
{
    return ic.isInitNeq;
}
bool Parameter::getSimulatePorousMedia()
{
    return ic.simulatePorousMedia;
}

bool Parameter::getIsF3()
{
    return this->isF3;
}

bool Parameter::getIsBodyForce()
{
    return this->isBodyForce;
}

bool Parameter::getIsGeometryValues()
{
    return ic.GeometryValues;
}
bool Parameter::getCalc2ndOrderMoments()
{
    return ic.is2ndOrderMoments;
}
bool Parameter::getCalc3rdOrderMoments()
{
    return ic.is3rdOrderMoments;
}
bool Parameter::getCalcHighOrderMoments()
{
    return ic.isHighOrderMoments;
}
bool Parameter::getIsProp()
{
    return ic.isProp;
}
bool Parameter::overWritingRestart(uint t)
{
    return t == getTimeDoRestart();
}
unsigned int Parameter::getTimestepForMP()
{
    return ic.timeStepForMP;
}
unsigned int Parameter::getTimestepOfCoarseLevel()
{
    return this->timestep;
}
double Parameter::getMemsizeGPU()
{
    return this->memsizeGPU;
}
// 1D domain decomposition
std::vector<std::string> Parameter::getPossNeighborFiles(std::string sor)
{
    if (sor == "send") {
        return this->possNeighborFilesSend;
    } else if (sor == "recv") {
        return this->possNeighborFilesRecv;
    }
    throw std::runtime_error("Parameter string invalid.");
}
unsigned int Parameter::getNumberOfProcessNeighbors(int level, std::string sor)
{
    if (sor == "send") {
        return (unsigned int)parH[level]->sendProcessNeighbor.size();
    } else if (sor == "recv") {
        return (unsigned int)parH[level]->recvProcessNeighbor.size();
    }
    throw std::runtime_error("Parameter string invalid.");
}
bool Parameter::getIsNeighbor()
{
    return this->isNeigbor;
}
// 3D domain decomposition
std::vector<std::string> Parameter::getPossNeighborFilesX(std::string sor)
{
    if (sor == "send") {
        return this->possNeighborFilesSendX;
    } else if (sor == "recv") {
        return this->possNeighborFilesRecvX;
    }
    throw std::runtime_error("Parameter string invalid.");
}
std::vector<std::string> Parameter::getPossNeighborFilesY(std::string sor)
{
    if (sor == "send") {
        return this->possNeighborFilesSendY;
    } else if (sor == "recv") {
        return this->possNeighborFilesRecvY;
    }
    throw std::runtime_error("Parameter string invalid.");
}
std::vector<std::string> Parameter::getPossNeighborFilesZ(std::string sor)
{
    if (sor == "send") {
        return this->possNeighborFilesSendZ;
    } else if (sor == "recv") {
        return this->possNeighborFilesRecvZ;
    }
    throw std::runtime_error("Parameter string invalid.");
}
unsigned int Parameter::getNumberOfProcessNeighborsX(int level, std::string sor)
{
    if (sor == "send") {
        return (unsigned int)parH[level]->sendProcessNeighborX.size();
    } else if (sor == "recv") {
        return (unsigned int)parH[level]->recvProcessNeighborX.size();
    }
    throw std::runtime_error("getNumberOfProcessNeighborsX: Parameter string invalid.");
}
unsigned int Parameter::getNumberOfProcessNeighborsY(int level, std::string sor)
{
    if (sor == "send") {
        return (unsigned int)parH[level]->sendProcessNeighborY.size();
    } else if (sor == "recv") {
        return (unsigned int)parH[level]->recvProcessNeighborY.size();
    }
    throw std::runtime_error("getNumberOfProcessNeighborsY: Parameter string invalid.");
}
unsigned int Parameter::getNumberOfProcessNeighborsZ(int level, std::string sor)
{
    if (sor == "send") {
        return (unsigned int)parH[level]->sendProcessNeighborZ.size();
    } else if (sor == "recv") {
        return (unsigned int)parH[level]->recvProcessNeighborZ.size();
    }
    throw std::runtime_error("getNumberOfProcessNeighborsZ: Parameter string invalid.");
}

bool Parameter::getIsNeighborX()
{
    return this->isNeigborX;
}
bool Parameter::getIsNeighborY()
{
    return this->isNeigborY;
}
bool Parameter::getIsNeighborZ()
{
    return this->isNeigborZ;
}
std::string Parameter::getgeomBoundaryNormalX()
{
    return ic.geomNormalX;
}
std::string Parameter::getgeomBoundaryNormalY()
{
    return ic.geomNormalY;
}
std::string Parameter::getgeomBoundaryNormalZ()
{
    return ic.geomNormalZ;
}
std::string Parameter::getInflowBoundaryNormalX()
{
    return ic.inflowNormalX;
}
std::string Parameter::getInflowBoundaryNormalY()
{
    return ic.inflowNormalY;
}
std::string Parameter::getInflowBoundaryNormalZ()
{
    return ic.inflowNormalZ;
}
std::string Parameter::getOutflowBoundaryNormalX()
{
    return ic.outflowNormalX;
}
std::string Parameter::getOutflowBoundaryNormalY()
{
    return ic.outflowNormalY;
}
std::string Parameter::getOutflowBoundaryNormalZ()
{
    return ic.outflowNormalZ;
}
curandState *Parameter::getRandomState()
{
    return this->devState;
}

std::string Parameter::getMainKernel()
{
    return mainKernel;
}
bool Parameter::getMultiKernelOn()
{
    return multiKernelOn;
}
std::vector<int> Parameter::getMultiKernelLevel()
{
    return multiKernelLevel;
}
std::vector<std::string> Parameter::getMultiKernel()
{
    return multiKernel;
}
std::string Parameter::getADKernel()
{
    return adKernel;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initial condition fluid
void Parameter::setInitialCondition(
    std::function<void(real, real, real, real &, real &, real &, real &)> initialCondition)
{
    this->initialCondition = initialCondition;
}

std::function<void(real, real, real, real &, real &, real &, real &)> &Parameter::getInitialCondition()
{
    return this->initialCondition;
}

// initial condition concentration
void Parameter::setInitialConditionAD(std::function<void(real, real, real, real&)> initialConditionAD)
{
    this->initialConditionAD = initialConditionAD;
}

std::function<void(real, real, real, real&)>& Parameter::getInitialConditionAD()
{
    return this->initialConditionAD;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

real Parameter::TrafoXtoWorld(int CoordX, int level)
{
    return (parH[level]->mTtoWx * CoordX + parH[level]->cTtoWx);
}
real Parameter::TrafoYtoWorld(int CoordY, int level)
{
    return (parH[level]->mTtoWy * CoordY + parH[level]->cTtoWy);
}
real Parameter::TrafoZtoWorld(int CoordZ, int level)
{
    return (parH[level]->mTtoWz * CoordZ + parH[level]->cTtoWz);
}
real Parameter::TrafoXtoMGsWorld(int CoordX, int level)
{
    real temp = 0;
    for (int i = 0; i <= level; i++) {
        temp += (parH[i]->XdistKn + 0.25f) * 2.f * parH[i]->dx;
    }
    temp += (real)((CoordX)*parH[level]->dx);
    return temp;
}
real Parameter::TrafoYtoMGsWorld(int CoordY, int level)
{
    real temp = 0;
    for (int i = 0; i <= level; i++) {
        temp += (parH[i]->YdistKn + 0.25f) * 2.f * parH[i]->dx;
    }
    temp += (real)((CoordY)*parH[level]->dx);
    return temp;
}
real Parameter::TrafoZtoMGsWorld(int CoordZ, int level)
{
    real temp = 0;
    for (int i = 0; i <= level; i++) {
        temp += (parH[i]->ZdistKn + 0.25f) * 2.f * parH[i]->dx;
    }
    temp += (real)((CoordZ)*parH[level]->dx);
    return temp;
}

void Parameter::setUseStreams(bool useStreams)
{
    if (useStreams) {
        if (this->getNumprocs() != 1) {
            this->useStreams = useStreams;
            this->cudaStreamManager = std::make_unique<CudaStreamManager>();
            return;
        } else {
            std::cout << "Can't use streams with only one process!" << std::endl;
        }
    }
    this->useStreams = false;
}

bool Parameter::getUseStreams()
{
    return this->useStreams;
}

std::unique_ptr<CudaStreamManager> &Parameter::getStreamManager()
{
    return this->cudaStreamManager;
}

bool Parameter::getKernelNeedsFluidNodeIndicesToRun()
{
    return this->kernelNeedsFluidNodeIndicesToRun;
}

void Parameter::setKernelNeedsFluidNodeIndicesToRun(bool  kernelNeedsFluidNodeIndicesToRun){
    this->kernelNeedsFluidNodeIndicesToRun = kernelNeedsFluidNodeIndicesToRun;
}

void Parameter::initProcessNeighborsAfterFtoCX(int level)
{
    this->getParH(level)->sendProcessNeighborsAfterFtoCX.resize(this->getParH(level)->sendProcessNeighborX.size());
    this->getParH(level)->recvProcessNeighborsAfterFtoCX.resize(this->getParH(level)->recvProcessNeighborX.size());
    this->getParD(level)->sendProcessNeighborsAfterFtoCX.resize(
        this->getParH(level)->sendProcessNeighborsAfterFtoCX.size());
    this->getParD(level)->recvProcessNeighborsAfterFtoCX.resize(
        this->getParH(level)->recvProcessNeighborsAfterFtoCX.size());
}

void Parameter::initProcessNeighborsAfterFtoCY(int level)
{
    this->getParH(level)->sendProcessNeighborsAfterFtoCY.resize(this->getParH(level)->sendProcessNeighborY.size());
    this->getParH(level)->recvProcessNeighborsAfterFtoCY.resize(this->getParH(level)->recvProcessNeighborY.size());
    this->getParD(level)->sendProcessNeighborsAfterFtoCY.resize(
        this->getParH(level)->sendProcessNeighborsAfterFtoCY.size());
    this->getParD(level)->recvProcessNeighborsAfterFtoCY.resize(
        this->getParH(level)->recvProcessNeighborsAfterFtoCY.size());
}

void Parameter::initProcessNeighborsAfterFtoCZ(int level)
{
    this->getParH(level)->sendProcessNeighborsAfterFtoCZ.resize(this->getParH(level)->sendProcessNeighborZ.size());
    this->getParH(level)->recvProcessNeighborsAfterFtoCZ.resize(this->getParH(level)->recvProcessNeighborZ.size());
    this->getParD(level)->sendProcessNeighborsAfterFtoCZ.resize(
        this->getParH(level)->sendProcessNeighborsAfterFtoCZ.size());
    this->getParD(level)->recvProcessNeighborsAfterFtoCZ.resize(
        this->getParH(level)->recvProcessNeighborsAfterFtoCZ.size());
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
