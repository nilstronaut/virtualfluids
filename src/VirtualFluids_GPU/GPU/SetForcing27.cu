/* Device code */
#include "LBM/D3Q27.h"
#include "GPU/constant.h"

////////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void GetVeloforForcing27( doubflo* DD, 
												int* bcIndex, 
												int nonAtBC, 
												doubflo* Vx,
												doubflo* Vy,
												doubflo* Vz,
												unsigned int* neighborX,
												unsigned int* neighborY,
												unsigned int* neighborZ,
												unsigned int size_Mat, 
												bool evenOrOdd)
{
	Distributions27 D;
	if (evenOrOdd==false)
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
	if(k < nonAtBC)
	{
		////////////////////////////////////////////////////////////////////////////////
		//index
		unsigned int KQK  = bcIndex[k];
		unsigned int kzero= KQK;
		unsigned int ke   = KQK;
		unsigned int kw   = neighborX[KQK];
		unsigned int kn   = KQK;
		unsigned int ks   = neighborY[KQK];
		unsigned int kt   = KQK;
		unsigned int kb   = neighborZ[KQK];
		unsigned int ksw  = neighborY[kw];
		unsigned int kne  = KQK;
		unsigned int kse  = ks;
		unsigned int knw  = kw;
		unsigned int kbw  = neighborZ[kw];
		unsigned int kte  = KQK;
		unsigned int kbe  = kb;
		unsigned int ktw  = kw;
		unsigned int kbs  = neighborZ[ks];
		unsigned int ktn  = KQK;
		unsigned int kbn  = kb;
		unsigned int kts  = ks;
		unsigned int ktse = ks;
		unsigned int kbnw = kbw;
		unsigned int ktnw = kw;
		unsigned int kbse = kbs;
		unsigned int ktsw = ksw;
		unsigned int kbne = kb;
		unsigned int ktne = KQK;
		unsigned int kbsw = neighborZ[ksw];
		////////////////////////////////////////////////////////////////////////////////
		doubflo mfcbb = (D.f[dirE   ])[ke   ];
		doubflo mfabb = (D.f[dirW   ])[kw   ];
		doubflo mfbcb = (D.f[dirN   ])[kn   ];
		doubflo mfbab = (D.f[dirS   ])[ks   ];
		doubflo mfbbc = (D.f[dirT   ])[kt   ];
		doubflo mfbba = (D.f[dirB   ])[kb   ];
		doubflo mfccb = (D.f[dirNE  ])[kne  ];
		doubflo mfaab = (D.f[dirSW  ])[ksw  ];
		doubflo mfcab = (D.f[dirSE  ])[kse  ];
		doubflo mfacb = (D.f[dirNW  ])[knw  ];
		doubflo mfcbc = (D.f[dirTE  ])[kte  ];
		doubflo mfaba = (D.f[dirBW  ])[kbw  ];
		doubflo mfcba = (D.f[dirBE  ])[kbe  ];
		doubflo mfabc = (D.f[dirTW  ])[ktw  ];
		doubflo mfbcc = (D.f[dirTN  ])[ktn  ];
		doubflo mfbaa = (D.f[dirBS  ])[kbs  ];
		doubflo mfbca = (D.f[dirBN  ])[kbn  ];
		doubflo mfbac = (D.f[dirTS  ])[kts  ];
		doubflo mfbbb = (D.f[dirZERO])[kzero];
		doubflo mfccc = (D.f[dirTNE ])[ktne ];
		doubflo mfaac = (D.f[dirTSW ])[ktsw ];
		doubflo mfcac = (D.f[dirTSE ])[ktse ];
		doubflo mfacc = (D.f[dirTNW ])[ktnw ];
		doubflo mfcca = (D.f[dirBNE ])[kbne ];
		doubflo mfaaa = (D.f[dirBSW ])[kbsw ];
		doubflo mfcaa = (D.f[dirBSE ])[kbse ];
		doubflo mfaca = (D.f[dirBNW ])[kbnw ];
		////////////////////////////////////////////////////////////////////////////////////
		doubflo rho   = (mfccc+mfaaa + mfaca+mfcac + mfacc+mfcaa + mfaac+mfcca + 
					 	 mfbac+mfbca + mfbaa+mfbcc + mfabc+mfcba + mfaba+mfcbc + mfacb+mfcab + mfaab+mfccb +
						 mfabb+mfcbb + mfbab+mfbcb + mfbba+mfbbc + mfbbb + one);//!!!!Achtung + one
		////////////////////////////////////////////////////////////////////////////////////
		doubflo vx =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfcaa-mfacc) + (mfcca-mfaac))) + 
			         (((mfcba-mfabc) + (mfcbc-mfaba)) + ((mfcab-mfacb) + (mfccb-mfaab))) +
				       (mfcbb-mfabb))/ rho;
		doubflo vy =((((mfccc-mfaaa) + (mfaca-mfcac)) + ((mfacc-mfcaa) + (mfcca-mfaac))) + 
			         (((mfbca-mfbac) + (mfbcc-mfbaa)) + ((mfacb-mfcab) + (mfccb-mfaab))) +
			           (mfbcb-mfbab)) / rho;
		doubflo vz =((((mfccc-mfaaa) + (mfcac-mfaca)) + ((mfacc-mfcaa) + (mfaac-mfcca))) + 
			         (((mfbac-mfbca) + (mfbcc-mfbaa)) + ((mfabc-mfcba) + (mfcbc-mfaba))) +
			           (mfbbc-mfbba)) / rho;
		////////////////////////////////////////////////////////////////////////////////////
		Vx[k] = vx;
		Vy[k] = vy;
		Vz[k] = vz;
		////////////////////////////////////////////////////////////////////////////////////
	}
}

