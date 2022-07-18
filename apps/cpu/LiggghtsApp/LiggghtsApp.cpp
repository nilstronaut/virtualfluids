#include <iostream>
#include <string>
#include <memory>

#include "VirtualFluids.h"

//#include "lammps.h"
//#include "input.h"
//#include "atom.h"
//#include "modify.h"
//#include "fix_lb_coupling_onetoone.h"

#include "LiggghtsCouplingCoProcessor.h"

#include "LiggghtsCouplingWrapper.h"


using namespace std;


int main(int argc, char *argv[])
{
    SPtr<Communicator> comm = MPICommunicator::getInstance();
    int myid                                        = comm->getProcessID();


    // bounding box
    double g_minX1 = 0;
    double g_minX2 = 0;
    double g_minX3 = 0;

    double g_maxX1 = 1;
    double g_maxX2 = 1;
    double g_maxX3 = 2;

    int blockNX[3] = { 10, 10, 20 };

    double dx = 0.1;

    double nuLB = 0.005;

    SPtr<Grid3D> grid = make_shared<Grid3D>(comm);
    grid->setPeriodicX1(true);
    grid->setPeriodicX2(true);
    grid->setPeriodicX3(true);
    grid->setDeltaX(dx);
    grid->setBlockNX(blockNX[0], blockNX[1], blockNX[2]);

    string outputPath = "d:/temp/lll2";
    
    SPtr<GbObject3D> gridCube = make_shared <GbCuboid3D>(g_minX1, g_minX2, g_minX3, g_maxX1, g_maxX2, g_maxX3);
    if (myid == 0)
        GbSystem3D::writeGeoObject(gridCube.get(), outputPath + "/geo/gridCube", WbWriterVtkXmlBinary::getInstance());

    GenBlocksGridVisitor genBlocks(gridCube);
    grid->accept(genBlocks);

    SPtr<CoProcessor> ppblocks =
        make_shared <WriteBlocksCoProcessor>(grid, SPtr<UbScheduler>(new UbScheduler(1)), outputPath,
                                                          WbWriterVtkXmlBinary::getInstance(), comm);
    ppblocks->process(0);
    ppblocks.reset();

    SPtr<LBMKernel> kernel = make_shared<IncompressibleCumulantLBMKernel>();
    SPtr<BCProcessor> bcProc = make_shared<BCProcessor>();
    kernel->setBCProcessor(bcProc);

    SetKernelBlockVisitor kernelVisitor(kernel, nuLB, 1e9, 1e9);
    grid->accept(kernelVisitor);

    InitDistributionsBlockVisitor initVisitor;
    grid->accept(initVisitor);

    SPtr<UbScheduler> lScheduler                    = make_shared<UbScheduler>(1);
    string inFile1                                   = "d:/Projects/VirtualFluids_LIGGGHTS_coupling/apps/cpu/LiggghtsApp/in.lbdem";
    //string inFile1 = "d:/Tools/LIGGGHTS/examples/LIGGGHTS/Tutorials_public/chute_wear/in.chute_wear2";
    string inFile2                                   = "d:/Projects/VirtualFluids_LIGGGHTS_coupling/apps/cpu/LiggghtsApp/in2.lbdem";
    MPI_Comm mpi_comm       = *(MPI_Comm*)(comm->getNativeCommunicator());
    LiggghtsCouplingWrapper wrapper(argv, mpi_comm);

    double d_part = 0.1;
    double v_frac = 0.1;
    double dt_phys  = 1; // units.getPhysTime(1);
    int demSubsteps = 1;
    double dt_dem   = 1e-1; //dt_phys / (double)demSubsteps;
    int vtkSteps    = 1;
    string demOutDir = "d:/temp/lll2/";

    wrapper.setVariable("r_part", d_part / 2);
    wrapper.setVariable("v_frac", v_frac);

    wrapper.execFile((char*)inFile1.c_str());

    //// set timestep and output directory
    wrapper.setVariable("t_step", dt_dem);
    wrapper.setVariable("dmp_stp", vtkSteps * demSubsteps);
    wrapper.setVariable("dmp_dir", demOutDir);

    wrapper.execFile((char *)inFile2.c_str());
    //wrapper.runUpto(demSubsteps - 1);

    SPtr<LiggghtsCouplingCoProcessor> lcCoProcessor =
        make_shared<LiggghtsCouplingCoProcessor>(grid, lScheduler, comm, wrapper, demSubsteps);

    // write data for visualization of macroscopic quantities
    SPtr<UbScheduler> visSch(new UbScheduler(vtkSteps));
    SPtr<WriteMacroscopicQuantitiesCoProcessor> writeMQCoProcessor(
        new WriteMacroscopicQuantitiesCoProcessor(grid, visSch, outputPath, WbWriterVtkXmlASCII::getInstance(),
                                                  SPtr<LBMUnitConverter>(new LBMUnitConverter()), comm));

    int endTime = 20;
    SPtr<Calculator> calculator(new BasicCalculator(grid, lScheduler, endTime));
    calculator->addCoProcessor(lcCoProcessor);
    calculator->addCoProcessor(writeMQCoProcessor);

    if (myid == 0) UBLOG(logINFO, "Simulation-start");
    calculator->calculate();
    if (myid == 0) UBLOG(logINFO, "Simulation-end");

    //MPI_Init(&argc, &argv);
    //MPI_Comm mpi_comm       = *(MPI_Comm*)(comm->getNativeCommunicator());
    //LiggghtsCouplingWrapper wrapper(argv, mpi_comm);

    //wrapper.execFile("in2.lbdem");
    //wrapper.runUpto(demSubsteps - 1);

	//LAMMPS_NS::LAMMPS *lmp;
 //   // custom argument vector for LAMMPS library
 //   const char *lmpargv[] {"liblammps", "-log", "none"};
 //   int lmpargc = sizeof(lmpargv)/sizeof(const char *);

 //   // explicitly initialize MPI
 //   MPI_Init(&argc, &argv);

 //   // create LAMMPS instance
 //   lmp = new LAMMPS_NS::LAMMPS(lmpargc, (char **)lmpargv, MPI_COMM_WORLD);
 //   lmp->input->file("in.lbdem");
 //   //lmp->input->one("run 1");
 //   
 //   //# Try extracting a global value
 //   //    print("")
 //   //    print("Attempting to get the number of atoms in simulation")
 //   //    numAtoms = lmp.extract_global("natoms", 0)
 //   //    print("natoms =", numAtoms)

 //   //    # Try extracting atom's positions
 //   //    print("")
 //   //    print("Attempting to get the atom's positions")
 //   //    pos = lmp.extract_atom("x",3)
 //   //    for k in range(0,numAtoms):
 //   //        print("Pos[%i] = [%f, %f, %f]" % (k, pos[k][0], pos[k][1], pos[k][2]))

 //   LAMMPS_NS::FixLbCouplingOnetoone 
 //       *couplingFix 
 //       = dynamic_cast<LAMMPS_NS::FixLbCouplingOnetoone*>
 //       (lmp->modify->find_fix_style("couple/lb/onetoone",0));

 //   cout << "test1\n";
 //   
 //   //double **t_liggghts = couplingFix->get_torque_ptr();
 //   cout << "test2\n";

 //   lmp->input->one("run 9 upto");

 //   for (int step = 0; step < 10; step++)
 //   {
 //       

 //       int numAtoms = lmp->atom->natoms;

 //       //double** pos = (double**)lmp->atom->extract("x");
 //       double** pos = lmp->atom->x;
 //       
 //       //double* forceX = lmp->atom->fx;

 //       for (int i = 0; i < numAtoms; i++)
 //       {
 //           double **f_liggghts = couplingFix->get_force_ptr();
 //           double** force = lmp->atom->f;
 //           cout << "Pos[" << i << "] = [" << pos[i][0] << ", " << pos[i][1] << ", " << pos[i][2] << "]\n";
 //           cout << "Force1[" << i << "] = [" << f_liggghts[i][0] << ", " << f_liggghts[i][1] << ", " << f_liggghts[i][2] << "]\n";
 //           f_liggghts[i][0] += 0;
 //           f_liggghts[i][1] += 0;
 //           f_liggghts[i][2] += 500;
 //           cout << "Force2[" << i << "] = [" << force[i][0] << ", " << force[i][1] << ", " << force[i][2] << "]\n";
 //       }

 //       couplingFix->comm_force_torque();

 //       lmp->input->one("run 10000");
 //      
 //   }

 //   // delete LAMMPS instance
 //   delete lmp;

 //   // stop MPI environment
    //MPI_Finalize();
    return 0;
}
