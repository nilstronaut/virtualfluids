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
//! \file simulation.cpp
//! \ingroup submodules
//! \author Henry Korb
//=======================================================================================
#include <pybind11/pybind11.h>
#include <gpu/VirtualFluids_GPU/LBM/Simulation.h>
#include <gpu/VirtualFluids_GPU/Communication/Communicator.h>
#include <gpu/VirtualFluids_GPU/Kernel/Utilities/KernelFactory/KernelFactory.h>
#include <gpu/VirtualFluids_GPU/PreProcessor/PreProcessorFactory/PreProcessorFactory.h>
#include <gpu/VirtualFluids_GPU/DataStructureInitializer/GridProvider.h>
#include <gpu/VirtualFluids_GPU/Parameter/Parameter.h>
#include <gpu/VirtualFluids_GPU/GPU/CudaMemoryManager.h>
#include <gpu/VirtualFluids_GPU/DataStructureInitializer/GridProvider.h>
#include <gpu/VirtualFluids_GPU/Output/DataWriter.h>
#include "gpu/VirtualFluids_GPU/Factories/BoundaryConditionFactory.h"
#include "gpu/VirtualFluids_GPU/TurbulenceModels/TurbulenceModelFactory.h"
#include "gpu/VirtualFluids_GPU/Factories/GridScalingFactory.h"

namespace simulation
{
    namespace py = pybind11;

    void makeModule(py::module_ &parentModule)
    {
        // missing setFactories and setDataWriter, not possible to wrap these functions as long as they take unique ptr arguments
        py::class_<Simulation>(parentModule, "Simulation")
        .def(py::init<  std::shared_ptr<Parameter>,
                        std::shared_ptr<CudaMemoryManager>,
                        vf::gpu::Communicator &,
                        GridProvider &,
                        BoundaryConditionFactory*,
                        GridScalingFactory*>(), 
                        py::arg("parameter"),
                        py::arg("memoryManager"),
                        py::arg("communicator"),
                        py::arg("gridProvider"),
                        py::arg("bcFactory"),
                        py::arg("gridScalingFactory"))
        .def(py::init<  std::shared_ptr<Parameter>,
                        std::shared_ptr<CudaMemoryManager>,
                        vf::gpu::Communicator &,
                        GridProvider &,
                        BoundaryConditionFactory*>(), 
                        py::arg("parameter"),
                        py::arg("memoryManager"),
                        py::arg("communicator"),
                        py::arg("gridProvider"),
                        py::arg("bcFactory"))
        .def(py::init<  std::shared_ptr<Parameter>,
                        std::shared_ptr<CudaMemoryManager>,
                        vf::gpu::Communicator &,
                        GridProvider &,
                        BoundaryConditionFactory*,
                        std::shared_ptr<TurbulenceModelFactory>,
                        GridScalingFactory*>(), 
                        py::arg("parameter"),
                        py::arg("memoryManager"),
                        py::arg("communicator"),
                        py::arg("gridProvider"),
                        py::arg("bcFactory"),
                        py::arg("tmFactory"),
                        py::arg("gridScalingFactory"))
        .def("run", &Simulation::run)
        .def("addKineticEnergyAnalyzer", &Simulation::addKineticEnergyAnalyzer, py::arg("t_analyse"))
        .def("addEnstrophyAnalyzer", &Simulation::addEnstrophyAnalyzer, py::arg("t_analyse"));
    }
}