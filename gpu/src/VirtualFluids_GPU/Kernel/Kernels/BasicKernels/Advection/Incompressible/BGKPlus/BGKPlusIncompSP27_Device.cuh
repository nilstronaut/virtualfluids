#ifndef LB_KERNEL_BGK_PLUS_INCOMP_SP_27_H
#define LB_KERNEL_BGK_PLUS_INCOMP_SP_27_H

#include <DataTypes.h>
#include <curand.h>

extern "C" __global__ void LB_Kernel_BGK_Plus_Incomp_SP_27(real omega,
	unsigned int* bcMatD,
	unsigned int* neighborX,
	unsigned int* neighborY,
	unsigned int* neighborZ,
	real* DDStart,
	int size_Mat,
	bool EvenOrOdd);

#endif