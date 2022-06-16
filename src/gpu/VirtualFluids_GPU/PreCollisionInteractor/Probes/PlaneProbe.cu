#include "Probe.h"
#include "PlaneProbe.h"

#include <cuda/CudaGrid.h>

#include <cuda.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>

#include "Parameter/Parameter.h"
#include "DataStructureInitializer/GridProvider.h"
#include "GPU/CudaMemoryManager.h"


bool PlaneProbe::isAvailableStatistic(Statistic _variable)
{
    bool isAvailable;
    switch (_variable)
    {
        case Statistic::Instantaneous:
        case Statistic::Means:
        case Statistic::Variances:
            isAvailable = true;
            break;
        case Statistic::SpatialMeans:
        case Statistic::SpatioTemporalMeans:
        case Statistic::SpatialCovariances:
        case Statistic::SpatioTemporalCovariances:
        case Statistic::SpatialSkewness:
        case Statistic::SpatioTemporalSkewness:
        case Statistic::SpatialFlatness:
        case Statistic::SpatioTemporalFlatness:
            isAvailable = false;
            break;
        default:
            isAvailable = false;
    }
    return isAvailable;
}


std::vector<PostProcessingVariable> PlaneProbe::getPostProcessingVariables(Statistic statistic)
{
    std::vector<PostProcessingVariable> postProcessingVariables;
    switch (statistic)
    {
    case Statistic::Instantaneous:
        postProcessingVariables.push_back( PostProcessingVariable("vx",  this->velocityRatio) );
        postProcessingVariables.push_back( PostProcessingVariable("vy",  this->velocityRatio) );
        postProcessingVariables.push_back( PostProcessingVariable("vz",  this->velocityRatio) );
        postProcessingVariables.push_back( PostProcessingVariable("rho", this->densityRatio ) );
        break;
    case Statistic::Means:
        postProcessingVariables.push_back( PostProcessingVariable("vx_mean",  this->velocityRatio) );
        postProcessingVariables.push_back( PostProcessingVariable("vy_mean",  this->velocityRatio) );
        postProcessingVariables.push_back( PostProcessingVariable("vz_mean",  this->velocityRatio) );
        postProcessingVariables.push_back( PostProcessingVariable("rho_mean", this->densityRatio ) );
        break;
    case Statistic::Variances:
        postProcessingVariables.push_back( PostProcessingVariable("vx_var",  pow(this->velocityRatio, 2.0)) );
        postProcessingVariables.push_back( PostProcessingVariable("vy_var",  pow(this->velocityRatio, 2.0)) );
        postProcessingVariables.push_back( PostProcessingVariable("vz_var",  pow(this->velocityRatio, 2.0)) );
        postProcessingVariables.push_back( PostProcessingVariable("rho_var", pow(this->densityRatio,  2.0)) );
        break;

    default:
        printf("Statistic unavailable in PlaneProbe\n");
        assert(false);
        break;
    }
    return postProcessingVariables;
}

void PlaneProbe::findPoints(Parameter* para, GridProvider* gridProvider, std::vector<int>& probeIndices_level,
                            std::vector<real>& distX_level, std::vector<real>& distY_level, std::vector<real>& distZ_level,      
                            std::vector<real>& pointCoordsX_level, std::vector<real>& pointCoordsY_level, std::vector<real>& pointCoordsZ_level,
                            int level)
{
    real dx = abs(para->getParH(level)->coordinateX[1]-para->getParH(level)->coordinateX[para->getParH(level)->neighborX[1]]);
    for(uint j=1; j<para->getParH(level)->numberOfNodes; j++ )
    {
        real pointCoordX = para->getParH(level)->coordinateX[j];
        real pointCoordY = para->getParH(level)->coordinateY[j];
        real pointCoordZ = para->getParH(level)->coordinateZ[j];
        real distX = pointCoordX - this->posX;
        real distY = pointCoordY - this->posY;
        real distZ = pointCoordZ - this->posZ;

        if( distX <= this->deltaX && distY <= this->deltaY && distZ <= this->deltaZ &&
            distX >=0.f && distY >=0.f && distZ >=0.f)
        {
            probeIndices_level.push_back(j);
            distX_level.push_back( distX/dx );
            distY_level.push_back( distY/dx );
            distZ_level.push_back( distZ/dx );
            pointCoordsX_level.push_back( pointCoordX );
            pointCoordsY_level.push_back( pointCoordY );
            pointCoordsZ_level.push_back( pointCoordZ );
        }
    }
}

void PlaneProbe::calculateQuantities(SPtr<ProbeStruct> probeStruct, Parameter* para, uint t, int level)
{
    vf::cuda::CudaGrid grid = vf::cuda::CudaGrid(para->getParH(level)->numberofthreads, probeStruct->nPoints);
    calcQuantitiesKernel<<<grid.grid, grid.threads>>>(  probeStruct->pointIndicesD, probeStruct->nPoints, probeStruct->vals,
    para->getParD(level)->velocityX, para->getParD(level)->velocityY, para->getParD(level)->velocityZ, para->getParD(level)->rho, 
    para->getParD(level)->neighborX, para->getParD(level)->neighborY, para->getParD(level)->neighborZ, 
    probeStruct->quantitiesD, probeStruct->arrayOffsetsD, probeStruct->quantitiesArrayD);
}