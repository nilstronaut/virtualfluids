#include "CompressibleOffsetMomentsInterpolationProcessor.h"

#include <algorithm>

#include <lbm/refinement/Interpolation_CF.h>
#include <lbm/refinement/Interpolation_FC.h>
#include <lbm/refinement/Coefficients.h>


void calculateCoefficients(vf::lbm::Coefficients& coefficients, const D3Q27ICell& icell, real omega, real xoff, real yoff, real zoff)
{
    vf::lbm::MomentsOnSourceNodeSet moments_set;

    moments_set.calculate_MMM(icell.BSW, omega);
    moments_set.calculate_MMP(icell.TSW, omega);
    moments_set.calculate_MPP(icell.TNW, omega);
    moments_set.calculate_MPM(icell.BNW, omega);
    moments_set.calculate_PMM(icell.BSE, omega);
    moments_set.calculate_PPP(icell.TNE, omega);
    moments_set.calculate_PMP(icell.TSE, omega);
    moments_set.calculate_PPM(icell.BNE, omega);

    moments_set.calculateCoefficients(coefficients, xoff, yoff, zoff);
}

CompressibleOffsetMomentsInterpolationProcessor::CompressibleOffsetMomentsInterpolationProcessor(real omegaC, real omegaF)
   : omegaC(omegaC), omegaF(omegaF)
{
}

InterpolationProcessorPtr CompressibleOffsetMomentsInterpolationProcessor::clone()
{
   return InterpolationProcessorPtr (new CompressibleOffsetMomentsInterpolationProcessor(this->omegaC, this->omegaF));
}

void CompressibleOffsetMomentsInterpolationProcessor::setOmegas(real omegaC, real omegaF)
{
   this->omegaC = omegaC;
   this->omegaF = omegaF;
}

void CompressibleOffsetMomentsInterpolationProcessor::interpolateCoarseToFine(D3Q27ICell& icellC, D3Q27ICell& icellF, real xoff, real yoff, real zoff)
{
    setOffsets(xoff, yoff, zoff);

    vf::lbm::Coefficients coefficients;
    calculateCoefficients(coefficients, icellC, omegaC, xoff, yoff, zoff);

     vf::lbm::interpolate_cf(icellF.BSW, omegaF, vf::lbm::constant::c1o2, coefficients, -0.25, -0.25, -0.25);
     vf::lbm::interpolate_cf(icellF.BNE, omegaF, vf::lbm::constant::c1o2, coefficients,  0.25,  0.25, -0.25);
     vf::lbm::interpolate_cf(icellF.TNW, omegaF, vf::lbm::constant::c1o2, coefficients, -0.25,  0.25,  0.25);
     vf::lbm::interpolate_cf(icellF.TSE, omegaF, vf::lbm::constant::c1o2, coefficients,  0.25, -0.25,  0.25);
     vf::lbm::interpolate_cf(icellF.BNW, omegaF, vf::lbm::constant::c1o2, coefficients, -0.25,  0.25, -0.25);
     vf::lbm::interpolate_cf(icellF.BSE, omegaF, vf::lbm::constant::c1o2, coefficients,  0.25, -0.25, -0.25);
     vf::lbm::interpolate_cf(icellF.TSW, omegaF, vf::lbm::constant::c1o2, coefficients, -0.25, -0.25,  0.25);
     vf::lbm::interpolate_cf(icellF.TNE, omegaF, vf::lbm::constant::c1o2, coefficients,  0.25,  0.25,  0.25);
}

void CompressibleOffsetMomentsInterpolationProcessor::interpolateFineToCoarse(D3Q27ICell& icellF, real* icellC, real xoff, real yoff, real zoff)
{
   setOffsets(xoff, yoff, zoff);

    vf::lbm::Coefficients coefficients;
    calculateCoefficients(coefficients, icellF, omegaF, xoff, yoff, zoff);

    const real eps_new = vf::lbm::constant::c2o1; // ratio of grid resolutions
    vf::lbm::interpolate_fc(icellC, eps_new, omegaC, coefficients);
}
