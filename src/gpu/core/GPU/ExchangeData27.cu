/* Device code */
#include "LBM/LB.h" 
#include "lbm/constants/D3Q27.h"
#include <basics/constants/NumericConstants.h>

using namespace vf::basics::constant;
using namespace vf::lbm::dir;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__global__ void getSendFsPost27(real* DD,
										   real* bufferFs,
										   int* sendIndex,
                                           int buffmax,
                                           unsigned int* neighborX,
                                           unsigned int* neighborY,
                                           unsigned int* neighborZ,
                                           unsigned long long numberOfLBnodes, 
                                           bool isEvenTimestep)
{
   ////////////////////////////////////////////////////////////////////////////////
   const unsigned  x = threadIdx.x;  // Globaler x-Index 
   const unsigned  y = blockIdx.x;   // Globaler y-Index 
   const unsigned  z = blockIdx.y;   // Globaler z-Index 

   const unsigned nx = blockDim.x;
   const unsigned ny = gridDim.x;

   const unsigned k = nx*(ny*z + y) + x;
   //////////////////////////////////////////////////////////////////////////

   if(k<buffmax)
   {
      ////////////////////////////////////////////////////////////////////////////////
      //set index
      unsigned int kIndex  = sendIndex[k];
      unsigned int kzero   = kIndex;
      unsigned int ke      = kIndex;
      unsigned int kw      = neighborX[kIndex];
      unsigned int kn      = kIndex;
      unsigned int ks      = neighborY[kIndex];
      unsigned int kt      = kIndex;
      unsigned int kb      = neighborZ[kIndex];
      unsigned int ksw     = neighborY[kw];
      unsigned int kne     = kIndex;
      unsigned int kse     = ks;
      unsigned int knw     = kw;
      unsigned int kbw     = neighborZ[kw];
      unsigned int kte     = kIndex;
      unsigned int kbe     = kb;
      unsigned int ktw     = kw;
      unsigned int kbs     = neighborZ[ks];
      unsigned int ktn     = kIndex;
      unsigned int kbn     = kb;
      unsigned int kts     = ks;
      unsigned int ktse    = ks;
      unsigned int kbnw    = kbw;
      unsigned int ktnw    = kw;
      unsigned int kbse    = kbs;
      unsigned int ktsw    = ksw;
      unsigned int kbne    = kb;
      unsigned int ktne    = kIndex;
      unsigned int kbsw    = neighborZ[ksw];
      ////////////////////////////////////////////////////////////////////////////////
	  //set Pointer for Fs
      Distributions27 D;
      if (isEvenTimestep==true)
      {
         D.f[dP00] = &DD[dP00 * numberOfLBnodes];
         D.f[dM00] = &DD[dM00 * numberOfLBnodes];
         D.f[d0P0] = &DD[d0P0 * numberOfLBnodes];
         D.f[d0M0] = &DD[d0M0 * numberOfLBnodes];
         D.f[d00P] = &DD[d00P * numberOfLBnodes];
         D.f[d00M] = &DD[d00M * numberOfLBnodes];
         D.f[dPP0] = &DD[dPP0 * numberOfLBnodes];
         D.f[dMM0] = &DD[dMM0 * numberOfLBnodes];
         D.f[dPM0] = &DD[dPM0 * numberOfLBnodes];
         D.f[dMP0] = &DD[dMP0 * numberOfLBnodes];
         D.f[dP0P] = &DD[dP0P * numberOfLBnodes];
         D.f[dM0M] = &DD[dM0M * numberOfLBnodes];
         D.f[dP0M] = &DD[dP0M * numberOfLBnodes];
         D.f[dM0P] = &DD[dM0P * numberOfLBnodes];
         D.f[d0PP] = &DD[d0PP * numberOfLBnodes];
         D.f[d0MM] = &DD[d0MM * numberOfLBnodes];
         D.f[d0PM] = &DD[d0PM * numberOfLBnodes];
         D.f[d0MP] = &DD[d0MP * numberOfLBnodes];
         D.f[d000] = &DD[d000 * numberOfLBnodes];
         D.f[dPPP] = &DD[dPPP * numberOfLBnodes];
         D.f[dMMP] = &DD[dMMP * numberOfLBnodes];
         D.f[dPMP] = &DD[dPMP * numberOfLBnodes];
         D.f[dMPP] = &DD[dMPP * numberOfLBnodes];
         D.f[dPPM] = &DD[dPPM * numberOfLBnodes];
         D.f[dMMM] = &DD[dMMM * numberOfLBnodes];
         D.f[dPMM] = &DD[dPMM * numberOfLBnodes];
         D.f[dMPM] = &DD[dMPM * numberOfLBnodes];
      } 
      else
      {
         D.f[dM00] = &DD[dP00 * numberOfLBnodes];
         D.f[dP00] = &DD[dM00 * numberOfLBnodes];
         D.f[d0M0] = &DD[d0P0 * numberOfLBnodes];
         D.f[d0P0] = &DD[d0M0 * numberOfLBnodes];
         D.f[d00M] = &DD[d00P * numberOfLBnodes];
         D.f[d00P] = &DD[d00M * numberOfLBnodes];
         D.f[dMM0] = &DD[dPP0 * numberOfLBnodes];
         D.f[dPP0] = &DD[dMM0 * numberOfLBnodes];
         D.f[dMP0] = &DD[dPM0 * numberOfLBnodes];
         D.f[dPM0] = &DD[dMP0 * numberOfLBnodes];
         D.f[dM0M] = &DD[dP0P * numberOfLBnodes];
         D.f[dP0P] = &DD[dM0M * numberOfLBnodes];
         D.f[dM0P] = &DD[dP0M * numberOfLBnodes];
         D.f[dP0M] = &DD[dM0P * numberOfLBnodes];
         D.f[d0MM] = &DD[d0PP * numberOfLBnodes];
         D.f[d0PP] = &DD[d0MM * numberOfLBnodes];
         D.f[d0MP] = &DD[d0PM * numberOfLBnodes];
         D.f[d0PM] = &DD[d0MP * numberOfLBnodes];
         D.f[d000] = &DD[d000 * numberOfLBnodes];
         D.f[dPPP] = &DD[dMMM * numberOfLBnodes];
         D.f[dMMP] = &DD[dPPM * numberOfLBnodes];
         D.f[dPMP] = &DD[dMPM * numberOfLBnodes];
         D.f[dMPP] = &DD[dPMM * numberOfLBnodes];
         D.f[dPPM] = &DD[dMMP * numberOfLBnodes];
         D.f[dMMM] = &DD[dPPP * numberOfLBnodes];
         D.f[dPMM] = &DD[dMPP * numberOfLBnodes];
         D.f[dMPM] = &DD[dPMP * numberOfLBnodes];
      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //set Pointer for Buffer Fs
      Distributions27 Dbuff;
      Dbuff.f[dP00] = &bufferFs[dP00 * buffmax];
      Dbuff.f[dM00] = &bufferFs[dM00 * buffmax];
      Dbuff.f[d0P0] = &bufferFs[d0P0 * buffmax];
      Dbuff.f[d0M0] = &bufferFs[d0M0 * buffmax];
      Dbuff.f[d00P] = &bufferFs[d00P * buffmax];
      Dbuff.f[d00M] = &bufferFs[d00M * buffmax];
      Dbuff.f[dPP0] = &bufferFs[dPP0 * buffmax];
      Dbuff.f[dMM0] = &bufferFs[dMM0 * buffmax];
      Dbuff.f[dPM0] = &bufferFs[dPM0 * buffmax];
      Dbuff.f[dMP0] = &bufferFs[dMP0 * buffmax];
      Dbuff.f[dP0P] = &bufferFs[dP0P * buffmax];
      Dbuff.f[dM0M] = &bufferFs[dM0M * buffmax];
      Dbuff.f[dP0M] = &bufferFs[dP0M * buffmax];
      Dbuff.f[dM0P] = &bufferFs[dM0P * buffmax];
      Dbuff.f[d0PP] = &bufferFs[d0PP * buffmax];
      Dbuff.f[d0MM] = &bufferFs[d0MM * buffmax];
      Dbuff.f[d0PM] = &bufferFs[d0PM * buffmax];
      Dbuff.f[d0MP] = &bufferFs[d0MP * buffmax];
      Dbuff.f[d000] = &bufferFs[d000 * buffmax];
      Dbuff.f[dPPP] = &bufferFs[dPPP * buffmax];
      Dbuff.f[dMMP] = &bufferFs[dMMP * buffmax];
      Dbuff.f[dPMP] = &bufferFs[dPMP * buffmax];
      Dbuff.f[dMPP] = &bufferFs[dMPP * buffmax];
      Dbuff.f[dPPM] = &bufferFs[dPPM * buffmax];
      Dbuff.f[dMMM] = &bufferFs[dMMM * buffmax];
      Dbuff.f[dPMM] = &bufferFs[dPMM * buffmax];
      Dbuff.f[dMPM] = &bufferFs[dMPM * buffmax];
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //copy to buffer
      //(Dbuff.f[dP00])[k] = (D.f[dP00])[ke   ];
      //(Dbuff.f[dM00])[k] = (D.f[dM00])[kw   ];
      //(Dbuff.f[d0P0])[k] = (D.f[d0P0])[kn   ];
      //(Dbuff.f[d0M0])[k] = (D.f[d0M0])[ks   ];
      //(Dbuff.f[d00P])[k] = (D.f[d00P])[kt   ];
      //(Dbuff.f[d00M])[k] = (D.f[d00M])[kb   ];
      //(Dbuff.f[dPP0])[k] = (D.f[dPP0])[kne  ];
      //(Dbuff.f[dMM0])[k] = (D.f[dMM0])[ksw  ];
      //(Dbuff.f[dPM0])[k] = (D.f[dPM0])[kse  ];
      //(Dbuff.f[dMP0])[k] = (D.f[dMP0])[knw  ];
      //(Dbuff.f[dP0P])[k] = (D.f[dP0P])[kte  ];
      //(Dbuff.f[dM0M])[k] = (D.f[dM0M])[kbw  ];
      //(Dbuff.f[dP0M])[k] = (D.f[dP0M])[kbe  ];
      //(Dbuff.f[dM0P])[k] = (D.f[dM0P])[ktw  ];
      //(Dbuff.f[d0PP])[k] = (D.f[d0PP])[ktn  ];
      //(Dbuff.f[d0MM])[k] = (D.f[d0MM])[kbs  ];
      //(Dbuff.f[d0PM])[k] = (D.f[d0PM])[kbn  ];
      //(Dbuff.f[d0MP])[k] = (D.f[d0MP])[kts  ];
      //(Dbuff.f[d000])[k] = (D.f[d000])[kzero];
      //(Dbuff.f[dPPP])[k] = (D.f[dPPP])[ktne ];
      //(Dbuff.f[dMMP])[k] = (D.f[dMMP])[ktsw ];
      //(Dbuff.f[dPMP])[k] = (D.f[dPMP])[ktse ];
      //(Dbuff.f[dMPP])[k] = (D.f[dMPP])[ktnw ];
      //(Dbuff.f[dPPM])[k] = (D.f[dPPM])[kbne ];
      //(Dbuff.f[dMMM])[k] = (D.f[dMMM])[kbsw ];
      //(Dbuff.f[dPMM])[k] = (D.f[dPMM])[kbse ];
      //(Dbuff.f[dMPM])[k] = (D.f[dMPM])[kbnw ];
      (Dbuff.f[dP00])[k] = (D.f[dM00])[kw   ];
      (Dbuff.f[dM00])[k] = (D.f[dP00])[ke   ];
      (Dbuff.f[d0P0])[k] = (D.f[d0M0])[ks   ];
      (Dbuff.f[d0M0])[k] = (D.f[d0P0])[kn   ];
      (Dbuff.f[d00P])[k] = (D.f[d00M])[kb   ];
      (Dbuff.f[d00M])[k] = (D.f[d00P])[kt   ];
      (Dbuff.f[dPP0])[k] = (D.f[dMM0])[ksw  ];
      (Dbuff.f[dMM0])[k] = (D.f[dPP0])[kne  ];
      (Dbuff.f[dPM0])[k] = (D.f[dMP0])[knw  ];
      (Dbuff.f[dMP0])[k] = (D.f[dPM0])[kse  ];
      (Dbuff.f[dP0P])[k] = (D.f[dM0M])[kbw  ];
      (Dbuff.f[dM0M])[k] = (D.f[dP0P])[kte  ];
      (Dbuff.f[dP0M])[k] = (D.f[dM0P])[ktw  ];
      (Dbuff.f[dM0P])[k] = (D.f[dP0M])[kbe  ];
      (Dbuff.f[d0PP])[k] = (D.f[d0MM])[kbs  ];
      (Dbuff.f[d0MM])[k] = (D.f[d0PP])[ktn  ];
      (Dbuff.f[d0PM])[k] = (D.f[d0MP])[kts  ];
      (Dbuff.f[d0MP])[k] = (D.f[d0PM])[kbn  ];
      (Dbuff.f[d000])[k] = (D.f[d000])[kzero];
      (Dbuff.f[dPPP])[k] = (D.f[dMMM])[kbsw ];
      (Dbuff.f[dMMP])[k] = (D.f[dPPM])[kbne ];
      (Dbuff.f[dPMP])[k] = (D.f[dMPM])[kbnw ];
      (Dbuff.f[dMPP])[k] = (D.f[dPMM])[kbse ];
      (Dbuff.f[dPPM])[k] = (D.f[dMMP])[ktsw ];
      (Dbuff.f[dMMM])[k] = (D.f[dPPP])[ktne ];
      (Dbuff.f[dPMM])[k] = (D.f[dMPP])[ktnw ];
      (Dbuff.f[dMPM])[k] = (D.f[dPMP])[ktse ];
   }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






























////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__global__ void setRecvFsPost27(real* DD,
										   real* bufferFs,
										   int* recvIndex,
                                           int buffmax,
                                           unsigned int* neighborX,
                                           unsigned int* neighborY,
                                           unsigned int* neighborZ,
                                           unsigned long long numberOfLBnodes, 
                                           bool isEvenTimestep)
{
   ////////////////////////////////////////////////////////////////////////////////
   const unsigned  x = threadIdx.x;  // Globaler x-Index 
   const unsigned  y = blockIdx.x;   // Globaler y-Index 
   const unsigned  z = blockIdx.y;   // Globaler z-Index 

   const unsigned nx = blockDim.x;
   const unsigned ny = gridDim.x;

   const unsigned k = nx*(ny*z + y) + x;
   //////////////////////////////////////////////////////////////////////////

   if(k<buffmax)
   {
      ////////////////////////////////////////////////////////////////////////////////
      //set index
      unsigned int kIndex  = recvIndex[k];
      unsigned int kzero   = kIndex;
      unsigned int ke      = kIndex;
      unsigned int kw      = neighborX[kIndex];
      unsigned int kn      = kIndex;
      unsigned int ks      = neighborY[kIndex];
      unsigned int kt      = kIndex;
      unsigned int kb      = neighborZ[kIndex];
      unsigned int ksw     = neighborY[kw];
      unsigned int kne     = kIndex;
      unsigned int kse     = ks;
      unsigned int knw     = kw;
      unsigned int kbw     = neighborZ[kw];
      unsigned int kte     = kIndex;
      unsigned int kbe     = kb;
      unsigned int ktw     = kw;
      unsigned int kbs     = neighborZ[ks];
      unsigned int ktn     = kIndex;
      unsigned int kbn     = kb;
      unsigned int kts     = ks;
      unsigned int ktse    = ks;
      unsigned int kbnw    = kbw;
      unsigned int ktnw    = kw;
      unsigned int kbse    = kbs;
      unsigned int ktsw    = ksw;
      unsigned int kbne    = kb;
      unsigned int ktne    = kIndex;
      unsigned int kbsw    = neighborZ[ksw];
      ////////////////////////////////////////////////////////////////////////////////
	  //set Pointer for Fs
      Distributions27 D;
      if (isEvenTimestep==true)
      {
         D.f[dP00] = &DD[dP00 * numberOfLBnodes];
         D.f[dM00] = &DD[dM00 * numberOfLBnodes];
         D.f[d0P0] = &DD[d0P0 * numberOfLBnodes];
         D.f[d0M0] = &DD[d0M0 * numberOfLBnodes];
         D.f[d00P] = &DD[d00P * numberOfLBnodes];
         D.f[d00M] = &DD[d00M * numberOfLBnodes];
         D.f[dPP0] = &DD[dPP0 * numberOfLBnodes];
         D.f[dMM0] = &DD[dMM0 * numberOfLBnodes];
         D.f[dPM0] = &DD[dPM0 * numberOfLBnodes];
         D.f[dMP0] = &DD[dMP0 * numberOfLBnodes];
         D.f[dP0P] = &DD[dP0P * numberOfLBnodes];
         D.f[dM0M] = &DD[dM0M * numberOfLBnodes];
         D.f[dP0M] = &DD[dP0M * numberOfLBnodes];
         D.f[dM0P] = &DD[dM0P * numberOfLBnodes];
         D.f[d0PP] = &DD[d0PP * numberOfLBnodes];
         D.f[d0MM] = &DD[d0MM * numberOfLBnodes];
         D.f[d0PM] = &DD[d0PM * numberOfLBnodes];
         D.f[d0MP] = &DD[d0MP * numberOfLBnodes];
         D.f[d000] = &DD[d000 * numberOfLBnodes];
         D.f[dPPP] = &DD[dPPP * numberOfLBnodes];
         D.f[dMMP] = &DD[dMMP * numberOfLBnodes];
         D.f[dPMP] = &DD[dPMP * numberOfLBnodes];
         D.f[dMPP] = &DD[dMPP * numberOfLBnodes];
         D.f[dPPM] = &DD[dPPM * numberOfLBnodes];
         D.f[dMMM] = &DD[dMMM * numberOfLBnodes];
         D.f[dPMM] = &DD[dPMM * numberOfLBnodes];
         D.f[dMPM] = &DD[dMPM * numberOfLBnodes];
      } 
      else
      {
         D.f[dM00] = &DD[dP00 * numberOfLBnodes];
         D.f[dP00] = &DD[dM00 * numberOfLBnodes];
         D.f[d0M0] = &DD[d0P0 * numberOfLBnodes];
         D.f[d0P0] = &DD[d0M0 * numberOfLBnodes];
         D.f[d00M] = &DD[d00P * numberOfLBnodes];
         D.f[d00P] = &DD[d00M * numberOfLBnodes];
         D.f[dMM0] = &DD[dPP0 * numberOfLBnodes];
         D.f[dPP0] = &DD[dMM0 * numberOfLBnodes];
         D.f[dMP0] = &DD[dPM0 * numberOfLBnodes];
         D.f[dPM0] = &DD[dMP0 * numberOfLBnodes];
         D.f[dM0M] = &DD[dP0P * numberOfLBnodes];
         D.f[dP0P] = &DD[dM0M * numberOfLBnodes];
         D.f[dM0P] = &DD[dP0M * numberOfLBnodes];
         D.f[dP0M] = &DD[dM0P * numberOfLBnodes];
         D.f[d0MM] = &DD[d0PP * numberOfLBnodes];
         D.f[d0PP] = &DD[d0MM * numberOfLBnodes];
         D.f[d0MP] = &DD[d0PM * numberOfLBnodes];
         D.f[d0PM] = &DD[d0MP * numberOfLBnodes];
         D.f[d000] = &DD[d000 * numberOfLBnodes];
         D.f[dPPP] = &DD[dMMM * numberOfLBnodes];
         D.f[dMMP] = &DD[dPPM * numberOfLBnodes];
         D.f[dPMP] = &DD[dMPM * numberOfLBnodes];
         D.f[dMPP] = &DD[dPMM * numberOfLBnodes];
         D.f[dPPM] = &DD[dMMP * numberOfLBnodes];
         D.f[dMMM] = &DD[dPPP * numberOfLBnodes];
         D.f[dPMM] = &DD[dMPP * numberOfLBnodes];
         D.f[dMPM] = &DD[dPMP * numberOfLBnodes];
      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //set Pointer for Buffer Fs
      Distributions27 Dbuff;
      Dbuff.f[dP00] = &bufferFs[dP00 * buffmax];
      Dbuff.f[dM00] = &bufferFs[dM00 * buffmax];
      Dbuff.f[d0P0] = &bufferFs[d0P0 * buffmax];
      Dbuff.f[d0M0] = &bufferFs[d0M0 * buffmax];
      Dbuff.f[d00P] = &bufferFs[d00P * buffmax];
      Dbuff.f[d00M] = &bufferFs[d00M * buffmax];
      Dbuff.f[dPP0] = &bufferFs[dPP0 * buffmax];
      Dbuff.f[dMM0] = &bufferFs[dMM0 * buffmax];
      Dbuff.f[dPM0] = &bufferFs[dPM0 * buffmax];
      Dbuff.f[dMP0] = &bufferFs[dMP0 * buffmax];
      Dbuff.f[dP0P] = &bufferFs[dP0P * buffmax];
      Dbuff.f[dM0M] = &bufferFs[dM0M * buffmax];
      Dbuff.f[dP0M] = &bufferFs[dP0M * buffmax];
      Dbuff.f[dM0P] = &bufferFs[dM0P * buffmax];
      Dbuff.f[d0PP] = &bufferFs[d0PP * buffmax];
      Dbuff.f[d0MM] = &bufferFs[d0MM * buffmax];
      Dbuff.f[d0PM] = &bufferFs[d0PM * buffmax];
      Dbuff.f[d0MP] = &bufferFs[d0MP * buffmax];
      Dbuff.f[d000] = &bufferFs[d000 * buffmax];
      Dbuff.f[dPPP] = &bufferFs[dPPP * buffmax];
      Dbuff.f[dMMP] = &bufferFs[dMMP * buffmax];
      Dbuff.f[dPMP] = &bufferFs[dPMP * buffmax];
      Dbuff.f[dMPP] = &bufferFs[dMPP * buffmax];
      Dbuff.f[dPPM] = &bufferFs[dPPM * buffmax];
      Dbuff.f[dMMM] = &bufferFs[dMMM * buffmax];
      Dbuff.f[dPMM] = &bufferFs[dPMM * buffmax];
      Dbuff.f[dMPM] = &bufferFs[dMPM * buffmax];
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //copy from buffer
      //(D.f[dP00])[ke   ] = (Dbuff.f[dP00])[k];
      //(D.f[dM00])[kw   ] = (Dbuff.f[dM00])[k];
      //(D.f[d0P0])[kn   ] = (Dbuff.f[d0P0])[k];
      //(D.f[d0M0])[ks   ] = (Dbuff.f[d0M0])[k];
      //(D.f[d00P])[kt   ] = (Dbuff.f[d00P])[k];
      //(D.f[d00M])[kb   ] = (Dbuff.f[d00M])[k];
      //(D.f[dPP0])[kne  ] = (Dbuff.f[dPP0])[k];
      //(D.f[dMM0])[ksw  ] = (Dbuff.f[dMM0])[k];
      //(D.f[dPM0])[kse  ] = (Dbuff.f[dPM0])[k];
      //(D.f[dMP0])[knw  ] = (Dbuff.f[dMP0])[k];
      //(D.f[dP0P])[kte  ] = (Dbuff.f[dP0P])[k];
      //(D.f[dM0M])[kbw  ] = (Dbuff.f[dM0M])[k];
      //(D.f[dP0M])[kbe  ] = (Dbuff.f[dP0M])[k];
      //(D.f[dM0P])[ktw  ] = (Dbuff.f[dM0P])[k];
      //(D.f[d0PP])[ktn  ] = (Dbuff.f[d0PP])[k];
      //(D.f[d0MM])[kbs  ] = (Dbuff.f[d0MM])[k];
      //(D.f[d0PM])[kbn  ] = (Dbuff.f[d0PM])[k];
      //(D.f[d0MP])[kts  ] = (Dbuff.f[d0MP])[k];
      //(D.f[d000])[kzero] = (Dbuff.f[d000])[k];
      //(D.f[dPPP])[ktne ] = (Dbuff.f[dPPP])[k];
      //(D.f[dMMP])[ktsw ] = (Dbuff.f[dMMP])[k];
      //(D.f[dPMP])[ktse ] = (Dbuff.f[dPMP])[k];
      //(D.f[dMPP])[ktnw ] = (Dbuff.f[dMPP])[k];
      //(D.f[dPPM])[kbne ] = (Dbuff.f[dPPM])[k];
      //(D.f[dMMM])[kbsw ] = (Dbuff.f[dMMM])[k];
      //(D.f[dPMM])[kbse ] = (Dbuff.f[dPMM])[k];
      //(D.f[dMPM])[kbnw ] = (Dbuff.f[dMPM])[k];
      (D.f[dM00])[kw   ] = (Dbuff.f[dP00])[k];
      (D.f[dP00])[ke   ] = (Dbuff.f[dM00])[k];
      (D.f[d0M0])[ks   ] = (Dbuff.f[d0P0])[k];
      (D.f[d0P0])[kn   ] = (Dbuff.f[d0M0])[k];
      (D.f[d00M])[kb   ] = (Dbuff.f[d00P])[k];
      (D.f[d00P])[kt   ] = (Dbuff.f[d00M])[k];
      (D.f[dMM0])[ksw  ] = (Dbuff.f[dPP0])[k];
      (D.f[dPP0])[kne  ] = (Dbuff.f[dMM0])[k];
      (D.f[dMP0])[knw  ] = (Dbuff.f[dPM0])[k];
      (D.f[dPM0])[kse  ] = (Dbuff.f[dMP0])[k];
      (D.f[dM0M])[kbw  ] = (Dbuff.f[dP0P])[k];
      (D.f[dP0P])[kte  ] = (Dbuff.f[dM0M])[k];
      (D.f[dM0P])[ktw  ] = (Dbuff.f[dP0M])[k];
      (D.f[dP0M])[kbe  ] = (Dbuff.f[dM0P])[k];
      (D.f[d0MM])[kbs  ] = (Dbuff.f[d0PP])[k];
      (D.f[d0PP])[ktn  ] = (Dbuff.f[d0MM])[k];
      (D.f[d0MP])[kts  ] = (Dbuff.f[d0PM])[k];
      (D.f[d0PM])[kbn  ] = (Dbuff.f[d0MP])[k];
      (D.f[d000])[kzero] = (Dbuff.f[d000])[k];
      (D.f[dMMM])[kbsw ] = (Dbuff.f[dPPP])[k];
      (D.f[dPPM])[kbne ] = (Dbuff.f[dMMP])[k];
      (D.f[dMPM])[kbnw ] = (Dbuff.f[dPMP])[k];
      (D.f[dPMM])[kbse ] = (Dbuff.f[dMPP])[k];
      (D.f[dMMP])[ktsw ] = (Dbuff.f[dPPM])[k];
      (D.f[dPPP])[ktne ] = (Dbuff.f[dMMM])[k];
      (D.f[dMPP])[ktnw ] = (Dbuff.f[dPMM])[k];
      (D.f[dPMP])[ktse ] = (Dbuff.f[dMPM])[k];
   }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





























////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__global__ void getSendFsPre27(real* DD,
										  real* bufferFs,
										  int* sendIndex,
                                          int buffmax,
                                          unsigned int* neighborX,
                                          unsigned int* neighborY,
                                          unsigned int* neighborZ,
                                          unsigned long long numberOfLBnodes, 
                                          bool isEvenTimestep)
{
   ////////////////////////////////////////////////////////////////////////////////
   const unsigned  x = threadIdx.x;  // Globaler x-Index 
   const unsigned  y = blockIdx.x;   // Globaler y-Index 
   const unsigned  z = blockIdx.y;   // Globaler z-Index 

   const unsigned nx = blockDim.x;
   const unsigned ny = gridDim.x;

   const unsigned k = nx*(ny*z + y) + x;
   //////////////////////////////////////////////////////////////////////////

   if(k<buffmax)
   {
      ////////////////////////////////////////////////////////////////////////////////
      //set index
      unsigned int kIndex  = sendIndex[k];
      unsigned int kzero   = kIndex;
      unsigned int ke      = kIndex;
      unsigned int kw      = neighborX[kIndex];
      unsigned int kn      = kIndex;
      unsigned int ks      = neighborY[kIndex];
      unsigned int kt      = kIndex;
      unsigned int kb      = neighborZ[kIndex];
      unsigned int ksw     = neighborY[kw];
      unsigned int kne     = kIndex;
      unsigned int kse     = ks;
      unsigned int knw     = kw;
      unsigned int kbw     = neighborZ[kw];
      unsigned int kte     = kIndex;
      unsigned int kbe     = kb;
      unsigned int ktw     = kw;
      unsigned int kbs     = neighborZ[ks];
      unsigned int ktn     = kIndex;
      unsigned int kbn     = kb;
      unsigned int kts     = ks;
      unsigned int ktse    = ks;
      unsigned int kbnw    = kbw;
      unsigned int ktnw    = kw;
      unsigned int kbse    = kbs;
      unsigned int ktsw    = ksw;
      unsigned int kbne    = kb;
      unsigned int ktne    = kIndex;
      unsigned int kbsw    = neighborZ[ksw];
      ////////////////////////////////////////////////////////////////////////////////
	  //set Pointer for Fs
      Distributions27 D;
      if (isEvenTimestep==true)
      {
         D.f[dP00] = &DD[dP00 * numberOfLBnodes];
         D.f[dM00] = &DD[dM00 * numberOfLBnodes];
         D.f[d0P0] = &DD[d0P0 * numberOfLBnodes];
         D.f[d0M0] = &DD[d0M0 * numberOfLBnodes];
         D.f[d00P] = &DD[d00P * numberOfLBnodes];
         D.f[d00M] = &DD[d00M * numberOfLBnodes];
         D.f[dPP0] = &DD[dPP0 * numberOfLBnodes];
         D.f[dMM0] = &DD[dMM0 * numberOfLBnodes];
         D.f[dPM0] = &DD[dPM0 * numberOfLBnodes];
         D.f[dMP0] = &DD[dMP0 * numberOfLBnodes];
         D.f[dP0P] = &DD[dP0P * numberOfLBnodes];
         D.f[dM0M] = &DD[dM0M * numberOfLBnodes];
         D.f[dP0M] = &DD[dP0M * numberOfLBnodes];
         D.f[dM0P] = &DD[dM0P * numberOfLBnodes];
         D.f[d0PP] = &DD[d0PP * numberOfLBnodes];
         D.f[d0MM] = &DD[d0MM * numberOfLBnodes];
         D.f[d0PM] = &DD[d0PM * numberOfLBnodes];
         D.f[d0MP] = &DD[d0MP * numberOfLBnodes];
         D.f[d000] = &DD[d000 * numberOfLBnodes];
         D.f[dPPP] = &DD[dPPP * numberOfLBnodes];
         D.f[dMMP] = &DD[dMMP * numberOfLBnodes];
         D.f[dPMP] = &DD[dPMP * numberOfLBnodes];
         D.f[dMPP] = &DD[dMPP * numberOfLBnodes];
         D.f[dPPM] = &DD[dPPM * numberOfLBnodes];
         D.f[dMMM] = &DD[dMMM * numberOfLBnodes];
         D.f[dPMM] = &DD[dPMM * numberOfLBnodes];
         D.f[dMPM] = &DD[dMPM * numberOfLBnodes];
      } 
      else
      {
         D.f[dM00] = &DD[dP00 * numberOfLBnodes];
         D.f[dP00] = &DD[dM00 * numberOfLBnodes];
         D.f[d0M0] = &DD[d0P0 * numberOfLBnodes];
         D.f[d0P0] = &DD[d0M0 * numberOfLBnodes];
         D.f[d00M] = &DD[d00P * numberOfLBnodes];
         D.f[d00P] = &DD[d00M * numberOfLBnodes];
         D.f[dMM0] = &DD[dPP0 * numberOfLBnodes];
         D.f[dPP0] = &DD[dMM0 * numberOfLBnodes];
         D.f[dMP0] = &DD[dPM0 * numberOfLBnodes];
         D.f[dPM0] = &DD[dMP0 * numberOfLBnodes];
         D.f[dM0M] = &DD[dP0P * numberOfLBnodes];
         D.f[dP0P] = &DD[dM0M * numberOfLBnodes];
         D.f[dM0P] = &DD[dP0M * numberOfLBnodes];
         D.f[dP0M] = &DD[dM0P * numberOfLBnodes];
         D.f[d0MM] = &DD[d0PP * numberOfLBnodes];
         D.f[d0PP] = &DD[d0MM * numberOfLBnodes];
         D.f[d0MP] = &DD[d0PM * numberOfLBnodes];
         D.f[d0PM] = &DD[d0MP * numberOfLBnodes];
         D.f[d000] = &DD[d000 * numberOfLBnodes];
         D.f[dPPP] = &DD[dMMM * numberOfLBnodes];
         D.f[dMMP] = &DD[dPPM * numberOfLBnodes];
         D.f[dPMP] = &DD[dMPM * numberOfLBnodes];
         D.f[dMPP] = &DD[dPMM * numberOfLBnodes];
         D.f[dPPM] = &DD[dMMP * numberOfLBnodes];
         D.f[dMMM] = &DD[dPPP * numberOfLBnodes];
         D.f[dPMM] = &DD[dMPP * numberOfLBnodes];
         D.f[dMPM] = &DD[dPMP * numberOfLBnodes];
      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //set Pointer for Buffer Fs
      Distributions27 Dbuff;
      Dbuff.f[dP00] = &bufferFs[dP00 * buffmax];
      Dbuff.f[dM00] = &bufferFs[dM00 * buffmax];
      Dbuff.f[d0P0] = &bufferFs[d0P0 * buffmax];
      Dbuff.f[d0M0] = &bufferFs[d0M0 * buffmax];
      Dbuff.f[d00P] = &bufferFs[d00P * buffmax];
      Dbuff.f[d00M] = &bufferFs[d00M * buffmax];
      Dbuff.f[dPP0] = &bufferFs[dPP0 * buffmax];
      Dbuff.f[dMM0] = &bufferFs[dMM0 * buffmax];
      Dbuff.f[dPM0] = &bufferFs[dPM0 * buffmax];
      Dbuff.f[dMP0] = &bufferFs[dMP0 * buffmax];
      Dbuff.f[dP0P] = &bufferFs[dP0P * buffmax];
      Dbuff.f[dM0M] = &bufferFs[dM0M * buffmax];
      Dbuff.f[dP0M] = &bufferFs[dP0M * buffmax];
      Dbuff.f[dM0P] = &bufferFs[dM0P * buffmax];
      Dbuff.f[d0PP] = &bufferFs[d0PP * buffmax];
      Dbuff.f[d0MM] = &bufferFs[d0MM * buffmax];
      Dbuff.f[d0PM] = &bufferFs[d0PM * buffmax];
      Dbuff.f[d0MP] = &bufferFs[d0MP * buffmax];
      Dbuff.f[d000] = &bufferFs[d000 * buffmax];
      Dbuff.f[dPPP] = &bufferFs[dPPP * buffmax];
      Dbuff.f[dMMP] = &bufferFs[dMMP * buffmax];
      Dbuff.f[dPMP] = &bufferFs[dPMP * buffmax];
      Dbuff.f[dMPP] = &bufferFs[dMPP * buffmax];
      Dbuff.f[dPPM] = &bufferFs[dPPM * buffmax];
      Dbuff.f[dMMM] = &bufferFs[dMMM * buffmax];
      Dbuff.f[dPMM] = &bufferFs[dPMM * buffmax];
      Dbuff.f[dMPM] = &bufferFs[dMPM * buffmax];
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //copy to buffer
      (Dbuff.f[dP00])[k] = (D.f[dP00])[ke   ];
      (Dbuff.f[dM00])[k] = (D.f[dM00])[kw   ];
      (Dbuff.f[d0P0])[k] = (D.f[d0P0])[kn   ];
      (Dbuff.f[d0M0])[k] = (D.f[d0M0])[ks   ];
      (Dbuff.f[d00P])[k] = (D.f[d00P])[kt   ];
      (Dbuff.f[d00M])[k] = (D.f[d00M])[kb   ];
      (Dbuff.f[dPP0])[k] = (D.f[dPP0])[kne  ];
      (Dbuff.f[dMM0])[k] = (D.f[dMM0])[ksw  ];
      (Dbuff.f[dPM0])[k] = (D.f[dPM0])[kse  ];
      (Dbuff.f[dMP0])[k] = (D.f[dMP0])[knw  ];
      (Dbuff.f[dP0P])[k] = (D.f[dP0P])[kte  ];
      (Dbuff.f[dM0M])[k] = (D.f[dM0M])[kbw  ];
      (Dbuff.f[dP0M])[k] = (D.f[dP0M])[kbe  ];
      (Dbuff.f[dM0P])[k] = (D.f[dM0P])[ktw  ];
      (Dbuff.f[d0PP])[k] = (D.f[d0PP])[ktn  ];
      (Dbuff.f[d0MM])[k] = (D.f[d0MM])[kbs  ];
      (Dbuff.f[d0PM])[k] = (D.f[d0PM])[kbn  ];
      (Dbuff.f[d0MP])[k] = (D.f[d0MP])[kts  ];
      (Dbuff.f[d000])[k] = (D.f[d000])[kzero];
      (Dbuff.f[dPPP])[k] = (D.f[dPPP])[ktne ];
      (Dbuff.f[dMMP])[k] = (D.f[dMMP])[ktsw ];
      (Dbuff.f[dPMP])[k] = (D.f[dPMP])[ktse ];
      (Dbuff.f[dMPP])[k] = (D.f[dMPP])[ktnw ];
      (Dbuff.f[dPPM])[k] = (D.f[dPPM])[kbne ];
      (Dbuff.f[dMMM])[k] = (D.f[dMMM])[kbsw ];
      (Dbuff.f[dPMM])[k] = (D.f[dPMM])[kbse ];
      (Dbuff.f[dMPM])[k] = (D.f[dMPM])[kbnw ];
   }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






























////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__global__ void setRecvFsPre27(real* DD,
										  real* bufferFs,
										  int* recvIndex,
                                          int buffmax,
                                          unsigned int* neighborX,
                                          unsigned int* neighborY,
                                          unsigned int* neighborZ,
                                          unsigned long long numberOfLBnodes, 
                                          bool isEvenTimestep)
{
   ////////////////////////////////////////////////////////////////////////////////
   const unsigned  x = threadIdx.x;  // Globaler x-Index 
   const unsigned  y = blockIdx.x;   // Globaler y-Index 
   const unsigned  z = blockIdx.y;   // Globaler z-Index 

   const unsigned nx = blockDim.x;
   const unsigned ny = gridDim.x;

   const unsigned k = nx*(ny*z + y) + x;
   //////////////////////////////////////////////////////////////////////////

   if(k<buffmax)
   {
      ////////////////////////////////////////////////////////////////////////////////
      //set index
      unsigned int kIndex  = recvIndex[k];
      unsigned int kzero   = kIndex;
      unsigned int ke      = kIndex;
      unsigned int kw      = neighborX[kIndex];
      unsigned int kn      = kIndex;
      unsigned int ks      = neighborY[kIndex];
      unsigned int kt      = kIndex;
      unsigned int kb      = neighborZ[kIndex];
      unsigned int ksw     = neighborY[kw];
      unsigned int kne     = kIndex;
      unsigned int kse     = ks;
      unsigned int knw     = kw;
      unsigned int kbw     = neighborZ[kw];
      unsigned int kte     = kIndex;
      unsigned int kbe     = kb;
      unsigned int ktw     = kw;
      unsigned int kbs     = neighborZ[ks];
      unsigned int ktn     = kIndex;
      unsigned int kbn     = kb;
      unsigned int kts     = ks;
      unsigned int ktse    = ks;
      unsigned int kbnw    = kbw;
      unsigned int ktnw    = kw;
      unsigned int kbse    = kbs;
      unsigned int ktsw    = ksw;
      unsigned int kbne    = kb;
      unsigned int ktne    = kIndex;
      unsigned int kbsw    = neighborZ[ksw];
      ////////////////////////////////////////////////////////////////////////////////
	  //set Pointer for Fs
      Distributions27 D;
      if (isEvenTimestep==true)
      {
         D.f[dP00] = &DD[dP00 * numberOfLBnodes];
         D.f[dM00] = &DD[dM00 * numberOfLBnodes];
         D.f[d0P0] = &DD[d0P0 * numberOfLBnodes];
         D.f[d0M0] = &DD[d0M0 * numberOfLBnodes];
         D.f[d00P] = &DD[d00P * numberOfLBnodes];
         D.f[d00M] = &DD[d00M * numberOfLBnodes];
         D.f[dPP0] = &DD[dPP0 * numberOfLBnodes];
         D.f[dMM0] = &DD[dMM0 * numberOfLBnodes];
         D.f[dPM0] = &DD[dPM0 * numberOfLBnodes];
         D.f[dMP0] = &DD[dMP0 * numberOfLBnodes];
         D.f[dP0P] = &DD[dP0P * numberOfLBnodes];
         D.f[dM0M] = &DD[dM0M * numberOfLBnodes];
         D.f[dP0M] = &DD[dP0M * numberOfLBnodes];
         D.f[dM0P] = &DD[dM0P * numberOfLBnodes];
         D.f[d0PP] = &DD[d0PP * numberOfLBnodes];
         D.f[d0MM] = &DD[d0MM * numberOfLBnodes];
         D.f[d0PM] = &DD[d0PM * numberOfLBnodes];
         D.f[d0MP] = &DD[d0MP * numberOfLBnodes];
         D.f[d000] = &DD[d000 * numberOfLBnodes];
         D.f[dPPP] = &DD[dPPP * numberOfLBnodes];
         D.f[dMMP] = &DD[dMMP * numberOfLBnodes];
         D.f[dPMP] = &DD[dPMP * numberOfLBnodes];
         D.f[dMPP] = &DD[dMPP * numberOfLBnodes];
         D.f[dPPM] = &DD[dPPM * numberOfLBnodes];
         D.f[dMMM] = &DD[dMMM * numberOfLBnodes];
         D.f[dPMM] = &DD[dPMM * numberOfLBnodes];
         D.f[dMPM] = &DD[dMPM * numberOfLBnodes];
      } 
      else
      {
         D.f[dM00] = &DD[dP00 * numberOfLBnodes];
         D.f[dP00] = &DD[dM00 * numberOfLBnodes];
         D.f[d0M0] = &DD[d0P0 * numberOfLBnodes];
         D.f[d0P0] = &DD[d0M0 * numberOfLBnodes];
         D.f[d00M] = &DD[d00P * numberOfLBnodes];
         D.f[d00P] = &DD[d00M * numberOfLBnodes];
         D.f[dMM0] = &DD[dPP0 * numberOfLBnodes];
         D.f[dPP0] = &DD[dMM0 * numberOfLBnodes];
         D.f[dMP0] = &DD[dPM0 * numberOfLBnodes];
         D.f[dPM0] = &DD[dMP0 * numberOfLBnodes];
         D.f[dM0M] = &DD[dP0P * numberOfLBnodes];
         D.f[dP0P] = &DD[dM0M * numberOfLBnodes];
         D.f[dM0P] = &DD[dP0M * numberOfLBnodes];
         D.f[dP0M] = &DD[dM0P * numberOfLBnodes];
         D.f[d0MM] = &DD[d0PP * numberOfLBnodes];
         D.f[d0PP] = &DD[d0MM * numberOfLBnodes];
         D.f[d0MP] = &DD[d0PM * numberOfLBnodes];
         D.f[d0PM] = &DD[d0MP * numberOfLBnodes];
         D.f[d000] = &DD[d000 * numberOfLBnodes];
         D.f[dPPP] = &DD[dMMM * numberOfLBnodes];
         D.f[dMMP] = &DD[dPPM * numberOfLBnodes];
         D.f[dPMP] = &DD[dMPM * numberOfLBnodes];
         D.f[dMPP] = &DD[dPMM * numberOfLBnodes];
         D.f[dPPM] = &DD[dMMP * numberOfLBnodes];
         D.f[dMMM] = &DD[dPPP * numberOfLBnodes];
         D.f[dPMM] = &DD[dMPP * numberOfLBnodes];
         D.f[dMPM] = &DD[dPMP * numberOfLBnodes];
      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //set Pointer for Buffer Fs
      Distributions27 Dbuff;
      Dbuff.f[dP00] = &bufferFs[dP00 * buffmax];
      Dbuff.f[dM00] = &bufferFs[dM00 * buffmax];
      Dbuff.f[d0P0] = &bufferFs[d0P0 * buffmax];
      Dbuff.f[d0M0] = &bufferFs[d0M0 * buffmax];
      Dbuff.f[d00P] = &bufferFs[d00P * buffmax];
      Dbuff.f[d00M] = &bufferFs[d00M * buffmax];
      Dbuff.f[dPP0] = &bufferFs[dPP0 * buffmax];
      Dbuff.f[dMM0] = &bufferFs[dMM0 * buffmax];
      Dbuff.f[dPM0] = &bufferFs[dPM0 * buffmax];
      Dbuff.f[dMP0] = &bufferFs[dMP0 * buffmax];
      Dbuff.f[dP0P] = &bufferFs[dP0P * buffmax];
      Dbuff.f[dM0M] = &bufferFs[dM0M * buffmax];
      Dbuff.f[dP0M] = &bufferFs[dP0M * buffmax];
      Dbuff.f[dM0P] = &bufferFs[dM0P * buffmax];
      Dbuff.f[d0PP] = &bufferFs[d0PP * buffmax];
      Dbuff.f[d0MM] = &bufferFs[d0MM * buffmax];
      Dbuff.f[d0PM] = &bufferFs[d0PM * buffmax];
      Dbuff.f[d0MP] = &bufferFs[d0MP * buffmax];
      Dbuff.f[d000] = &bufferFs[d000 * buffmax];
      Dbuff.f[dPPP] = &bufferFs[dPPP * buffmax];
      Dbuff.f[dMMP] = &bufferFs[dMMP * buffmax];
      Dbuff.f[dPMP] = &bufferFs[dPMP * buffmax];
      Dbuff.f[dMPP] = &bufferFs[dMPP * buffmax];
      Dbuff.f[dPPM] = &bufferFs[dPPM * buffmax];
      Dbuff.f[dMMM] = &bufferFs[dMMM * buffmax];
      Dbuff.f[dPMM] = &bufferFs[dPMM * buffmax];
      Dbuff.f[dMPM] = &bufferFs[dMPM * buffmax];
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //copy from buffer
      (D.f[dP00])[ke   ] = (Dbuff.f[dP00])[k];
      (D.f[dM00])[kw   ] = (Dbuff.f[dM00])[k];
      (D.f[d0P0])[kn   ] = (Dbuff.f[d0P0])[k];
      (D.f[d0M0])[ks   ] = (Dbuff.f[d0M0])[k];
      (D.f[d00P])[kt   ] = (Dbuff.f[d00P])[k];
      (D.f[d00M])[kb   ] = (Dbuff.f[d00M])[k];
      (D.f[dPP0])[kne  ] = (Dbuff.f[dPP0])[k];
      (D.f[dMM0])[ksw  ] = (Dbuff.f[dMM0])[k];
      (D.f[dPM0])[kse  ] = (Dbuff.f[dPM0])[k];
      (D.f[dMP0])[knw  ] = (Dbuff.f[dMP0])[k];
      (D.f[dP0P])[kte  ] = (Dbuff.f[dP0P])[k];
      (D.f[dM0M])[kbw  ] = (Dbuff.f[dM0M])[k];
      (D.f[dP0M])[kbe  ] = (Dbuff.f[dP0M])[k];
      (D.f[dM0P])[ktw  ] = (Dbuff.f[dM0P])[k];
      (D.f[d0PP])[ktn  ] = (Dbuff.f[d0PP])[k];
      (D.f[d0MM])[kbs  ] = (Dbuff.f[d0MM])[k];
      (D.f[d0PM])[kbn  ] = (Dbuff.f[d0PM])[k];
      (D.f[d0MP])[kts  ] = (Dbuff.f[d0MP])[k];
      (D.f[d000])[kzero] = (Dbuff.f[d000])[k];
      (D.f[dPPP])[ktne ] = (Dbuff.f[dPPP])[k];
      (D.f[dMMP])[ktsw ] = (Dbuff.f[dMMP])[k];
      (D.f[dPMP])[ktse ] = (Dbuff.f[dPMP])[k];
      (D.f[dMPP])[ktnw ] = (Dbuff.f[dMPP])[k];
      (D.f[dPPM])[kbne ] = (Dbuff.f[dPPM])[k];
      (D.f[dMMM])[kbsw ] = (Dbuff.f[dMMM])[k];
      (D.f[dPMM])[kbse ] = (Dbuff.f[dPMM])[k];
      (D.f[dMPM])[kbnw ] = (Dbuff.f[dMPM])[k];
   }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





























////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__global__ void getSendGsF3(
	real* G6,
	real* bufferGs,
	int* sendIndex,
	int buffmax,
	unsigned int* neighborX,
	unsigned int* neighborY,
	unsigned int* neighborZ,
	unsigned long long numberOfLBnodes,
	bool isEvenTimestep)
{
	////////////////////////////////////////////////////////////////////////////////
	const unsigned  x = threadIdx.x;  // Globaler x-Index 
	const unsigned  y = blockIdx.x;   // Globaler y-Index 
	const unsigned  z = blockIdx.y;   // Globaler z-Index 

	const unsigned nx = blockDim.x;
	const unsigned ny = gridDim.x;

	const unsigned k = nx*(ny*z + y) + x;
	//////////////////////////////////////////////////////////////////////////

	if (k < buffmax)
	{
		////////////////////////////////////////////////////////////////////////////////
		//set index
		unsigned int kIndex = sendIndex[k];
		unsigned int kr = kIndex;
		unsigned int kw = neighborX[kIndex];
		unsigned int ks = neighborY[kIndex];
		unsigned int kb = neighborZ[kIndex];
		////////////////////////////////////////////////////////////////////////////////
		//set Pointer for Gs
		Distributions6 G;
		if (isEvenTimestep)
		{
			G.g[dP00] = &G6[dP00 * numberOfLBnodes];
			G.g[dM00] = &G6[dM00 * numberOfLBnodes];
			G.g[d0P0] = &G6[d0P0 * numberOfLBnodes];
			G.g[d0M0] = &G6[d0M0 * numberOfLBnodes];
			G.g[d00P] = &G6[d00P * numberOfLBnodes];
			G.g[d00M] = &G6[d00M * numberOfLBnodes];
		}
		else
		{
			G.g[dM00] = &G6[dP00 * numberOfLBnodes];
			G.g[dP00] = &G6[dM00 * numberOfLBnodes];
			G.g[d0M0] = &G6[d0P0 * numberOfLBnodes];
			G.g[d0P0] = &G6[d0M0 * numberOfLBnodes];
			G.g[d00M] = &G6[d00P * numberOfLBnodes];
			G.g[d00P] = &G6[d00M * numberOfLBnodes];
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//set Pointer for Buffer Gs
		Distributions6 Dbuff;
		Dbuff.g[dP00] = &bufferGs[dP00 * buffmax];
		Dbuff.g[dM00] = &bufferGs[dM00 * buffmax];
		Dbuff.g[d0P0] = &bufferGs[d0P0 * buffmax];
		Dbuff.g[d0M0] = &bufferGs[d0M0 * buffmax];
		Dbuff.g[d00P] = &bufferGs[d00P * buffmax];
		Dbuff.g[d00M] = &bufferGs[d00M * buffmax];
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//write Gs to buffer
		(Dbuff.g[dP00])[k] = (G.g[dM00])[kw];
		(Dbuff.g[dM00])[k] = (G.g[dP00])[kr];
		(Dbuff.g[d0P0])[k] = (G.g[d0M0])[ks];
		(Dbuff.g[d0M0])[k] = (G.g[d0P0])[kr];
		(Dbuff.g[d00P])[k] = (G.g[d00M])[kb];
		(Dbuff.g[d00M])[k] = (G.g[d00P])[kr];
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






























////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__global__ void setRecvGsF3(
	real* G6,
	real* bufferGs,
	int* recvIndex,
	int buffmax,
	unsigned int* neighborX,
	unsigned int* neighborY,
	unsigned int* neighborZ,
	unsigned long long numberOfLBnodes,
	bool isEvenTimestep)
{
	////////////////////////////////////////////////////////////////////////////////
	const unsigned  x = threadIdx.x;  // Globaler x-Index 
	const unsigned  y = blockIdx.x;   // Globaler y-Index 
	const unsigned  z = blockIdx.y;   // Globaler z-Index 

	const unsigned nx = blockDim.x;
	const unsigned ny = gridDim.x;

	const unsigned k = nx*(ny*z + y) + x;
	//////////////////////////////////////////////////////////////////////////

	if (k < buffmax)
	{
		////////////////////////////////////////////////////////////////////////////////
		//set index
		unsigned int kIndex = recvIndex[k];
		unsigned int kr = kIndex;
		unsigned int kw = neighborX[kIndex];
		unsigned int ks = neighborY[kIndex];
		unsigned int kb = neighborZ[kIndex];
		////////////////////////////////////////////////////////////////////////////////
		//set Pointer for Gs
		Distributions6 G;
		if (isEvenTimestep)
		{
			G.g[dP00] = &G6[dP00 * numberOfLBnodes];
			G.g[dM00] = &G6[dM00 * numberOfLBnodes];
			G.g[d0P0] = &G6[d0P0 * numberOfLBnodes];
			G.g[d0M0] = &G6[d0M0 * numberOfLBnodes];
			G.g[d00P] = &G6[d00P * numberOfLBnodes];
			G.g[d00M] = &G6[d00M * numberOfLBnodes];
		}
		else
		{
			G.g[dM00] = &G6[dP00 * numberOfLBnodes];
			G.g[dP00] = &G6[dM00 * numberOfLBnodes];
			G.g[d0M0] = &G6[d0P0 * numberOfLBnodes];
			G.g[d0P0] = &G6[d0M0 * numberOfLBnodes];
			G.g[d00M] = &G6[d00P * numberOfLBnodes];
			G.g[d00P] = &G6[d00M * numberOfLBnodes];
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//set Pointer for Buffer Gs
		Distributions6 Dbuff;
		Dbuff.g[dP00] = &bufferGs[dP00 * buffmax];
		Dbuff.g[dM00] = &bufferGs[dM00 * buffmax];
		Dbuff.g[d0P0] = &bufferGs[d0P0 * buffmax];
		Dbuff.g[d0M0] = &bufferGs[d0M0 * buffmax];
		Dbuff.g[d00P] = &bufferGs[d00P * buffmax];
		Dbuff.g[d00M] = &bufferGs[d00M * buffmax];
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//write buffer to Gs
		(G.g[dM00])[kw] = (Dbuff.g[dP00])[k];
		(G.g[dP00])[kr] = (Dbuff.g[dM00])[k];
		(G.g[d0M0])[ks] = (Dbuff.g[d0P0])[k];
		(G.g[d0P0])[kr] = (Dbuff.g[d0M0])[k];
		(G.g[d00M])[kb] = (Dbuff.g[d00P])[k];
		(G.g[d00P])[kr] = (Dbuff.g[d00M])[k];
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
