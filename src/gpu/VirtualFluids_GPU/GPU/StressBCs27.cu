/* Device code */
#include "LBM/LB.h" 
#include "LBM/D3Q27.h"
#include <lbm/constants/NumericConstants.h>

using namespace vf::lbm::constant;

//////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void QStressDeviceComp27(real* DD, 
											   int* k_Q, 
                                    int* k_N, 
											   real* QQ,
                                    unsigned int sizeQ,
                                    real om1, 
                                    real* turbViscosity,
                                    real* vx,
                                    real* vy,
                                    real* vz,
                                    real* normalX,
                                    real* normalY,
                                    real* normalZ,
                                    real* vx_bc,
                                    real* vy_bc,
                                    real* vz_bc,
                                    int* samplingOffset,
                                    real* z0,
											   unsigned int* neighborX,
                                    unsigned int* neighborY,
                                    unsigned int* neighborZ,
                                    unsigned int size_Mat, 
                                    bool evenOrOdd)
{
   Distributions27 D;
   if (evenOrOdd==true)//get right array of post coll f's
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

   if(k<sizeQ/*kQ*/)
   {
      ////////////////////////////////////////////////////////////////////////////////
      real *q_dirE,   *q_dirW,   *q_dirN,   *q_dirS,   *q_dirT,   *q_dirB, 
            *q_dirNE,  *q_dirSW,  *q_dirSE,  *q_dirNW,  *q_dirTE,  *q_dirBW,
            *q_dirBE,  *q_dirTW,  *q_dirTN,  *q_dirBS,  *q_dirBN,  *q_dirTS,
            *q_dirTNE, *q_dirTSW, *q_dirTSE, *q_dirTNW, *q_dirBNE, *q_dirBSW,
            *q_dirBSE, *q_dirBNW; 
      q_dirE   = &QQ[dirE   *sizeQ];
      q_dirW   = &QQ[dirW   *sizeQ];
      q_dirN   = &QQ[dirN   *sizeQ];
      q_dirS   = &QQ[dirS   *sizeQ];
      q_dirT   = &QQ[dirT   *sizeQ];
      q_dirB   = &QQ[dirB   *sizeQ];
      q_dirNE  = &QQ[dirNE  *sizeQ];
      q_dirSW  = &QQ[dirSW  *sizeQ];
      q_dirSE  = &QQ[dirSE  *sizeQ];
      q_dirNW  = &QQ[dirNW  *sizeQ];
      q_dirTE  = &QQ[dirTE  *sizeQ];
      q_dirBW  = &QQ[dirBW  *sizeQ];
      q_dirBE  = &QQ[dirBE  *sizeQ];
      q_dirTW  = &QQ[dirTW  *sizeQ];
      q_dirTN  = &QQ[dirTN  *sizeQ];
      q_dirBS  = &QQ[dirBS  *sizeQ];
      q_dirBN  = &QQ[dirBN  *sizeQ];
      q_dirTS  = &QQ[dirTS  *sizeQ];
      q_dirTNE = &QQ[dirTNE *sizeQ];
      q_dirTSW = &QQ[dirTSW *sizeQ];
      q_dirTSE = &QQ[dirTSE *sizeQ];
      q_dirTNW = &QQ[dirTNW *sizeQ];
      q_dirBNE = &QQ[dirBNE *sizeQ];
      q_dirBSW = &QQ[dirBSW *sizeQ];
      q_dirBSE = &QQ[dirBSE *sizeQ];
      q_dirBNW = &QQ[dirBNW *sizeQ];
      ////////////////////////////////////////////////////////////////////////////////
      //index
      unsigned int KQK  = k_Q[k];
      unsigned int kzero= KQK;      //get right adress of post-coll f's
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
      real f_E,  f_W,  f_N,  f_S,  f_T,  f_B,   f_NE,  f_SW,  f_SE,  f_NW,  f_TE,  f_BW,  f_BE,
         f_TW, f_TN, f_BS, f_BN, f_TS, f_TNE, f_TSW, f_TSE, f_TNW, f_BNE, f_BSW, f_BSE, f_BNW;

      f_W    = (D.f[dirE   ])[ke   ];     //post-coll f's
      f_E    = (D.f[dirW   ])[kw   ];
      f_S    = (D.f[dirN   ])[kn   ];
      f_N    = (D.f[dirS   ])[ks   ];
      f_B    = (D.f[dirT   ])[kt   ];
      f_T    = (D.f[dirB   ])[kb   ];
      f_SW   = (D.f[dirNE  ])[kne  ];
      f_NE   = (D.f[dirSW  ])[ksw  ];
      f_NW   = (D.f[dirSE  ])[kse  ];
      f_SE   = (D.f[dirNW  ])[knw  ];
      f_BW   = (D.f[dirTE  ])[kte  ];
      f_TE   = (D.f[dirBW  ])[kbw  ];
      f_TW   = (D.f[dirBE  ])[kbe  ];
      f_BE   = (D.f[dirTW  ])[ktw  ];
      f_BS   = (D.f[dirTN  ])[ktn  ];
      f_TN   = (D.f[dirBS  ])[kbs  ];
      f_TS   = (D.f[dirBN  ])[kbn  ];
      f_BN   = (D.f[dirTS  ])[kts  ];
      f_BSW  = (D.f[dirTNE ])[ktne ];
      f_BNE  = (D.f[dirTSW ])[ktsw ];
      f_BNW  = (D.f[dirTSE ])[ktse ];
      f_BSE  = (D.f[dirTNW ])[ktnw ];
      f_TSW  = (D.f[dirBNE ])[kbne ];
      f_TNE  = (D.f[dirBSW ])[kbsw ];
      f_TNW  = (D.f[dirBSE ])[kbse ];
      f_TSE  = (D.f[dirBNW ])[kbnw ];

      ////////////////////////////////////////////////////////////////////////////////
      real vx1, vx2, vx3, drho, feq, q;
      drho   =  f_TSE + f_TNW + f_TNE + f_TSW + f_BSE + f_BNW + f_BNE + f_BSW +
                f_BN + f_TS + f_TN + f_BS + f_BE + f_TW + f_TE + f_BW + f_SE + f_NW + f_NE + f_SW + 
                f_T + f_B + f_N + f_S + f_E + f_W + ((D.f[dirZERO])[kzero]); 

      vx1    =  (((f_TSE - f_BNW) - (f_TNW - f_BSE)) + ((f_TNE - f_BSW) - (f_TSW - f_BNE)) +
                ((f_BE - f_TW)   + (f_TE - f_BW))   + ((f_SE - f_NW)   + (f_NE - f_SW)) +
                (f_E - f_W)) / (c1o1 + drho); 
         

      vx2    =   ((-(f_TSE - f_BNW) + (f_TNW - f_BSE)) + ((f_TNE - f_BSW) - (f_TSW - f_BNE)) +
                 ((f_BN - f_TS)   + (f_TN - f_BS))    + (-(f_SE - f_NW)  + (f_NE - f_SW)) +
                 (f_N - f_S)) / (c1o1 + drho); 

      vx3    =   (((f_TSE - f_BNW) + (f_TNW - f_BSE)) + ((f_TNE - f_BSW) + (f_TSW - f_BNE)) +
                 (-(f_BN - f_TS)  + (f_TN - f_BS))   + ((f_TE - f_BW)   - (f_BE - f_TW)) +
                 (f_T - f_B)) / (c1o1 + drho); 

      real cu_sq=c3o2*(vx1*vx1+vx2*vx2+vx3*vx3) * (c1o1 + drho);

      real om_turb = om1 / (c1o1 + c3o1*om1*max(c0o1, turbViscosity[k_N[k]]));
      //////////////////////////////////////////////////////////////////////////
      if (evenOrOdd==false)      //get adress where incoming f's should be written to
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
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Compute incoming f's with zero wall velocity
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // real VeloX=0.0, VeloY=0.0, VeloZ=0.0; 
      
      // if(k==3071){ // 3071 9120 9215
      //    printf("======================================\n");
      //    printf("Stress=====================\n");
      //    printf("k \t %u\n", k);
      //    printf("kQk \t %u\n", KQK);
      // //    printf("E \t %f \n", q_dirE[k]);
      // //    printf("NE \t %f \n", q_dirNE[k]);
      // //    printf("W \t %f \n", q_dirW[k]);
      // //    printf("NW \t %f \n", q_dirNW[k]);
      // //    printf("N \t %f \n", q_dirN[k]);
      // //    printf("S \t %f \n", q_dirS[k]);
      // //    printf("SE \t %f \n", q_dirSE[k]);
      // //    printf("NE \t %f \n", q_dirNE[k]);
      // //    printf("SW \t %f \n", q_dirSW[k]);
      // //    printf("B \t %f \n", q_dirB[k]);
      // //    printf("BS \t %f \n", q_dirBS[k]);
      // //    printf("BN \t %f \n", q_dirBN[k]);
      // //    printf("BW \t %f \n", q_dirBW[k]);
      // //    printf("BE \t %f \n", q_dirBE[k]);
      // //    printf("BNE \t %f \n", q_dirBNE[k]);
      // //    printf("BNW \t %f \n", q_dirBNW[k]);
      // //    printf("BSE \t %f \n", q_dirBSE[k]);
      // //    printf("BSW \t %f \n", q_dirBSW[k]);
      // //    printf("T \t %f \n", q_dirT[k]);
      // //    printf("TS \t %f \n", q_dirTS[k]);
      // //    printf("TN \t %f \n", q_dirTN[k]);
      // //    printf("TW \t %f \n", q_dirTW[k]);
      // //    printf("TE \t %f \n", q_dirTE[k]);
      // //    printf("TNE \t %f \n", q_dirTNE[k]);
      // //    printf("TNW \t %f \n", q_dirTNW[k]);
      // //    printf("TSE \t %f \n", q_dirTSE[k]);
      // //    printf("TSW \t %f \n\n", q_dirTSW[k]);

      //    printf("E \t %f \n", f_E);
      //    printf("NE \t %f \n", f_NE);
      //    printf("W \t %f \n", f_W);
      //    printf("NW \t %f \n", f_NW);
      //    printf("N \t %f \n", f_N);
      //    printf("S \t %f \n", f_S);
      //    printf("SE \t %f \n", f_SE);
      //    printf("NE \t %f \n", f_NE);
      //    printf("SW \t %f \n", f_SW);
      //    printf("B \t %f \n", f_B);
      //    printf("BS \t %f \n", f_BS);
      //    printf("BN \t %f \n", f_BN);
      //    printf("BW \t %f \n", f_BW);
      //    printf("BE \t %f \n", f_BE);
      //    printf("BNE \t %f \n", f_BNE);
      //    printf("BNW \t %f \n", f_BNW);
      //    printf("BSE \t %f \n", f_BSE);
      //    printf("BSW \t %f \n", f_BSW);
      //    printf("T \t %f \n", f_T);
      //    printf("TS \t %f \n", f_TS);
      //    printf("TN \t %f \n", f_TN);
      //    printf("TW \t %f \n", f_TW);
      //    printf("TE \t %f \n", f_TE);
      //    printf("TNE \t %f \n", f_TNE);
      //    printf("TNW \t %f \n", f_TNW);
      //    printf("TSE \t %f \n", f_TSE);
      //    printf("TSW \t %f \n\n", f_TSW);
      // }

      // incoming f's from bounce back
      real f_E_in,  f_W_in,  f_N_in,  f_S_in,  f_T_in,  f_B_in,   f_NE_in,  f_SW_in,  f_SE_in,  f_NW_in,  f_TE_in,  f_BW_in,  f_BE_in,
         f_TW_in, f_TN_in, f_BS_in, f_BN_in, f_TS_in, f_TNE_in, f_TSW_in, f_TSE_in, f_TNW_in, f_BNE_in, f_BSW_in, f_BSE_in, f_BNW_in;
      // momentum exchanged with wall at rest
      real wallMomentumX = 0.0, wallMomentumY = 0.0, wallMomentumZ = 0.0;

      q = q_dirE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c2o27* (drho/*+three*( vx1        )*/+c9o2*( vx1        )*( vx1        ) * (c1o1 + drho)-cu_sq); 
         f_W_in=(c1o1-q)/(c1o1+q)*(f_E-f_W+(f_E+f_W-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_E+f_W))/(c1o1+q) - c2o27 * drho;
         wallMomentumX += f_E+f_W_in;
      }

      q = q_dirW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c2o27* (drho/*+three*(-vx1        )*/+c9o2*(-vx1        )*(-vx1        ) * (c1o1 + drho)-cu_sq); 
         f_E_in=(c1o1-q)/(c1o1+q)*(f_W-f_E+(f_W+f_E-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_W+f_E))/(c1o1+q) - c2o27 * drho;
         wallMomentumX -= f_W+f_E_in;
      }

      q = q_dirN[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c2o27* (drho/*+three*(    vx2     )*/+c9o2*(     vx2    )*(     vx2    ) * (c1o1 + drho)-cu_sq); 
         f_S_in=(c1o1-q)/(c1o1+q)*(f_N-f_S+(f_N+f_S-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_N+f_S))/(c1o1+q) - c2o27 * drho;
         wallMomentumY += f_N+f_S_in;
      }

      q = q_dirS[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c2o27* (drho/*+three*(   -vx2     )*/+c9o2*(    -vx2    )*(    -vx2    ) * (c1o1 + drho)-cu_sq); 
         f_N_in=(c1o1-q)/(c1o1+q)*(f_S-f_N+(f_S+f_N-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_S+f_N))/(c1o1+q) - c2o27 * drho;
         wallMomentumY -= f_S+f_N_in;
      }

      q = q_dirT[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c2o27* (drho/*+three*(         vx3)*/+c9o2*(         vx3)*(         vx3) * (c1o1 + drho)-cu_sq); 
         f_B_in=(c1o1-q)/(c1o1+q)*(f_T-f_B+(f_T+f_B-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_T+f_B))/(c1o1+q) - c2o27 * drho;
         wallMomentumZ += f_T+f_B_in;
      }

      q = q_dirB[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c2o27* (drho/*+three*(        -vx3)*/+c9o2*(        -vx3)*(        -vx3) * (c1o1 + drho)-cu_sq); 
         f_T_in=(c1o1-q)/(c1o1+q)*(f_B-f_T+(f_B+f_T-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_B+f_T))/(c1o1+q) - c2o27 * drho;
         wallMomentumZ -= f_B+f_T_in;
      }

      q = q_dirNE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*( vx1+vx2    )*/+c9o2*( vx1+vx2    )*( vx1+vx2    ) * (c1o1 + drho)-cu_sq); 
         f_SW_in=(c1o1-q)/(c1o1+q)*(f_NE-f_SW+(f_NE+f_SW-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_NE+f_SW))/(c1o1+q) - c1o54 * drho;
         wallMomentumX += f_NE+f_SW_in;
         wallMomentumY += f_NE+f_SW_in;
      }

      q = q_dirSW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*(-vx1-vx2    )*/+c9o2*(-vx1-vx2    )*(-vx1-vx2    ) * (c1o1 + drho)-cu_sq); 
         f_NE_in=(c1o1-q)/(c1o1+q)*(f_SW-f_NE+(f_SW+f_NE-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_SW+f_NE))/(c1o1+q) - c1o54 * drho;
         wallMomentumX -= f_SW+f_NE_in;
         wallMomentumY -= f_SW+f_NE_in;
      }

      q = q_dirSE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*( vx1-vx2    )*/+c9o2*( vx1-vx2    )*( vx1-vx2    ) * (c1o1 + drho)-cu_sq); 
         f_NW_in=(c1o1-q)/(c1o1+q)*(f_SE-f_NW+(f_SE+f_NW-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_SE+f_NW))/(c1o1+q) - c1o54 * drho;
         wallMomentumX += f_SE+f_NW_in;
         wallMomentumY -= f_SE+f_NW_in;
      }

      q = q_dirNW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*(-vx1+vx2    )*/+c9o2*(-vx1+vx2    )*(-vx1+vx2    ) * (c1o1 + drho)-cu_sq); 
         f_SE_in=(c1o1-q)/(c1o1+q)*(f_NW-f_SE+(f_NW+f_SE-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_NW+f_SE))/(c1o1+q) - c1o54 * drho;
         wallMomentumX -= f_NW+f_SE_in;
         wallMomentumY += f_NW+f_SE_in;
      }

      q = q_dirTE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*( vx1    +vx3)*/+c9o2*( vx1    +vx3)*( vx1    +vx3) * (c1o1 + drho)-cu_sq); 
         f_BW_in=(c1o1-q)/(c1o1+q)*(f_TE-f_BW+(f_TE+f_BW-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_TE+f_BW))/(c1o1+q) - c1o54 * drho;
         wallMomentumX += f_TE+f_BW_in;
         wallMomentumZ += f_TE+f_BW_in;
      }

      q = q_dirBW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*(-vx1    -vx3)*/+c9o2*(-vx1    -vx3)*(-vx1    -vx3) * (c1o1 + drho)-cu_sq); 
         f_TE_in=(c1o1-q)/(c1o1+q)*(f_BW-f_TE+(f_BW+f_TE-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_BW+f_TE))/(c1o1+q) - c1o54 * drho;
         wallMomentumX -= f_BW+f_TE_in;
         wallMomentumZ -= f_BW+f_TE_in;
      }

      q = q_dirBE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*( vx1    -vx3)*/+c9o2*( vx1    -vx3)*( vx1    -vx3) * (c1o1 + drho)-cu_sq); 
         f_TW_in=(c1o1-q)/(c1o1+q)*(f_BE-f_TW+(f_BE+f_TW-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_BE+f_TW))/(c1o1+q) - c1o54 * drho;
         wallMomentumX += f_BE+f_TW_in;
         wallMomentumZ -= f_BE+f_TW_in;
      }

      q = q_dirTW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*(-vx1    +vx3)*/+c9o2*(-vx1    +vx3)*(-vx1    +vx3) * (c1o1 + drho)-cu_sq); 
         f_BE_in=(c1o1-q)/(c1o1+q)*(f_TW-f_BE+(f_TW+f_BE-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_TW+f_BE))/(c1o1+q) - c1o54 * drho;
         wallMomentumX -= f_TW+f_BE_in;
         wallMomentumZ += f_TW+f_BE_in;
      }

      q = q_dirTN[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*(     vx2+vx3)*/+c9o2*(     vx2+vx3)*(     vx2+vx3) * (c1o1 + drho)-cu_sq); 
         f_BS_in=(c1o1-q)/(c1o1+q)*(f_TN-f_BS+(f_TN+f_BS-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_TN+f_BS))/(c1o1+q) - c1o54 * drho;
         wallMomentumY += f_TN+f_BS_in;
         wallMomentumZ += f_TN+f_BS_in;
      }

      q = q_dirBS[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*(    -vx2-vx3)*/+c9o2*(    -vx2-vx3)*(    -vx2-vx3) * (c1o1 + drho)-cu_sq); 
         f_TN_in=(c1o1-q)/(c1o1+q)*(f_BS-f_TN+(f_BS+f_TN-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_BS+f_TN))/(c1o1+q) - c1o54 * drho;
         wallMomentumY -= f_BS+f_TN_in;
         wallMomentumZ -= f_BS+f_TN_in;
      }

      q = q_dirBN[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*(     vx2-vx3)*/+c9o2*(     vx2-vx3)*(     vx2-vx3) * (c1o1 + drho)-cu_sq); 
         f_TS_in=(c1o1-q)/(c1o1+q)*(f_BN-f_TS+(f_BN+f_TS-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_BN+f_TS))/(c1o1+q) - c1o54 * drho;
         wallMomentumY += f_BN+f_TS_in;
         wallMomentumZ -= f_BN+f_TS_in;
      }

      q = q_dirTS[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o54* (drho/*+three*(    -vx2+vx3)*/+c9o2*(    -vx2+vx3)*(    -vx2+vx3) * (c1o1 + drho)-cu_sq); 
         f_BN_in=(c1o1-q)/(c1o1+q)*(f_TS-f_BN+(f_TS+f_BN-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_TS+f_BN))/(c1o1+q) - c1o54 * drho;
         wallMomentumY -= f_TS+f_BN_in;
         wallMomentumZ += f_TS+f_BN_in;
      }

      q = q_dirTNE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o216*(drho/*+three*( vx1+vx2+vx3)*/+c9o2*( vx1+vx2+vx3)*( vx1+vx2+vx3) * (c1o1 + drho)-cu_sq); 
         f_BSW_in=(c1o1-q)/(c1o1+q)*(f_TNE-f_BSW+(f_TNE+f_BSW-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_TNE+f_BSW))/(c1o1+q) - c1o216 * drho;
         wallMomentumX += f_TNE+f_BSW_in;
         wallMomentumY += f_TNE+f_BSW_in;
         wallMomentumZ += f_TNE+f_BSW_in;
      }

      q = q_dirBSW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o216*(drho/*+three*(-vx1-vx2-vx3)*/+c9o2*(-vx1-vx2-vx3)*(-vx1-vx2-vx3) * (c1o1 + drho)-cu_sq); 
         f_TNE_in=(c1o1-q)/(c1o1+q)*(f_BSW-f_TNE+(f_BSW+f_TNE-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_BSW+f_TNE))/(c1o1+q) - c1o216 * drho;
         wallMomentumX -= f_BSW+f_TNE_in;
         wallMomentumY -= f_BSW+f_TNE_in;
         wallMomentumZ -= f_BSW+f_TNE_in;
      }

      q = q_dirBNE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o216*(drho/*+three*( vx1+vx2-vx3)*/+c9o2*( vx1+vx2-vx3)*( vx1+vx2-vx3) * (c1o1 + drho)-cu_sq); 
         f_TSW_in=(c1o1-q)/(c1o1+q)*(f_BNE-f_TSW+(f_BNE+f_TSW-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_BNE+f_TSW))/(c1o1+q) - c1o216 * drho;
         wallMomentumX += f_BNE+f_TSW_in;
         wallMomentumY += f_BNE+f_TSW_in;
         wallMomentumZ -= f_BNE+f_TSW_in;
      }

      q = q_dirTSW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o216*(drho/*+three*(-vx1-vx2+vx3)*/+c9o2*(-vx1-vx2+vx3)*(-vx1-vx2+vx3) * (c1o1 + drho)-cu_sq); 
         f_BNE_in=(c1o1-q)/(c1o1+q)*(f_TSW-f_BNE+(f_TSW+f_BNE-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_TSW+f_BNE))/(c1o1+q) - c1o216 * drho;
         wallMomentumX -= f_TSW+f_BNE_in;
         wallMomentumY -= f_TSW+f_BNE_in;
         wallMomentumZ += f_TSW+f_BNE_in;
      }

      q = q_dirTSE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o216*(drho/*+three*( vx1-vx2+vx3)*/+c9o2*( vx1-vx2+vx3)*( vx1-vx2+vx3) * (c1o1 + drho)-cu_sq); 
         f_BNW_in=(c1o1-q)/(c1o1+q)*(f_TSE-f_BNW+(f_TSE+f_BNW-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_TSE+f_BNW))/(c1o1+q) - c1o216 * drho;
         wallMomentumX += f_TSE+f_BNW_in;
         wallMomentumY -= f_TSE+f_BNW_in;
         wallMomentumZ += f_TSE+f_BNW_in;
      }

      q = q_dirBNW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o216*(drho/*+three*(-vx1+vx2-vx3)*/+c9o2*(-vx1+vx2-vx3)*(-vx1+vx2-vx3) * (c1o1 + drho)-cu_sq); 
         f_TSE_in=(c1o1-q)/(c1o1+q)*(f_BNW-f_TSE+(f_BNW+f_TSE-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_BNW+f_TSE))/(c1o1+q) - c1o216 * drho;
         wallMomentumX -= f_BNW+f_TSE_in;
         wallMomentumY += f_BNW+f_TSE_in;
         wallMomentumZ -= f_BNW+f_TSE_in;
      }

      q = q_dirBSE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o216*(drho/*+three*( vx1-vx2-vx3)*/+c9o2*( vx1-vx2-vx3)*( vx1-vx2-vx3) * (c1o1 + drho)-cu_sq); 
         f_TNW_in=(c1o1-q)/(c1o1+q)*(f_BSE-f_TNW+(f_BSE+f_TNW-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_BSE+f_TNW))/(c1o1+q) - c1o216 * drho;
         wallMomentumX += f_BSE+f_TNW_in;
         wallMomentumY -= f_BSE+f_TNW_in;
         wallMomentumZ -= f_BSE+f_TNW_in;
      }

      q = q_dirTNW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         feq=c1o216*(drho/*+three*(-vx1+vx2+vx3)*/+c9o2*(-vx1+vx2+vx3)*(-vx1+vx2+vx3) * (c1o1 + drho)-cu_sq); 
         f_BSE_in=(c1o1-q)/(c1o1+q)*(f_TNW-f_BSE+(f_TNW+f_BSE-c2o1*feq*om_turb)/(c1o1-om_turb))*c1o2+(q*(f_TNW+f_BSE))/(c1o1+q) - c1o216 * drho;
         wallMomentumX -= f_TNW+f_BSE_in;
         wallMomentumY += f_TNW+f_BSE_in;
         wallMomentumZ += f_TNW+f_BSE_in;
      }

      // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // //Compute wall velocity
      // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      real VeloX=0.0, VeloY=0.0, VeloZ=0.0; 

      real wallNormalX = normalX[k];
      real wallNormalY = normalY[k];
      real wallNormalZ = normalZ[k];

      //Sample velocity at exchange location and filter temporally
      real eps = 0.001;
      real vxEL = eps*vx[k_N[k]]+(1.0-eps)*vx_bc[k];
      real vyEL = eps*vy[k_N[k]]+(1.0-eps)*vy_bc[k];
      real vzEL = eps*vz[k_N[k]]+(1.0-eps)*vz_bc[k];
      vx_bc[k] = vxEL;
      vy_bc[k] = vyEL;
      vz_bc[k] = vzEL;

      //Subtract wall-normal velocity component
      real vDotN = vxEL*wallNormalX+vyEL*wallNormalY+vzEL*wallNormalZ;
      vxEL -= vDotN*wallNormalX;
      vyEL -= vDotN*wallNormalY;
      vzEL -= vDotN*wallNormalZ;
      real vMag = sqrt(vxEL*vxEL+vyEL*vyEL+vzEL*vzEL);
      
      
      //Compute wall shear stress tau_w via MOST
      real z = (real)samplingOffset[k] + 0.5; //assuming q=0.5, could be replaced by wall distance via wall normal
      real kappa = 0.4;
      real u_star = vMag*kappa/(log(z/z0[k]));

      real tau_w = u_star*u_star;                  //assuming rho=1
      real A = 1.0;                                //wall area (obviously 1 for grid aligned walls, can come from grid builder later for complex geometries)
      
      //Momentum to be applied via wall velocity
      real wallMomDotN = wallMomentumX*wallNormalX+wallMomentumY*wallNormalY+wallMomentumZ*wallNormalZ;
      real F_x = (tau_w*A) * (vxEL/vMag) - ( wallMomentumX - wallMomDotN*wallNormalX);
      real F_y = (tau_w*A) * (vyEL/vMag) - ( wallMomentumY - wallMomDotN*wallNormalY);
      real F_z = (tau_w*A) * (vzEL/vMag) - ( wallMomentumZ - wallMomDotN*wallNormalZ);

      //Corresponding wall velocity (only valid for wall-normals in z)
      q = 0.5f;
      VeloX = -3.0*F_x*(c1o1+q);
      VeloY = -3.0*F_y*(c1o1+q);
      VeloZ = -3.0*F_z*(c1o1+q);
      if(false && k==0)
      {     
         printf("==========================samp, z, z0: \t %i \t %f \t %f  \n u,v,w, vMag \t %f \t %f \t %f \t %f \n", samplingOffset[k], z, z0[k], vxEL,vyEL,vzEL,vMag );
         printf("u_star: %f \t\n\n", u_star);
         printf("Velo: %f \t %f \t %f \t\n", VeloX, VeloY, VeloZ);
         printf("Wall tan momentum: %f \t %f \t %f \t\n", wallMomentumX - wallMomDotN*wallMomentumX, wallMomentumY - wallMomDotN*wallMomentumY, wallMomentumZ - wallMomDotN*wallMomentumZ);
         printf("FMEM before:\t %1.14f \t %1.14f \t %1.14f \nFWM: \t %1.14f \t %1.14f \t %1.14f \n", wallMomentumX, wallMomentumY, wallMomentumZ, (tau_w*A) * (vxEL/vMag), (tau_w*A) * (vyEL/vMag), (tau_w*A) * (vzEL/vMag));
      } 
      // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // //Add wall velocity and write f's
      // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      q = q_dirE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirW])[kw] = f_W_in - (c6o1*c2o27*( VeloX     ))/(c1o1+q);
         wallMomentumX += -(c6o1*c2o27*( VeloX     ))/(c1o1+q);
      }

      q = q_dirW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirE])[ke] = f_E_in - (c6o1*c2o27*(-VeloX     ))/(c1o1+q);
         wallMomentumX -= - (c6o1*c2o27*(-VeloX     ))/(c1o1+q);
      }

      q = q_dirN[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirS])[ks] = f_S_in - (c6o1*c2o27*( VeloY     ))/(c1o1+q);
         wallMomentumY += - (c6o1*c2o27*( VeloY     ))/(c1o1+q);
      }

      q = q_dirS[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirN])[kn] = f_N_in - (c6o1*c2o27*(-VeloY     ))/(c1o1+q);
         wallMomentumY -=  -(c6o1*c2o27*(-VeloY     ))/(c1o1+q);
      }

      q = q_dirT[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirB])[kb] = f_B_in - (c6o1*c2o27*( VeloZ     ))/(c1o1+q);
         wallMomentumZ += - (c6o1*c2o27*( VeloZ     ))/(c1o1+q);
      }

      q = q_dirB[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirT])[kt] = f_T_in - (c6o1*c2o27*(-VeloZ     ))/(c1o1+q);
         wallMomentumZ -= -(c6o1*c2o27*(-VeloZ     ))/(c1o1+q);
      }

      q = q_dirNE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirSW])[ksw] = f_SW_in - (c6o1*c1o54*(VeloX+VeloY))/(c1o1+q);
         wallMomentumX +=  -(c6o1*c1o54*(VeloX+VeloY))/(c1o1+q);
         wallMomentumY +=  -(c6o1*c1o54*(VeloX+VeloY))/(c1o1+q);
      }

      q = q_dirSW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirNE])[kne] = f_NE_in - (c6o1*c1o54*(-VeloX-VeloY))/(c1o1+q);
         wallMomentumX -= - (c6o1*c1o54*(-VeloX-VeloY))/(c1o1+q);
         wallMomentumY -= - (c6o1*c1o54*(-VeloX-VeloY))/(c1o1+q);
      }

      q = q_dirSE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirNW])[knw] = f_NW_in - (c6o1*c1o54*( VeloX-VeloY))/(c1o1+q);
         wallMomentumX += -(c6o1*c1o54*( VeloX-VeloY))/(c1o1+q);
         wallMomentumY -= -(c6o1*c1o54*( VeloX-VeloY))/(c1o1+q);
      }

      q = q_dirNW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirSE])[kse] = f_SE_in - (c6o1*c1o54*(-VeloX+VeloY))/(c1o1+q);
         wallMomentumX -= - (c6o1*c1o54*(-VeloX+VeloY))/(c1o1+q);
         wallMomentumY += - (c6o1*c1o54*(-VeloX+VeloY))/(c1o1+q);
      }

      q = q_dirTE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirBW])[kbw] = f_BW_in - (c6o1*c1o54*( VeloX+VeloZ))/(c1o1+q); 
         wallMomentumX += - (c6o1*c1o54*( VeloX+VeloZ))/(c1o1+q);
         wallMomentumZ += - (c6o1*c1o54*( VeloX+VeloZ))/(c1o1+q);
      }

      q = q_dirBW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirTE])[kte] = f_TE_in - (c6o1*c1o54*(-VeloX-VeloZ))/(c1o1+q);
         wallMomentumX -= - (c6o1*c1o54*(-VeloX-VeloZ))/(c1o1+q);
         wallMomentumZ -= - (c6o1*c1o54*(-VeloX-VeloZ))/(c1o1+q);
      }

      q = q_dirBE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirTW])[ktw] = f_TW_in - (c6o1*c1o54*( VeloX-VeloZ))/(c1o1+q);
         wallMomentumX += - (c6o1*c1o54*( VeloX-VeloZ))/(c1o1+q);
         wallMomentumZ -= - (c6o1*c1o54*( VeloX-VeloZ))/(c1o1+q);
      }

      q = q_dirTW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirBE])[kbe] = f_BE_in - (c6o1*c1o54*(-VeloX+VeloZ))/(c1o1+q);
         wallMomentumX -= - (c6o1*c1o54*(-VeloX+VeloZ))/(c1o1+q);
         wallMomentumZ += - (c6o1*c1o54*(-VeloX+VeloZ))/(c1o1+q);
      }

      q = q_dirTN[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirBS])[kbs] = f_BS_in - (c6o1*c1o54*( VeloY+VeloZ))/(c1o1+q);
         wallMomentumY += - (c6o1*c1o54*( VeloY+VeloZ))/(c1o1+q);
         wallMomentumZ += - (c6o1*c1o54*( VeloY+VeloZ))/(c1o1+q);
      }

      q = q_dirBS[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirTN])[ktn] = f_TN_in - (c6o1*c1o54*( -VeloY-VeloZ))/(c1o1+q);
         wallMomentumY -= - (c6o1*c1o54*( -VeloY-VeloZ))/(c1o1+q);
         wallMomentumZ -= - (c6o1*c1o54*( -VeloY-VeloZ))/(c1o1+q);
      }

      q = q_dirBN[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirTS])[kts] = f_TS_in - (c6o1*c1o54*( VeloY-VeloZ))/(c1o1+q);
         wallMomentumY += - (c6o1*c1o54*( VeloY-VeloZ))/(c1o1+q);
         wallMomentumZ -= - (c6o1*c1o54*( VeloY-VeloZ))/(c1o1+q);
      }

      q = q_dirTS[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirBN])[kbn] = f_BN_in - (c6o1*c1o54*( -VeloY+VeloZ))/(c1o1+q);
         wallMomentumY -= - (c6o1*c1o54*( -VeloY+VeloZ))/(c1o1+q);
         wallMomentumZ += - (c6o1*c1o54*( -VeloY+VeloZ))/(c1o1+q);
      }

      q = q_dirTNE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirBSW])[kbsw] = f_BSW_in - (c6o1*c1o216*( VeloX+VeloY+VeloZ))/(c1o1+q);
         wallMomentumX += - (c6o1*c1o216*( VeloX+VeloY+VeloZ))/(c1o1+q);
         wallMomentumY += - (c6o1*c1o216*( VeloX+VeloY+VeloZ))/(c1o1+q);
         wallMomentumZ += - (c6o1*c1o216*( VeloX+VeloY+VeloZ))/(c1o1+q);
      }

      q = q_dirBSW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirTNE])[ktne] = f_TNE_in - (c6o1*c1o216*(-VeloX-VeloY-VeloZ))/(c1o1+q);
         wallMomentumX -= - (c6o1*c1o216*(-VeloX-VeloY-VeloZ))/(c1o1+q);
         wallMomentumY -= - (c6o1*c1o216*(-VeloX-VeloY-VeloZ))/(c1o1+q);
         wallMomentumZ -= - (c6o1*c1o216*(-VeloX-VeloY-VeloZ))/(c1o1+q);
      }

      q = q_dirBNE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirTSW])[ktsw] = f_TSW_in - (c6o1*c1o216*( VeloX+VeloY-VeloZ))/(c1o1+q);
         wallMomentumX += - (c6o1*c1o216*( VeloX+VeloY-VeloZ))/(c1o1+q);
         wallMomentumY += - (c6o1*c1o216*( VeloX+VeloY-VeloZ))/(c1o1+q);
         wallMomentumZ -= - (c6o1*c1o216*( VeloX+VeloY-VeloZ))/(c1o1+q);
      }

      q = q_dirTSW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirBNE])[kbne] = f_BNE_in - (c6o1*c1o216*(-VeloX-VeloY+VeloZ))/(c1o1+q);
         wallMomentumX -= - (c6o1*c1o216*(-VeloX-VeloY+VeloZ))/(c1o1+q);
         wallMomentumY -= - (c6o1*c1o216*(-VeloX-VeloY+VeloZ))/(c1o1+q);
         wallMomentumZ += - (c6o1*c1o216*(-VeloX-VeloY+VeloZ))/(c1o1+q);
      }

      q = q_dirTSE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirBNW])[kbnw] = f_BNW_in - (c6o1*c1o216*( VeloX-VeloY+VeloZ))/(c1o1+q);
         wallMomentumX += - (c6o1*c1o216*( VeloX-VeloY+VeloZ))/(c1o1+q);
         wallMomentumY -= - (c6o1*c1o216*( VeloX-VeloY+VeloZ))/(c1o1+q);
         wallMomentumZ += - (c6o1*c1o216*( VeloX-VeloY+VeloZ))/(c1o1+q);
      }

      q = q_dirBNW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirTSE])[ktse] = f_TSE_in - (c6o1*c1o216*(-VeloX+VeloY-VeloZ))/(c1o1+q);
         wallMomentumX -= - (c6o1*c1o216*(-VeloX+VeloY-VeloZ))/(c1o1+q);
         wallMomentumY += - (c6o1*c1o216*(-VeloX+VeloY-VeloZ))/(c1o1+q);
         wallMomentumZ -= - (c6o1*c1o216*(-VeloX+VeloY-VeloZ))/(c1o1+q);
      }

      q = q_dirBSE[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirTNW])[ktnw] = f_TNW_in - (c6o1*c1o216*( VeloX-VeloY-VeloZ))/(c1o1+q);
         wallMomentumX += - (c6o1*c1o216*( VeloX-VeloY-VeloZ))/(c1o1+q);
         wallMomentumY -= - (c6o1*c1o216*( VeloX-VeloY-VeloZ))/(c1o1+q);
         wallMomentumZ -= - (c6o1*c1o216*( VeloX-VeloY-VeloZ))/(c1o1+q);
      }

      q = q_dirTNW[k];
      if (q>=c0o1 && q<=c1o1)
      {
         (D.f[dirBSE])[kbse] = f_BSE_in - (c6o1*c1o216*(-VeloX+VeloY+VeloZ))/(c1o1+q);
         wallMomentumX -= - (c6o1*c1o216*(-VeloX+VeloY+VeloZ))/(c1o1+q);
         wallMomentumY += - (c6o1*c1o216*(-VeloX+VeloY+VeloZ))/(c1o1+q);
         wallMomentumZ += - (c6o1*c1o216*(-VeloX+VeloY+VeloZ))/(c1o1+q);
      }

      if(false && k==0)
      {     
         printf("FMEM after: \t %1.14f \t %1.14f \t %1.14f  \n \n", wallMomentumX,wallMomentumY,wallMomentumZ );
      } 

   }
}