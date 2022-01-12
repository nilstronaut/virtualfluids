#include "ActuatorLine.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>

#include <cuda/CudaGrid.h>
#include "lbm/constants/NumericConstants.h"
#include "VirtualFluids_GPU/GPU/GeometryUtils.h"

#include "Parameter/Parameter.h"
#include "DataStructureInitializer/GridProvider.h"
#include "GPU/CudaMemoryManager.h"

__host__ __device__ __inline__ uint calcNode(uint bladeNode, uint blade, uint nBlades)
{
    return bladeNode*nBlades+blade;
}

__host__ __device__ __inline__ real calcGaussian3D(real posX, real posY, real posZ, real destX, real destY, real destZ, real epsilon)
{
    real distX = destX-posX;
    real distY = destY-posY;
    real distZ = destZ-posZ;
    real dist = sqrt(distX*distX+distY*distY+distZ*distZ);
    return pow(epsilon,-3)*pow(vf::lbm::constant::cPi,-1.5f)*exp(-pow(dist/epsilon,2));
}


__global__ void interpolateVelocities(real* gridCoordsX, real* gridCoordsY, real* gridCoordsZ, 
                                      uint* neighborsX, uint* neighborsY, uint* neighborsZ, 
                                      uint* neighborsWSB, 
                                      real* vx, real* vy, real* vz, 
                                      uint numberOfIndices, 
                                      real* bladeCoordsX, real* bladeCoordsY, real* bladeCoordsZ, 
                                      real* bladeVelocitiesX, real* bladeVelocitiesY, real* bladeVelocitiesZ, 
                                      uint* bladeIndices, uint numberOfNodes, real velocityRatio)
{
    const uint x = threadIdx.x; 
    const uint y = blockIdx.x;
    const uint z = blockIdx.y;

    const uint nx = blockDim.x;
    const uint ny = gridDim.x;

    const uint node = nx*(ny*z + y) + x;

    if(node>=numberOfNodes) return;

    real bladePosX = bladeCoordsX[node];
    real bladePosY = bladeCoordsY[node];
    real bladePosZ = bladeCoordsZ[node];

    uint old_index = bladeIndices[node];

    uint k, ke, kn, kt;
    uint kne, kte, ktn, ktne;

    k = findNearestCellBSW(old_index, 
                           gridCoordsX, gridCoordsY, gridCoordsZ, 
                           bladePosX, bladePosY, bladePosZ, 
                           neighborsX, neighborsY, neighborsZ, neighborsWSB);
        
    bladeIndices[node] = k;

    //TODO calculate velocities from distributions

    getNeighborIndicesOfBSW(k, ke, kn, kt, kne, kte, ktn, ktne, neighborsX, neighborsY, neighborsZ);

    real dW, dE, dN, dS, dT, dB;

    real invDeltaX = 1.f/(gridCoordsX[ktne]-gridCoordsX[k]);
    real distX = invDeltaX*(gridCoordsX[ktne]-bladePosX);
    real distY = invDeltaX*(gridCoordsY[ktne]-bladePosY);
    real distZ = invDeltaX*(gridCoordsZ[ktne]-bladePosZ);

    getInterpolationWeights(dW, dE, dN, dS, dT, dB, 
                            distX, distY, distZ);

    bladeVelocitiesX[node] = trilinearInterpolation(dW, dE, dN, dS, dT, dB, k, ke, kn, kt, kne, kte, ktn, ktne, vx)*velocityRatio;
    bladeVelocitiesY[node] = trilinearInterpolation(dW, dE, dN, dS, dT, dB, k, ke, kn, kt, kne, kte, ktn, ktne, vy)*velocityRatio;
    bladeVelocitiesZ[node] = trilinearInterpolation(dW, dE, dN, dS, dT, dB, k, ke, kn, kt, kne, kte, ktn, ktne, vz)*velocityRatio;
}


__global__ void applyBodyForces(real* gridCoordsX, real* gridCoordsY, real* gridCoordsZ,
                           real* gridForcesX, real* gridForcesY, real* gridForcesZ, 
                           uint* gridIndices, uint numberOfIndices, 
                           real* bladeCoordsX, real* bladeCoordsY, real* bladeCoordsZ, 
                           real* bladeForcesX, real* bladeForcesY,real* bladeForcesZ,
                           real* bladeRadii,
                           real radius,
                           uint nBlades, uint nBladeNodes,
                           real epsilon, real delta_x, real forceRatio)
{
    const uint x = threadIdx.x; 
    const uint y = blockIdx.x;
    const uint z = blockIdx.y;

    const uint nx = blockDim.x;
    const uint ny = gridDim.x;

    const uint index = nx*(ny*z + y) + x;

    if(index>=numberOfIndices) return;

    int gridIndex = gridIndices[index];

    real posX = gridCoordsX[gridIndex];
    real posY = gridCoordsY[gridIndex];
    real posZ = gridCoordsZ[gridIndex];

    real fXYZ_X = 0.0f;
    real fXYZ_Y = 0.0f;
    real fXYZ_Z = 0.0f;

    real eta = 0.0f;

    real delta_x_cubed = pow(delta_x,3);
    real invForceRatio = 1.f/forceRatio;

    real last_r = 0.0f;
    real r = 0.0f;

    for( uint bladeNode=0; bladeNode<nBladeNodes; bladeNode++)
    {
        r = bladeRadii[bladeNode];

        for( uint blade=0; blade<nBlades; blade++)
        {   
            uint node = calcNode(bladeNode, blade, nBlades);
            eta = calcGaussian3D(posX, posY, posZ, bladeCoordsX[node], bladeCoordsY[node], bladeCoordsZ[node], epsilon)*delta_x_cubed;

            fXYZ_X += bladeForcesX[node]*(r-last_r)*eta;
            fXYZ_Y += bladeForcesY[node]*(r-last_r)*eta;
            fXYZ_Z += bladeForcesZ[node]*(r-last_r)*eta;
        } 

        last_r = r;
    }

    for( uint blade=0; blade<nBlades; blade++)
    {  
        fXYZ_X += bladeForcesX[calcNode(nBladeNodes-1, blade, nBlades)]*(radius-last_r)*eta;
        fXYZ_Y += bladeForcesY[calcNode(nBladeNodes-1, blade, nBlades)]*(radius-last_r)*eta;
        fXYZ_Z += bladeForcesZ[calcNode(nBladeNodes-1, blade, nBlades)]*(radius-last_r)*eta;
    }
    gridForcesX[gridIndex] = fXYZ_X*invForceRatio;
    gridForcesY[gridIndex] = fXYZ_Y*invForceRatio;
    gridForcesZ[gridIndex] = fXYZ_Z*invForceRatio;
}


void ActuatorLine::init(Parameter* para, GridProvider* gridProvider, CudaMemoryManager* cudaManager)
{
    if(!para->getIsBodyForce()) throw std::runtime_error("try to allocate ActuatorLine but BodyForce is not set in Parameter.");
    // if(!para->getCalcMacQuantsEveryTimeStep()) throw std::runtime_error("try to allocate ActuatorLine but does not compute macroscopic quantities every time step.");
    this->initBladeRadii(cudaManager);
    this->initBladeCoords(cudaManager);    
    this->initBladeIndices(para, cudaManager);
    this->initBladeVelocities(cudaManager);
    this->initBladeForces(cudaManager);    
    this->initBoundingSphere(para, cudaManager);
}


void ActuatorLine::interact(Parameter* para, CudaMemoryManager* cudaManager, int level, unsigned int t)
{
    if (level != this->level) return;

    cudaManager->cudaCopyBladeCoordsHtoD(this);

    uint numberOfThreads = para->getParH(level)->numberofthreads;
    vf::cuda::CudaGrid bladeGrid = vf::cuda::CudaGrid(numberOfThreads, this->numberOfNodes);

    interpolateVelocities<<< bladeGrid.grid, bladeGrid.threads >>>(
        para->getParD(this->level)->coordX_SP, para->getParD(this->level)->coordY_SP, para->getParD(this->level)->coordZ_SP,        
        para->getParD(this->level)->neighborX_SP, para->getParD(this->level)->neighborY_SP, para->getParD(this->level)->neighborZ_SP, para->getParD(this->level)->neighborWSB_SP,
        para->getParD(this->level)->vx_SP, para->getParD(this->level)->vy_SP, para->getParD(this->level)->vz_SP,
        this->numberOfIndices,
        this->bladeCoordsXD, this->bladeCoordsYD, this->bladeCoordsZD,  
        this->bladeVelocitiesXD, this->bladeVelocitiesYD, this->bladeVelocitiesZD,  
        this->bladeIndicesD, this->numberOfNodes, para->getVelocityRatio());

    cudaManager->cudaCopyBladeVelocitiesDtoH(this);

    this->calcBladeForces();

    cudaManager->cudaCopyBladeForcesHtoD(this);

    vf::cuda::CudaGrid sphereGrid = vf::cuda::CudaGrid(numberOfThreads, this->numberOfIndices);

    applyBodyForces<<<sphereGrid.grid, sphereGrid.threads>>>(
        para->getParD(this->level)->coordX_SP, para->getParD(this->level)->coordY_SP, para->getParD(this->level)->coordZ_SP,        
        para->getParD(this->level)->forceX_SP, para->getParD(this->level)->forceY_SP, para->getParD(this->level)->forceZ_SP,        
        this->boundingSphereIndicesD, this->numberOfIndices,
        this->bladeCoordsXD, this->bladeCoordsYD, this->bladeCoordsZD,  
        this->bladeForcesXD, this->bladeForcesYD, this->bladeForcesZD,
        this->bladeRadiiD,
        this->diameter*0.5f,  
        this->nBlades, this->nBladeNodes,
        this->epsilon, this->delta_x, this->density*pow(para->getViscosityRatio(),2));

    real dazimuth = this->omega*this->delta_t;

    this->azimuth = fmod(this->azimuth+dazimuth,2*vf::lbm::constant::cPi);
    this->rotateBlades(dazimuth);
}


void ActuatorLine::free(Parameter* para, CudaMemoryManager* cudaManager)
{
    cudaManager->cudaFreeBladeRadii(this);
    cudaManager->cudaFreeBladeCoords(this);
    cudaManager->cudaFreeBladeVelocities(this);
    cudaManager->cudaFreeBladeForces(this);
    cudaManager->cudaFreeBladeIndices(this);
    cudaManager->cudaFreeSphereIndices(this);
}


void ActuatorLine::calcForcesEllipticWing()
{
    real localAzimuth;
    uint node;
    real uXYZ_X, uXYZ_Y, uXYZ_Z;
    real uRTZ_X, uRTZ_Y, uRTZ_Z;
    real fXYZ_X, fXYZ_Y, fXYZ_Z;
    real fRTZ_X, fRTZ_Y, fRTZ_Z;
    real r;
    real u_rel, v_rel, u_rel_sq;
    real phi;
    real Cl = 1.f;
    real Cd = 0.f;
    real c0 = 1.f;

    real c, Cn, Ct;

        
    for( uint bladeNode=0; bladeNode<this->nBladeNodes; bladeNode++)
    {   
        r = this->bladeRadiiH[bladeNode];
        
        for( uint blade=0; blade<this->nBlades; blade++)
        {
            localAzimuth = this->azimuth+2*blade*vf::lbm::constant::cPi/this->nBlades;
            node = calcNode(bladeNode, blade, this->nBlades);
            uXYZ_X = this->bladeVelocitiesXH[node];
            uXYZ_Y = this->bladeVelocitiesYH[node];
            uXYZ_Z = this->bladeVelocitiesZH[node];

            invRotateAboutX3D(localAzimuth, uXYZ_X, uXYZ_Y, uXYZ_Z, uRTZ_X, uRTZ_Y, uRTZ_Z);
            

            u_rel = uRTZ_X;
            v_rel = uRTZ_Y+this->omega*r;
            u_rel_sq = u_rel*u_rel+v_rel*v_rel;
            phi = atan2(u_rel, v_rel);

            c = c0 * sqrt( 1.f- pow(4.f*r/this->diameter-1.f, 2.f) );
            Cn =   Cl*cos(phi)+Cd*sin(phi);
            Ct =  -Cl*sin(phi)+Cd*cos(phi);

            fRTZ_X = 0.5f*u_rel_sq*c*this->density*Cn;
            fRTZ_Y = 0.5f*u_rel_sq*c*this->density*Ct;
            fRTZ_Z = 0.0;

            rotateAboutX3D(localAzimuth, fRTZ_X, fRTZ_Y, fRTZ_Z, fXYZ_X, fXYZ_Y, fXYZ_Z);
        
            this->bladeForcesXH[node] = fXYZ_X;
            this->bladeForcesYH[node] = fXYZ_Y;
            this->bladeForcesZH[node] = fXYZ_Z;
        }
    }
}

void ActuatorLine::calcBladeForces()
{
    this->calcForcesEllipticWing();
}

void ActuatorLine::rotateBlades(real angle)
{
    for(uint node=0; node<this->nBladeNodes*this->nBlades; node++)
    {
        real oldCoordX = this->bladeCoordsXH[node];
        real oldCoordY = this->bladeCoordsYH[node];
        real oldCoordZ = this->bladeCoordsZH[node];

        real newCoordX, newCoordY, newCoordZ;
        rotateAboutX3D(angle, oldCoordX, oldCoordY, oldCoordZ, newCoordX, newCoordY, newCoordZ, this->turbinePosX, this->turbinePosY, this->turbinePosZ);
        
        this->bladeCoordsYH[node] = newCoordX;
        this->bladeCoordsYH[node] = newCoordY;
        this->bladeCoordsZH[node] = newCoordZ;
    }
}

void ActuatorLine::initBladeRadii(CudaMemoryManager* cudaManager)
{   
    cudaManager->cudaAllocBladeRadii(this);

    real dx = 0.5f*this->diameter/this->nBladeNodes;        
    for(uint node=0; node<this->nBladeNodes; node++)
    {
        this->bladeRadiiH[node] = dx*(node+1);
    }
    cudaManager->cudaCopyBladeRadiiHtoD(this);
}

void ActuatorLine::initBladeCoords(CudaMemoryManager* cudaManager)
{   
    cudaManager->cudaAllocBladeCoords(this);

    for(uint bladeNode=0; bladeNode<this->nBladeNodes; bladeNode++)
    {
        real x = 0.f;
        real y = 0.f;
        real z = this->bladeRadiiH[bladeNode];
        for( uint blade=0; blade<this->nBlades; blade++)
        {
            uint node = calcNode(bladeNode, blade, this->nBlades);

            real localAzimuth = this->azimuth+(2*vf::lbm::constant::cPi/this->nBlades)*blade;
            real coordX, coordY, coordZ;

            rotateAboutX3D(localAzimuth, x, y, z, coordX, coordY, coordZ);
            this->bladeCoordsXH[node] = coordX+this->turbinePosX;
            this->bladeCoordsYH[node] = coordY+this->turbinePosY;
            this->bladeCoordsZH[node] = coordZ+this->turbinePosZ;
        }
    }
    cudaManager->cudaCopyBladeCoordsHtoD(this);
}

void ActuatorLine::initBladeVelocities(CudaMemoryManager* cudaManager)
{   
    cudaManager->cudaAllocBladeVelocities(this);

    for(uint node=0; node<this->numberOfNodes; node++)
    {
        this->bladeVelocitiesXH[node] = 0.f;
        this->bladeVelocitiesYH[node] = 0.f;
        this->bladeVelocitiesZH[node] = 0.f;
    }
    cudaManager->cudaCopyBladeVelocitiesHtoD(this);
}

void ActuatorLine::initBladeForces(CudaMemoryManager* cudaManager)
{   
    cudaManager->cudaAllocBladeForces(this);

    for(uint node=0; node<this->numberOfNodes; node++)
    {
        this->bladeForcesXH[node] = 0.f;
        this->bladeForcesYH[node] = 0.f;
        this->bladeForcesZH[node] = 0.f;
    }
    cudaManager->cudaCopyBladeForcesHtoD(this);
}

void ActuatorLine::initBladeIndices(Parameter* para, CudaMemoryManager* cudaManager)
{   
    cudaManager->cudaAllocBladeIndices(this);

    real* coordsX = para->getParH(this->level)->coordX_SP;
    real* coordsY = para->getParH(this->level)->coordY_SP;
    real* coordsZ = para->getParH(this->level)->coordZ_SP;

    for(uint node=0; node<this->numberOfNodes; node++)
    {
        this->bladeIndicesH[node] = findNearestCellBSW(1, coordsX, coordsY, coordsZ, 
                                                       this->bladeCoordsXH[node], this->bladeCoordsYH[node], this->bladeCoordsZH[node],
                                                       para->getParH(this->level)->neighborX_SP, para->getParH(this->level)->neighborY_SP, para->getParH(this->level)->neighborZ_SP,
                                                       para->getParH(this->level)->neighborWSB_SP);
        
    }
    cudaManager->cudaCopyBladeIndicesHtoD(this);
}

void ActuatorLine::initBoundingSphere(Parameter* para, CudaMemoryManager* cudaManager)
{
    // Actuator line exists only on 1 level
    std::vector<int> nodesInSphere;

    for (uint j = 1; j <= para->getParH(this->level)->size_Mat_SP; j++)
    {
        const real coordX = para->getParH(this->level)->coordX_SP[j];
        const real coordY = para->getParH(this->level)->coordY_SP[j];
        const real coordZ = para->getParH(this->level)->coordZ_SP[j];
        const real dist = sqrt(pow(coordX-this->turbinePosX,2)+pow(coordY-this->turbinePosY,2)+pow(coordZ-this->turbinePosZ,2));
        
        if(dist < 0.6*this->diameter) nodesInSphere.push_back(j);
    }

    this->numberOfIndices = uint(nodesInSphere.size());
    cudaManager->cudaAllocSphereIndices(this);
    std::copy(nodesInSphere.begin(), nodesInSphere.end(), this->boundingSphereIndicesH);
    cudaManager->cudaCopySphereIndicesHtoD(this);
}

void ActuatorLine::setBladeCoords(real* _bladeCoordsX, real* _bladeCoordsY, real* _bladeCoordsZ)
{ 

    for(uint node=0; node<this->numberOfNodes; node++)
    {
        this->bladeCoordsXH[node] = _bladeCoordsX[node];
        this->bladeCoordsYH[node] = _bladeCoordsY[node];
        this->bladeCoordsZH[node] = _bladeCoordsZ[node];
    }
}

void ActuatorLine::setBladeVelocities(real* _bladeVelocitiesX, real* _bladeVelocitiesY, real* _bladeVelocitiesZ)
{ 
    for(uint node=0; node<this->numberOfNodes; node++)
    {
        this->bladeVelocitiesXH[node] = _bladeVelocitiesX[node];
        this->bladeVelocitiesYH[node] = _bladeVelocitiesY[node];
        this->bladeVelocitiesZH[node] = _bladeVelocitiesZ[node];
    }
}

void ActuatorLine::setBladeForces(real* _bladeForcesX, real* _bladeForcesY, real* _bladeForcesZ)
{ 
    for(uint node=0; node<this->numberOfNodes; node++)
    {
        this->bladeForcesXH[node] = _bladeForcesX[node];
        this->bladeForcesYH[node] = _bladeForcesY[node];
        this->bladeForcesZH[node] = _bladeForcesZ[node];
    }
}