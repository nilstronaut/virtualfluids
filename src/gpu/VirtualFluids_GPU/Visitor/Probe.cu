#include "Probe.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <unordered_set>

#include "VirtualFluids_GPU/GPU/GeometryUtils.h"
#include "Kernel/Utilities/CudaGrid.h"
#include "basics/writer/WbWriterVtkXmlBinary.h"
#include <Core/StringUtilities/StringUtil.h>

#include "Parameter/Parameter.h"
#include "DataStructureInitializer/GridProvider.h"
#include "GPU/CudaMemoryManager.h"

std::vector<std::string> getPostProcessingVariableNames(PostProcessingVariable variable)
{
    std::vector<std::string> varNames;
    switch (variable)
    {
    case PostProcessingVariable::Means:
        varNames.push_back("vx_mean");
        varNames.push_back("vy_mean");
        varNames.push_back("vz_mean");
        varNames.push_back("rho_mean");
        break;
    case PostProcessingVariable::Variances:
        varNames.push_back("vx_var");
        varNames.push_back("vy_var");
        varNames.push_back("vz_var");
        varNames.push_back("rho_var");
        break;
    default:
        break;
    }
    return varNames;
}

__device__ void calculateQuantities(uint n, real* quantityArray, bool* quantities, uint* quantityArrayOffsets, uint nPoints, uint node, real vx, real vy, real vz, real rho)
{
    //"https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm"
    // also has extensions for higher order and covariances
    real inv_n = 1/real(n);

    if(quantities[int(PostProcessingVariable::Means)])
    {
        uint arrOff = quantityArrayOffsets[int(PostProcessingVariable::Means)];
        real vx_m_old  = quantityArray[(arrOff+0)*nPoints+node];
        real vy_m_old  = quantityArray[(arrOff+1)*nPoints+node];
        real vz_m_old  = quantityArray[(arrOff+2)*nPoints+node];
        real rho_m_old = quantityArray[(arrOff+3)*nPoints+node];

        real vx_m_new  = ( (n-1)*vx_m_old + vx  )*inv_n;
        real vy_m_new  = ( (n-1)*vy_m_old + vy  )*inv_n;
        real vz_m_new  = ( (n-1)*vz_m_old + vz  )*inv_n;
        real rho_m_new = ( (n-1)*rho_m_old+ rho )*inv_n;

        quantityArray[(arrOff+0)*nPoints+node] = vx_m_new;
        quantityArray[(arrOff+1)*nPoints+node] = vy_m_new;
        quantityArray[(arrOff+2)*nPoints+node] = vz_m_new;
        quantityArray[(arrOff+3)*nPoints+node] = rho_m_new;
    
        if(quantities[int(PostProcessingVariable::Variances)])
        {
            arrOff = quantityArrayOffsets[int(PostProcessingVariable::Variances)];

            real vx_var_old  = quantityArray[(arrOff+0)*nPoints+node];
            real vy_var_old  = quantityArray[(arrOff+1)*nPoints+node];
            real vz_var_old  = quantityArray[(arrOff+2)*nPoints+node];
            real rho_var_old = quantityArray[(arrOff+3)*nPoints+node];

            real vx_var_new  = ( (n-1)*(vx_var_old )+(vx  - vx_m_old )*(vx  - vx_m_new ) )*inv_n;
            real vy_var_new  = ( (n-1)*(vy_var_old )+(vy  - vy_m_old )*(vy  - vy_m_new ) )*inv_n;
            real vz_var_new  = ( (n-1)*(vz_var_old )+(vz  - vz_m_old )*(vz  - vz_m_new ) )*inv_n;
            real rho_var_new = ( (n-1)*(rho_var_old)+(rho - rho_m_old)*(rho - rho_m_new) )*inv_n;

            quantityArray[(arrOff+0)*nPoints+node] = vx_var_new;
            quantityArray[(arrOff+1)*nPoints+node] = vy_var_new;
            quantityArray[(arrOff+2)*nPoints+node] = vz_var_new;
            quantityArray[(arrOff+3)*nPoints+node] = rho_var_new; 
        }
    }
}

__global__ void interpQuantities(   uint* pointIndices,
                                    uint nPoints, uint n,
                                    real* distX, real* distY, real* distZ,
                                    real* vx, real* vy, real* vz, real* rho,            
                                    uint* neighborX, uint* neighborY, uint* neighborZ,
                                    bool* quantities,
                                    uint* quantityArrayOffsets, real* quantityArray
                                )
{
    const uint x = threadIdx.x; 
    const uint y = blockIdx.x;
    const uint z = blockIdx.y;

    const uint nx = blockDim.x;
    const uint ny = gridDim.x;

    const uint node = nx*(ny*z + y) + x;

    if(node>=nPoints) return;

    // Get indices of neighbor nodes. 
    // node referring to BSW cell as seen from probe point
    uint k = pointIndices[node];
    uint ke, kn, kt, kne, kte, ktn, ktne;
    getNeighborIndicesBSW(  k, ke, kn, kt, kne, kte, ktn, ktne, neighborX, neighborY, neighborZ);

    // Trilinear interpolation of macroscopic quantities to probe point
    real dW, dE, dN, dS, dT, dB;
    getInterpolationWeights(dW, dE, dN, dS, dT, dB, distX[node], distY[node], distZ[node]);

    real u_interpX, u_interpY, u_interpZ, rho_interp;

    u_interpX  = trilinearInterpolation( dW, dE, dN, dS, dT, dB, k, ke, kn, kt, kne, kte, ktn, ktne, vx );
    u_interpY  = trilinearInterpolation( dW, dE, dN, dS, dT, dB, k, ke, kn, kt, kne, kte, ktn, ktne, vy );
    u_interpZ  = trilinearInterpolation( dW, dE, dN, dS, dT, dB, k, ke, kn, kt, kne, kte, ktn, ktne, vz );
    rho_interp = trilinearInterpolation( dW, dE, dN, dS, dT, dB, k, ke, kn, kt, kne, kte, ktn, ktne, rho );

    calculateQuantities(n, quantityArray, quantities, quantityArrayOffsets, nPoints, node, u_interpX, u_interpY, u_interpZ, rho_interp);

}


void Probe::init(Parameter* para, GridProvider* gridProvider, CudaMemoryManager* cudaManager)
{

    probeParams.resize(para->getMaxLevel()+1);

    for(int level=0; level<=para->getMaxLevel(); level++)
    {
        std::vector<int> probeIndices_level;
        std::vector<real> distX_level;
        std::vector<real> distY_level;
        std::vector<real> distZ_level;        
        std::vector<real> pointCoordsX_level;
        std::vector<real> pointCoordsY_level;
        std::vector<real> pointCoordsZ_level;
        real dx = abs(para->getParH(level)->coordX_SP[1]-para->getParH(level)->coordX_SP[para->getParH(level)->neighborX_SP[1]]);
        for(uint j=1; j<para->getParH(level)->size_Mat_SP; j++ )
        {    
            for(uint point=0; point<this->nProbePoints; point++)
            {
                real pointCoordX = this->pointCoordsX[point];
                real pointCoordY = this->pointCoordsY[point];
                real pointCoordZ = this->pointCoordsZ[point];
                real distX = pointCoordX-para->getParH(level)->coordX_SP[j];
                real distY = pointCoordY-para->getParH(level)->coordY_SP[j];
                real distZ = pointCoordZ-para->getParH(level)->coordZ_SP[j];
                if( distX <=dx && distY <=dx && distZ <=dx &&
                    distX >0.f && distY >0.f && distZ >0.f)
                {
                    probeIndices_level.push_back(j);
                    distX_level.push_back( distX/dx );
                    distY_level.push_back( distY/dx );
                    distZ_level.push_back( distZ/dx );
                    pointCoordsX_level.push_back( pointCoordX );
                    pointCoordsY_level.push_back( pointCoordY );
                    pointCoordsZ_level.push_back( pointCoordZ );
                    // printf("x %f y %f z %f", pointCoordX, pointCoordY, pointCoordZ);
                }
            }
        }
        
        probeParams[level] = new ProbeStruct;
        probeParams[level]->vals = 1;
        probeParams[level]->nPoints = uint(probeIndices_level.size());

        probeParams[level]->pointCoordsX = (real*)malloc(probeParams[level]->nPoints*sizeof(real));
        probeParams[level]->pointCoordsY = (real*)malloc(probeParams[level]->nPoints*sizeof(real));
        probeParams[level]->pointCoordsZ = (real*)malloc(probeParams[level]->nPoints*sizeof(real));

        std::copy(pointCoordsX_level.begin(), pointCoordsX_level.end(), probeParams[level]->pointCoordsX);
        std::copy(pointCoordsY_level.begin(), pointCoordsY_level.end(), probeParams[level]->pointCoordsY);
        std::copy(pointCoordsZ_level.begin(), pointCoordsZ_level.end(), probeParams[level]->pointCoordsZ);

        // Might have to catch nPoints=0 ?!?!
        cudaManager->cudaAllocProbeDistances(this, level);
        cudaManager->cudaAllocProbeIndices(this, level);

        std::copy(distX_level.begin(), distX_level.end(), probeParams[level]->distXH);
        std::copy(distY_level.begin(), distY_level.end(), probeParams[level]->distYH);
        std::copy(distZ_level.begin(), distZ_level.end(), probeParams[level]->distZH);
        std::copy(probeIndices_level.begin(), probeIndices_level.end(), probeParams[level]->pointIndicesH);

        cudaManager->cudaCopyProbeDistancesHtoD(this, level);
        cudaManager->cudaCopyProbeIndicesHtoD(this, level);

        uint arrOffset = 0;

        cudaManager->cudaAllocProbeQuantitiesAndOffsets(this, level);

        for( int var=0; var<int(PostProcessingVariable::LAST); var++){
        if(this->quantities[var])
        {

            probeParams[level]->quantitiesH[var] = true;
            probeParams[level]->arrayOffsetsH[var] = arrOffset;

            arrOffset += getPostProcessingVariableNames(static_cast<PostProcessingVariable>(var)).size();

        }}
        
        cudaManager->cudaCopyProbeQuantitiesAndOffsetsHtoD(this, level);

        probeParams[level]->nArrays = arrOffset;

        cudaManager->cudaAllocProbeQuantityArray(this, level);

        for(uint arr=0; arr<probeParams[level]->nArrays; arr++)
        {
            for( uint point=0; point<probeParams[level]->nPoints; point++)
            {
                probeParams[level]->quantitiesArrayH[arr*probeParams[level]->nPoints+point] = 0.0f;
            }
        }
        cudaManager->cudaCopyProbeQuantityArrayHtoD(this, level);

    }
}


void Probe::visit(Parameter* para, CudaMemoryManager* cudaManager, int level, uint t)
{

    ProbeStruct* probeStruct = this->getProbeStruct(level);

    vf::gpu::CudaGrid grid = vf::gpu::CudaGrid(128, probeStruct->nPoints);

    if(t>this->tStartAvg)
    {
        
        interpQuantities<<<grid.grid, grid.threads>>>(  probeStruct->pointIndicesD, probeStruct->nPoints, probeStruct->vals,
                                                        probeStruct->distXD, probeStruct->distYD, probeStruct->distZD,
                                                        para->getParD(level)->vx_SP, para->getParD(level)->vy_SP, para->getParD(level)->vz_SP, para->getParD(level)->rho_SP, 
                                                        para->getParD(level)->neighborX_SP, para->getParD(level)->neighborY_SP, para->getParD(level)->neighborZ_SP, 
                                                        probeStruct->quantitiesD, probeStruct->arrayOffsetsD, probeStruct->quantitiesArrayD  );
        probeStruct->vals++;

        if(max(int(t) - int(this->tStartOut), -1) % this->tOut == 0)
        {
            cudaManager->cudaCopyProbeQuantityArrayDtoH(this, level);
         
            this->write(para, level, t);
        }

    }
}

void Probe::free(Parameter* para, CudaMemoryManager* cudaManager)
{
    for(int level=0; level<=para->getMaxLevel(); level++)
    {
        cudaManager->cudaFreeProbeDistances(this, level);
        cudaManager->cudaFreeProbeIndices(this, level);
        cudaManager->cudaFreeProbeQuantityArray(this, level);
        cudaManager->cudaFreeProbeQuantitiesAndOffsets(this, level);
    }
}

void Probe::setProbePointsFromList(std::vector<real> &_pointCoordsX, std::vector<real> &_pointCoordsY, std::vector<real> &_pointCoordsZ)
{
    bool isSameLength = ( (_pointCoordsX.size()==_pointCoordsY.size()) && (_pointCoordsY.size()==_pointCoordsZ.size()));
    assert("Probe: point lists have different lengths" && isSameLength);
    this->pointCoordsX = _pointCoordsX;
    this->pointCoordsY = _pointCoordsY;
    this->pointCoordsZ = _pointCoordsZ;
    this->nProbePoints = uint(_pointCoordsX.size());
    printf("Added list of %u  points \n", this->nProbePoints );
}

void Probe::setProbePointsFromXNormalPlane(real pos_x, real pos0_y, real pos0_z, real pos1_y, real pos1_z, real delta_y, real delta_z)
{
    int n_points_y = int((pos1_y-pos0_y)/delta_y);
    int n_points_z = int((pos1_z-pos0_z)/delta_z);

    std::vector<real> pointCoordsXtmp, pointCoordsYtmp, pointCoordsZtmp;

    for(int n_y=0; n_y<n_points_y; n_y++)
    {
        for(int n_z=0; n_z<n_points_z; n_z++)
        {
            pointCoordsXtmp.push_back(pos_x);
            pointCoordsYtmp.push_back(pos0_y+delta_y*n_y);
            pointCoordsZtmp.push_back(pos0_z+delta_z*n_z);
        }
    }
    this->setProbePointsFromList(pointCoordsXtmp, pointCoordsYtmp, pointCoordsZtmp);
}

void Probe::addPostProcessingVariable(PostProcessingVariable variable)
{
    this->quantities[int(variable)] = true;
    switch(variable)
    {
        case PostProcessingVariable::Variances: 
            this->addPostProcessingVariable(PostProcessingVariable::Means); break;
        default: break;
    }
}

void Probe::write(Parameter* para, int level, int t)
{
    const uint numberOfParts = this->getProbeStruct(level)->nPoints / para->getlimitOfNodesForVTK() + 1;

    std::vector<std::string> fnames;
    for (uint i = 1; i <= numberOfParts; i++)
	{
        std::string fname = this->probeName + "_bin_lev_" + StringUtil::toString<int>(level)
                                         + "_ID_" + StringUtil::toString<int>(para->getMyID())
                                         + "_Part_" + StringUtil::toString<int>(i) 
                                         + "_t_" + StringUtil::toString<int>(t) 
                                         + ".vtk";
		fnames.push_back(fname);
        this->fileNamesForCollectionFile.push_back(fname);
    }
    this->writeGridFiles(para, level, fnames, t);

    if(level == 0) this->writeCollectionFile(para, t);
}

void Probe::writeCollectionFile(Parameter* para, int t)
{
    std::string filename = this->probeName + "_bin_ID_" + StringUtil::toString<int>(para->getMyID()) 
                                           + "_t_" + StringUtil::toString<int>(t) 
                                           + ".vtk";

    std::ofstream file;

    file.open( filename + ".pvtu" );

    //////////////////////////////////////////////////////////////////////////
    
    file << "<VTKFile type=\"PUnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">" << std::endl;
    file << "  <PUnstructuredGrid GhostLevel=\"1\">" << std::endl;

    file << "    <PPointData>" << std::endl;

    for(std::string varName: this->getVarNames())
    {
        file << "       <DataArray type=\"Float64\" Name=\""<< varName << "\" /> " << std::endl;
    }
    file << "    </PPointData>" << std::endl;

    file << "    <PPoints>" << std::endl;
    file << "      <PDataArray type=\"Float32\" Name=\"Points\" NumberOfComponents=\"3\"/>" << std::endl;
    file << "    </PPoints>" << std::endl;

    for( auto& fname : this->fileNamesForCollectionFile )
    {
        const auto filenameWithoutPath=fname.substr( fname.find_last_of('/') + 1 );
        file << "    <Piece Source=\"" << filenameWithoutPath << ".bin.vtu\"/>" << std::endl;
    }

    file << "  </PUnstructuredGrid>" << std::endl;
    file << "</VTKFile>" << std::endl;

    //////////////////////////////////////////////////////////////////////////

    file.close();

    this->fileNamesForCollectionFile.clear();
}

void Probe::writeGridFiles(Parameter* para, int level, std::vector<std::string>& fnames, int t)
{
    std::vector< UbTupleFloat3 > nodes;
    std::vector< std::string > nodedatanames = this->getVarNames();

    uint startpos = 0;
    uint endpos = 0;
    uint sizeOfNodes = 0;
    std::vector< std::vector< double > > nodedata(nodedatanames.size());

    ProbeStruct* probeStruct = this->getProbeStruct(level);

    for (uint part = 0; part < fnames.size(); part++)
    {        
        startpos = part * para->getlimitOfNodesForVTK();
        sizeOfNodes = min(para->getlimitOfNodesForVTK(), probeStruct->nPoints - startpos);
        endpos = startpos + sizeOfNodes;

        //////////////////////////////////////////////////////////////////////////
        nodes.resize(sizeOfNodes);

        for (uint pos = startpos; pos < endpos; pos++)
        {
            nodes[pos-startpos] = makeUbTuple(  float(probeStruct->pointCoordsX[pos]),
                                                float(probeStruct->pointCoordsY[pos]),
                                                float(probeStruct->pointCoordsZ[pos]));
        }

        for( auto it=nodedata.begin(); it!=nodedata.end(); it++) it->resize(sizeOfNodes);

        for( int var=0; var < int(PostProcessingVariable::LAST); var++){
        if(this->quantities[var])
        {
            PostProcessingVariable quantity = static_cast<PostProcessingVariable>(var);
            real coeff;
            uint n_arrs = getPostProcessingVariableNames(quantity).size();

            switch(quantity)
            {
            case PostProcessingVariable::Means:
                coeff = para->getVelocityRatio();
            break;
            case PostProcessingVariable::Variances:
                coeff = pow(para->getVelocityRatio(),2);
            break;
            default: break;
            }

            uint arrOff = probeStruct->arrayOffsetsH[var];
            uint arrLen = probeStruct->nPoints;

            for(uint arr=0; arr<n_arrs; arr++)
            {
                for (uint pos = startpos; pos < endpos; pos++)
                {
                    nodedata[arrOff+arr][pos-startpos] = double(probeStruct->quantitiesArrayH[(arrOff+arr)*arrLen+pos]*coeff);
                }
            }
        }}
        WbWriterVtkXmlBinary::getInstance()->writeNodesWithNodeData(fnames[part], nodes, nodedatanames, nodedata);
    }
}

std::vector<std::string> Probe::getVarNames()
{
    std::vector<std::string> varNames;
    for( int var=0; var < int(PostProcessingVariable::LAST); var++){
    if(this->quantities[var])
    {
        std::vector<std::string> names = getPostProcessingVariableNames(static_cast<PostProcessingVariable>(var));
        varNames.insert(varNames.end(), names.begin(), names.end());
    }}
    return varNames;
}

