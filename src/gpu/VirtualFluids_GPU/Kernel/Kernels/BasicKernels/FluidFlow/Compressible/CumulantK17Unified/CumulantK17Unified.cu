#include "CumulantK17Unified.h"

#include <stdexcept>

#include "Parameter/Parameter.h"
#include "../RunLBMKernel.cuh"
#include "Kernel/Utilities/CudaGrid.h"

#include <lbm/CumulantChimera.h>


std::shared_ptr<CumulantK17Unified> CumulantK17Unified::getNewInstance(std::shared_ptr<Parameter> para, int level)
{
    return std::make_shared<CumulantK17Unified>(para, level);
}

void CumulantK17Unified::run()
{
    vf::gpu::GPUKernelParameter kernelParameter{ para->getParD(level)->omega,
                                                 para->getParD(level)->geoSP,
                                                 para->getParD(level)->neighborX_SP,
                                                 para->getParD(level)->neighborY_SP,
                                                 para->getParD(level)->neighborZ_SP,
                                                 para->getParD(level)->d0SP.f[0],
                                                 (int)para->getParD(level)->size_Mat_SP,
                                                 para->getParD(level)->forcing,
                                                 para->getParD(level)->evenOrOdd };

    auto lambda = [] __device__(vf::lbm::KernelParameter parameter) {
        return vf::lbm::cumulantChimera(parameter, vf::lbm::setRelaxationRatesK17);
    };

    vf::gpu::runKernel<<<cudaGrid.grid, cudaGrid.threads>>>(lambda, kernelParameter);

    getLastCudaError("LB_Kernel_CumulantK17Unified execution failed");
}

CumulantK17Unified::CumulantK17Unified(std::shared_ptr<Parameter> para, int level)
{
#ifndef BUILD_CUDA_LTO
    throw std::invalid_argument("To use the CumulantK17Unified kernel, pass -DBUILD_CUDA_LTO=ON to cmake. Requires: CUDA 11.2 & cc 5.0");
#endif

    this->para  = para;
    this->level = level;

    myPreProcessorTypes.push_back(InitCompSP27);

    myKernelGroup = BasicKernel;

    this->cudaGrid = vf::gpu::CudaGrid(para->getParD(level)->numberofthreads, para->getParD(level)->size_Mat_SP);
}
