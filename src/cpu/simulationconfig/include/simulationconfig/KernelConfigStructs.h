#ifndef VIRTUALFLUIDSPYTHONBINDINGS_KERNELCONFIGSTRUCTS_H
#define VIRTUALFLUIDSPYTHONBINDINGS_KERNELCONFIGSTRUCTS_H

#include <string>
#include <basics/DataTypes.h>

struct LBMKernelConfiguration {
    KernelFactory::KernelType kernelType;
    bool useForcing = false;
    real forcingX1{};
    real forcingX2{};
    real forcingX3{};

    explicit LBMKernelConfiguration(KernelFactory::KernelType kernelType) : kernelType(kernelType)
    {
    }
};

#endif //VIRTUALFLUIDSPYTHONBINDINGS_KERNELCONFIGSTRUCTS_H
