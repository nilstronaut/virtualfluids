/* Device code */
#include "LBM/D3Q27.h"
#include "GPU/constant.h"

////////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void LBCalc2ndMomentsIncompSP27(  doubflo* kxyFromfcNEQ,
														doubflo* kyzFromfcNEQ,
														doubflo* kxzFromfcNEQ,
														doubflo* kxxMyyFromfcNEQ,
														doubflo* kxxMzzFromfcNEQ,
														unsigned int* geoD,
														unsigned int* neighborX,
														unsigned int* neighborY,
														unsigned int* neighborZ,
														unsigned int size_Mat,
														doubflo* DD,
														bool evenOrOdd)
{
   Distributions27 D;
   if (evenOrOdd==true)
   {
      D.f[dirE   ] = &DD[dirE   *size_Mat];
      D.f[dirW   ] = &DD[dirW   *size_Mat];
      D.f[dirN   ] = &DD[dirN   *size_Mat];
      D.f[dirS   ] = &DD[dirS   *size_Mat];
      D.f[dirT   ] = &DD[dirT   *size_Mat];
      D.f[dirB   ] = &DD[dirB   *size_Mat];
      D.f[dirNE  ] = &DD[dirNE  *size_Mat];
      D.f[dirSW  ] = &DD[dirSW  *size_Mat];
      D.f[dirSE  ] = &DD[dirSE  *size_Mat];
      D.f[dirNW  ] = &DD[dirNW  *size_Mat];
      D.f[dirTE  ] = &DD[dirTE  *size_Mat];
      D.f[dirBW  ] = &DD[dirBW  *size_Mat];
      D.f[dirBE  ] = &DD[dirBE  *size_Mat];
      D.f[dirTW  ] = &DD[dirTW  *size_Mat];
      D.f[dirTN  ] = &DD[dirTN  *size_Mat];
      D.f[dirBS  ] = &DD[dirBS  *size_Mat];
      D.f[dirBN  ] = &DD[dirBN  *size_Mat];
      D.f[dirTS  ] = &DD[dirTS  *size_Mat];
      D.f[dirZERO] = &DD[dirZERO*size_Mat];
      D.f[dirTNE ] = &DD[dirTNE *size_Mat];
      D.f[dirTSW ] = &DD[dirTSW *size_Mat];
      D.f[dirTSE ] = &DD[dirTSE *size_Mat];
      D.f[dirTNW ] = &DD[dirTNW *size_Mat];
      D.f[dirBNE ] = &DD[dirBNE *size_Mat];
      D.f[dirBSW ] = &DD[dirBSW *size_Mat];
      D.f[dirBSE ] = &DD[dirBSE *size_Mat];
      D.f[dirBNW ] = &DD[dirBNW *size_Mat];
   } 
   else
   {
      D.f[dirW   ] = &DD[dirE   *size_Mat];
      D.f[dirE   ] = &DD[dirW   *size_Mat];
      D.f[dirS   ] = &DD[dirN   *size_Mat];
      D.f[dirN   ] = &DD[dirS   *size_Mat];
      D.f[dirB   ] = &DD[dirT   *size_Mat];
      D.f[dirT   ] = &DD[dirB   *size_Mat];
      D.f[dirSW  ] = &DD[dirNE  *size_Mat];
      D.f[dirNE  ] = &DD[dirSW  *size_Mat];
      D.f[dirNW  ] = &DD[dirSE  *size_Mat];
      D.f[dirSE  ] = &DD[dirNW  *size_Mat];
      D.f[dirBW  ] = &DD[dirTE  *size_Mat];
      D.f[dirTE  ] = &DD[dirBW  *size_Mat];
      D.f[dirTW  ] = &DD[dirBE  *size_Mat];
      D.f[dirBE  ] = &DD[dirTW  *size_Mat];
      D.f[dirBS  ] = &DD[dirTN  *size_Mat];
      D.f[dirTN  ] = &DD[dirBS  *size_Mat];
      D.f[dirTS  ] = &DD[dirBN  *size_Mat];
      D.f[dirBN  ] = &DD[dirTS  *size_Mat];
      D.f[dirZERO] = &DD[dirZERO*size_Mat];
      D.f[dirTNE ] = &DD[dirBSW *size_Mat];
      D.f[dirTSW ] = &DD[dirBNE *size_Mat];
      D.f[dirTSE ] = &DD[dirBNW *size_Mat];
      D.f[dirTNW ] = &DD[dirBSE *size_Mat];
      D.f[dirBNE ] = &DD[dirTSW *size_Mat];
      D.f[dirBSW ] = &DD[dirTNE *size_Mat];
      D.f[dirBSE ] = &DD[dirTNW *size_Mat];
      D.f[dirBNW ] = &DD[dirTSE *size_Mat];
   }
   ////////////////////////////////////////////////////////////////////////////////
   const unsigned  x = threadIdx.x;  // Globaler x-Index 
   const unsigned  y = blockIdx.x;   // Globaler y-Index 
   const unsigned  z = blockIdx.y;   // Globaler z-Index 

   const unsigned nx = blockDim.x;
   const unsigned ny = gridDim.x;

   const unsigned k = nx*(ny*z + y) + x;
   //////////////////////////////////////////////////////////////////////////

   if(k < size_Mat)
   {
      //////////////////////////////////////////////////////////////////////////
      //index
      //unsigned int kzero= k;
      unsigned int ke   = k;
      unsigned int kw   = neighborX[k];
      unsigned int kn   = k;
      unsigned int ks   = neighborY[k];
      unsigned int kt   = k;
      unsigned int kb   = neighborZ[k];
      unsigned int ksw  = neighborY[kw];
      unsigned int kne  = k;
      unsigned int kse  = ks;
      unsigned int knw  = kw;
      unsigned int kbw  = neighborZ[kw];
      unsigned int kte  = k;
      unsigned int kbe  = kb;
      unsigned int ktw  = kw;
      unsigned int kbs  = neighborZ[ks];
      unsigned int ktn  = k;
      unsigned int kbn  = kb;
      unsigned int kts  = ks;
      unsigned int ktse = ks;
      unsigned int kbnw = kbw;
      unsigned int ktnw = kw;
      unsigned int kbse = kbs;
      unsigned int ktsw = ksw;
      unsigned int kbne = kb;
      unsigned int ktne = k;
      unsigned int kbsw = neighborZ[ksw];
      //////////////////////////////////////////////////////////////////////////
      doubflo        f_E,f_W,f_N,f_S,f_T,f_B,f_NE,f_SW,f_SE,f_NW,f_TE,f_BW,f_BE,f_TW,f_TN,f_BS,f_BN,f_TS,/*f_ZERO,*/f_TNE, f_TSW, f_TSE, f_TNW, f_BNE, f_BSW, f_BSE, f_BNW;
	  f_E    = (D.f[dirE   ])[ke   ];
	  f_W    = (D.f[dirW   ])[kw   ];
	  f_N    = (D.f[dirN   ])[kn   ];
	  f_S    = (D.f[dirS   ])[ks   ];
	  f_T    = (D.f[dirT   ])[kt   ];
	  f_B    = (D.f[dirB   ])[kb   ];
	  f_NE   = (D.f[dirNE  ])[kne  ];
	  f_SW   = (D.f[dirSW  ])[ksw  ];
	  f_SE   = (D.f[dirSE  ])[kse  ];
	  f_NW   = (D.f[dirNW  ])[knw  ];
	  f_TE   = (D.f[dirTE  ])[kte  ];
	  f_BW   = (D.f[dirBW  ])[kbw  ];
	  f_BE   = (D.f[dirBE  ])[kbe  ];
	  f_TW   = (D.f[dirTW  ])[ktw  ];
	  f_TN   = (D.f[dirTN  ])[ktn  ];
	  f_BS   = (D.f[dirBS  ])[kbs  ];
	  f_BN   = (D.f[dirBN  ])[kbn  ];
	  f_TS   = (D.f[dirTS  ])[kts  ];
	  //f_ZERO = (D.f[dirZERO])[kzero];
	  f_TNE  = (D.f[dirTNE ])[ktne ];
	  f_TSW  = (D.f[dirTSW ])[ktsw ];
	  f_TSE  = (D.f[dirTSE ])[ktse ];
	  f_TNW  = (D.f[dirTNW ])[ktnw ];
	  f_BNE  = (D.f[dirBNE ])[kbne ];
	  f_BSW  = (D.f[dirBSW ])[kbsw ];
	  f_BSE  = (D.f[dirBSE ])[kbse ];
	  f_BNW  = (D.f[dirBNW ])[kbnw ];
      //////////////////////////////////////////////////////////////////////////
	  doubflo vx1, vx2, vx3;
      kxyFromfcNEQ[k]       = zero;
	  kyzFromfcNEQ[k]       = zero;
	  kxzFromfcNEQ[k]       = zero;
	  kxxMyyFromfcNEQ[k]    = zero;
	  kxxMzzFromfcNEQ[k]    = zero;

      if(geoD[k] == GEO_FLUID)
      {
		  vx1                = ((f_TNE-f_BSW)+(f_BSE-f_TNW)+(f_BNE-f_TSW)+(f_TSE-f_BNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W);
		  vx2                = ((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S);
		  vx3                = ((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSW-f_BNE)+(f_TSE-f_BNW)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B);
		  kxyFromfcNEQ[k]    = -three *(f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE-(vx1*vx2));
		  kyzFromfcNEQ[k]    = -three *(f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW-(vx2*vx3));
		  kxzFromfcNEQ[k]    = -three *(f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE-(vx1*vx3));
		  kxxMyyFromfcNEQ[k] = -c3o2 * (f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE-(vx1*vx1-vx2*vx2));		//all E+W minus all N+S (no combinations of xy left)
		  kxxMzzFromfcNEQ[k] = -c3o2 * (f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE-(vx1*vx1-vx3*vx3));		//all E+W minus all T+B (no combinations of xz left)
      }
   }
}
////////////////////////////////////////////////////////////////////////////////































////////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void LBCalc2ndMomentsCompSP27(doubflo* kxyFromfcNEQ,
													doubflo* kyzFromfcNEQ,
													doubflo* kxzFromfcNEQ,
													doubflo* kxxMyyFromfcNEQ,
													doubflo* kxxMzzFromfcNEQ,
													unsigned int* geoD,
													unsigned int* neighborX,
													unsigned int* neighborY,
													unsigned int* neighborZ,
													unsigned int size_Mat,
													doubflo* DD,
													bool evenOrOdd)
{
   Distributions27 D;
   if (evenOrOdd==true)
   {
      D.f[dirE   ] = &DD[dirE   *size_Mat];
      D.f[dirW   ] = &DD[dirW   *size_Mat];
      D.f[dirN   ] = &DD[dirN   *size_Mat];
      D.f[dirS   ] = &DD[dirS   *size_Mat];
      D.f[dirT   ] = &DD[dirT   *size_Mat];
      D.f[dirB   ] = &DD[dirB   *size_Mat];
      D.f[dirNE  ] = &DD[dirNE  *size_Mat];
      D.f[dirSW  ] = &DD[dirSW  *size_Mat];
      D.f[dirSE  ] = &DD[dirSE  *size_Mat];
      D.f[dirNW  ] = &DD[dirNW  *size_Mat];
      D.f[dirTE  ] = &DD[dirTE  *size_Mat];
      D.f[dirBW  ] = &DD[dirBW  *size_Mat];
      D.f[dirBE  ] = &DD[dirBE  *size_Mat];
      D.f[dirTW  ] = &DD[dirTW  *size_Mat];
      D.f[dirTN  ] = &DD[dirTN  *size_Mat];
      D.f[dirBS  ] = &DD[dirBS  *size_Mat];
      D.f[dirBN  ] = &DD[dirBN  *size_Mat];
      D.f[dirTS  ] = &DD[dirTS  *size_Mat];
      D.f[dirZERO] = &DD[dirZERO*size_Mat];
      D.f[dirTNE ] = &DD[dirTNE *size_Mat];
      D.f[dirTSW ] = &DD[dirTSW *size_Mat];
      D.f[dirTSE ] = &DD[dirTSE *size_Mat];
      D.f[dirTNW ] = &DD[dirTNW *size_Mat];
      D.f[dirBNE ] = &DD[dirBNE *size_Mat];
      D.f[dirBSW ] = &DD[dirBSW *size_Mat];
      D.f[dirBSE ] = &DD[dirBSE *size_Mat];
      D.f[dirBNW ] = &DD[dirBNW *size_Mat];
   } 
   else
   {
      D.f[dirW   ] = &DD[dirE   *size_Mat];
      D.f[dirE   ] = &DD[dirW   *size_Mat];
      D.f[dirS   ] = &DD[dirN   *size_Mat];
      D.f[dirN   ] = &DD[dirS   *size_Mat];
      D.f[dirB   ] = &DD[dirT   *size_Mat];
      D.f[dirT   ] = &DD[dirB   *size_Mat];
      D.f[dirSW  ] = &DD[dirNE  *size_Mat];
      D.f[dirNE  ] = &DD[dirSW  *size_Mat];
      D.f[dirNW  ] = &DD[dirSE  *size_Mat];
      D.f[dirSE  ] = &DD[dirNW  *size_Mat];
      D.f[dirBW  ] = &DD[dirTE  *size_Mat];
      D.f[dirTE  ] = &DD[dirBW  *size_Mat];
      D.f[dirTW  ] = &DD[dirBE  *size_Mat];
      D.f[dirBE  ] = &DD[dirTW  *size_Mat];
      D.f[dirBS  ] = &DD[dirTN  *size_Mat];
      D.f[dirTN  ] = &DD[dirBS  *size_Mat];
      D.f[dirTS  ] = &DD[dirBN  *size_Mat];
      D.f[dirBN  ] = &DD[dirTS  *size_Mat];
      D.f[dirZERO] = &DD[dirZERO*size_Mat];
      D.f[dirTNE ] = &DD[dirBSW *size_Mat];
      D.f[dirTSW ] = &DD[dirBNE *size_Mat];
      D.f[dirTSE ] = &DD[dirBNW *size_Mat];
      D.f[dirTNW ] = &DD[dirBSE *size_Mat];
      D.f[dirBNE ] = &DD[dirTSW *size_Mat];
      D.f[dirBSW ] = &DD[dirTNE *size_Mat];
      D.f[dirBSE ] = &DD[dirTNW *size_Mat];
      D.f[dirBNW ] = &DD[dirTSE *size_Mat];
   }
   ////////////////////////////////////////////////////////////////////////////////
   const unsigned  x = threadIdx.x;  // Globaler x-Index 
   const unsigned  y = blockIdx.x;   // Globaler y-Index 
   const unsigned  z = blockIdx.y;   // Globaler z-Index 

   const unsigned nx = blockDim.x;
   const unsigned ny = gridDim.x;

   const unsigned k = nx*(ny*z + y) + x;
   //////////////////////////////////////////////////////////////////////////

   if(k < size_Mat)
   {
      //////////////////////////////////////////////////////////////////////////
      //index
      unsigned int kzero= k;
      unsigned int ke   = k;
      unsigned int kw   = neighborX[k];
      unsigned int kn   = k;
      unsigned int ks   = neighborY[k];
      unsigned int kt   = k;
      unsigned int kb   = neighborZ[k];
      unsigned int ksw  = neighborY[kw];
      unsigned int kne  = k;
      unsigned int kse  = ks;
      unsigned int knw  = kw;
      unsigned int kbw  = neighborZ[kw];
      unsigned int kte  = k;
      unsigned int kbe  = kb;
      unsigned int ktw  = kw;
      unsigned int kbs  = neighborZ[ks];
      unsigned int ktn  = k;
      unsigned int kbn  = kb;
      unsigned int kts  = ks;
      unsigned int ktse = ks;
      unsigned int kbnw = kbw;
      unsigned int ktnw = kw;
      unsigned int kbse = kbs;
      unsigned int ktsw = ksw;
      unsigned int kbne = kb;
      unsigned int ktne = k;
      unsigned int kbsw = neighborZ[ksw];
      //////////////////////////////////////////////////////////////////////////
      doubflo        f_E,f_W,f_N,f_S,f_T,f_B,f_NE,f_SW,f_SE,f_NW,f_TE,f_BW,f_BE,f_TW,f_TN,f_BS,f_BN,f_TS,f_ZERO,f_TNE, f_TSW, f_TSE, f_TNW, f_BNE, f_BSW, f_BSE, f_BNW;
	  f_E    = (D.f[dirE   ])[ke   ];
	  f_W    = (D.f[dirW   ])[kw   ];
	  f_N    = (D.f[dirN   ])[kn   ];
	  f_S    = (D.f[dirS   ])[ks   ];
	  f_T    = (D.f[dirT   ])[kt   ];
	  f_B    = (D.f[dirB   ])[kb   ];
	  f_NE   = (D.f[dirNE  ])[kne  ];
	  f_SW   = (D.f[dirSW  ])[ksw  ];
	  f_SE   = (D.f[dirSE  ])[kse  ];
	  f_NW   = (D.f[dirNW  ])[knw  ];
	  f_TE   = (D.f[dirTE  ])[kte  ];
	  f_BW   = (D.f[dirBW  ])[kbw  ];
	  f_BE   = (D.f[dirBE  ])[kbe  ];
	  f_TW   = (D.f[dirTW  ])[ktw  ];
	  f_TN   = (D.f[dirTN  ])[ktn  ];
	  f_BS   = (D.f[dirBS  ])[kbs  ];
	  f_BN   = (D.f[dirBN  ])[kbn  ];
	  f_TS   = (D.f[dirTS  ])[kts  ];
	  f_ZERO = (D.f[dirZERO])[kzero];
	  f_TNE  = (D.f[dirTNE ])[ktne ];
	  f_TSW  = (D.f[dirTSW ])[ktsw ];
	  f_TSE  = (D.f[dirTSE ])[ktse ];
	  f_TNW  = (D.f[dirTNW ])[ktnw ];
	  f_BNE  = (D.f[dirBNE ])[kbne ];
	  f_BSW  = (D.f[dirBSW ])[kbsw ];
	  f_BSE  = (D.f[dirBSE ])[kbse ];
	  f_BNW  = (D.f[dirBNW ])[kbnw ];
      //////////////////////////////////////////////////////////////////////////
	  doubflo vx1, vx2, vx3, drho, rho;
      kxyFromfcNEQ[k]       = zero;
	  kyzFromfcNEQ[k]       = zero;
	  kxzFromfcNEQ[k]       = zero;
	  kxxMyyFromfcNEQ[k]    = zero;
	  kxxMzzFromfcNEQ[k]    = zero;

      if(geoD[k] == GEO_FLUID)
      {
		  drho               = ((f_TNE+f_BSW)+(f_BSE+f_TNW)+(f_BNE+f_TSW)+(f_TSE+f_BNW)) + 
							   ((f_NE+f_SW)+(f_TE+f_BW)+(f_SE+f_NW)+(f_BE+f_TW)+(f_BN+f_TS)+(f_TN+f_BS)) + 
							   ((f_E-f_W) + (f_N-f_S) + (f_T-f_B)) + f_ZERO;
		  rho                = rho + one;
		  vx1                = ((f_TNE-f_BSW)+(f_BSE-f_TNW)+(f_BNE-f_TSW)+(f_TSE-f_BNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W) / rho;
		  vx2                = ((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S) / rho;
		  vx3                = ((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSW-f_BNE)+(f_TSE-f_BNW)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B) / rho;
		  kxyFromfcNEQ[k]    = -three *(f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE-(vx1*vx2));
		  kyzFromfcNEQ[k]    = -three *(f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW-(vx2*vx3));
		  kxzFromfcNEQ[k]    = -three *(f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE-(vx1*vx3));
		  kxxMyyFromfcNEQ[k] = -c3o2 * (f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE-(vx1*vx1-vx2*vx2));		//all E+W minus all N+S (no combinations of xy left)
		  kxxMzzFromfcNEQ[k] = -c3o2 * (f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE-(vx1*vx1-vx3*vx3));		//all E+W minus all T+B (no combinations of xz left)
      }
   }
}
////////////////////////////////////////////////////////////////////////////////































////////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void LBCalc3rdMomentsIncompSP27(  doubflo* CUMbbb,
														doubflo* CUMabc,
														doubflo* CUMbac,
														doubflo* CUMbca,
														doubflo* CUMcba,
														doubflo* CUMacb,
														doubflo* CUMcab,
														unsigned int* bcMatD,
														unsigned int* neighborX,
														unsigned int* neighborY,
														unsigned int* neighborZ,
														doubflo* DDStart,
														int size_Mat,
														bool EvenOrOdd)
{
	////////////////////////////////////////////////////////////////////////////////
	const unsigned  x = threadIdx.x;  // Globaler x-Index 
	const unsigned  y = blockIdx.x;   // Globaler y-Index 
	const unsigned  z = blockIdx.y;   // Globaler z-Index 

	const unsigned nx = blockDim.x;
	const unsigned ny = gridDim.x;

	const unsigned k = nx*(ny*z + y) + x;
	//////////////////////////////////////////////////////////////////////////

	if(k<size_Mat)
	{
		////////////////////////////////////////////////////////////////////////////////
		unsigned int BC;
		BC = bcMatD[k];

		if( (BC != GEO_SOLID) && (BC != GEO_VOID) )
		{
			Distributions27 D;
			if (EvenOrOdd==true)
			{
				D.f[dirE   ] = &DDStart[dirE   *size_Mat];
				D.f[dirW   ] = &DDStart[dirW   *size_Mat];
				D.f[dirN   ] = &DDStart[dirN   *size_Mat];
				D.f[dirS   ] = &DDStart[dirS   *size_Mat];
				D.f[dirT   ] = &DDStart[dirT   *size_Mat];
				D.f[dirB   ] = &DDStart[dirB   *size_Mat];
				D.f[dirNE  ] = &DDStart[dirNE  *size_Mat];
				D.f[dirSW  ] = &DDStart[dirSW  *size_Mat];
				D.f[dirSE  ] = &DDStart[dirSE  *size_Mat];
				D.f[dirNW  ] = &DDStart[dirNW  *size_Mat];
				D.f[dirTE  ] = &DDStart[dirTE  *size_Mat];
				D.f[dirBW  ] = &DDStart[dirBW  *size_Mat];
				D.f[dirBE  ] = &DDStart[dirBE  *size_Mat];
				D.f[dirTW  ] = &DDStart[dirTW  *size_Mat];
				D.f[dirTN  ] = &DDStart[dirTN  *size_Mat];
				D.f[dirBS  ] = &DDStart[dirBS  *size_Mat];
				D.f[dirBN  ] = &DDStart[dirBN  *size_Mat];
				D.f[dirTS  ] = &DDStart[dirTS  *size_Mat];
				D.f[dirZERO] = &DDStart[dirZERO*size_Mat];
				D.f[dirTNE ] = &DDStart[dirTNE *size_Mat];
				D.f[dirTSW ] = &DDStart[dirTSW *size_Mat];
				D.f[dirTSE ] = &DDStart[dirTSE *size_Mat];
				D.f[dirTNW ] = &DDStart[dirTNW *size_Mat];
				D.f[dirBNE ] = &DDStart[dirBNE *size_Mat];
				D.f[dirBSW ] = &DDStart[dirBSW *size_Mat];
				D.f[dirBSE ] = &DDStart[dirBSE *size_Mat];
				D.f[dirBNW ] = &DDStart[dirBNW *size_Mat];
			}
			else
			{
				D.f[dirW   ] = &DDStart[dirE   *size_Mat];
				D.f[dirE   ] = &DDStart[dirW   *size_Mat];
				D.f[dirS   ] = &DDStart[dirN   *size_Mat];
				D.f[dirN   ] = &DDStart[dirS   *size_Mat];
				D.f[dirB   ] = &DDStart[dirT   *size_Mat];
				D.f[dirT   ] = &DDStart[dirB   *size_Mat];
				D.f[dirSW  ] = &DDStart[dirNE  *size_Mat];
				D.f[dirNE  ] = &DDStart[dirSW  *size_Mat];
				D.f[dirNW  ] = &DDStart[dirSE  *size_Mat];
				D.f[dirSE  ] = &DDStart[dirNW  *size_Mat];
				D.f[dirBW  ] = &DDStart[dirTE  *size_Mat];
				D.f[dirTE  ] = &DDStart[dirBW  *size_Mat];
				D.f[dirTW  ] = &DDStart[dirBE  *size_Mat];
				D.f[dirBE  ] = &DDStart[dirTW  *size_Mat];
				D.f[dirBS  ] = &DDStart[dirTN  *size_Mat];
				D.f[dirTN  ] = &DDStart[dirBS  *size_Mat];
				D.f[dirTS  ] = &DDStart[dirBN  *size_Mat];
				D.f[dirBN  ] = &DDStart[dirTS  *size_Mat];
				D.f[dirZERO] = &DDStart[dirZERO*size_Mat];
				D.f[dirBSW ] = &DDStart[dirTNE *size_Mat];
				D.f[dirBNE ] = &DDStart[dirTSW *size_Mat];
				D.f[dirBNW ] = &DDStart[dirTSE *size_Mat];
				D.f[dirBSE ] = &DDStart[dirTNW *size_Mat];
				D.f[dirTSW ] = &DDStart[dirBNE *size_Mat];
				D.f[dirTNE ] = &DDStart[dirBSW *size_Mat];
				D.f[dirTNW ] = &DDStart[dirBSE *size_Mat];
				D.f[dirTSE ] = &DDStart[dirBNW *size_Mat];
			}

			////////////////////////////////////////////////////////////////////////////////
			//index
			unsigned int kw   = neighborX[k];
			unsigned int ks   = neighborY[k];
			unsigned int kb   = neighborZ[k];
			unsigned int ksw  = neighborY[kw];
			unsigned int kbw  = neighborZ[kw];
			unsigned int kbs  = neighborZ[ks];
			unsigned int kbsw = neighborZ[ksw];
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			doubflo mfcbb = (D.f[dirE   ])[k  ];
			doubflo mfabb = (D.f[dirW   ])[kw ];
			doubflo mfbcb = (D.f[dirN   ])[k  ];
			doubflo mfbab = (D.f[dirS   ])[ks ];
			doubflo mfbbc = (D.f[dirT   ])[k  ];
			doubflo mfbba = (D.f[dirB   ])[kb ];
			doubflo mfccb = (D.f[dirNE  ])[k  ];
			doubflo mfaab = (D.f[dirSW  ])[ksw];
			doubflo mfcab = (D.f[dirSE  ])[ks ];
			doubflo mfacb = (D.f[dirNW  ])[kw ];
			doubflo mfcbc = (D.f[dirTE  ])[k  ];
			doubflo mfaba = (D.f[dirBW  ])[kbw];
			doubflo mfcba = (D.f[dirBE  ])[kb ];
			doubflo mfabc = (D.f[dirTW  ])[kw ];
			doubflo mfbcc = (D.f[dirTN  ])[k  ];
			doubflo mfbaa = (D.f[dirBS  ])[kbs];
			doubflo mfbca = (D.f[dirBN  ])[kb ];
			doubflo mfbac = (D.f[dirTS  ])[ks ];
			doubflo mfbbb = (D.f[dirZERO])[k  ];
			doubflo mfccc = (D.f[dirTNE ])[k  ];
			doubflo mfaac = (D.f[dirTSW ])[ksw];
			doubflo mfcac = (D.f[dirTSE ])[ks ];
			doubflo mfacc = (D.f[dirTNW ])[kw ];
			doubflo mfcca = (D.f[dirBNE ])[kb ];
			doubflo mfaaa = (D.f[dirBSW ])[kbsw];
			doubflo mfcaa = (D.f[dirBSE ])[kbs];
			doubflo mfaca = (D.f[dirBNW ])[kbw];
			////////////////////////////////////////////////////////////////////////////////////
			doubflo vvx    =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfcaa-mfacc) + (mfcca-mfaac))) + 
						     (((mfcba-mfabc) + (mfcbc-mfaba)) + ((mfcab-mfacb) + (mfccb-mfaab))) +
						       (mfcbb-mfabb));
			doubflo vvy    =((((mfccc-mfaaa) + (mfaca-mfcac)) + ((mfacc-mfcaa) + (mfcca-mfaac))) + 
				             (((mfbca-mfbac) + (mfbcc-mfbaa)) + ((mfacb-mfcab) + (mfccb-mfaab))) +
				               (mfbcb-mfbab));
			doubflo vvz    =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfacc-mfcaa) + (mfaac-mfcca))) + 
				             (((mfbac-mfbca) + (mfbcc-mfbaa)) + ((mfabc-mfcba) + (mfcbc-mfaba))) +
				               (mfbbc-mfbba));
			////////////////////////////////////////////////////////////////////////////////////
			doubflo oMdrho = one - (mfccc+mfaaa + mfaca+mfcac + mfacc+mfcaa + mfaac+mfcca + 
								   mfbac+mfbca + mfbaa+mfbcc + mfabc+mfcba + mfaba+mfcbc + mfacb+mfcab + mfaab+mfccb +
								   mfabb+mfcbb + mfbab+mfbcb + mfbba+mfbbc + mfbbb);
			////////////////////////////////////////////////////////////////////////////////////
			doubflo m0, m1, m2;	
			doubflo vx2;
			doubflo vy2;
			doubflo vz2;
			vx2=vvx*vvx;
			vy2=vvy*vvy;
			vz2=vvz*vvz;
			////////////////////////////////////////////////////////////////////////////////////
			//Hin
			////////////////////////////////////////////////////////////////////////////////////
			// mit 1/36, 1/9, 1/36, 1/9, 4/9, 1/9, 1/36, 1/9, 1/36  Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// Z - Dir
			m2    = mfaaa	+ mfaac;
			m1    = mfaac	- mfaaa;
			m0    = m2		+ mfaab;
			mfaaa = m0;
			m0   += c1o36 * oMdrho;	
			mfaab = m1 -		m0 * vvz;
			mfaac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaba  + mfabc;
			m1    = mfabc  - mfaba;
			m0    = m2		+ mfabb;
			mfaba = m0;
			m0   += c1o9 * oMdrho;
			mfabb = m1 -		m0 * vvz;
			mfabc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaca  + mfacc;
			m1    = mfacc  - mfaca;
			m0    = m2		+ mfacb;
			mfaca = m0;
			m0   += c1o36 * oMdrho;
			mfacb = m1 -		m0 * vvz;
			mfacc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbaa	+ mfbac;
			m1    = mfbac	- mfbaa;
			m0    = m2		+ mfbab;
			mfbaa = m0;
			m0   += c1o9 * oMdrho;
			mfbab = m1 -		m0 * vvz;
			mfbac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbba  + mfbbc;
			m1    = mfbbc  - mfbba;
			m0    = m2		+ mfbbb;
			mfbba = m0;
			m0   += c4o9 * oMdrho;
			mfbbb = m1 -		m0 * vvz;
			mfbbc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbca  + mfbcc;
			m1    = mfbcc  - mfbca;
			m0    = m2		+ mfbcb;
			mfbca = m0;
			m0   += c1o9 * oMdrho;
			mfbcb = m1 -		m0 * vvz;
			mfbcc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcaa	+ mfcac;
			m1    = mfcac	- mfcaa;
			m0    = m2		+ mfcab;
			mfcaa = m0;
			m0   += c1o36 * oMdrho;
			mfcab = m1 -		m0 * vvz;
			mfcac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcba  + mfcbc;
			m1    = mfcbc  - mfcba;
			m0    = m2		+ mfcbb;
			mfcba = m0;
			m0   += c1o9 * oMdrho;
			mfcbb = m1 -		m0 * vvz;
			mfcbc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcca  + mfccc;
			m1    = mfccc  - mfcca;
			m0    = m2		+ mfccb;
			mfcca = m0;
			m0   += c1o36 * oMdrho;
			mfccb = m1 -		m0 * vvz;
			mfccc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// mit  1/6, 0, 1/18, 2/3, 0, 2/9, 1/6, 0, 1/18 Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// Y - Dir
			m2    = mfaaa	+ mfaca;
			m1    = mfaca	- mfaaa;
			m0    = m2		+ mfaba;
			mfaaa = m0;
			m0   += c1o6 * oMdrho;
			mfaba = m1 -		m0 * vvy;
			mfaca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaab  + mfacb;
			m1    = mfacb  - mfaab;
			m0    = m2		+ mfabb;
			mfaab = m0;
			mfabb = m1 -		m0 * vvy;
			mfacb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaac  + mfacc;
			m1    = mfacc  - mfaac;
			m0    = m2		+ mfabc;
			mfaac = m0;
			m0   += c1o18 * oMdrho;
			mfabc = m1 -		m0 * vvy;
			mfacc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbaa	+ mfbca;
			m1    = mfbca	- mfbaa;
			m0    = m2		+ mfbba;
			mfbaa = m0;
			m0   += c2o3 * oMdrho;
			mfbba = m1 -		m0 * vvy;
			mfbca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbab  + mfbcb;
			m1    = mfbcb  - mfbab;
			m0    = m2		+ mfbbb;
			mfbab = m0;
			mfbbb = m1 -		m0 * vvy;
			mfbcb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbac  + mfbcc;
			m1    = mfbcc  - mfbac;
			m0    = m2		+ mfbbc;
			mfbac = m0;
			m0   += c2o9 * oMdrho;
			mfbbc = m1 -		m0 * vvy;
			mfbcc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcaa	+ mfcca;
			m1    = mfcca	- mfcaa;
			m0    = m2		+ mfcba;
			mfcaa = m0;
			m0   += c1o6 * oMdrho;
			mfcba = m1 -		m0 * vvy;
			mfcca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcab  + mfccb;
			m1    = mfccb  - mfcab;
			m0    = m2		+ mfcbb;
			mfcab = m0;
			mfcbb = m1 -		m0 * vvy;
			mfccb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcac  + mfccc;
			m1    = mfccc  - mfcac;
			m0    = m2		+ mfcbc;
			mfcac = m0;
			m0   += c1o18 * oMdrho;
			mfcbc = m1 -		m0 * vvy;
			mfccc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// mit     1, 0, 1/3, 0, 0, 0, 1/3, 0, 1/9		Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// X - Dir
			m2    = mfaaa	+ mfcaa;
			m1    = mfcaa	- mfaaa;
			m0    = m2		+ mfbaa;
			mfaaa = m0;
			m0   += one* oMdrho;
			mfbaa = m1 -		m0 * vvx;
			mfcaa = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaba  + mfcba;
			m1    = mfcba  - mfaba;
			m0    = m2		+ mfbba;
			mfaba = m0;
			mfbba = m1 -		m0 * vvx;
			mfcba = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaca  + mfcca;
			m1    = mfcca  - mfaca;
			m0    = m2		+ mfbca;
			mfaca = m0;
			m0   += c1o3 * oMdrho;
			mfbca = m1 -		m0 * vvx;
			mfcca = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaab	+ mfcab;
			m1    = mfcab	- mfaab;
			m0    = m2		+ mfbab;
			mfaab = m0;
			mfbab = m1 -		m0 * vvx;
			mfcab = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfabb  + mfcbb;
			m1    = mfcbb  - mfabb;
			m0    = m2		+ mfbbb;
			mfabb = m0;
			mfbbb = m1 -		m0 * vvx;
			mfcbb = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfacb  + mfccb;
			m1    = mfccb  - mfacb;
			m0    = m2		+ mfbcb;
			mfacb = m0;
			mfbcb = m1 -		m0 * vvx;
			mfccb = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaac	+ mfcac;
			m1    = mfcac	- mfaac;
			m0    = m2		+ mfbac;
			mfaac = m0;
			m0   += c1o3 * oMdrho;
			mfbac = m1 -		m0 * vvx;
			mfcac = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfabc  + mfcbc;
			m1    = mfcbc  - mfabc;
			m0    = m2		+ mfbbc;
			mfabc = m0;
			mfbbc = m1 -		m0 * vvx;
			mfcbc = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfacc  + mfccc;
			m1    = mfccc  - mfacc;
			m0    = m2		+ mfbcc;
			mfacc = m0;
			m0   += c1o9 * oMdrho;
			mfbcc = m1 -		m0 * vvx;
			mfccc = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////


			////////////////////////////////////////////////////////////////////////////////////
			// Cumulants
			////////////////////////////////////////////////////////////////////////////////////
			//3.
			CUMbbb[k] = mfbbb;
			CUMabc[k] = mfabc;
			CUMbac[k] = mfbac;
			CUMbca[k] = mfbca;
			CUMcba[k] = mfcba;
			CUMacb[k] = mfacb;
			CUMcab[k] = mfcab;
			////////////////////////////////////////////////////////////////////////////////////
		}                                                                                                                    
	}
}
////////////////////////////////////////////////////////////////////////////////































////////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void LBCalc3rdMomentsCompSP27(doubflo* CUMbbb,
													doubflo* CUMabc,
													doubflo* CUMbac,
													doubflo* CUMbca,
													doubflo* CUMcba,
													doubflo* CUMacb,
													doubflo* CUMcab,
													unsigned int* bcMatD,
													unsigned int* neighborX,
													unsigned int* neighborY,
													unsigned int* neighborZ,
													doubflo* DDStart,
													int size_Mat,
													bool EvenOrOdd)
{
	////////////////////////////////////////////////////////////////////////////////
	const unsigned  x = threadIdx.x;  // Globaler x-Index 
	const unsigned  y = blockIdx.x;   // Globaler y-Index 
	const unsigned  z = blockIdx.y;   // Globaler z-Index 

	const unsigned nx = blockDim.x;
	const unsigned ny = gridDim.x;

	const unsigned k = nx*(ny*z + y) + x;
	//////////////////////////////////////////////////////////////////////////

	if(k<size_Mat)
	{
		////////////////////////////////////////////////////////////////////////////////
		unsigned int BC;
		BC = bcMatD[k];

		if( (BC != GEO_SOLID) && (BC != GEO_VOID) )
		{
			Distributions27 D;
			if (EvenOrOdd==true)
			{
				D.f[dirE   ] = &DDStart[dirE   *size_Mat];
				D.f[dirW   ] = &DDStart[dirW   *size_Mat];
				D.f[dirN   ] = &DDStart[dirN   *size_Mat];
				D.f[dirS   ] = &DDStart[dirS   *size_Mat];
				D.f[dirT   ] = &DDStart[dirT   *size_Mat];
				D.f[dirB   ] = &DDStart[dirB   *size_Mat];
				D.f[dirNE  ] = &DDStart[dirNE  *size_Mat];
				D.f[dirSW  ] = &DDStart[dirSW  *size_Mat];
				D.f[dirSE  ] = &DDStart[dirSE  *size_Mat];
				D.f[dirNW  ] = &DDStart[dirNW  *size_Mat];
				D.f[dirTE  ] = &DDStart[dirTE  *size_Mat];
				D.f[dirBW  ] = &DDStart[dirBW  *size_Mat];
				D.f[dirBE  ] = &DDStart[dirBE  *size_Mat];
				D.f[dirTW  ] = &DDStart[dirTW  *size_Mat];
				D.f[dirTN  ] = &DDStart[dirTN  *size_Mat];
				D.f[dirBS  ] = &DDStart[dirBS  *size_Mat];
				D.f[dirBN  ] = &DDStart[dirBN  *size_Mat];
				D.f[dirTS  ] = &DDStart[dirTS  *size_Mat];
				D.f[dirZERO] = &DDStart[dirZERO*size_Mat];
				D.f[dirTNE ] = &DDStart[dirTNE *size_Mat];
				D.f[dirTSW ] = &DDStart[dirTSW *size_Mat];
				D.f[dirTSE ] = &DDStart[dirTSE *size_Mat];
				D.f[dirTNW ] = &DDStart[dirTNW *size_Mat];
				D.f[dirBNE ] = &DDStart[dirBNE *size_Mat];
				D.f[dirBSW ] = &DDStart[dirBSW *size_Mat];
				D.f[dirBSE ] = &DDStart[dirBSE *size_Mat];
				D.f[dirBNW ] = &DDStart[dirBNW *size_Mat];
			}
			else
			{
				D.f[dirW   ] = &DDStart[dirE   *size_Mat];
				D.f[dirE   ] = &DDStart[dirW   *size_Mat];
				D.f[dirS   ] = &DDStart[dirN   *size_Mat];
				D.f[dirN   ] = &DDStart[dirS   *size_Mat];
				D.f[dirB   ] = &DDStart[dirT   *size_Mat];
				D.f[dirT   ] = &DDStart[dirB   *size_Mat];
				D.f[dirSW  ] = &DDStart[dirNE  *size_Mat];
				D.f[dirNE  ] = &DDStart[dirSW  *size_Mat];
				D.f[dirNW  ] = &DDStart[dirSE  *size_Mat];
				D.f[dirSE  ] = &DDStart[dirNW  *size_Mat];
				D.f[dirBW  ] = &DDStart[dirTE  *size_Mat];
				D.f[dirTE  ] = &DDStart[dirBW  *size_Mat];
				D.f[dirTW  ] = &DDStart[dirBE  *size_Mat];
				D.f[dirBE  ] = &DDStart[dirTW  *size_Mat];
				D.f[dirBS  ] = &DDStart[dirTN  *size_Mat];
				D.f[dirTN  ] = &DDStart[dirBS  *size_Mat];
				D.f[dirTS  ] = &DDStart[dirBN  *size_Mat];
				D.f[dirBN  ] = &DDStart[dirTS  *size_Mat];
				D.f[dirZERO] = &DDStart[dirZERO*size_Mat];
				D.f[dirBSW ] = &DDStart[dirTNE *size_Mat];
				D.f[dirBNE ] = &DDStart[dirTSW *size_Mat];
				D.f[dirBNW ] = &DDStart[dirTSE *size_Mat];
				D.f[dirBSE ] = &DDStart[dirTNW *size_Mat];
				D.f[dirTSW ] = &DDStart[dirBNE *size_Mat];
				D.f[dirTNE ] = &DDStart[dirBSW *size_Mat];
				D.f[dirTNW ] = &DDStart[dirBSE *size_Mat];
				D.f[dirTSE ] = &DDStart[dirBNW *size_Mat];
			}

			////////////////////////////////////////////////////////////////////////////////
			//index
			unsigned int kw   = neighborX[k];
			unsigned int ks   = neighborY[k];
			unsigned int kb   = neighborZ[k];
			unsigned int ksw  = neighborY[kw];
			unsigned int kbw  = neighborZ[kw];
			unsigned int kbs  = neighborZ[ks];
			unsigned int kbsw = neighborZ[ksw];
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			doubflo mfcbb = (D.f[dirE   ])[k  ];
			doubflo mfabb = (D.f[dirW   ])[kw ];
			doubflo mfbcb = (D.f[dirN   ])[k  ];
			doubflo mfbab = (D.f[dirS   ])[ks ];
			doubflo mfbbc = (D.f[dirT   ])[k  ];
			doubflo mfbba = (D.f[dirB   ])[kb ];
			doubflo mfccb = (D.f[dirNE  ])[k  ];
			doubflo mfaab = (D.f[dirSW  ])[ksw];
			doubflo mfcab = (D.f[dirSE  ])[ks ];
			doubflo mfacb = (D.f[dirNW  ])[kw ];
			doubflo mfcbc = (D.f[dirTE  ])[k  ];
			doubflo mfaba = (D.f[dirBW  ])[kbw];
			doubflo mfcba = (D.f[dirBE  ])[kb ];
			doubflo mfabc = (D.f[dirTW  ])[kw ];
			doubflo mfbcc = (D.f[dirTN  ])[k  ];
			doubflo mfbaa = (D.f[dirBS  ])[kbs];
			doubflo mfbca = (D.f[dirBN  ])[kb ];
			doubflo mfbac = (D.f[dirTS  ])[ks ];
			doubflo mfbbb = (D.f[dirZERO])[k  ];
			doubflo mfccc = (D.f[dirTNE ])[k  ];
			doubflo mfaac = (D.f[dirTSW ])[ksw];
			doubflo mfcac = (D.f[dirTSE ])[ks ];
			doubflo mfacc = (D.f[dirTNW ])[kw ];
			doubflo mfcca = (D.f[dirBNE ])[kb ];
			doubflo mfaaa = (D.f[dirBSW ])[kbsw];
			doubflo mfcaa = (D.f[dirBSE ])[kbs];
			doubflo mfaca = (D.f[dirBNW ])[kbw];
			////////////////////////////////////////////////////////////////////////////////////
			doubflo drho = ((((mfccc+mfaaa) + (mfaca+mfcac)) + ((mfacc+mfcaa) + (mfaac+mfcca))) + 
							(((mfbac+mfbca) + (mfbaa+mfbcc)) + ((mfabc+mfcba) + (mfaba+mfcbc)) + ((mfacb+mfcab) + (mfaab+mfccb))) +
							((mfabb+mfcbb) + (mfbab+mfbcb)) + (mfbba+mfbbc)) + mfbbb;

			doubflo rho = one+drho;
			////////////////////////////////////////////////////////////////////////////////////
			doubflo vvx    =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfcaa-mfacc) + (mfcca-mfaac))) + 
						     (((mfcba-mfabc) + (mfcbc-mfaba)) + ((mfcab-mfacb) + (mfccb-mfaab))) +
						       (mfcbb-mfabb)) / rho;
			doubflo vvy    =((((mfccc-mfaaa) + (mfaca-mfcac)) + ((mfacc-mfcaa) + (mfcca-mfaac))) + 
				             (((mfbca-mfbac) + (mfbcc-mfbaa)) + ((mfacb-mfcab) + (mfccb-mfaab))) +
				               (mfbcb-mfbab)) / rho;
			doubflo vvz    =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfacc-mfcaa) + (mfaac-mfcca))) + 
				             (((mfbac-mfbca) + (mfbcc-mfbaa)) + ((mfabc-mfcba) + (mfcbc-mfaba))) +
				               (mfbbc-mfbba)) / rho;
			////////////////////////////////////////////////////////////////////////////////////
			doubflo oMdrho = one; // comp special
			////////////////////////////////////////////////////////////////////////////////////
			doubflo m0, m1, m2;	
			doubflo vx2;
			doubflo vy2;
			doubflo vz2;
			vx2=vvx*vvx;
			vy2=vvy*vvy;
			vz2=vvz*vvz;
			////////////////////////////////////////////////////////////////////////////////////
			//Hin
			////////////////////////////////////////////////////////////////////////////////////
			// mit 1/36, 1/9, 1/36, 1/9, 4/9, 1/9, 1/36, 1/9, 1/36  Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// Z - Dir
			m2    = mfaaa	+ mfaac;
			m1    = mfaac	- mfaaa;
			m0    = m2		+ mfaab;
			mfaaa = m0;
			m0   += c1o36 * oMdrho;	
			mfaab = m1 -		m0 * vvz;
			mfaac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaba  + mfabc;
			m1    = mfabc  - mfaba;
			m0    = m2		+ mfabb;
			mfaba = m0;
			m0   += c1o9 * oMdrho;
			mfabb = m1 -		m0 * vvz;
			mfabc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaca  + mfacc;
			m1    = mfacc  - mfaca;
			m0    = m2		+ mfacb;
			mfaca = m0;
			m0   += c1o36 * oMdrho;
			mfacb = m1 -		m0 * vvz;
			mfacc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbaa	+ mfbac;
			m1    = mfbac	- mfbaa;
			m0    = m2		+ mfbab;
			mfbaa = m0;
			m0   += c1o9 * oMdrho;
			mfbab = m1 -		m0 * vvz;
			mfbac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbba  + mfbbc;
			m1    = mfbbc  - mfbba;
			m0    = m2		+ mfbbb;
			mfbba = m0;
			m0   += c4o9 * oMdrho;
			mfbbb = m1 -		m0 * vvz;
			mfbbc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbca  + mfbcc;
			m1    = mfbcc  - mfbca;
			m0    = m2		+ mfbcb;
			mfbca = m0;
			m0   += c1o9 * oMdrho;
			mfbcb = m1 -		m0 * vvz;
			mfbcc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcaa	+ mfcac;
			m1    = mfcac	- mfcaa;
			m0    = m2		+ mfcab;
			mfcaa = m0;
			m0   += c1o36 * oMdrho;
			mfcab = m1 -		m0 * vvz;
			mfcac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcba  + mfcbc;
			m1    = mfcbc  - mfcba;
			m0    = m2		+ mfcbb;
			mfcba = m0;
			m0   += c1o9 * oMdrho;
			mfcbb = m1 -		m0 * vvz;
			mfcbc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcca  + mfccc;
			m1    = mfccc  - mfcca;
			m0    = m2		+ mfccb;
			mfcca = m0;
			m0   += c1o36 * oMdrho;
			mfccb = m1 -		m0 * vvz;
			mfccc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// mit  1/6, 0, 1/18, 2/3, 0, 2/9, 1/6, 0, 1/18 Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// Y - Dir
			m2    = mfaaa	+ mfaca;
			m1    = mfaca	- mfaaa;
			m0    = m2		+ mfaba;
			mfaaa = m0;
			m0   += c1o6 * oMdrho;
			mfaba = m1 -		m0 * vvy;
			mfaca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaab  + mfacb;
			m1    = mfacb  - mfaab;
			m0    = m2		+ mfabb;
			mfaab = m0;
			mfabb = m1 -		m0 * vvy;
			mfacb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaac  + mfacc;
			m1    = mfacc  - mfaac;
			m0    = m2		+ mfabc;
			mfaac = m0;
			m0   += c1o18 * oMdrho;
			mfabc = m1 -		m0 * vvy;
			mfacc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbaa	+ mfbca;
			m1    = mfbca	- mfbaa;
			m0    = m2		+ mfbba;
			mfbaa = m0;
			m0   += c2o3 * oMdrho;
			mfbba = m1 -		m0 * vvy;
			mfbca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbab  + mfbcb;
			m1    = mfbcb  - mfbab;
			m0    = m2		+ mfbbb;
			mfbab = m0;
			mfbbb = m1 -		m0 * vvy;
			mfbcb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbac  + mfbcc;
			m1    = mfbcc  - mfbac;
			m0    = m2		+ mfbbc;
			mfbac = m0;
			m0   += c2o9 * oMdrho;
			mfbbc = m1 -		m0 * vvy;
			mfbcc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcaa	+ mfcca;
			m1    = mfcca	- mfcaa;
			m0    = m2		+ mfcba;
			mfcaa = m0;
			m0   += c1o6 * oMdrho;
			mfcba = m1 -		m0 * vvy;
			mfcca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcab  + mfccb;
			m1    = mfccb  - mfcab;
			m0    = m2		+ mfcbb;
			mfcab = m0;
			mfcbb = m1 -		m0 * vvy;
			mfccb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcac  + mfccc;
			m1    = mfccc  - mfcac;
			m0    = m2		+ mfcbc;
			mfcac = m0;
			m0   += c1o18 * oMdrho;
			mfcbc = m1 -		m0 * vvy;
			mfccc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// mit     1, 0, 1/3, 0, 0, 0, 1/3, 0, 1/9		Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// X - Dir
			m2    = mfaaa	+ mfcaa;
			m1    = mfcaa	- mfaaa;
			m0    = m2		+ mfbaa;
			mfaaa = m0;
			m0   += one* oMdrho;
			mfbaa = m1 -		m0 * vvx;
			mfcaa = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaba  + mfcba;
			m1    = mfcba  - mfaba;
			m0    = m2		+ mfbba;
			mfaba = m0;
			mfbba = m1 -		m0 * vvx;
			mfcba = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaca  + mfcca;
			m1    = mfcca  - mfaca;
			m0    = m2		+ mfbca;
			mfaca = m0;
			m0   += c1o3 * oMdrho;
			mfbca = m1 -		m0 * vvx;
			mfcca = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaab	+ mfcab;
			m1    = mfcab	- mfaab;
			m0    = m2		+ mfbab;
			mfaab = m0;
			mfbab = m1 -		m0 * vvx;
			mfcab = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfabb  + mfcbb;
			m1    = mfcbb  - mfabb;
			m0    = m2		+ mfbbb;
			mfabb = m0;
			mfbbb = m1 -		m0 * vvx;
			mfcbb = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfacb  + mfccb;
			m1    = mfccb  - mfacb;
			m0    = m2		+ mfbcb;
			mfacb = m0;
			mfbcb = m1 -		m0 * vvx;
			mfccb = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaac	+ mfcac;
			m1    = mfcac	- mfaac;
			m0    = m2		+ mfbac;
			mfaac = m0;
			m0   += c1o3 * oMdrho;
			mfbac = m1 -		m0 * vvx;
			mfcac = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfabc  + mfcbc;
			m1    = mfcbc  - mfabc;
			m0    = m2		+ mfbbc;
			mfabc = m0;
			mfbbc = m1 -		m0 * vvx;
			mfcbc = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfacc  + mfccc;
			m1    = mfccc  - mfacc;
			m0    = m2		+ mfbcc;
			mfacc = m0;
			m0   += c1o9 * oMdrho;
			mfbcc = m1 -		m0 * vvx;
			mfccc = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////


			////////////////////////////////////////////////////////////////////////////////////
			// Cumulants
			////////////////////////////////////////////////////////////////////////////////////
			//3.
			CUMbbb[k] = mfbbb;
			CUMabc[k] = mfabc;
			CUMbac[k] = mfbac;
			CUMbca[k] = mfbca;
			CUMcba[k] = mfcba;
			CUMacb[k] = mfacb;
			CUMcab[k] = mfcab;
			////////////////////////////////////////////////////////////////////////////////////
		}                                                                                                                    
	}
}
////////////////////////////////////////////////////////////////////////////////































////////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void LBCalcHigherMomentsIncompSP27(   doubflo* CUMcbb,
															doubflo* CUMbcb,
															doubflo* CUMbbc,
															doubflo* CUMcca,
															doubflo* CUMcac,
															doubflo* CUMacc,
															doubflo* CUMbcc,
															doubflo* CUMcbc,
															doubflo* CUMccb,
															doubflo* CUMccc,
															unsigned int* bcMatD,
															unsigned int* neighborX,
															unsigned int* neighborY,
															unsigned int* neighborZ,
															doubflo* DDStart,
															int size_Mat,
															bool EvenOrOdd)
{
	////////////////////////////////////////////////////////////////////////////////
	const unsigned  x = threadIdx.x;  // Globaler x-Index 
	const unsigned  y = blockIdx.x;   // Globaler y-Index 
	const unsigned  z = blockIdx.y;   // Globaler z-Index 

	const unsigned nx = blockDim.x;
	const unsigned ny = gridDim.x;

	const unsigned k = nx*(ny*z + y) + x;
	//////////////////////////////////////////////////////////////////////////

	if(k<size_Mat)
	{
		////////////////////////////////////////////////////////////////////////////////
		unsigned int BC;
		BC = bcMatD[k];

		if( (BC != GEO_SOLID) && (BC != GEO_VOID) )
		{
			Distributions27 D;
			if (EvenOrOdd==true)
			{
				D.f[dirE   ] = &DDStart[dirE   *size_Mat];
				D.f[dirW   ] = &DDStart[dirW   *size_Mat];
				D.f[dirN   ] = &DDStart[dirN   *size_Mat];
				D.f[dirS   ] = &DDStart[dirS   *size_Mat];
				D.f[dirT   ] = &DDStart[dirT   *size_Mat];
				D.f[dirB   ] = &DDStart[dirB   *size_Mat];
				D.f[dirNE  ] = &DDStart[dirNE  *size_Mat];
				D.f[dirSW  ] = &DDStart[dirSW  *size_Mat];
				D.f[dirSE  ] = &DDStart[dirSE  *size_Mat];
				D.f[dirNW  ] = &DDStart[dirNW  *size_Mat];
				D.f[dirTE  ] = &DDStart[dirTE  *size_Mat];
				D.f[dirBW  ] = &DDStart[dirBW  *size_Mat];
				D.f[dirBE  ] = &DDStart[dirBE  *size_Mat];
				D.f[dirTW  ] = &DDStart[dirTW  *size_Mat];
				D.f[dirTN  ] = &DDStart[dirTN  *size_Mat];
				D.f[dirBS  ] = &DDStart[dirBS  *size_Mat];
				D.f[dirBN  ] = &DDStart[dirBN  *size_Mat];
				D.f[dirTS  ] = &DDStart[dirTS  *size_Mat];
				D.f[dirZERO] = &DDStart[dirZERO*size_Mat];
				D.f[dirTNE ] = &DDStart[dirTNE *size_Mat];
				D.f[dirTSW ] = &DDStart[dirTSW *size_Mat];
				D.f[dirTSE ] = &DDStart[dirTSE *size_Mat];
				D.f[dirTNW ] = &DDStart[dirTNW *size_Mat];
				D.f[dirBNE ] = &DDStart[dirBNE *size_Mat];
				D.f[dirBSW ] = &DDStart[dirBSW *size_Mat];
				D.f[dirBSE ] = &DDStart[dirBSE *size_Mat];
				D.f[dirBNW ] = &DDStart[dirBNW *size_Mat];
			}
			else
			{
				D.f[dirW   ] = &DDStart[dirE   *size_Mat];
				D.f[dirE   ] = &DDStart[dirW   *size_Mat];
				D.f[dirS   ] = &DDStart[dirN   *size_Mat];
				D.f[dirN   ] = &DDStart[dirS   *size_Mat];
				D.f[dirB   ] = &DDStart[dirT   *size_Mat];
				D.f[dirT   ] = &DDStart[dirB   *size_Mat];
				D.f[dirSW  ] = &DDStart[dirNE  *size_Mat];
				D.f[dirNE  ] = &DDStart[dirSW  *size_Mat];
				D.f[dirNW  ] = &DDStart[dirSE  *size_Mat];
				D.f[dirSE  ] = &DDStart[dirNW  *size_Mat];
				D.f[dirBW  ] = &DDStart[dirTE  *size_Mat];
				D.f[dirTE  ] = &DDStart[dirBW  *size_Mat];
				D.f[dirTW  ] = &DDStart[dirBE  *size_Mat];
				D.f[dirBE  ] = &DDStart[dirTW  *size_Mat];
				D.f[dirBS  ] = &DDStart[dirTN  *size_Mat];
				D.f[dirTN  ] = &DDStart[dirBS  *size_Mat];
				D.f[dirTS  ] = &DDStart[dirBN  *size_Mat];
				D.f[dirBN  ] = &DDStart[dirTS  *size_Mat];
				D.f[dirZERO] = &DDStart[dirZERO*size_Mat];
				D.f[dirBSW ] = &DDStart[dirTNE *size_Mat];
				D.f[dirBNE ] = &DDStart[dirTSW *size_Mat];
				D.f[dirBNW ] = &DDStart[dirTSE *size_Mat];
				D.f[dirBSE ] = &DDStart[dirTNW *size_Mat];
				D.f[dirTSW ] = &DDStart[dirBNE *size_Mat];
				D.f[dirTNE ] = &DDStart[dirBSW *size_Mat];
				D.f[dirTNW ] = &DDStart[dirBSE *size_Mat];
				D.f[dirTSE ] = &DDStart[dirBNW *size_Mat];
			}

			////////////////////////////////////////////////////////////////////////////////
			//index
			unsigned int kw   = neighborX[k];
			unsigned int ks   = neighborY[k];
			unsigned int kb   = neighborZ[k];
			unsigned int ksw  = neighborY[kw];
			unsigned int kbw  = neighborZ[kw];
			unsigned int kbs  = neighborZ[ks];
			unsigned int kbsw = neighborZ[ksw];
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			doubflo mfcbb = (D.f[dirE   ])[k  ];
			doubflo mfabb = (D.f[dirW   ])[kw ];
			doubflo mfbcb = (D.f[dirN   ])[k  ];
			doubflo mfbab = (D.f[dirS   ])[ks ];
			doubflo mfbbc = (D.f[dirT   ])[k  ];
			doubflo mfbba = (D.f[dirB   ])[kb ];
			doubflo mfccb = (D.f[dirNE  ])[k  ];
			doubflo mfaab = (D.f[dirSW  ])[ksw];
			doubflo mfcab = (D.f[dirSE  ])[ks ];
			doubflo mfacb = (D.f[dirNW  ])[kw ];
			doubflo mfcbc = (D.f[dirTE  ])[k  ];
			doubflo mfaba = (D.f[dirBW  ])[kbw];
			doubflo mfcba = (D.f[dirBE  ])[kb ];
			doubflo mfabc = (D.f[dirTW  ])[kw ];
			doubflo mfbcc = (D.f[dirTN  ])[k  ];
			doubflo mfbaa = (D.f[dirBS  ])[kbs];
			doubflo mfbca = (D.f[dirBN  ])[kb ];
			doubflo mfbac = (D.f[dirTS  ])[ks ];
			doubflo mfbbb = (D.f[dirZERO])[k  ];
			doubflo mfccc = (D.f[dirTNE ])[k  ];
			doubflo mfaac = (D.f[dirTSW ])[ksw];
			doubflo mfcac = (D.f[dirTSE ])[ks ];
			doubflo mfacc = (D.f[dirTNW ])[kw ];
			doubflo mfcca = (D.f[dirBNE ])[kb ];
			doubflo mfaaa = (D.f[dirBSW ])[kbsw];
			doubflo mfcaa = (D.f[dirBSE ])[kbs];
			doubflo mfaca = (D.f[dirBNW ])[kbw];
			////////////////////////////////////////////////////////////////////////////////////
			doubflo vvx    =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfcaa-mfacc) + (mfcca-mfaac))) + 
						     (((mfcba-mfabc) + (mfcbc-mfaba)) + ((mfcab-mfacb) + (mfccb-mfaab))) +
						       (mfcbb-mfabb));
			doubflo vvy    =((((mfccc-mfaaa) + (mfaca-mfcac)) + ((mfacc-mfcaa) + (mfcca-mfaac))) + 
				             (((mfbca-mfbac) + (mfbcc-mfbaa)) + ((mfacb-mfcab) + (mfccb-mfaab))) +
				               (mfbcb-mfbab));
			doubflo vvz    =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfacc-mfcaa) + (mfaac-mfcca))) + 
				             (((mfbac-mfbca) + (mfbcc-mfbaa)) + ((mfabc-mfcba) + (mfcbc-mfaba))) +
				               (mfbbc-mfbba));
			////////////////////////////////////////////////////////////////////////////////////
			doubflo oMdrho = one - (mfccc+mfaaa + mfaca+mfcac + mfacc+mfcaa + mfaac+mfcca + 
								   mfbac+mfbca + mfbaa+mfbcc + mfabc+mfcba + mfaba+mfcbc + mfacb+mfcab + mfaab+mfccb +
								   mfabb+mfcbb + mfbab+mfbcb + mfbba+mfbbc + mfbbb);
			////////////////////////////////////////////////////////////////////////////////////
			doubflo m0, m1, m2;	
			doubflo vx2;
			doubflo vy2;
			doubflo vz2;
			vx2=vvx*vvx;
			vy2=vvy*vvy;
			vz2=vvz*vvz;
			////////////////////////////////////////////////////////////////////////////////////
			//Hin
			////////////////////////////////////////////////////////////////////////////////////
			// mit 1/36, 1/9, 1/36, 1/9, 4/9, 1/9, 1/36, 1/9, 1/36  Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// Z - Dir
			m2    = mfaaa	+ mfaac;
			m1    = mfaac	- mfaaa;
			m0    = m2		+ mfaab;
			mfaaa = m0;
			m0   += c1o36 * oMdrho;	
			mfaab = m1 -		m0 * vvz;
			mfaac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaba  + mfabc;
			m1    = mfabc  - mfaba;
			m0    = m2		+ mfabb;
			mfaba = m0;
			m0   += c1o9 * oMdrho;
			mfabb = m1 -		m0 * vvz;
			mfabc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaca  + mfacc;
			m1    = mfacc  - mfaca;
			m0    = m2		+ mfacb;
			mfaca = m0;
			m0   += c1o36 * oMdrho;
			mfacb = m1 -		m0 * vvz;
			mfacc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbaa	+ mfbac;
			m1    = mfbac	- mfbaa;
			m0    = m2		+ mfbab;
			mfbaa = m0;
			m0   += c1o9 * oMdrho;
			mfbab = m1 -		m0 * vvz;
			mfbac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbba  + mfbbc;
			m1    = mfbbc  - mfbba;
			m0    = m2		+ mfbbb;
			mfbba = m0;
			m0   += c4o9 * oMdrho;
			mfbbb = m1 -		m0 * vvz;
			mfbbc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbca  + mfbcc;
			m1    = mfbcc  - mfbca;
			m0    = m2		+ mfbcb;
			mfbca = m0;
			m0   += c1o9 * oMdrho;
			mfbcb = m1 -		m0 * vvz;
			mfbcc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcaa	+ mfcac;
			m1    = mfcac	- mfcaa;
			m0    = m2		+ mfcab;
			mfcaa = m0;
			m0   += c1o36 * oMdrho;
			mfcab = m1 -		m0 * vvz;
			mfcac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcba  + mfcbc;
			m1    = mfcbc  - mfcba;
			m0    = m2		+ mfcbb;
			mfcba = m0;
			m0   += c1o9 * oMdrho;
			mfcbb = m1 -		m0 * vvz;
			mfcbc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcca  + mfccc;
			m1    = mfccc  - mfcca;
			m0    = m2		+ mfccb;
			mfcca = m0;
			m0   += c1o36 * oMdrho;
			mfccb = m1 -		m0 * vvz;
			mfccc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// mit  1/6, 0, 1/18, 2/3, 0, 2/9, 1/6, 0, 1/18 Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// Y - Dir
			m2    = mfaaa	+ mfaca;
			m1    = mfaca	- mfaaa;
			m0    = m2		+ mfaba;
			mfaaa = m0;
			m0   += c1o6 * oMdrho;
			mfaba = m1 -		m0 * vvy;
			mfaca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaab  + mfacb;
			m1    = mfacb  - mfaab;
			m0    = m2		+ mfabb;
			mfaab = m0;
			mfabb = m1 -		m0 * vvy;
			mfacb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaac  + mfacc;
			m1    = mfacc  - mfaac;
			m0    = m2		+ mfabc;
			mfaac = m0;
			m0   += c1o18 * oMdrho;
			mfabc = m1 -		m0 * vvy;
			mfacc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbaa	+ mfbca;
			m1    = mfbca	- mfbaa;
			m0    = m2		+ mfbba;
			mfbaa = m0;
			m0   += c2o3 * oMdrho;
			mfbba = m1 -		m0 * vvy;
			mfbca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbab  + mfbcb;
			m1    = mfbcb  - mfbab;
			m0    = m2		+ mfbbb;
			mfbab = m0;
			mfbbb = m1 -		m0 * vvy;
			mfbcb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbac  + mfbcc;
			m1    = mfbcc  - mfbac;
			m0    = m2		+ mfbbc;
			mfbac = m0;
			m0   += c2o9 * oMdrho;
			mfbbc = m1 -		m0 * vvy;
			mfbcc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcaa	+ mfcca;
			m1    = mfcca	- mfcaa;
			m0    = m2		+ mfcba;
			mfcaa = m0;
			m0   += c1o6 * oMdrho;
			mfcba = m1 -		m0 * vvy;
			mfcca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcab  + mfccb;
			m1    = mfccb  - mfcab;
			m0    = m2		+ mfcbb;
			mfcab = m0;
			mfcbb = m1 -		m0 * vvy;
			mfccb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcac  + mfccc;
			m1    = mfccc  - mfcac;
			m0    = m2		+ mfcbc;
			mfcac = m0;
			m0   += c1o18 * oMdrho;
			mfcbc = m1 -		m0 * vvy;
			mfccc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// mit     1, 0, 1/3, 0, 0, 0, 1/3, 0, 1/9		Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// X - Dir
			m2    = mfaaa	+ mfcaa;
			m1    = mfcaa	- mfaaa;
			m0    = m2		+ mfbaa;
			mfaaa = m0;
			m0   += one* oMdrho;
			mfbaa = m1 -		m0 * vvx;
			mfcaa = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaba  + mfcba;
			m1    = mfcba  - mfaba;
			m0    = m2		+ mfbba;
			mfaba = m0;
			mfbba = m1 -		m0 * vvx;
			mfcba = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaca  + mfcca;
			m1    = mfcca  - mfaca;
			m0    = m2		+ mfbca;
			mfaca = m0;
			m0   += c1o3 * oMdrho;
			mfbca = m1 -		m0 * vvx;
			mfcca = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaab	+ mfcab;
			m1    = mfcab	- mfaab;
			m0    = m2		+ mfbab;
			mfaab = m0;
			mfbab = m1 -		m0 * vvx;
			mfcab = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfabb  + mfcbb;
			m1    = mfcbb  - mfabb;
			m0    = m2		+ mfbbb;
			mfabb = m0;
			mfbbb = m1 -		m0 * vvx;
			mfcbb = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfacb  + mfccb;
			m1    = mfccb  - mfacb;
			m0    = m2		+ mfbcb;
			mfacb = m0;
			mfbcb = m1 -		m0 * vvx;
			mfccb = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaac	+ mfcac;
			m1    = mfcac	- mfaac;
			m0    = m2		+ mfbac;
			mfaac = m0;
			m0   += c1o3 * oMdrho;
			mfbac = m1 -		m0 * vvx;
			mfcac = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfabc  + mfcbc;
			m1    = mfcbc  - mfabc;
			m0    = m2		+ mfbbc;
			mfabc = m0;
			mfbbc = m1 -		m0 * vvx;
			mfcbc = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfacc  + mfccc;
			m1    = mfccc  - mfacc;
			m0    = m2		+ mfbcc;
			mfacc = m0;
			m0   += c1o9 * oMdrho;
			mfbcc = m1 -		m0 * vvx;
			mfccc = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////


			////////////////////////////////////////////////////////////////////////////////////
			// Cumulants
			////////////////////////////////////////////////////////////////////////////////////
			//Cum 4.
			CUMcbb[k]      = mfcbb - ((mfcaa + c1o3 * oMdrho) * mfabb + two * mfbba * mfbab); 
			CUMbcb[k]      = mfbcb - ((mfaca + c1o3 * oMdrho) * mfbab + two * mfbba * mfabb); 
			CUMbbc[k]      = mfbbc - ((mfaac + c1o3 * oMdrho) * mfbba + two * mfbab * mfabb); 

			CUMcca[k]      = mfcca - ((mfcaa * mfaca + two * mfbba * mfbba) + c1o3 * (mfcaa + mfaca) * oMdrho + c1o9*(oMdrho-one)*oMdrho);
			CUMcac[k]      = mfcac - ((mfcaa * mfaac + two * mfbab * mfbab) + c1o3 * (mfcaa + mfaac) * oMdrho + c1o9*(oMdrho-one)*oMdrho);
			CUMacc[k]      = mfacc - ((mfaac * mfaca + two * mfabb * mfabb) + c1o3 * (mfaac + mfaca) * oMdrho + c1o9*(oMdrho-one)*oMdrho);

			//Cum 5.
			CUMbcc[k]      = mfbcc - (mfaac * mfbca + mfaca * mfbac + four * mfabb * mfbbb + two * (mfbab * mfacb + mfbba * mfabc)) - c1o3 * (mfbca + mfbac) * oMdrho;
			CUMcbc[k]      = mfcbc - (mfaac * mfcba + mfcaa * mfabc + four * mfbab * mfbbb + two * (mfabb * mfcab + mfbba * mfbac)) - c1o3 * (mfcba + mfabc) * oMdrho;
			CUMccb[k]      = mfccb - (mfcaa * mfacb + mfaca * mfcab + four * mfbba * mfbbb + two * (mfbab * mfbca + mfabb * mfcba)) - c1o3 * (mfacb + mfcab) * oMdrho;

			//Cum 6.
			CUMccc[k]      = mfccc  +((-four *  mfbbb * mfbbb  
							-           (mfcaa * mfacc + mfaca * mfcac + mfaac * mfcca)
							-    four * (mfabb * mfcbb + mfbab * mfbcb + mfbba * mfbbc)
							-     two * (mfbca * mfbac + mfcba * mfabc + mfcab * mfacb))
							+(   four * (mfbab * mfbab * mfaca + mfabb * mfabb * mfcaa + mfbba * mfbba * mfaac)
							+     two * (mfcaa * mfaca * mfaac)
							+ sixteen *  mfbba * mfbab * mfabb)
							-    c1o3 * (mfacc + mfcac + mfcca) * oMdrho  -c1o9*oMdrho*oMdrho
							-    c1o9 * (mfcaa + mfaca + mfaac) * oMdrho*(one-two* oMdrho)- c1o27* oMdrho * oMdrho*(-two* oMdrho)
							+(    two * (mfbab * mfbab + mfabb * mfabb + mfbba * mfbba)
							+           (mfaac * mfaca + mfaac * mfcaa + mfaca * mfcaa)) * c2o3*oMdrho) +c1o27*oMdrho;
			////////////////////////////////////////////////////////////////////////////////////
		}                                                                                                                    
	}
}































////////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void LBCalcHigherMomentsCompSP27( doubflo* CUMcbb,
														doubflo* CUMbcb,
														doubflo* CUMbbc,
														doubflo* CUMcca,
														doubflo* CUMcac,
														doubflo* CUMacc,
														doubflo* CUMbcc,
														doubflo* CUMcbc,
														doubflo* CUMccb,
														doubflo* CUMccc,
														unsigned int* bcMatD,
														unsigned int* neighborX,
														unsigned int* neighborY,
														unsigned int* neighborZ,
														doubflo* DDStart,
														int size_Mat,
														bool EvenOrOdd)
{
	////////////////////////////////////////////////////////////////////////////////
	const unsigned  x = threadIdx.x;  // Globaler x-Index 
	const unsigned  y = blockIdx.x;   // Globaler y-Index 
	const unsigned  z = blockIdx.y;   // Globaler z-Index 

	const unsigned nx = blockDim.x;
	const unsigned ny = gridDim.x;

	const unsigned k = nx*(ny*z + y) + x;
	//////////////////////////////////////////////////////////////////////////

	if(k<size_Mat)
	{
		////////////////////////////////////////////////////////////////////////////////
		unsigned int BC;
		BC = bcMatD[k];

		if( (BC != GEO_SOLID) && (BC != GEO_VOID) )
		{
			Distributions27 D;
			if (EvenOrOdd==true)
			{
				D.f[dirE   ] = &DDStart[dirE   *size_Mat];
				D.f[dirW   ] = &DDStart[dirW   *size_Mat];
				D.f[dirN   ] = &DDStart[dirN   *size_Mat];
				D.f[dirS   ] = &DDStart[dirS   *size_Mat];
				D.f[dirT   ] = &DDStart[dirT   *size_Mat];
				D.f[dirB   ] = &DDStart[dirB   *size_Mat];
				D.f[dirNE  ] = &DDStart[dirNE  *size_Mat];
				D.f[dirSW  ] = &DDStart[dirSW  *size_Mat];
				D.f[dirSE  ] = &DDStart[dirSE  *size_Mat];
				D.f[dirNW  ] = &DDStart[dirNW  *size_Mat];
				D.f[dirTE  ] = &DDStart[dirTE  *size_Mat];
				D.f[dirBW  ] = &DDStart[dirBW  *size_Mat];
				D.f[dirBE  ] = &DDStart[dirBE  *size_Mat];
				D.f[dirTW  ] = &DDStart[dirTW  *size_Mat];
				D.f[dirTN  ] = &DDStart[dirTN  *size_Mat];
				D.f[dirBS  ] = &DDStart[dirBS  *size_Mat];
				D.f[dirBN  ] = &DDStart[dirBN  *size_Mat];
				D.f[dirTS  ] = &DDStart[dirTS  *size_Mat];
				D.f[dirZERO] = &DDStart[dirZERO*size_Mat];
				D.f[dirTNE ] = &DDStart[dirTNE *size_Mat];
				D.f[dirTSW ] = &DDStart[dirTSW *size_Mat];
				D.f[dirTSE ] = &DDStart[dirTSE *size_Mat];
				D.f[dirTNW ] = &DDStart[dirTNW *size_Mat];
				D.f[dirBNE ] = &DDStart[dirBNE *size_Mat];
				D.f[dirBSW ] = &DDStart[dirBSW *size_Mat];
				D.f[dirBSE ] = &DDStart[dirBSE *size_Mat];
				D.f[dirBNW ] = &DDStart[dirBNW *size_Mat];
			}
			else
			{
				D.f[dirW   ] = &DDStart[dirE   *size_Mat];
				D.f[dirE   ] = &DDStart[dirW   *size_Mat];
				D.f[dirS   ] = &DDStart[dirN   *size_Mat];
				D.f[dirN   ] = &DDStart[dirS   *size_Mat];
				D.f[dirB   ] = &DDStart[dirT   *size_Mat];
				D.f[dirT   ] = &DDStart[dirB   *size_Mat];
				D.f[dirSW  ] = &DDStart[dirNE  *size_Mat];
				D.f[dirNE  ] = &DDStart[dirSW  *size_Mat];
				D.f[dirNW  ] = &DDStart[dirSE  *size_Mat];
				D.f[dirSE  ] = &DDStart[dirNW  *size_Mat];
				D.f[dirBW  ] = &DDStart[dirTE  *size_Mat];
				D.f[dirTE  ] = &DDStart[dirBW  *size_Mat];
				D.f[dirTW  ] = &DDStart[dirBE  *size_Mat];
				D.f[dirBE  ] = &DDStart[dirTW  *size_Mat];
				D.f[dirBS  ] = &DDStart[dirTN  *size_Mat];
				D.f[dirTN  ] = &DDStart[dirBS  *size_Mat];
				D.f[dirTS  ] = &DDStart[dirBN  *size_Mat];
				D.f[dirBN  ] = &DDStart[dirTS  *size_Mat];
				D.f[dirZERO] = &DDStart[dirZERO*size_Mat];
				D.f[dirBSW ] = &DDStart[dirTNE *size_Mat];
				D.f[dirBNE ] = &DDStart[dirTSW *size_Mat];
				D.f[dirBNW ] = &DDStart[dirTSE *size_Mat];
				D.f[dirBSE ] = &DDStart[dirTNW *size_Mat];
				D.f[dirTSW ] = &DDStart[dirBNE *size_Mat];
				D.f[dirTNE ] = &DDStart[dirBSW *size_Mat];
				D.f[dirTNW ] = &DDStart[dirBSE *size_Mat];
				D.f[dirTSE ] = &DDStart[dirBNW *size_Mat];
			}

			////////////////////////////////////////////////////////////////////////////////
			//index
			unsigned int kw   = neighborX[k];
			unsigned int ks   = neighborY[k];
			unsigned int kb   = neighborZ[k];
			unsigned int ksw  = neighborY[kw];
			unsigned int kbw  = neighborZ[kw];
			unsigned int kbs  = neighborZ[ks];
			unsigned int kbsw = neighborZ[ksw];
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			doubflo mfcbb = (D.f[dirE   ])[k  ];
			doubflo mfabb = (D.f[dirW   ])[kw ];
			doubflo mfbcb = (D.f[dirN   ])[k  ];
			doubflo mfbab = (D.f[dirS   ])[ks ];
			doubflo mfbbc = (D.f[dirT   ])[k  ];
			doubflo mfbba = (D.f[dirB   ])[kb ];
			doubflo mfccb = (D.f[dirNE  ])[k  ];
			doubflo mfaab = (D.f[dirSW  ])[ksw];
			doubflo mfcab = (D.f[dirSE  ])[ks ];
			doubflo mfacb = (D.f[dirNW  ])[kw ];
			doubflo mfcbc = (D.f[dirTE  ])[k  ];
			doubflo mfaba = (D.f[dirBW  ])[kbw];
			doubflo mfcba = (D.f[dirBE  ])[kb ];
			doubflo mfabc = (D.f[dirTW  ])[kw ];
			doubflo mfbcc = (D.f[dirTN  ])[k  ];
			doubflo mfbaa = (D.f[dirBS  ])[kbs];
			doubflo mfbca = (D.f[dirBN  ])[kb ];
			doubflo mfbac = (D.f[dirTS  ])[ks ];
			doubflo mfbbb = (D.f[dirZERO])[k  ];
			doubflo mfccc = (D.f[dirTNE ])[k  ];
			doubflo mfaac = (D.f[dirTSW ])[ksw];
			doubflo mfcac = (D.f[dirTSE ])[ks ];
			doubflo mfacc = (D.f[dirTNW ])[kw ];
			doubflo mfcca = (D.f[dirBNE ])[kb ];
			doubflo mfaaa = (D.f[dirBSW ])[kbsw];
			doubflo mfcaa = (D.f[dirBSE ])[kbs];
			doubflo mfaca = (D.f[dirBNW ])[kbw];
			////////////////////////////////////////////////////////////////////////////////////
			doubflo drho = ((((mfccc+mfaaa) + (mfaca+mfcac)) + ((mfacc+mfcaa) + (mfaac+mfcca))) + 
							(((mfbac+mfbca) + (mfbaa+mfbcc)) + ((mfabc+mfcba) + (mfaba+mfcbc)) + ((mfacb+mfcab) + (mfaab+mfccb))) +
							((mfabb+mfcbb) + (mfbab+mfbcb)) + (mfbba+mfbbc)) + mfbbb;

			doubflo rho = one+drho;
			////////////////////////////////////////////////////////////////////////////////////
			doubflo vvx    =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfcaa-mfacc) + (mfcca-mfaac))) + 
						     (((mfcba-mfabc) + (mfcbc-mfaba)) + ((mfcab-mfacb) + (mfccb-mfaab))) +
						       (mfcbb-mfabb)) / rho;
			doubflo vvy    =((((mfccc-mfaaa) + (mfaca-mfcac)) + ((mfacc-mfcaa) + (mfcca-mfaac))) + 
				             (((mfbca-mfbac) + (mfbcc-mfbaa)) + ((mfacb-mfcab) + (mfccb-mfaab))) +
				               (mfbcb-mfbab)) / rho;
			doubflo vvz    =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfacc-mfcaa) + (mfaac-mfcca))) + 
				             (((mfbac-mfbca) + (mfbcc-mfbaa)) + ((mfabc-mfcba) + (mfcbc-mfaba))) +
				               (mfbbc-mfbba)) / rho;
			////////////////////////////////////////////////////////////////////////////////////
			doubflo oMdrho = one; // comp special
			////////////////////////////////////////////////////////////////////////////////////
			doubflo m0, m1, m2;	
			doubflo vx2;
			doubflo vy2;
			doubflo vz2;
			vx2=vvx*vvx;
			vy2=vvy*vvy;
			vz2=vvz*vvz;
			////////////////////////////////////////////////////////////////////////////////////
			//Hin
			////////////////////////////////////////////////////////////////////////////////////
			// mit 1/36, 1/9, 1/36, 1/9, 4/9, 1/9, 1/36, 1/9, 1/36  Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// Z - Dir
			m2    = mfaaa	+ mfaac;
			m1    = mfaac	- mfaaa;
			m0    = m2		+ mfaab;
			mfaaa = m0;
			m0   += c1o36 * oMdrho;	
			mfaab = m1 -		m0 * vvz;
			mfaac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaba  + mfabc;
			m1    = mfabc  - mfaba;
			m0    = m2		+ mfabb;
			mfaba = m0;
			m0   += c1o9 * oMdrho;
			mfabb = m1 -		m0 * vvz;
			mfabc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaca  + mfacc;
			m1    = mfacc  - mfaca;
			m0    = m2		+ mfacb;
			mfaca = m0;
			m0   += c1o36 * oMdrho;
			mfacb = m1 -		m0 * vvz;
			mfacc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbaa	+ mfbac;
			m1    = mfbac	- mfbaa;
			m0    = m2		+ mfbab;
			mfbaa = m0;
			m0   += c1o9 * oMdrho;
			mfbab = m1 -		m0 * vvz;
			mfbac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbba  + mfbbc;
			m1    = mfbbc  - mfbba;
			m0    = m2		+ mfbbb;
			mfbba = m0;
			m0   += c4o9 * oMdrho;
			mfbbb = m1 -		m0 * vvz;
			mfbbc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbca  + mfbcc;
			m1    = mfbcc  - mfbca;
			m0    = m2		+ mfbcb;
			mfbca = m0;
			m0   += c1o9 * oMdrho;
			mfbcb = m1 -		m0 * vvz;
			mfbcc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcaa	+ mfcac;
			m1    = mfcac	- mfcaa;
			m0    = m2		+ mfcab;
			mfcaa = m0;
			m0   += c1o36 * oMdrho;
			mfcab = m1 -		m0 * vvz;
			mfcac = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcba  + mfcbc;
			m1    = mfcbc  - mfcba;
			m0    = m2		+ mfcbb;
			mfcba = m0;
			m0   += c1o9 * oMdrho;
			mfcbb = m1 -		m0 * vvz;
			mfcbc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcca  + mfccc;
			m1    = mfccc  - mfcca;
			m0    = m2		+ mfccb;
			mfcca = m0;
			m0   += c1o36 * oMdrho;
			mfccb = m1 -		m0 * vvz;
			mfccc = m2 - two*	m1 * vvz + vz2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// mit  1/6, 0, 1/18, 2/3, 0, 2/9, 1/6, 0, 1/18 Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// Y - Dir
			m2    = mfaaa	+ mfaca;
			m1    = mfaca	- mfaaa;
			m0    = m2		+ mfaba;
			mfaaa = m0;
			m0   += c1o6 * oMdrho;
			mfaba = m1 -		m0 * vvy;
			mfaca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaab  + mfacb;
			m1    = mfacb  - mfaab;
			m0    = m2		+ mfabb;
			mfaab = m0;
			mfabb = m1 -		m0 * vvy;
			mfacb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaac  + mfacc;
			m1    = mfacc  - mfaac;
			m0    = m2		+ mfabc;
			mfaac = m0;
			m0   += c1o18 * oMdrho;
			mfabc = m1 -		m0 * vvy;
			mfacc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbaa	+ mfbca;
			m1    = mfbca	- mfbaa;
			m0    = m2		+ mfbba;
			mfbaa = m0;
			m0   += c2o3 * oMdrho;
			mfbba = m1 -		m0 * vvy;
			mfbca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbab  + mfbcb;
			m1    = mfbcb  - mfbab;
			m0    = m2		+ mfbbb;
			mfbab = m0;
			mfbbb = m1 -		m0 * vvy;
			mfbcb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfbac  + mfbcc;
			m1    = mfbcc  - mfbac;
			m0    = m2		+ mfbbc;
			mfbac = m0;
			m0   += c2o9 * oMdrho;
			mfbbc = m1 -		m0 * vvy;
			mfbcc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcaa	+ mfcca;
			m1    = mfcca	- mfcaa;
			m0    = m2		+ mfcba;
			mfcaa = m0;
			m0   += c1o6 * oMdrho;
			mfcba = m1 -		m0 * vvy;
			mfcca = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcab  + mfccb;
			m1    = mfccb  - mfcab;
			m0    = m2		+ mfcbb;
			mfcab = m0;
			mfcbb = m1 -		m0 * vvy;
			mfccb = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfcac  + mfccc;
			m1    = mfccc  - mfcac;
			m0    = m2		+ mfcbc;
			mfcac = m0;
			m0   += c1o18 * oMdrho;
			mfcbc = m1 -		m0 * vvy;
			mfccc = m2 - two*	m1 * vvy + vy2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			// mit     1, 0, 1/3, 0, 0, 0, 1/3, 0, 1/9		Konditionieren
			////////////////////////////////////////////////////////////////////////////////////
			// X - Dir
			m2    = mfaaa	+ mfcaa;
			m1    = mfcaa	- mfaaa;
			m0    = m2		+ mfbaa;
			mfaaa = m0;
			m0   += one* oMdrho;
			mfbaa = m1 -		m0 * vvx;
			mfcaa = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaba  + mfcba;
			m1    = mfcba  - mfaba;
			m0    = m2		+ mfbba;
			mfaba = m0;
			mfbba = m1 -		m0 * vvx;
			mfcba = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaca  + mfcca;
			m1    = mfcca  - mfaca;
			m0    = m2		+ mfbca;
			mfaca = m0;
			m0   += c1o3 * oMdrho;
			mfbca = m1 -		m0 * vvx;
			mfcca = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaab	+ mfcab;
			m1    = mfcab	- mfaab;
			m0    = m2		+ mfbab;
			mfaab = m0;
			mfbab = m1 -		m0 * vvx;
			mfcab = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfabb  + mfcbb;
			m1    = mfcbb  - mfabb;
			m0    = m2		+ mfbbb;
			mfabb = m0;
			mfbbb = m1 -		m0 * vvx;
			mfcbb = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfacb  + mfccb;
			m1    = mfccb  - mfacb;
			m0    = m2		+ mfbcb;
			mfacb = m0;
			mfbcb = m1 -		m0 * vvx;
			mfccb = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfaac	+ mfcac;
			m1    = mfcac	- mfaac;
			m0    = m2		+ mfbac;
			mfaac = m0;
			m0   += c1o3 * oMdrho;
			mfbac = m1 -		m0 * vvx;
			mfcac = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfabc  + mfcbc;
			m1    = mfcbc  - mfabc;
			m0    = m2		+ mfbbc;
			mfabc = m0;
			mfbbc = m1 -		m0 * vvx;
			mfcbc = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			m2    = mfacc  + mfccc;
			m1    = mfccc  - mfacc;
			m0    = m2		+ mfbcc;
			mfacc = m0;
			m0   += c1o9 * oMdrho;
			mfbcc = m1 -		m0 * vvx;
			mfccc = m2 - two*	m1 * vvx + vx2 * m0;
			////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////

			doubflo OxxPyyPzz = one;
			doubflo omega = one / (three*0.001 + c1o2);
			doubflo B = (four * omega * OxxPyyPzz * (nine * omega - sixteen) - four * omega * omega - two * OxxPyyPzz * OxxPyyPzz * (two + nine * omega * (omega - two))) /
				(three * (omega - OxxPyyPzz) * (OxxPyyPzz * (two + three * omega) - eight * omega));

			CUMbcc[k] = mfbcc - ((mfaac * mfbca + mfaca * mfbac + four * mfabb * mfbbb + two * (mfbab * mfacb + mfbba * mfabc)) + c1o3 * (mfbca + mfbac)*(one + rho*six*B / (two + three * B))) / rho;
			CUMcbc[k] = mfcbc - ((mfaac * mfcba + mfcaa * mfabc + four * mfbab * mfbbb + two * (mfabb * mfcab + mfbba * mfbac)) + c1o3 * (mfcba + mfabc)*(one + rho*six*B / (two + three * B))) / rho;
			CUMccb[k] = mfccb - ((mfcaa * mfacb + mfaca * mfcab + four * mfbba * mfbbb + two * (mfbab * mfbca + mfabb * mfcba)) + c1o3 * (mfacb + mfcab)*(one + rho*six*B / (two + three * B))) / rho;

			////////////////////////////////////////////////////////////////////////////////////
			// Cumulants
			////////////////////////////////////////////////////////////////////////////////////
			//central moments to cumulants
			//4.
			CUMcbb[k]      = mfcbb - ((mfcaa + c1o3) * mfabb + two * mfbba * mfbab) / rho;	
			CUMbcb[k]      = mfbcb - ((mfaca + c1o3) * mfbab + two * mfbba * mfabb) / rho; 
			CUMbbc[k]      = mfbbc - ((mfaac + c1o3) * mfbba + two * mfbab * mfabb) / rho; 
			 		
			CUMcca[k]      = mfcca - (((mfcaa * mfaca + two * mfbba * mfbba) + c1o3 * (mfcaa + mfaca)) / rho  - c1o9*(drho/rho));
			CUMcac[k]      = mfcac - (((mfcaa * mfaac + two * mfbab * mfbab) + c1o3 * (mfcaa + mfaac)) / rho  - c1o9*(drho/rho));
			CUMacc[k]      = mfacc - (((mfaac * mfaca + two * mfabb * mfabb) + c1o3 * (mfaac + mfaca)) / rho  - c1o9*(drho/rho));

			//5.
			//CUMbcc[k]      = mfbcc - ((mfaac * mfbca + mfaca * mfbac + four * mfabb * mfbbb + two * (mfbab * mfacb + mfbba * mfabc)) + c1o3 * (mfbca + mfbac) ) / rho ;
			//CUMcbc[k]      = mfcbc - ((mfaac * mfcba + mfcaa * mfabc + four * mfbab * mfbbb + two * (mfabb * mfcab + mfbba * mfbac)) + c1o3 * (mfcba + mfabc) ) / rho ;
			//CUMccb[k]      = mfccb - ((mfcaa * mfacb + mfaca * mfcab + four * mfbba * mfbbb + two * (mfbab * mfbca + mfabb * mfcba)) + c1o3 * (mfacb + mfcab) ) / rho ;
			
			//6.
			CUMccc[k]      = mfccc + ((-four *  mfbbb * mfbbb  
							-           (mfcaa * mfacc + mfaca * mfcac + mfaac * mfcca)
							-    four * (mfabb * mfcbb + mfbab * mfbcb + mfbba * mfbbc)
							-     two * (mfbca * mfbac + mfcba * mfabc + mfcab * mfacb)) / rho
							+(   four * (mfbab * mfbab * mfaca + mfabb * mfabb * mfcaa + mfbba * mfbba * mfaac)
							+     two * (mfcaa * mfaca * mfaac)
							+ sixteen *  mfbba * mfbab * mfabb) / (rho * rho)
							-    c1o3 * (mfacc + mfcac + mfcca) /rho 
							-    c1o9 * (mfcaa + mfaca + mfaac) /rho 
							+(    two * (mfbab * mfbab + mfabb * mfabb + mfbba * mfbba) 
							+           (mfaac * mfaca + mfaac * mfcaa + mfaca * mfcaa) + c1o3 *(mfaac + mfaca + mfcaa)) / (rho * rho) * c2o3 
							+ c1o27*((drho * drho - drho)/(rho*rho)));
			////////////////////////////////////////////////////////////////////////////////////
		}                                                                                                                    
	}
}
