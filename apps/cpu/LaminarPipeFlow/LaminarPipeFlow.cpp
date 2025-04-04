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
//! \addtogroup LaminarPipeFlow
//! \ingroup cpu_apps
//! \{
//! \author Konstantin Kutscher
//=======================================================================================
#include <iostream>
#include <string>
// #include <omp.h>

#include "VirtualFluids.h"

using namespace std;

void run(const vf::basics::ConfigurationFile& config)
{
    using namespace vf::lbm::dir;

    string pathname = config.getValue<string>("pathname");
    int numOfThreads = config.getValue<int>("numOfThreads");
    vector<int> blocknx = config.getVector<int>("blocknx");
    real endTime = config.getValue<real>("endTime");
    real outTime = config.getValue<real>("outTime");
    int refineLevel = config.getValue<int>("refineLevel");
    real dx = config.getValue<real>("dx");
    vector<real> length = config.getVector<real>("length");
    real restartStep = config.getValue<real>("restartStep");
    real cpStart = config.getValue<real>("cpStart");
    real cpStep = config.getValue<real>("cpStep");
    bool newStart = config.getValue<bool>("newStart");

    SPtr<vf::parallel::Communicator> comm = vf::parallel::MPICommunicator::getInstance();
    int myid = comm->getProcessID();

    real dLB = length[1] / dx;
    real rhoLB1 = 0.00001;
    real rhoLB2 = 0.0;
    real nuLB = 0.0064;

    SPtr<LBMUnitConverter> conv = SPtr<LBMUnitConverter>(new LBMUnitConverter());

    // boundary conditions
    //////////////////////////////////////////////////////////////////////////////
    SPtr<BC> noSlipBC(new NoSlipBC());
    noSlipBC->setBCStrategy(SPtr<BCStrategy>(new NoSlipInterpolated()));

    SPtr<BC> pressureBC1(new PressureBC(rhoLB1));
    pressureBC1->setBCStrategy(SPtr<BCStrategy>(new PressureNonEquilibrium()));

    SPtr<BC> pressureBC2(new PressureBC(rhoLB2));
    pressureBC2->setBCStrategy(SPtr<BCStrategy>(new PressureNonEquilibrium()));

    //////////////////////////////////////////////////////////////////////////////////
    // BC visitor
    BoundaryConditionsBlockVisitor bcVisitor;

    SPtr<Grid3D> grid(new Grid3D(comm));

    SPtr<BCSet> bcProc;
    bcProc = SPtr<BCSet>(new BCSet());

    SPtr<LBMKernel> kernel = SPtr<LBMKernel>(new K17CompressibleNavierStokes());
    kernel->setBCSet(bcProc);

    SPtr<Grid3DVisitor> metisVisitor(new MetisPartitioningGridVisitor(comm, MetisPartitioningGridVisitor::LevelBased, d00M));

    //////////////////////////////////////////////////////////////////////////
    // restart
    SPtr<UbScheduler> mSch(new UbScheduler(cpStep, cpStart));
    SPtr<MPIIOMigrationSimulationObserver> restart(
        new MPIIOMigrationSimulationObserver(grid, mSch, metisVisitor, pathname, comm));
    restart->setLBMKernel(kernel);
    restart->setBCSet(bcProc);
    //////////////////////////////////////////////////////////////////////////
    real R = length[1] / 2.0;
    real dp = (rhoLB1 / 3. - rhoLB2 / 3.);
    real L = length[0];
    real u_max = R * R / (4. * nuLB) * (dp / L);
    real Re = u_max * 2 * R / nuLB;

    if (myid == 0) {
        VF_LOG_INFO("Parameters:");
        VF_LOG_INFO("p1 = {}", rhoLB1 / 3.);
        VF_LOG_INFO("p1 = {}", rhoLB2 / 3.);
        VF_LOG_INFO("nuLb = {}", nuLB);
        VF_LOG_INFO("u_max = {}", u_max);
        VF_LOG_INFO("Re = {}", Re);
        VF_LOG_INFO("dx = {}", dx);
        VF_LOG_INFO("number of levels = {}", refineLevel + 1);
        VF_LOG_INFO("number of threads = {}", numOfThreads);
        VF_LOG_INFO("number of processes = {}", comm->getNumberOfProcesses());
        VF_LOG_INFO("path = {}", pathname);
        VF_LOG_INFO("Preprocess - start");
    }

    if (newStart) {

        // bounding box
        real g_minX1 = 0.0;
        real g_minX2 = -length[1] / 2.0;
        real g_minX3 = -length[2] / 2.0;

        real g_maxX1 = length[0];
        real g_maxX2 = length[1] / 2.0;
        real g_maxX3 = length[2] / 2.0;

        SPtr<GbObject3D> cylinder(new GbCylinder3D(g_minX1 - 2.0 * dx, 0.0, 0.0, g_maxX1 + 2.0 * dx, 0.0, 0.0, dLB / 2.0));
        GbSystem3D::writeGeoObject(cylinder.get(), pathname + "/geo/cylinder", WbWriterVtkXmlBinary::getInstance());

        SPtr<GbObject3D> gridCube(new GbCuboid3D(g_minX1, g_minX2, g_minX3, g_maxX1, g_maxX2, g_maxX3));
        if (myid == 0)
            GbSystem3D::writeGeoObject(gridCube.get(), pathname + "/geo/gridCube", WbWriterVtkXmlBinary::getInstance());

        real blockLength = 3. * dx;

        grid->setDeltaX(dx);
        grid->setBlockNX(blocknx[0], blocknx[1], blocknx[2]);
        grid->setPeriodicX1(false);
        grid->setPeriodicX2(false);
        grid->setPeriodicX3(false);

        if (myid == 0)
            GbSystem3D::writeGeoObject(gridCube.get(), pathname + "/geo/gridCube", WbWriterVtkXmlBinary::getInstance());

        GenBlocksGridVisitor genBlocks(gridCube);
        grid->accept(genBlocks);

        SPtr<GbObject3D> refineCube1_1(new GbCuboid3D(g_minX1, g_minX2, g_minX3, g_maxX1, g_minX2 + blockLength, g_maxX3));
        if (myid == 0)
            GbSystem3D::writeGeoObject(refineCube1_1.get(), pathname + "/geo/refineCube1_1",
                                       WbWriterVtkXmlBinary::getInstance());

        SPtr<GbObject3D> refineCube1_2(new GbCuboid3D(g_minX1, g_maxX2 - blockLength, g_minX3, g_maxX1, g_maxX2, g_maxX3));
        if (myid == 0)
            GbSystem3D::writeGeoObject(refineCube1_2.get(), pathname + "/geo/refineCube1_2",
                                       WbWriterVtkXmlBinary::getInstance());

        SPtr<GbObject3D> refineCube1_3(new GbCuboid3D(g_minX1, g_minX2, g_minX3, g_maxX1, g_maxX2, g_minX3 + blockLength));
        if (myid == 0)
            GbSystem3D::writeGeoObject(refineCube1_3.get(), pathname + "/geo/refineCube1_3",
                                       WbWriterVtkXmlBinary::getInstance());

        SPtr<GbObject3D> refineCube1_4(new GbCuboid3D(g_minX1, g_minX2, g_maxX3 - blockLength, g_maxX1, g_maxX2, g_maxX3));
        if (myid == 0)
            GbSystem3D::writeGeoObject(refineCube1_4.get(), pathname + "/geo/refineCube1_4",
                                       WbWriterVtkXmlBinary::getInstance());

        if (refineLevel > 0) {
            if (myid == 0)
                VF_LOG_INFO("Refinement - start");
            RefineCrossAndInsideGbObjectHelper refineHelper(grid, refineLevel, comm);
            refineHelper.addGbObject(refineCube1_1, 1);
            refineHelper.addGbObject(refineCube1_2, 1);
            refineHelper.addGbObject(refineCube1_3, 1);
            refineHelper.addGbObject(refineCube1_4, 1);
            refineHelper.refine();
            if (myid == 0)
                VF_LOG_INFO("Refinement - end");
        }

        // inflow
        GbCuboid3DPtr geoInflow(new GbCuboid3D(g_minX1 - blockLength, g_minX2 - blockLength, g_minX3 - blockLength, g_minX1,
                                               g_maxX2 + blockLength, g_maxX3 + blockLength));
        if (myid == 0)
            GbSystem3D::writeGeoObject(geoInflow.get(), pathname + "/geo/geoInflow", WbWriterVtkXmlASCII::getInstance());

        // outflow
        GbCuboid3DPtr geoOutflow(new GbCuboid3D(g_maxX1, g_minX2 - blockLength, g_minX3 - blockLength, g_maxX1 + blockLength,
                                                g_maxX2 + blockLength, g_maxX3 + blockLength));
        if (myid == 0)
            GbSystem3D::writeGeoObject(geoOutflow.get(), pathname + "/geo/geoOutflow", WbWriterVtkXmlASCII::getInstance());

        SPtr<D3Q27Interactor> cylinderInt(new D3Q27Interactor(cylinder, grid, noSlipBC, Interactor3D::INVERSESOLID));

        SPtr<D3Q27Interactor> inflowInt =
            SPtr<D3Q27Interactor>(new D3Q27Interactor(geoInflow, grid, pressureBC1, Interactor3D::SOLID));

        SPtr<D3Q27Interactor> outflowInt =
            SPtr<D3Q27Interactor>(new D3Q27Interactor(geoOutflow, grid, pressureBC2, Interactor3D::SOLID));

        InteractorsHelper intHelper(grid, metisVisitor);
        intHelper.addInteractor(cylinderInt);
        intHelper.addInteractor(inflowInt);
        intHelper.addInteractor(outflowInt);
        if (myid == 0)
            VF_LOG_INFO("Select blocks - start");
        intHelper.selectBlocks();
        if (myid == 0)
            VF_LOG_INFO("Select blocks - end");

        SPtr<SimulationObserver> ppblocks(new WriteBlocksSimulationObserver(
            grid, SPtr<UbScheduler>(new UbScheduler(1)), pathname, WbWriterVtkXmlBinary::getInstance(), comm));
        ppblocks->update(0);
        ppblocks.reset();

        if (myid == 0)
            VF_LOG_INFO("{}", Utilities::toString(grid, comm->getNumberOfProcesses()));

        SetKernelBlockVisitor kernelVisitor(kernel, nuLB, comm->getNumberOfProcesses());
        grid->accept(kernelVisitor);

        if (refineLevel > 0) {
            SetUndefinedNodesBlockVisitor undefNodesVisitor;
            grid->accept(undefNodesVisitor);
        }

        if (myid == 0)
            VF_LOG_INFO("Set BC - start");
        intHelper.setBC();
        if (myid == 0)
            VF_LOG_INFO("Set BC - end");

        // initialization of distributions
        if (myid == 0)
            VF_LOG_INFO("Initialization of distributions - start");
        InitDistributionsBlockVisitor initVisitor;
        grid->accept(initVisitor);
        if (myid == 0)
            VF_LOG_INFO("Initialization of distributions - end");

        // boundary conditions grid
        {
            SPtr<UbScheduler> geoSch(new UbScheduler(1));
            SPtr<SimulationObserver> ppgeo(new WriteBoundaryConditionsSimulationObserver(
                grid, geoSch, pathname, WbWriterVtkXmlBinary::getInstance(), comm));
            ppgeo->update(0);
            ppgeo.reset();
        }

    } else {
        restart->restart((int)restartStep);
        grid->setTimeStep(restartStep);

        if (myid == 0)
            VF_LOG_INFO("Restart - end");
    }

    grid->accept(bcVisitor);

    OneDistributionSetConnectorsBlockVisitor setConnsVisitor(comm);
    grid->accept(setConnsVisitor);

    SPtr<Interpolator> iProcessor(new CompressibleOffsetMomentsInterpolator());
    SetInterpolationConnectorsBlockVisitor setInterConnsVisitor(comm, nuLB, iProcessor);
    grid->accept(setInterConnsVisitor);

    SPtr<UbScheduler> visSch(new UbScheduler(outTime));
    SPtr<SimulationObserver> pp(new WriteMacroscopicQuantitiesSimulationObserver(
        grid, visSch, pathname, WbWriterVtkXmlBinary::getInstance(), conv, comm));

    SPtr<UbScheduler> nupsSch(new UbScheduler(10, 10, 100));
    SPtr<SimulationObserver> npr(new NUPSCounterSimulationObserver(grid, nupsSch, numOfThreads, comm));

#ifdef _OPENMP
    omp_set_num_threads(numOfThreads);
#endif

    if (myid == 0)
        VF_LOG_INFO("Preprocess - end");

    SPtr<UbScheduler> stepGhostLayer(visSch);
    SPtr<Simulation> simulation(new Simulation(grid, stepGhostLayer, int(endTime)));
    simulation->addSimulationObserver(npr);
    simulation->addSimulationObserver(pp);
    simulation->addSimulationObserver(restart);

    if (myid == 0)
        VF_LOG_INFO("Simulation-start");
    simulation->run();
    if (myid == 0)
        VF_LOG_INFO("Simulation-end");
}

int main(int argc, char* argv[])
{
    try {
        vf::logging::Logger::initializeLogger();
        auto config = vf::basics::loadConfig(argc, argv);
        run(config);
    } catch (const std::exception& e) {
        VF_LOG_WARNING("{}", e.what());
        return 1;
    }
    return 0;
}

//! \}
