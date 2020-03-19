#ifndef ThermalDependencies_H
#define ThermalDependencies_H

#ifdef __CUDACC__
#include <cuda_runtime.h>
#else
#define __host__
#define __device__
#endif

#include <math.h>

#include "Core/DataTypes.h"
#include "Core/RealConstants.h"

#include "Definitions/PassiveScalar.h"

#include "FlowStateData/FlowStateData.cuh"
#include "FlowStateData/HeatCapacities.cuh"

namespace GksGpu {

#ifdef USE_PASSIVE_SCALAR

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define R_U  real( 8.31445984848 )

#define M_A  real( 0.02884 )
#define M_P  real( 0.0276199095022624 )
#define M_F  real( 0.016 )

#define M_O2 real( 0.032 )

#define rX   real( 0.21 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__host__ __device__ inline real getT( const PrimitiveVariables& prim )
{
    real T = M_A / ( c2o1 * prim.lambda * R_U );

    return T;
}

__host__ __device__ inline void setLambdaFromT( PrimitiveVariables& prim, real T )
{
    prim.lambda =  M_A / ( c2o1 * T * R_U );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // USE_PASSIVE_SCALAR

} // namespace GksGpu




#endif

