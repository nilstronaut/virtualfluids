#include "ForceCalculator.h"
#include "BCProcessor.h"

#include "Communicator.h"
#include "D3Q27Interactor.h"
#include "DataSet3D.h"
#include "LBMKernel.h"
#include "Block3D.h"
#include "BoundaryConditions.h"
#include "BCArray3D.h"

ForceCalculator::ForceCalculator(CommunicatorPtr comm) : comm(comm), forceX1global(0), forceX2global(0), forceX3global(0)
{

}

ForceCalculator::~ForceCalculator()
{

}




Vector3D ForceCalculator::getForces(int x1, int x2, int x3, DistributionArray3DPtr distributions, BoundaryConditionsPtr bc, const Vector3D& boundaryVelocity) const
{
    double forceX1 = 0;
    double forceX2 = 0;
    double forceX3 = 0;
    if (bc)
    {
        for (int fdir = D3Q27System::FSTARTDIR; fdir <= D3Q27System::FENDDIR; fdir++)
        {
            if (bc->hasNoSlipBoundaryFlag(fdir) || bc->hasVelocityBoundaryFlag(fdir))
            {
                const int invDir = D3Q27System::INVDIR[fdir];
                const double f = distributions->getDistributionInvForDirection(x1, x2, x3, invDir);
                const double fnbr = distributions->getDistributionInvForDirection(x1 + D3Q27System::DX1[invDir], x2 + D3Q27System::DX2[invDir], x3 + D3Q27System::DX3[invDir], fdir);

                double correction[3] = { 0.0, 0.0, 0.0 };
                if(bc->hasVelocityBoundaryFlag(fdir))
                {
                    const double forceTerm = f - fnbr;
                    correction[0] = forceTerm * boundaryVelocity[0];
                    correction[1] = forceTerm * boundaryVelocity[1];
                    correction[2] = forceTerm * boundaryVelocity[2];
                }

                //UBLOG(logINFO, "c, c * bv(x,y,z): " << correction << ", " << correction * val<1>(boundaryVelocity) << ", " << correction * val<2>(boundaryVelocity) << ", " << correction * val<3>(boundaryVelocity));

                // force consists of the MEM part and the galilean invariance correction including the boundary velocity
                forceX1 += (f + fnbr) * D3Q27System::DX1[invDir] - correction[0];
                forceX2 += (f + fnbr) * D3Q27System::DX2[invDir] - correction[1];
                forceX3 += (f + fnbr) * D3Q27System::DX3[invDir] - correction[2];
            }
        }  
    }
    return Vector3D(forceX1, forceX2, forceX3);
}

void ForceCalculator::calculateForces(std::vector<D3Q27InteractorPtr> interactors)
{
    forceX1global = 0.0;
    forceX2global = 0.0;
    forceX3global = 0.0;

    for (const D3Q27InteractorPtr interactor : interactors)
    {
        for (BcNodeIndicesMap::value_type t : interactor->getBcNodeIndicesMap())
        {
            double forceX1 = 0.0;
            double forceX2 = 0.0;
            double forceX3 = 0.0;

            Block3DPtr block = t.first;
            ILBMKernelPtr kernel = block->getKernel();
            BCArray3DPtr bcArray = kernel->getBCProcessor()->getBCArray();
            DistributionArray3DPtr distributions = kernel->getDataSet()->getFdistributions();
            distributions->swap();

            std::set< std::vector<int> >& transNodeIndices = t.second;
            for (std::vector<int> node : transNodeIndices)
            {
                int x1 = node[0];
                int x2 = node[1];
                int x3 = node[2];

                if (kernel->isInsideOfDomain(x1, x2, x3) && bcArray->isFluid(x1, x2, x3))
                {
                    BoundaryConditionsPtr bc = bcArray->getBC(x1, x2, x3);
                    Vector3D forceVec = getForces(x1, x2, x3, distributions, bc);
                    forceX1 += forceVec[0];
                    forceX2 += forceVec[1];
                    forceX3 += forceVec[2];
                }
            }
            //if we have got discretization with more level
            // deltaX is LBM deltaX and equal LBM deltaT 
            double deltaX = LBMSystem::getDeltaT(block->getLevel()); //grid->getDeltaT(block);
            double deltaXquadrat = deltaX*deltaX;
            forceX1 *= deltaXquadrat;
            forceX2 *= deltaXquadrat;
            forceX3 *= deltaXquadrat;

            distributions->swap();

            forceX1global += forceX1;
            forceX2global += forceX2;
            forceX3global += forceX3;
        }
    }
    gatherGlobalForces();
}

void ForceCalculator::gatherGlobalForces()
{
    std::vector<double> values{ forceX1global , forceX2global, forceX3global };
    std::vector<double> rvalues = comm->gather(values);

    if (comm->isRoot())
    {
        forceX1global = 0.0;
        forceX2global = 0.0;
        forceX3global = 0.0;

        for (int i = 0; i < (int)rvalues.size(); i += 3)
        {
            forceX1global += rvalues[i];
            forceX2global += rvalues[i + 1];
            forceX3global += rvalues[i + 2];
        }
    }
}

Vector3D ForceCalculator::getGlobalForces() const
{
    return Vector3D(forceX1global, forceX2global, forceX3global);
}
