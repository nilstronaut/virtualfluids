#ifndef FlowStateData_H
#define FlowStateData_H

#ifdef __CUDACC__
#include <cuda_runtime.h>
#else
#define __host__
#define __device__
#endif

#include "Core/DataTypes.h"
#include "Core/RealConstants.h"

#include "Definitions/PassiveScalar.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct PrimitiveVariables
{
    real rho;
    real U;
    real V;
    real W;
    real lambda;
    #ifdef USE_PASSIVE_SCALAR
    real S_1;
    real S_2;
    #endif

    //////////////////////////////////////////////////////////////////////////

    __host__ __device__ PrimitiveVariables()
		: rho   (zero)
         ,U     (zero)
         ,V     (zero)
         ,W     (zero)
         ,lambda(zero)
    #ifdef USE_PASSIVE_SCALAR
         ,S_1   (zero)
         ,S_2   (zero)
    #endif
    {}

    //////////////////////////////////////////////////////////////////////////

    __host__ __device__ PrimitiveVariables(real rho
                                          ,real U
                                          ,real V
                                          ,real W
                                          ,real lambda
    #ifdef USE_PASSIVE_SCALAR
                                          ,real S_1 = zero
                                          ,real S_2 = zero
    #endif
    )
        : rho   (rho   )
         ,U     (U     )
         ,V     (V     )
         ,W     (W     )
         ,lambda(lambda)
    #ifdef USE_PASSIVE_SCALAR
         ,S_1   (S_1   )
         ,S_2   (S_2   )
    #endif
    {}

    //////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct ConservedVariables
{
    real rho;
    real rhoU;
    real rhoV;
    real rhoW;
    real rhoE;
    #ifdef USE_PASSIVE_SCALAR
    real rhoS_1;
    real rhoS_2;
    #endif

    //////////////////////////////////////////////////////////////////////////

    __host__ __device__ ConservedVariables()
        : rho (zero)
         ,rhoU(zero)
         ,rhoV(zero)
         ,rhoW(zero)
         ,rhoE(zero)
    #ifdef USE_PASSIVE_SCALAR
         ,rhoS_1(zero)
         ,rhoS_2(zero)
    #endif
    {}

    //////////////////////////////////////////////////////////////////////////
		  
    __host__ __device__ ConservedVariables(real rho
                                          ,real rhoU
                                          ,real rhoV
                                          ,real rhoW
                                          ,real rhoE
    #ifdef USE_PASSIVE_SCALAR
                                          ,real rhoS_1 = zero
                                          ,real rhoS_2 = zero
    #endif
    )
        : rho (rho )
         ,rhoU(rhoU)
         ,rhoV(rhoV)
         ,rhoW(rhoW)
         ,rhoE(rhoE)
    #ifdef USE_PASSIVE_SCALAR
         ,rhoS_1(rhoS_1)
         ,rhoS_2(rhoS_2)
    #endif
    {}

    //////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

__host__ __device__ inline ConservedVariables toConservedVariables( const PrimitiveVariables& prim, real K )
{
    return ConservedVariables(prim.rho
                             ,prim.U * prim.rho
                             ,prim.V * prim.rho
                             ,prim.W * prim.rho
                             ,( K + three ) / ( four * prim.lambda ) * prim.rho + c1o2 * prim.rho * ( prim.U * prim.U + prim.V * prim.V + prim.W * prim.W )
    #ifdef USE_PASSIVE_SCALAR
                             ,prim.S_1 * prim.rho
                             ,prim.S_2 * prim.rho
    #endif
    );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

__host__ __device__ inline PrimitiveVariables toPrimitiveVariables( const ConservedVariables& cons, real K )
{
	return PrimitiveVariables(cons.rho
						     ,cons.rhoU / cons.rho
						     ,cons.rhoV / cons.rho
						     ,cons.rhoW / cons.rho
						     ,( K + three ) * cons.rho / ( four * ( cons.rhoE - c1o2 * ( cons.rhoU * cons.rhoU + cons.rhoV * cons.rhoV + cons.rhoW * cons.rhoW ) / cons.rho ) )
    #ifdef USE_PASSIVE_SCALAR
                             ,cons.rhoS_1 / cons.rho
                             ,cons.rhoS_2 / cons.rho
    #endif
	);
}

#endif

