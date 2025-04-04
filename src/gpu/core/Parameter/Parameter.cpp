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
//! \addtogroup gpu_Parameter Parameter
//! \ingroup gpu_core core
//! \{
//! \author Martin Schoenherr
//=======================================================================================
#include "Parameter.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <optional>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma clang diagnostic ignored "-Wunused-but-set-parameter"
#endif
#include <curand_kernel.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "StringUtilities/StringUtil.h"

#include <basics/config/ConfigurationFile.h>

#include <logger/Logger.h>
#include "Cuda/CudaStreamManager.h"

Parameter::Parameter() : Parameter(1, 0, {}) {}

Parameter::Parameter(const vf::basics::ConfigurationFile* configData) : Parameter(1, 0, configData) {}

Parameter::Parameter(int numberOfProcesses, int myId) : Parameter(numberOfProcesses, myId, {}) {}

Parameter::Parameter(int numberOfProcesses, int myId, std::optional<const vf::basics::ConfigurationFile*> configData)
{
    this->numprocs = numberOfProcesses;
    this->myProcessId = myId;

    this->setQuadricLimiters(0.01, 0.01, 0.01);
    this->setForcing(0.0, 0.0, 0.0);

    if(configData)
        readConfigData(**configData);

    initGridPaths();
    initGridBasePoints();
    initDefaultLBMkernelAllLevels();

    this->cudaStreamManager = std::make_unique<CudaStreamManager>();
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
    if (configData.contains("calcMean"))
        this->setCalcMean(configData.getValue<bool>("calcMean"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("calcCp"))
        this->calcCp = configData.getValue<bool>("calcCp");
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("calcDrafLift"))
        this->calcDragLift = configData.getValue<bool>("calcDrafLift");
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("UseMeasurePoints"))
        this->setUseMeasurePoints(configData.getValue<bool>("UseMeasurePoints"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("UseInitNeq"))
        this->setUseInitNeq(configData.getValue<bool>("UseInitNeq"));
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
    if (configData.contains("TimeStartCalcMean"))
        this->setTimeCalcMedStart(configData.getValue<int>("TimeStartCalcMean"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("TimeEndCalcMean"))
        this->setTimeCalcMedEnd(configData.getValue<int>("TimeEndCalcMean"));

    //////////////////////////////////////////////////////////////////////////
    // second component
    if (configData.contains("DiffOn"))
        this->setDiffOn(configData.getValue<bool>("DiffOn"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Diffusivity"))
        this->setDiffusivity(configData.getValue<real>("Diffusivity"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("Concentration"))
        this->setConcentrationInit(configData.getValue<real>("Concentration"));
    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("ConcentrationBC"))
        this->setConcentrationBC(configData.getValue<real>("ConcentrationBC"));

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

    //////////////////////////////////////////////////////////////////////////
    if (configData.contains("measureClockCycle"))
        this->setclockCycleForMeasurePoints(configData.getValue<real>("measureClockCycle"));

    if (configData.contains("measureTimestep"))
        this->settimestepForMeasurePoints(configData.getValue<uint>("measureTimestep"));

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Kernel
    if (configData.contains("MainKernelName"))
        this->configureMainKernel(configData.getValue<std::string>("MainKernelName"));

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
        this->gridPath = gridPath;
    }

    // for multi-gpu add process id (if not already there)
    if (this->getNumprocs() > 1) {
        gridPath += StringUtil::toString(this->getMyProcessID()) + "/";
        this->gridPath = gridPath;
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
    this->setcpTop(gridPath + "cpTop.dat");
    this->setcpBottom(gridPath + "cpBottom.dat");
    this->setcpBottom2(gridPath + "cpBottom2.dat");
    this->setConcentration(gridPath + "conc.dat");

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
        parH[i]->viscosity        = this->vis * pow((real)2.0, i);
        parH[i]->diffusivity      = this->Diffusivity * pow((real)2.0, i);
        parH[i]->omega            = (real)1.0 / (real(3.0) * parH[i]->viscosity + real(0.5)); // omega :-) not s9 = -1.0f/(3.0f*parH[i]->vis+0.5f);//
    }

    // device
    for (int i = coarse; i <= fine; i++) {
        parD[i]                   = std::make_shared<LBMSimulationParameter>();
        parD[i]->numberofthreads  = parH[i]->numberofthreads;
        parD[i]->gridNX           = parH[i]->gridNX;
        parD[i]->gridNY           = parH[i]->gridNY;
        parD[i]->gridNZ           = parH[i]->gridNZ;
        parD[i]->viscosity        = parH[i]->viscosity;
        parD[i]->diffusivity      = parH[i]->diffusivity;
        parD[i]->omega            = parH[i]->omega;
    }

    checkParameterValidityCumulantK17();
}

void Parameter::checkParameterValidityCumulantK17() const
{
    if (this->mainKernel != vf::collisionKernel::compressible::K17CompressibleNavierStokes)
        return;

    const real viscosity = this->parH[maxlevel]->viscosity;
    const real viscosityLimit = 1.0 / 42.0;
    if (viscosity > viscosityLimit) {
        VF_LOG_WARNING("The viscosity (in LB units) at level {} is {:1.3g}. It is recommended to keep it smaller than {:1.3g} "
                       "for the CumulantK17 collision kernel.",
                       maxlevel, viscosity, viscosityLimit);
    }

    const real velocity = this->u0;
    const real velocityLimit = 0.1;
    if (velocity > velocityLimit) {
        VF_LOG_WARNING("The velocity (in LB units) is {:1.4g}. It is recommended to keep it smaller than {:1.4g} for the "
                       "CumulantK17 collision kernel.",
                       velocity, velocityLimit);
    }
}

void Parameter::copyMeasurePointsArrayToVector(int lev)
{
    int valuesPerClockCycle = (int)(getclockCycleForMeasurePoints() / getTimestepForMeasurePoints());
    for (int i = 0; i < (int)parH[lev]->MeasurePointVector.size(); i++) {
        for (int j = 0; j < valuesPerClockCycle; j++) {
            int index = i * valuesPerClockCycle + j;
            parH[lev]->MeasurePointVector[i].Vx.push_back(parH[lev]->velocityInXdirectionAtMeasurePoints[index]);
            parH[lev]->MeasurePointVector[i].Vy.push_back(parH[lev]->velocityInYdirectionAtMeasurePoints[index]);
            parH[lev]->MeasurePointVector[i].Vz.push_back(parH[lev]->velocityInZdirectionAtMeasurePoints[index]);
            parH[lev]->MeasurePointVector[i].Rho.push_back(parH[lev]->densityAtMeasurePoints[index]);
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
void Parameter::setStepEnsight(unsigned int step)
{
    this->stepEnsight = step;
}
void Parameter::setDiffOn(bool isDiff)
{
    diffOn = isDiff;
}
void Parameter::setD3Qxx(int d3qxx)
{
    this->D3Qxx = d3qxx;
}
void Parameter::setMaxLevel(int numberOfLevels)
{
    this->maxlevel = numberOfLevels - 1;
    this->fine = this->maxlevel;
    parH.resize(this->maxlevel + 1);
    parD.resize(this->maxlevel + 1);
}
void Parameter::setTimestepEnd(unsigned int tend)
{
    this->tend = tend;
}
void Parameter::setTimestepOut(unsigned int tout)
{
    this->tout = tout;
}
void Parameter::setTimestepStartOut(unsigned int tStartOut)
{
    this->tStartOut = tStartOut;
}
void Parameter::setTimestepOfCoarseLevel(unsigned int timestep)
{
    this->timestep = timestep;
}
void Parameter::setCalcTurbulenceIntensity(bool calcVelocityAndFluctuations)
{
    this->calcVelocityAndFluctuations = calcVelocityAndFluctuations;
}
void Parameter::setCalcMean(bool calcMean)
{
    this->calcMean = calcMean;
}
void Parameter::setCalcDragLift(bool calcDragLift)
{
    this->calcDragLift = calcDragLift;
}
void Parameter::setCalcCp(bool calcCp)
{
    this->calcCp = calcCp;
}
void Parameter::setTimeCalcMedStart(int CalcMedStart)
{
    this->tCalcMedStart = CalcMedStart;
}
void Parameter::setTimeCalcMedEnd(int CalcMedEnd)
{
    this->tCalcMedEnd = CalcMedEnd;
}
void Parameter::setOutputPath(std::string oPath)
{
    // add missing slash to outputPath
    if (oPath.back() != '/')
        oPath += "/";

    this->oPath = oPath;
    this->setPathAndFilename(this->getOutputPath() + this->getOutputPrefix());
}
void Parameter::setOutputPrefix(std::string oPrefix)
{
    this->oPrefix = oPrefix;
    this->setPathAndFilename(this->getOutputPath() + this->getOutputPrefix());
}
void Parameter::setPathAndFilename(std::string fname)
{
    this->fname = fname;
}
void Parameter::setGridPath(std::string gridPath)
{
    this->gridPath = gridPath;
    this->initGridPaths();
}
void Parameter::setPrintFiles(bool printfiles)
{
    this->printFiles = printfiles;
}
void Parameter::setReadGeo(bool readGeo)
{
    this->readGeo = readGeo;
}
void Parameter::setDiffusivity(real Diffusivity)
{
    this->Diffusivity = Diffusivity;
}
void Parameter::setConcentrationInit(real concentrationInit)
{
    this->concentrationInit = concentrationInit;
}
void Parameter::setConcentrationBC(real concentrationBC)
{
    this->concentrationBC = concentrationBC;
}
void Parameter::setViscosityLB(real Viscosity)
{
    this->vis = Viscosity;
}
void Parameter::setVelocityLB(real Velocity)
{
    this->u0 = Velocity;
}
void Parameter::setViscosityRatio(real ViscosityRatio)
{
    this->vis_ratio = ViscosityRatio;
}
void Parameter::setVelocityRatio(real VelocityRatio)
{
    this->u0_ratio = VelocityRatio;
}
void Parameter::setDensityRatio(real DensityRatio)
{
    this->delta_rho = DensityRatio;
}
void Parameter::setPressRatio(real PressRatio)
{
    this->delta_press = PressRatio;
}
real Parameter::getViscosityRatio()
{
    return this->vis_ratio;
}
real Parameter::getVelocityRatio() const
{
    return this->u0_ratio;
}
real Parameter::getDensityRatio()
{
    return this->delta_rho;
}
real Parameter::getPressureRatio()
{
    return this->delta_press;
}
real Parameter::getTimeRatio()
{
    return this->getViscosityRatio() * pow(this->getVelocityRatio(), -2);
}
real Parameter::getLengthRatio()
{
    return this->getViscosityRatio() / this->getVelocityRatio();
}
real Parameter::getForceRatio()
{
    return (this->getDensityRatio()+1.0) * this->getVelocityRatio()/this->getTimeRatio();
}
real Parameter::getScaledViscosityRatio(int level)
{
    return this->getViscosityRatio()/(level+1);
}
real Parameter::getScaledVelocityRatio(int level)
{
    return this->getVelocityRatio();
}
real Parameter::getScaledDensityRatio(int level)
{
    return this->getDensityRatio();
}
real Parameter::getScaledPressureRatio(int level)
{
    return this->getPressureRatio();
}
real Parameter::getScaledTimeRatio(int level)
{
    return this->getTimeRatio()/(level+1);
}
real Parameter::getScaledLengthRatio(int level)
{
    return this->getLengthRatio()/(level+1);
}
real Parameter::getScaledForceRatio(int level)
{
    return this->getForceRatio()*(level+1);
}
real Parameter::getScaledStressRatio(int level)
{
    return this->getVelocityRatio()*this->getVelocityRatio();
}
void Parameter::setRealX(real RealX)
{
    this->RealX = RealX;
}
void Parameter::setRealY(real RealY)
{
    this->RealY = RealY;
}
void Parameter::setOutflowPressureCorrectionFactor(real pressBCrhoCorrectionFactor)
{
    this->outflowPressureCorrectionFactor = pressBCrhoCorrectionFactor;
}
void Parameter::setMaxDev(int maxdev)
{
    this->maxdev = maxdev;
}
void Parameter::setMyID(int myid)
{
    this->myProcessId = myid;
}
void Parameter::setNumprocs(int numprocs)
{
    this->numprocs = numprocs;
}
void Parameter::setDevices(std::vector<uint> devices)
{
    this->devices = devices;
}
void Parameter::setRe(real Re)
{
    this->Re = Re;
}
void Parameter::setFactorPressBC(real factorPressBC)
{
    this->factorPressBC = factorPressBC;
}
void Parameter::setIsGeo(bool isGeo)
{
    this->isGeo = isGeo;
}
void Parameter::setIsCp(bool isCp)
{
    this->isCp = isCp;
}
void Parameter::setUseMeasurePoints(bool useMeasurePoints)
{
    this->isMeasurePoints = useMeasurePoints;
}
void Parameter::setUseInitNeq(bool useInitNeq)
{
    this->isInitNeq = useInitNeq;
}
void Parameter::setUseTurbulentViscosity(bool useTurbulentViscosity)
{
    this->isTurbulentViscosity = useTurbulentViscosity;
}
void Parameter::setTurbulenceModel(vf::lbm::TurbulenceModel turbulenceModel)
{
    this->turbulenceModel = turbulenceModel;
}
void Parameter::setSGSConstant(real SGSConstant)
{
    this->SGSConstant = SGSConstant;
}
void Parameter::setHasWallModelMonitor(bool hasWallModelMonitor)
{
    this->hasWallModelMonitor = hasWallModelMonitor;
}

void Parameter::setIsBodyForce(bool isBodyForce)
{
    this->isBodyForce = isBodyForce;
}

void Parameter::setGridX(std::vector<int> GridX)
{
    this->GridX = GridX;
}
void Parameter::setGridY(std::vector<int> GridY)
{
    this->GridY = GridY;
}
void Parameter::setGridZ(std::vector<int> GridZ)
{
    this->GridZ = GridZ;
}
void Parameter::setScaleLBMtoSI(std::vector<real> scaleLBMtoSI)
{
    this->scaleLBMtoSI = scaleLBMtoSI;
}
void Parameter::setTranslateLBMtoSI(std::vector<real> translateLBMtoSI)
{
    this->translateLBMtoSI = translateLBMtoSI;
}
void Parameter::setMinCoordX(std::vector<real> MinCoordX)
{
    this->minCoordX = MinCoordX;
}
void Parameter::setMinCoordY(std::vector<real> MinCoordY)
{
    this->minCoordY = MinCoordY;
}
void Parameter::setMinCoordZ(std::vector<real> MinCoordZ)
{
    this->minCoordZ = MinCoordZ;
}
void Parameter::setMaxCoordX(std::vector<real> MaxCoordX)
{
    this->maxCoordX = MaxCoordX;
}
void Parameter::setMaxCoordY(std::vector<real> MaxCoordY)
{
    this->maxCoordY = MaxCoordY;
}
void Parameter::setMaxCoordZ(std::vector<real> MaxCoordZ)
{
    this->maxCoordZ = MaxCoordZ;
}
void Parameter::setConcentrationNoSlipBCHost(AdvectionDiffusionNoSlipBoundaryConditions *concentrationNoSlipBCHost)
{
    this->concentrationNoSlipBCHost = concentrationNoSlipBCHost;
}
void Parameter::setConcentrationNoSlipBCDevice(AdvectionDiffusionNoSlipBoundaryConditions *concentrationNoSlipBCDevice)
{
    this->concentrationNoSlipBCDevice = concentrationNoSlipBCDevice;
}
void Parameter::setConcentrationDirichletBCHost(AdvectionDiffusionDirichletBoundaryConditions *concentrationDirichletBCHost)
{
    this->concentrationDirichletBCHost = concentrationDirichletBCHost;
}
void Parameter::setConcentrationDirichletBCDevice(AdvectionDiffusionDirichletBoundaryConditions *concentrationDirichletBCDevice)
{
    this->concentrationDirichletBCDevice = concentrationDirichletBCDevice;
}
void Parameter::setgeoVec(std::string geoVec)
{
    this->geoVec = geoVec;
}
void Parameter::setcoordX(std::string coordX)
{
    this->coordX = coordX;
}
void Parameter::setcoordY(std::string coordY)
{
    this->coordY = coordY;
}
void Parameter::setcoordZ(std::string coordZ)
{
    this->coordZ = coordZ;
}
void Parameter::setneighborX(std::string neighborX)
{
    this->neighborX = neighborX;
}
void Parameter::setneighborY(std::string neighborY)
{
    this->neighborY = neighborY;
}
void Parameter::setneighborZ(std::string neighborZ)
{
    this->neighborZ = neighborZ;
}
void Parameter::setneighborWSB(std::string neighborWSB)
{
    this->neighborWSB = neighborWSB;
}
void Parameter::setscaleCFC(std::string scaleCFC)
{
    this->scaleCFC = scaleCFC;
}
void Parameter::setscaleCFF(std::string scaleCFF)
{
    this->scaleCFF = scaleCFF;
}
void Parameter::setscaleFCC(std::string scaleFCC)
{
    this->scaleFCC = scaleFCC;
}
void Parameter::setscaleFCF(std::string scaleFCF)
{
    this->scaleFCF = scaleFCF;
}
void Parameter::setscaleOffsetCF(std::string scaleOffsetCF)
{
    this->scaleOffsetCF = scaleOffsetCF;
}
void Parameter::setscaleOffsetFC(std::string scaleOffsetFC)
{
    this->scaleOffsetFC = scaleOffsetFC;
}
void Parameter::setgeomBoundaryBcQs(std::string geomBoundaryBcQs)
{
    this->geomBoundaryBcQs = geomBoundaryBcQs;
}
void Parameter::setgeomBoundaryBcValues(std::string geomBoundaryBcValues)
{
    this->geomBoundaryBcValues = geomBoundaryBcValues;
}
void Parameter::setnoSlipBcPos(std::string noSlipBcPos)
{
    this->noSlipBcPos = noSlipBcPos;
}
void Parameter::setnoSlipBcQs(std::string noSlipBcQs)
{
    this->noSlipBcQs = noSlipBcQs;
}
void Parameter::setnoSlipBcValue(std::string noSlipBcValue)
{
    this->noSlipBcValue = noSlipBcValue;
}
void Parameter::setnoSlipBcValues(std::string noSlipBcValues)
{
    this->noSlipBcValues = noSlipBcValues;
}
void Parameter::setslipBcPos(std::string slipBcPos)
{
    this->slipBcPos = slipBcPos;
}
void Parameter::setslipBcQs(std::string slipBcQs)
{
    this->slipBcQs = slipBcQs;
}
void Parameter::setslipBcValue(std::string slipBcValue)
{
    this->slipBcValue = slipBcValue;
}
void Parameter::setpressBcPos(std::string pressBcPos)
{
    this->pressBcPos = pressBcPos;
}
void Parameter::setpressBcQs(std::string pressBcQs)
{
    this->pressBcQs = pressBcQs;
}
void Parameter::setpressBcValue(std::string pressBcValue)
{
    this->pressBcValue = pressBcValue;
}
void Parameter::setpressBcValues(std::string pressBcValues)
{
    this->pressBcValues = pressBcValues;
}
void Parameter::setvelBcQs(std::string velBcQs)
{
    this->velBcQs = velBcQs;
}
void Parameter::setvelBcValues(std::string velBcValues)
{
    this->velBcValues = velBcValues;
}
void Parameter::setinletBcQs(std::string inletBcQs)
{
    this->inletBcQs = inletBcQs;
}
void Parameter::setinletBcValues(std::string inletBcValues)
{
    this->inletBcValues = inletBcValues;
}
void Parameter::setoutletBcQs(std::string outletBcQs)
{
    this->outletBcQs = outletBcQs;
}
void Parameter::setoutletBcValues(std::string outletBcValues)
{
    this->outletBcValues = outletBcValues;
}
void Parameter::settopBcQs(std::string topBcQs)
{
    this->topBcQs = topBcQs;
}
void Parameter::settopBcValues(std::string topBcValues)
{
    this->topBcValues = topBcValues;
}
void Parameter::setbottomBcQs(std::string bottomBcQs)
{
    this->bottomBcQs = bottomBcQs;
}
void Parameter::setbottomBcValues(std::string bottomBcValues)
{
    this->bottomBcValues = bottomBcValues;
}
void Parameter::setfrontBcQs(std::string frontBcQs)
{
    this->frontBcQs = frontBcQs;
}
void Parameter::setfrontBcValues(std::string frontBcValues)
{
    this->frontBcValues = frontBcValues;
}
void Parameter::setbackBcQs(std::string backBcQs)
{
    this->backBcQs = backBcQs;
}
void Parameter::setbackBcValues(std::string backBcValues)
{
    this->backBcValues = backBcValues;
}
void Parameter::setwallBcQs(std::string wallBcQs)
{
    this->wallBcQs = wallBcQs;
}
void Parameter::setwallBcValues(std::string wallBcValues)
{
    this->wallBcValues = wallBcValues;
}
void Parameter::setperiodicBcQs(std::string periodicBcQs)
{
    this->periodicBcQs = periodicBcQs;
}
void Parameter::setperiodicBcValues(std::string periodicBcValues)
{
    this->periodicBcValues = periodicBcValues;
}
void Parameter::setmeasurePoints(std::string measurePoints)
{
    this->measurePoints = measurePoints;
}
void Parameter::setnumberNodes(std::string numberNodes)
{
    this->numberNodes = numberNodes;
}
void Parameter::setLBMvsSI(std::string LBMvsSI)
{
    this->LBMvsSI = LBMvsSI;
}
void Parameter::setcpTop(std::string cpTop)
{
    this->cpTop = cpTop;
}
void Parameter::setcpBottom(std::string cpBottom)
{
    this->cpBottom = cpBottom;
}
void Parameter::setcpBottom2(std::string cpBottom2)
{
    this->cpBottom2 = cpBottom2;
}
void Parameter::setConcentration(std::string concFile)
{
    this->concentration = concFile;
}
void Parameter::setclockCycleForMeasurePoints(real clockCycleForMP)
{
    this->clockCycleForMeasurePoints = clockCycleForMP;
}
void Parameter::setTimeDoCheckPoint(unsigned int tDoCheckPoint)
{
    this->tDoCheckPoint = tDoCheckPoint;
}
void Parameter::setTimeDoRestart(unsigned int tDoRestart)
{
    this->tDoRestart = tDoRestart;
}
void Parameter::setDoCheckPoint(bool doCheckPoint)
{
    this->doCheckPoint = doCheckPoint;
}
void Parameter::setDoRestart(bool doRestart)
{
    this->doRestart = doRestart;
}
void Parameter::settimestepForMeasurePoints(unsigned int timestepForMP)
{
    this->timeStepForMeasurePoints = timestepForMP;
}
void Parameter::setObj(std::string str, bool isObj)
{
    if (str == "geo") {
        this->setIsGeo(isObj);
    } else if (str == "cp") {
        this->setIsCp(isObj);
    }
}
void Parameter::setUseGeometryValues(bool useGeometryValues)
{
    this->GeometryValues = useGeometryValues;
}
void Parameter::setCalc2ndOrderMoments(bool is2ndOrderMoments)
{
    this->is2ndOrderMoments = is2ndOrderMoments;
}
void Parameter::setCalc3rdOrderMoments(bool is3rdOrderMoments)
{
    this->is3rdOrderMoments = is3rdOrderMoments;
}
void Parameter::setCalcHighOrderMoments(bool isHighOrderMoments)
{
    this->isHighOrderMoments = isHighOrderMoments;
}
void Parameter::setMemsizeGPU(double admem, bool reset)
{
    if (reset == true) {
        this->memsizeGPU = 0.;
    } else {
        this->memsizeGPU += admem;
    }
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
void Parameter::configureMainKernel(std::string kernel)
{
    this->mainKernel = kernel;
    if (kernel == vf::collisionKernel::compressible::K17CompressibleNavierStokes)
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
void Parameter::addInteractor(SPtr<PreCollisionInteractor> interactor)
{
    interactors.push_back(interactor);
}
void Parameter::addSampler(SPtr<Sampler> sampler)
{
    samplers.push_back(sampler);
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
real *Parameter::getQuadricLimitersHost() const
{
    return this->quadricLimitersH;
}
real *Parameter::getQuadricLimitersDev()
{
    return this->quadricLimitersD;
}
unsigned int Parameter::getStepEnsight()
{
    return this->stepEnsight;
}
std::shared_ptr<LBMSimulationParameter> Parameter::getParD(int level)
{
    return parD[level];
}
std::shared_ptr<LBMSimulationParameter> Parameter::getParH(int level) const
{
    return parH[level];
}

LBMSimulationParameter& Parameter::getParDeviceAsReference(int level) const
{
    return *parD[level];
}

LBMSimulationParameter& Parameter::getParHostAsReference(int level) const
{
    return *parH[level];
}

const std::vector<std::shared_ptr<LBMSimulationParameter>> &Parameter::getParHallLevels()
{
    return parH;
}
const std::vector<std::shared_ptr<LBMSimulationParameter>> &Parameter::getParDallLevels()
{
    return parD;
}

int Parameter::getFine() const
{
    return fine;
}
int Parameter::getCoarse() const
{
    return coarse;
}
bool Parameter::getEvenOrOdd(int level)
{
    return parD[level]->isEvenTimestep;
}
bool Parameter::getDiffOn()
{
    return diffOn;
}
int Parameter::getFactorNZ()
{
    return factor_gridNZ;
}
int Parameter::getD3Qxx()
{
    return this->D3Qxx;
}
int Parameter::getMaxLevel() const
{
    return this->maxlevel;
}
unsigned int Parameter::getTimestepStart() const
{
    if (getDoRestart()) {
        return getTimeDoRestart() + 1;
    } else {
        return 1;
    }
}
unsigned int Parameter::getTimestepInit()
{
    if (getDoRestart()) {
        return getTimeDoRestart();
    } else {
        return 0;
    }
}
unsigned int Parameter::getTimestepEnd() const
{
    return this->tend;
}
unsigned int Parameter::getTimestepOut()
{
    return this->tout;
}
unsigned int Parameter::getTimestepStartOut()
{
    return this->tStartOut;
}
bool Parameter::getCalcMean()
{
    return this->calcMean;
}
bool Parameter::getCalcDragLift()
{
    return this->calcDragLift;
}
bool Parameter::getCalcCp()
{
    return this->calcCp;
}
int Parameter::getTimeCalcMedStart()
{
    return this->tCalcMedStart;
}
int Parameter::getTimeCalcMedEnd()
{
    return this->tCalcMedEnd;
}
std::string Parameter::getOutputPath()
{
    return this->oPath;
}
std::string Parameter::getOutputPrefix() const
{
    return this->oPrefix;
}
std::string Parameter::getFName() const
{
    return this->fname;
}
std::string Parameter::getGridPath()
{
    return this->gridPath;
}
bool Parameter::getPrintFiles()
{
    return this->printFiles;
}
bool Parameter::getReadGeo()
{
    return this->readGeo;
}
bool Parameter::getCalcTurbulenceIntensity()
{
    return this->calcVelocityAndFluctuations;
}
real Parameter::getDiffusivity()
{
    return this->Diffusivity;
}
real Parameter::getConcentrationInit()
{
    return this->concentrationInit;
}
real Parameter::getConcentrationBC()
{
    return this->concentrationBC;
}
real Parameter::getViscosity() const
{
    return this->vis;
}
real Parameter::getVelocity() const
{
    return this->u0;
}
real Parameter::getRealX()
{
    return this->RealX;
}
real Parameter::getRealY()
{
    return this->RealY;
}
real Parameter::getOutflowPressureCorrectionFactor()
{
    return this->outflowPressureCorrectionFactor;
}
int Parameter::getMaxDev()
{
    return this->maxdev;
}
int Parameter::getMyProcessID() const
{
    return this->myProcessId;
}
int Parameter::getNumprocs() const
{
    return this->numprocs;
}
std::vector<uint> Parameter::getDevices() const
{
    return this->devices;
}
real Parameter::getRe() const
{
    return this->Re;
}
real Parameter::getFactorPressBC()
{
    return this->factorPressBC;
}
std::vector<int> Parameter::getGridX()
{
    return this->GridX;
}
std::vector<int> Parameter::getGridY()
{
    return this->GridY;
}
std::vector<int> Parameter::getGridZ()
{
    return this->GridZ;
}
std::vector<real> Parameter::getScaleLBMtoSI()
{
    return this->scaleLBMtoSI;
}
std::vector<real> Parameter::getTranslateLBMtoSI()
{
    return this->translateLBMtoSI;
}
std::vector<real> Parameter::getMinCoordX()
{
    return this->minCoordX;
}
std::vector<real> Parameter::getMinCoordY()
{
    return this->minCoordY;
}
std::vector<real> Parameter::getMinCoordZ()
{
    return this->minCoordZ;
}
std::vector<real> Parameter::getMaxCoordX()
{
    return this->maxCoordX;
}
std::vector<real> Parameter::getMaxCoordY()
{
    return this->maxCoordY;
}
std::vector<real> Parameter::getMaxCoordZ()
{
    return this->maxCoordZ;
}
AdvectionDiffusionNoSlipBoundaryConditions *Parameter::getConcentrationNoSlipBCHost()
{
    return this->concentrationNoSlipBCHost;
}
AdvectionDiffusionNoSlipBoundaryConditions *Parameter::getConcentrationNoSlipBCDevice()
{
    return this->concentrationNoSlipBCDevice;
}
AdvectionDiffusionDirichletBoundaryConditions *Parameter::getConcentrationDirichletBCHost()
{
    return this->concentrationDirichletBCHost;
}
AdvectionDiffusionDirichletBoundaryConditions *Parameter::getConcentrationDirichletBCDevice()
{
    return this->concentrationDirichletBCDevice;
}
std::string Parameter::getgeoVec()
{
    return this->geoVec;
}
std::string Parameter::getcoordX()
{
    return this->coordX;
}
std::string Parameter::getcoordY()
{
    return this->coordY;
}
std::string Parameter::getcoordZ()
{
    return this->coordZ;
}
std::string Parameter::getneighborX()
{
    return this->neighborX;
}
std::string Parameter::getneighborY()
{
    return this->neighborY;
}
std::string Parameter::getneighborZ()
{
    return this->neighborZ;
}
std::string Parameter::getneighborWSB()
{
    return this->neighborWSB;
}
std::string Parameter::getscaleCFC()
{
    return this->scaleCFC;
}
std::string Parameter::getscaleCFF()
{
    return this->scaleCFF;
}
std::string Parameter::getscaleFCC()
{
    return this->scaleFCC;
}
std::string Parameter::getscaleFCF()
{
    return this->scaleFCF;
}
std::string Parameter::getscaleOffsetCF()
{
    return this->scaleOffsetCF;
}
std::string Parameter::getscaleOffsetFC()
{
    return this->scaleOffsetFC;
}
std::string Parameter::getgeomBoundaryBcQs()
{
    return this->geomBoundaryBcQs;
}
std::string Parameter::getgeomBoundaryBcValues()
{
    return this->geomBoundaryBcValues;
}
std::string Parameter::getnoSlipBcPos()
{
    return this->noSlipBcPos;
}
std::string Parameter::getnoSlipBcQs()
{
    return this->noSlipBcQs;
}
std::string Parameter::getnoSlipBcValue()
{
    return this->noSlipBcValue;
}
std::string Parameter::getnoSlipBcValues()
{
    return this->noSlipBcValues;
}
std::string Parameter::getslipBcPos()
{
    return this->slipBcPos;
}
std::string Parameter::getslipBcQs()
{
    return this->slipBcQs;
}
std::string Parameter::getslipBcValue()
{
    return this->slipBcValue;
}
std::string Parameter::getpressBcPos()
{
    return this->pressBcPos;
}
std::string Parameter::getpressBcQs()
{
    return this->pressBcQs;
}
std::string Parameter::getpressBcValue()
{
    return this->pressBcValue;
}
std::string Parameter::getpressBcValues()
{
    return this->pressBcValues;
}
std::string Parameter::getvelBcQs()
{
    return this->velBcQs;
}
std::string Parameter::getvelBcValues()
{
    return this->velBcValues;
}
std::string Parameter::getinletBcQs()
{
    return this->inletBcQs;
}
std::string Parameter::getinletBcValues()
{
    return this->inletBcValues;
}
std::string Parameter::getoutletBcQs()
{
    return this->outletBcQs;
}
std::string Parameter::getoutletBcValues()
{
    return this->outletBcValues;
}
std::string Parameter::gettopBcQs()
{
    return this->topBcQs;
}
std::string Parameter::gettopBcValues()
{
    return this->topBcValues;
}
std::string Parameter::getbottomBcQs()
{
    return this->bottomBcQs;
}
std::string Parameter::getbottomBcValues()
{
    return this->bottomBcValues;
}
std::string Parameter::getfrontBcQs()
{
    return this->frontBcQs;
}
std::string Parameter::getfrontBcValues()
{
    return this->frontBcValues;
}
std::string Parameter::getbackBcQs()
{
    return this->backBcQs;
}
std::string Parameter::getbackBcValues()
{
    return this->backBcValues;
}
std::string Parameter::getwallBcQs()
{
    return this->wallBcQs;
}
std::string Parameter::getwallBcValues()
{
    return this->wallBcValues;
}
std::string Parameter::getperiodicBcQs()
{
    return this->periodicBcQs;
}
std::string Parameter::getperiodicBcValues()
{
    return this->periodicBcValues;
}
std::string Parameter::getmeasurePoints()
{
    return this->measurePoints;
}
std::string Parameter::getLBMvsSI()
{
    return this->LBMvsSI;
}
std::string Parameter::getnumberNodes()
{
    return this->numberNodes;
}
std::string Parameter::getcpTop()
{
    return this->cpTop;
}
std::string Parameter::getcpBottom()
{
    return this->cpBottom;
}
std::string Parameter::getcpBottom2()
{
    return this->cpBottom2;
}
std::string Parameter::getConcentration()
{
    return this->concentration;
}
real Parameter::getclockCycleForMeasurePoints()
{
    return this->clockCycleForMeasurePoints;
}
unsigned int Parameter::getTimeDoCheckPoint()
{
    return this->tDoCheckPoint;
}
unsigned int Parameter::getTimeDoRestart() const
{
    return this->tDoRestart;
}

//=======================================================================================
//! \brief Get current (sub)time step of a given level.
//! \param level 
//! \param t current time step (of level 0)
//! \param isPostCollision whether getTimeStep is called post- (before swap) or pre- (after swap) collision
//!
unsigned int Parameter::getTimeStep(int level, unsigned int t, bool isPostCollision)
{
    if(level>this->getMaxLevel()) throw std::runtime_error("Parameter::getTimeStep: level>this->getMaxLevel()!");
    unsigned int tLevel = t;                                                                  
    if(level>0)
    {
        for(int i=1; i<level; i++){ tLevel = 1 + 2*(tLevel-1) + !this->getEvenOrOdd(i); }     
        bool addOne = isPostCollision? !this->getEvenOrOdd(level): this->getEvenOrOdd(level); 
        tLevel = 1 + 2*(tLevel-1) + addOne;
    }
    return tLevel;
}

bool Parameter::getDoCheckPoint()
{
    return this->doCheckPoint;
}
bool Parameter::getDoRestart() const
{
    return this->doRestart;
}
bool Parameter::getIsGeo()
{
    return this->isGeo;
}
bool Parameter::getIsCp()
{
    return this->isCp;
}
bool Parameter::getUseMeasurePoints()
{
    return this->isMeasurePoints;
}
vf::lbm::TurbulenceModel Parameter::getTurbulenceModel()
{
    return this->turbulenceModel;
}
bool Parameter::getUseTurbulentViscosity()
{
    return this->isTurbulentViscosity;
}
real Parameter::getSGSConstant()
{
    return this->SGSConstant;
}
bool Parameter::getHasWallModelMonitor()
{
    return this->hasWallModelMonitor;
}
std::vector<SPtr<PreCollisionInteractor>> Parameter::getInteractors()
{
    return interactors;
}
std::vector<SPtr<Sampler>> Parameter::getSamplers()
{
    return samplers;
}
bool Parameter::getUseInitNeq()
{
    return this->isInitNeq;
}

bool Parameter::getIsBodyForce()
{
    return this->isBodyForce;
}

bool Parameter::getIsGeometryValues()
{
    return this->GeometryValues;
}
bool Parameter::getCalc2ndOrderMoments()
{
    return this->is2ndOrderMoments;
}
bool Parameter::getCalc3rdOrderMoments()
{
    return this->is3rdOrderMoments;
}
bool Parameter::getCalcHighOrderMoments()
{
    return this->isHighOrderMoments;
}
bool Parameter::overWritingRestart(uint t)
{
    return t == getTimeDoRestart();
}
unsigned int Parameter::getTimestepForMeasurePoints()
{
    return this->timeStepForMeasurePoints;
}
unsigned int Parameter::getTimestepOfCoarseLevel()
{
    return this->timestep;
}
double Parameter::getMemsizeGPU()
{
    return this->memsizeGPU;
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

std::string Parameter::getMainKernel() const
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

void Parameter::setUseStreams(bool useStreams)
{
    if (useStreams) {
        if (this->getNumprocs() != 1) {
            this->useStreams = useStreams;
            return; 
        } else {
            VF_LOG_INFO( "Can't use streams with only one process!");
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

//! \}
