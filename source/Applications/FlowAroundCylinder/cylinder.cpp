#include <iostream>
#include <string>

#include "VirtualFluids.h"

using namespace std;


//////////////////////////////////////////////////////////////////////////
void run()
{
   try
   {
      //DEBUG///////////////////////////////////////
      //Sleep(30000);
      /////////////////////////////////////////////
      
      string machine = QUOTEME(CAB_MACHINE);
      string pathname; 
      int numOfThreads = 4;
      double availMem = 0;

      CommunicatorPtr comm = MPICommunicator::getInstance();
      int myid = comm->getProcessID();

      if(machine == "BOMBADIL") 
      {
         pathname = "d:/temp/cylinderComp";
         numOfThreads = 1;
         availMem = 10.0e9;
      }
      else if(machine == "M01" || machine == "M02")      
      {
         pathname = "/work/koskuche/scratch/cylinder_Re20nu4l";
         numOfThreads = 1;
         availMem = 12.0e9;

         if(myid ==0)
         {
            stringstream logFilename;
            logFilename <<  pathname + "/logfile"+UbSystem::toString(UbSystem::getTimeStamp())+".txt";
            UbLog::output_policy::setStream(logFilename.str());
         }
      }
      else throw UbException(UB_EXARGS, "unknown CAB_MACHINE");

      UBLOG(logINFO, "Test case: flow around cylinder");

      double dx = 0.04;

      double L1 = 2.5*2.0;
      double L2, L3, H;
      L2 = L3 = H = 0.41*2.0;

      LBMReal radius = 0.05*2.0;
      LBMReal rhoReal = 1.0; //kg/m^3
      LBMReal uReal = 0.45;//m/s
      LBMReal uLB = 0.1;
      LBMReal Re = 20000.0;
      LBMReal rhoLB = 0.0;
      LBMReal l = L2 / dx;

      //LBMUnitConverterPtr conv = LBMUnitConverterPtr(new LBMUnitConverter(1.0, 1/sqrt(3.0)*(uReal/uLB), 1.0, 1.0/dx, dx*dx*dx));
      LBMUnitConverterPtr conv = LBMUnitConverterPtr(new LBMUnitConverter());

      const int baseLevel = 0;
      const int refineLevel = 1;

      //obstacle
      GbObject3DPtr cylinder(new GbCylinder3D(0.5*2.0, 0.2*2.0, -0.1, 0.5*2.0, 0.2*2.0, L3+0.1, radius));
      GbSystem3D::writeGeoObject(cylinder.get(),pathname + "/geo/cylinder", WbWriterVtkXmlBinary::getInstance());

      GbObject3DPtr refCylinder(new GbCylinder3D(0.5*2.0, 0.2*2.0, -0.1, 0.5*2.0, 0.2*2.0, L3+0.1, radius+7.0*dx/(1<<refineLevel)));
      GbSystem3D::writeGeoObject(refCylinder.get(),pathname + "/geo/refCylinder", WbWriterVtkXmlBinary::getInstance());

      D3Q27InteractorPtr cylinderInt;

      //bounding box
      double d_minX1 = 0.0;
      double d_minX2 = 0.0;
      double d_minX3 = 0.0;

      double d_maxX1 = L1;
      double d_maxX2 = L2;
      double d_maxX3 = L3;

      //double offs = dx;
      double offs = 0;

      //double g_minX1 = d_minX1-offs-0.499999*dx;
      double g_minX1 = d_minX1;
      double g_minX2 = d_minX2-7.0*dx;
      double g_minX3 = d_minX3;

      double g_maxX1 = d_maxX1;
      double g_maxX2 = d_maxX2;
      double g_maxX3 = d_maxX3;

      GbObject3DPtr gridCube(new GbCuboid3D(g_minX1, g_minX2, g_minX3, g_maxX1, g_maxX2, g_maxX3));

      const int blocknx1 = 8;
      const int blocknx2 = 8;
      const int blocknx3 = 8;

      //dx = (0.41+2.0*dx)/(10.0*(int)blocknx2);

      LBMReal nueLB = (((4.0/9.0)*uLB)*2.0*(radius/dx))/Re;

      double blockLength = blocknx1*dx;

      //refinement area
      double rf = cylinder->getLengthX1()/4;
      GbObject3DPtr refineCube(new  GbCuboid3D(cylinder->getX1Minimum()-rf, cylinder->getX2Minimum()-rf, cylinder->getX3Minimum(), 
         cylinder->getX1Maximum()+rf, cylinder->getX2Maximum()+rf, cylinder->getX3Maximum()));
      //       GbObject3DPtr refineCube(new  GbCuboid3D(g_minX1 + 7.05*blockLength, g_minX2 + 3.05*blockLength, cylinder->getX3Minimum(), 
      //          g_minX1 + 12.95*blockLength, g_maxX2 - 3.05*blockLength, cylinder->getX3Maximum()));

      Grid3DPtr grid(new Grid3D(comm));

      UbSchedulerPtr rSch(new UbScheduler(100000, 100000));
      //RestartPostprocessorPtr rp(new RestartPostprocessor(grid, rSch, comm, pathname+"/checkpoints", RestartPostprocessor::BINARY));

      //UbSchedulerPtr emSch(new UbScheduler(1000, 1000));
      //EmergencyExitPostprocessor em(grid, emSch, pathname+"/checkpoints/emex.txt", rp, comm);


      //BC
      BCAdapterPtr noSlipAdapter(new NoSlipBCAdapter());
      noSlipAdapter->setBcAlgorithm(BCAlgorithmPtr(new NoSlipBCAlgorithm()));

      mu::Parser fct;
      fct.SetExpr("16*U*x2*x3*(H-x2)*(H-x3)/H^4");
      fct.DefineConst("U", uLB);
      fct.DefineConst("H", H);
      BCAdapterPtr velBCAdapter(new VelocityBCAdapter(true, false, false, fct, 0, BCFunction::INFCONST));
      velBCAdapter->setBcAlgorithm(BCAlgorithmPtr(new NonReflectingVelocityBCAlgorithm()));

      BCAdapterPtr denBCAdapter(new DensityBCAdapter(rhoLB));
      denBCAdapter->setBcAlgorithm(BCAlgorithmPtr(new NonReflectingDensityBCAlgorithm()));
      
      BoundaryConditionsBlockVisitor bcVisitor;
      bcVisitor.addBC(noSlipAdapter);
      bcVisitor.addBC(velBCAdapter);
      bcVisitor.addBC(denBCAdapter);


      if (grid->getTimeStep() == 0)
      {
         if (myid==0)
         {
            UBLOG(logINFO, "Number of processes = "<<comm->getNumberOfProcesses());
            UBLOG(logINFO, "Number of threads = "<<numOfThreads);
            UBLOG(logINFO, "path = "<<pathname);
            UBLOG(logINFO, "L = "<<L1/dx);
            UBLOG(logINFO, "H = "<<H/dx);
            UBLOG(logINFO, "v = "<<uLB);
            UBLOG(logINFO, "rho = "<<rhoLB);
            UBLOG(logINFO, "nue = "<<nueLB);
            UBLOG(logINFO, "Re = "<<Re);
            UBLOG(logINFO, "dx = "<<dx);
            UBLOG(logINFO, "Number of level = "<<refineLevel+1);
            //UBLOG(logINFO,conv->toString() );
            UBLOG(logINFO, "Preprozess - start");
         }

         grid->setDeltaX(dx);
         grid->setBlockNX(blocknx1, blocknx2, blocknx3);
         grid->setPeriodicX3(true);

         // UbTupleDouble6 bouningBox(gridCube->getX1Minimum(),gridCube->getX2Minimum(),gridCube->getX3Minimum(),
         // gridCube->getX1Maximum(),gridCube->getX2Maximum(),gridCube->getX3Maximum());
         // UbTupleInt3 blockNx(blocknx1, blocknx2, blocknx3);
         // UbTupleInt3 gridNx(8, 16, 16);
         // grid = Grid3DPtr(new Grid3D(bouningBox, blockNx, gridNx));

         if (myid==0) GbSystem3D::writeGeoObject(gridCube.get(), pathname+"/geo/gridCube", WbWriterVtkXmlBinary::getInstance());
         if (myid==0) GbSystem3D::writeGeoObject(refineCube.get(), pathname+"/geo/refineCube", WbWriterVtkXmlBinary::getInstance());

         GenBlocksGridVisitor genBlocks(gridCube);
         grid->accept(genBlocks);

         //walls
         GbCuboid3DPtr addWallYmin(new GbCuboid3D(d_minX1-blockLength, d_minX2-blockLength, d_minX3-blockLength, d_maxX1+blockLength, d_minX2, d_maxX3+blockLength));
         if (myid==0) GbSystem3D::writeGeoObject(addWallYmin.get(), pathname+"/geo/addWallYmin", WbWriterVtkXmlASCII::getInstance());

         GbCuboid3DPtr addWallYmax(new GbCuboid3D(d_minX1-blockLength, d_maxX2, d_minX3-blockLength, d_maxX1+blockLength, d_maxX2+blockLength, d_maxX3+blockLength));
         if (myid==0) GbSystem3D::writeGeoObject(addWallYmax.get(), pathname+"/geo/addWallYmax", WbWriterVtkXmlASCII::getInstance());

         //GbCuboid3DPtr addWallZmin(new GbCuboid3D(d_minX1-blockLength, d_minX2-blockLength, d_minX3-blockLength, d_maxX1+blockLength, d_maxX2+blockLength, d_minX3));
         //if (myid==0) GbSystem3D::writeGeoObject(addWallZmin.get(), pathname+"/geo/addWallZmin", WbWriterVtkXmlASCII::getInstance());

         //GbCuboid3DPtr addWallZmax(new GbCuboid3D(d_minX1-blockLength, d_minX2-blockLength, d_maxX3, d_maxX1+blockLength, d_maxX2+blockLength, d_maxX3+blockLength));
         //if (myid==0) GbSystem3D::writeGeoObject(addWallZmax.get(), pathname+"/geo/addWallZmax", WbWriterVtkXmlASCII::getInstance());

         //inflow
         GbCuboid3DPtr geoInflow(new GbCuboid3D(d_minX1-blockLength, d_minX2-blockLength, d_minX3-blockLength, d_minX1, d_maxX2+blockLength, d_maxX3+blockLength));
         if (myid==0) GbSystem3D::writeGeoObject(geoInflow.get(), pathname+"/geo/geoInflow", WbWriterVtkXmlASCII::getInstance());

         //outflow
         GbCuboid3DPtr geoOutflow(new GbCuboid3D(d_maxX1, d_minX2-blockLength, d_minX3-blockLength, d_maxX1+blockLength, d_maxX2+blockLength, d_maxX3+blockLength));
         if (myid==0) GbSystem3D::writeGeoObject(geoOutflow.get(), pathname+"/geo/geoOutflow", WbWriterVtkXmlASCII::getInstance());

         WriteBlocksCoProcessorPtr ppblocks(new WriteBlocksCoProcessor(grid, UbSchedulerPtr(new UbScheduler(1)), pathname, WbWriterVtkXmlBinary::getInstance(), comm));

         if (refineLevel>0)
         {
            if (myid==0) UBLOG(logINFO, "Refinement - start");
            RefineCrossAndInsideGbObjectHelper refineHelper(grid, refineLevel);
            //refineHelper.addGbObject(refineCube, refineLevel);
            refineHelper.addGbObject(refCylinder, refineLevel);
            refineHelper.refine();
            if (myid==0) UBLOG(logINFO, "Refinement - end");
         }

         //cylinder
         cylinderInt = D3Q27InteractorPtr(new D3Q27Interactor(cylinder, grid, noSlipAdapter, Interactor3D::SOLID));

         //walls
         D3Q27InteractorPtr addWallYminInt(new D3Q27Interactor(addWallYmin, grid, noSlipAdapter, Interactor3D::SOLID));
         D3Q27InteractorPtr addWallYmaxInt(new D3Q27Interactor(addWallYmax, grid, noSlipAdapter, Interactor3D::SOLID));
         //D3Q27InteractorPtr addWallZminInt(new D3Q27Interactor(addWallZmin, grid, noSlipAdapter, Interactor3D::SOLID));
         //D3Q27InteractorPtr addWallZmaxInt(new D3Q27Interactor(addWallZmax, grid, noSlipAdapter, Interactor3D::SOLID));

         //inflow
         D3Q27InteractorPtr inflowInt = D3Q27InteractorPtr(new D3Q27Interactor(geoInflow, grid, velBCAdapter, Interactor3D::SOLID));

         //outflow
         D3Q27InteractorPtr outflowInt = D3Q27InteractorPtr(new D3Q27Interactor(geoOutflow, grid, denBCAdapter, Interactor3D::SOLID));

         //D3Q27InteractorPtr addWallYminInt(new D3Q27Interactor(addWallYmin, grid, denBCAdapter, Interactor3D::SOLID));
         //D3Q27InteractorPtr addWallYmaxInt(new D3Q27Interactor(addWallYmax, grid, denBCAdapter, Interactor3D::SOLID));
         
         Grid3DVisitorPtr metisVisitor(new MetisPartitioningGridVisitor(comm, MetisPartitioningGridVisitor::LevelBased, D3Q27System::B));
         InteractorsHelper intHelper(grid, metisVisitor);
         intHelper.addInteractor(cylinderInt);
         intHelper.addInteractor(addWallYminInt);
         intHelper.addInteractor(addWallYmaxInt);
         //intHelper.addInteractor(addWallZminInt);
         //intHelper.addInteractor(addWallZmaxInt);
         intHelper.addInteractor(inflowInt);
         intHelper.addInteractor(outflowInt);
         intHelper.selectBlocks();


         ppblocks->process(0);
         ppblocks.reset();

         //set connectors
         //D3Q27InterpolationProcessorPtr iProcessor(new D3Q27IncompressibleOffsetInterpolationProcessor());
         InterpolationProcessorPtr iProcessor(new CompressibleOffsetInterpolationProcessor());
         SetConnectorsBlockVisitor setConnsVisitor(comm, true, D3Q27System::ENDDIR, nueLB, iProcessor);
         grid->accept(setConnsVisitor);

         unsigned long nob = grid->getNumberOfBlocks();
         int gl = 3;
         unsigned long nod = nob * (blocknx1+gl) * (blocknx2+gl) * (blocknx3+gl);

         double needMemAll = double(nod*(27*sizeof(double)+sizeof(int)+sizeof(float)*4));
         double needMem = needMemAll/double(comm->getNumberOfProcesses());

         if (myid==0)
         {
            UBLOG(logINFO, "Number of blocks = "<<nob);
            UBLOG(logINFO, "Number of nodes  = "<<nod);
            UBLOG(logINFO, "Necessary memory  = "<<needMemAll<<" bytes");
            UBLOG(logINFO, "Necessary memory per process = "<<needMem<<" bytes");
            UBLOG(logINFO, "Available memory per process = "<<availMem<<" bytes");
         }

         //LBMKernel3DPtr kernel(new LBMKernelETD3Q27CCLB(blocknx1, blocknx2, blocknx3, LBMKernelETD3Q27CCLB::NORMAL));
         LBMKernelPtr kernel(new CompressibleCumulantLBMKernel(blocknx1, blocknx2, blocknx3, CompressibleCumulantLBMKernel::NORMAL));

         BCProcessorPtr bcProc(new BCProcessor());
         kernel->setBCProcessor(bcProc);

         SetKernelBlockVisitor kernelVisitor(kernel, nueLB, availMem, needMem);
         grid->accept(kernelVisitor);

         if (refineLevel>0)
         {
            SetUndefinedNodesBlockVisitor undefNodesVisitor;
            grid->accept(undefNodesVisitor);
         }

         intHelper.setBC();

         grid->accept(bcVisitor);

         //domain decomposition
         PQueuePartitioningGridVisitor pqPartVisitor(numOfThreads);
         grid->accept(pqPartVisitor);

         //initialization of distributions
         InitDistributionsBlockVisitor initVisitor(nueLB, rhoLB);
         //initVisitor.setVx1(fct);
         grid->accept(initVisitor);

         //Postrozess
         UbSchedulerPtr geoSch(new UbScheduler(1));
         WriteBoundaryConditionsCoProcessorPtr ppgeo(
            new WriteBoundaryConditionsCoProcessor(grid, geoSch, pathname, WbWriterVtkXmlBinary::getInstance(), conv, comm));
         ppgeo->process(0);
         ppgeo.reset();

         if (myid==0) UBLOG(logINFO, "Preprozess - end");
      }
      else
      {
         //set connectors
         InterpolationProcessorPtr iProcessor(new CompressibleOffsetInterpolationProcessor());
         SetConnectorsBlockVisitor setConnsVisitor(comm, true, D3Q27System::ENDDIR, nueLB, iProcessor);
         grid->accept(setConnsVisitor);

         //domain decomposition
         PQueuePartitioningGridVisitor pqPartVisitor(numOfThreads);
         grid->accept(pqPartVisitor);

      }

      double outTime = 10.0;
      UbSchedulerPtr visSch(new UbScheduler(outTime));
      //visSch->addSchedule(1000, 1000, 10000);
      //visSch->addSchedule(10000, 10000, 50000);
      //visSch->addSchedule(100, 100, 10000);

      WriteMacroscopicQuantitiesCoProcessor pp(grid, visSch, pathname, WbWriterVtkXmlBinary::getInstance(), conv, comm);

      double fdx = grid->getDeltaX(grid->getFinestInitializedLevel());
      double point1[3] = {0.45, 0.20, 0.205};
      double point2[3] = {0.55, 0.20, 0.205};

      //D3Q27IntegrateValuesHelperPtr h1(new D3Q27IntegrateValuesHelper(grid, comm, 
      //   point1[0]-1.0*fdx, point1[1]-1.0*fdx, point1[2]-1.0*fdx, 
      //   point1[0], point1[1], point1[2]));
      //if(myid ==0) GbSystem3D::writeGeoObject(h1->getBoundingBox().get(),pathname + "/geo/iv1", WbWriterVtkXmlBinary::getInstance());
      //D3Q27IntegrateValuesHelperPtr h2(new D3Q27IntegrateValuesHelper(grid, comm, 
      //   point2[0], point2[1]-1.0*fdx, point2[2]-1.0*fdx, 
      //   point2[0]+1.0*fdx, point2[1], point2[2]));
      //if(myid ==0) GbSystem3D::writeGeoObject(h2->getBoundingBox().get(),pathname + "/geo/iv2", WbWriterVtkXmlBinary::getInstance());
      ////D3Q27PressureDifferencePostprocessor rhopp(grid, visSch, pathname + "/results/rho_diff.txt", h1, h2, conv, comm);
      //D3Q27PressureDifferencePostprocessor rhopp(grid, visSch, pathname + "/results/rho_diff.txt", h1, h2, rhoReal, uReal, uLB, comm);

      //double area = (2.0*radius*H)/(dx*dx);
      //double v    = 4.0*uLB/9.0;
      //CalculateForcesCoProcessor fp(grid, visSch, pathname + "/results/forces.txt", comm, v, area);
      //fp.addInteractor(cylinderInt);

      UbSchedulerPtr nupsSch(new UbScheduler(10, 10, 40));
      NUPSCounterCoProcessor npr(grid, nupsSch, numOfThreads, comm);

      double endTime = 100000;
      //CalculationManagerPtr calculation(new CalculationManager(grid, numOfThreads, endTime, visSch));
      CalculationManagerPtr calculation(new CalculationManager(grid, numOfThreads, endTime, visSch, CalculationManager::PrePostBc));
      if(myid == 0) UBLOG(logINFO,"Simulation-start");
      calculation->calculate();
      if(myid == 0) UBLOG(logINFO,"Simulation-end");
   }
   catch(std::exception& e)
   {
      cerr << e.what() << endl << flush;
   }
   catch(std::string& s)
   {
      cerr << s << endl;
   }
   catch(...)
   {
      cerr << "unknown exception" << endl;
   }

}
//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
   run();
   return 0;
}

