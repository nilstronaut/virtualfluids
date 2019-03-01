#ifndef HeatCapacities_H
#define HeatCapacities_H

#ifdef __CUDACC__
#include <cuda_runtime.h>
#else
#define __host__
#define __device__
#endif

#include "Core/DataTypes.h"
#include "Core/RealConstants.h"

#include "Definitions/PassiveScalar.h"

#include "FlowStateData/FlowStateData.cuh"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef USE_PASSIVE_SCALAR

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

__host__ __device__ inline real getCpO2( real T )
{
    real CpData [] = {
                        29.106,
                        29.385,
                        31.091,
                        32.981,
                        34.355,
                        35.300,
                        35.988,
                        36.544,
                        37.040,
                        37.510,
                        37.969,
                        38.419,
                        38.856,
                        39.276,
                        39.674,
                        40.048,
                        40.395,
                        40.716,
                        41.013,
                        41.289
                     };

    real T0 = 100.0;
    real dT = 200.0;

    int i = int( ( T - T0 ) / dT );

    real CpLow  = CpData[i];
    real CpHigh = CpData[i+1];

    real x = (T - T0 - i * dT) / dT;

    return CpLow + x * ( CpHigh - CpLow );
}

__host__ __device__ inline real getCpN2( real T )
{
    real CpData [] = {
                        29.104,
                        29.125,
                        29.580,
                        30.754,
                        32.090,
                        33.241,
                        34.147,
                        34.843,
                        35.378,
                        35.796,
                        36.126,
                        36.395,
                        36.616,
                        36.801,
                        36.959,
                        37.096,
                        37.216,
                        37.323,
                        37.420,
                        37.508
                     };

    real T0 = 100.0;
    real dT = 200.0;

    int i = int( ( T - T0 ) / dT );

    real CpLow  = CpData[i];
    real CpHigh = CpData[i+1];

    real x = (T - T0 - i * dT) / dT;

    return CpLow + x * ( CpHigh - CpLow );
}

__host__ __device__ inline real getCpCH4( real T )
{
    real CpData [] = {
                         33.258,
                         35.708,
                         46.342,
                         57.794,
                         67.601,
                         75.529,
                         81.744,
                         86.556,
                         90.283,
                         93.188,
                         95.477,
                         97.301,
                         98.772,
                         99.971,
                        100.960,
                        101.782,
                        102.474,
                        103.060,
                        103.560,
                        103.990 
                     };

    real T0 = 100.0;
    real dT = 200.0;

    int i = int( ( T - T0 ) / dT );

    real CpLow  = CpData[i];
    real CpHigh = CpData[i+1];

    real x = (T - T0 - i * dT) / dT;

    return CpLow + x * ( CpHigh - CpLow );
}

__host__ __device__ inline real getCpH2O( real T )
{
    real CpData [] = {
                        33.299,
                        33.596,
                        35.226,
                        37.495,
                        39.987,
                        42.536,
                        44.945,
                        47.090,
                        48.935,
                        50.496,
                        51.823,
                        52.947,
                        53.904,
                        54.723,
                        55.430,
                        56.044,
                        56.583,
                        57.058,
                        57.480,
                        57.859
                     };

    real T0 = 100.0;
    real dT = 200.0;

    int i = int( ( T - T0 ) / dT );

    real CpLow  = CpData[i];
    real CpHigh = CpData[i+1];

    real x = (T - T0 - i * dT) / dT;

    return CpLow + x * ( CpHigh - CpLow );
}

__host__ __device__ inline real getCpCO2( real T )
{
    real CpData [] = {
                        29.208,
                        37.221,
                        44.627,
                        49.564,
                        52.999,
                        55.409,
                        57.137,
                        58.379,
                        59.317,
                        60.049,
                        60.622,
                        61.086,
                        61.471,
                        61.802,
                        62.095,
                        62.347,
                        62.573,
                        62.785,
                        62.980,
                        63.166
                     };

    real T0 = 100.0;
    real dT = 200.0;

    int i = int( ( T - T0 ) / dT );

    real CpLow  = CpData[i];
    real CpHigh = CpData[i+1];

    real x = (T - T0 - i * dT) / dT;

    return CpLow + x * ( CpHigh - CpLow );
}



#endif



#endif

