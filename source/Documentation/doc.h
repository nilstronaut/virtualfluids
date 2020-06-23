//index page for doxygen

/*! \mainpage
 *
 *  \section intro_sec Introduction
 *  <i>VirtualFluids</i> is an adaptive, parallel Lattice-Boltzmann flow solver 
 *  which is comprised of various cores 
 *  and use second order accurate compact interpolation at the interfaces, 
 *  coupling grids of different resolutions. 
 *  The software framework is based on object-oriented technology and uses tree-like data structures. 
 *  These data structures are also suitable for hierarchical parallelization using a combination of PThreads and MPI and dynamic load balancing. 
 *   
 *  \section install_sec Installation
 *
 *  \subsection step1 Step 1: CMake
 *  Download and install <a href="http://www.cmake.org/">CMake</a>. 
 *  If you have Windows OS oder X-Windows for Linux/Unix start <i>cmake-gui</i>.
 *  If you work in terminal start <i>ccmake</i>.
 *  \subsection step2 Step 2: project configuration
 *  You need set a path for <i>VirtualFluids</i> source code. It is allays <i>source</i> directory at the end of path.
 *  E.g. c:/vf/source. You need also set a path to build binaries. If you click <b>Configure</b> button start a project wizard and you can choose a compiler.
 *  Set <b>Grouped</b> and <b>Advanced</b> check boxes in CMake. In the group <i>USE</i> you have follow important options:
 *  \li USE_BOND    - using an agent based communication framework for fault tolerance BOND
 *  \li USE_METIS   - using a domain decomposition tool METIS    
 *  \li USE_MPI       - using MPI for distributed memory parallelization   
 *  \li USE_YAML     - using YAML
 *  \li USE_ZOLTAN - using domain decomposition tool ZOLTAN 
 *  There are externals library and you need additionally compile them.    
 *  \subsection step3 Step 4: project compilation
 *  Compile you project with suitable compiler.  
 */