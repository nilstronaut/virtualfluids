/* Device code */
#include "LBM/LB.h" 
#include "LBM/D3Q27.h"
#include <lbm/constants/NumericConstants.h>

using namespace vf::lbm::constant;


//////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void InitParticles( real* coordX,
										  real* coordY,
										  real* coordZ, 
										  real* coordParticleXlocal,
										  real* coordParticleYlocal,
										  real* coordParticleZlocal,
										  real* coordParticleXglobal,
										  real* coordParticleYglobal,
										  real* coordParticleZglobal,
										  real* veloParticleX,
										  real* veloParticleY,
										  real* veloParticleZ,
										  real* randArray,
										  unsigned int* particleID,
										  unsigned int* cellBaseID,
										  unsigned int* bcMatD,
										  unsigned int* neighborX,
										  unsigned int* neighborY,
										  unsigned int* neighborZ,
										  unsigned int* neighborWSB,
										  int level,
									      unsigned int numberOfParticles, 
										  unsigned int size_Mat)
{
   ////////////////////////////////////////////////////////////////////////////////
   const unsigned  x = threadIdx.x;  // Globaler x-Index 
   const unsigned  y = blockIdx.x;   // Globaler y-Index 
   const unsigned  z = blockIdx.y;   // Globaler z-Index 

   const unsigned nx = blockDim.x;
   const unsigned ny = gridDim.x;

   const unsigned k = nx*(ny*z + y) + x;
   //////////////////////////////////////////////////////////////////////////

   if(k < numberOfParticles)
   {
	 //   real centerX = one;						//uebergabeparameter
	 //   real centerY = 10.5f;					//uebergabeparameter
		//real centerZ = 10.5f;					//uebergabeparameter
		//real diameter = 21.0f;					//uebergabeparameter
		//unsigned int numberOfParticleSizes = 41;	//uebergabeparameter
		//unsigned int nops = (unsigned int)(randArray[k]*numberOfParticleSizes);
		//real xCoordPart = one;
		//real yCoordPart = (real)(randArray[k]*diameter);
		//real zCoordPart = one;
		//if (k==0)
		//{
		//	zCoordPart = (real)(randArray[k+1]*diameter);
		//}
		//else
		//{
		//	zCoordPart = (real)(randArray[k-1]*diameter);
		//}
		//real distance = powf((zCoordPart-centerZ),2) + powf((yCoordPart-centerY),2);
		//real refDistance = powf((diameter*c1o2),2);
		//if (distance > refDistance)
		//{
		//	zCoordPart = sqrtf(powf((diameter*c1o2),2) - powf((yCoordPart-centerY),2)) + centerZ;
		//}



		////////////////////////////////////////////////////////////////////////////////
		//find random node of the fluid domain
		unsigned int cbID = (unsigned int)(randArray[k]*size_Mat);
		for(int i = 0; i < size_Mat;i++)
		{
			//if (coordX[cbID] < 15 && coordX[cbID] > 5 && coordY[cbID] < 15 && coordY[cbID] > 5 && coordZ[cbID] < 15 && coordZ[cbID] > 5)	break;
			if (coordX[cbID] < 5 && coordX[cbID] > 2)	break;
			cbID = (unsigned int)(randArray[k]*(size_Mat - i)); 
		}
	   
		real coordinateX;
		real coordinateY;
		real coordinateZ;

		unsigned int BC  = bcMatD[cbID];
		unsigned int BCx = bcMatD[neighborX[cbID]];
		unsigned int BCy = bcMatD[neighborY[cbID]];
		unsigned int BCz = bcMatD[neighborZ[cbID]];

		if( (BC == GEO_FLUID) && (BCx == GEO_FLUID) && (BCy == GEO_FLUID) && (BCz == GEO_FLUID))
		{
		   coordinateX = coordX[cbID];
		   coordinateY = coordY[cbID];
		   coordinateZ = coordZ[cbID];

		}
		else if(BC == GEO_FLUID)
		{
		   cbID = neighborWSB[neighborWSB[cbID]];
		   coordinateX = coordX[cbID];
		   coordinateY = coordY[cbID];
		   coordinateZ = coordZ[cbID];
		}
		else
		{
		   cbID = neighborZ[neighborY[neighborX[cbID]]];
		   coordinateX = coordX[cbID];
		   coordinateY = coordY[cbID];
		   coordinateZ = coordZ[cbID];
		}


		real localX = randArray[k] / (real)(pow((double)c2o1, (double)level));
        real localY = randArray[k] / (real)(pow((double)c2o1, (double)level));
        real localZ = randArray[k] / (real)(pow((double)c2o1, (double)level));

		real globalX = coordinateX + localX;
		real globalY = coordinateY + localY;
		real globalZ = coordinateZ + localZ;

  		real veloX = c0o1;
		real veloY = c0o1;
		real veloZ = c0o1;

		particleID[k]           = k      ;
		cellBaseID[k]           = cbID   ;
		veloParticleX[k]        = veloX  ;
		veloParticleY[k]        = veloY  ;
		veloParticleZ[k]        = veloZ  ;
		coordParticleXlocal[k]  = localX ;
		coordParticleYlocal[k]  = localY ;
		coordParticleZlocal[k]  = localZ ;
		coordParticleXglobal[k] = globalX;
		coordParticleYglobal[k] = globalY;
		coordParticleZglobal[k] = globalZ;
		////////////////////////////////////////////////////////////////////////////////
   }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


















//////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void MoveParticles( real* coordX,
										  real* coordY,
										  real* coordZ, 
										  real* coordParticleXlocal,
										  real* coordParticleYlocal,
										  real* coordParticleZlocal,
										  real* coordParticleXglobal,
										  real* coordParticleYglobal,
										  real* coordParticleZglobal,
										  real* veloParticleX,
										  real* veloParticleY,
										  real* veloParticleZ,
										  real* DD,
										  real  omega,
										  unsigned int* particleID,
										  unsigned int* cellBaseID,
										  unsigned int* bcMatD,
										  unsigned int* neighborX,
										  unsigned int* neighborY,
										  unsigned int* neighborZ,
										  unsigned int* neighborWSB,
										  int level,
										  unsigned int timestep, 
										  unsigned int numberOfTimesteps, 
									      unsigned int numberOfParticles, 
										  unsigned int size_Mat,
										  bool isEvenTimestep)
{
   ////////////////////////////////////////////////////////////////////////////////
   const unsigned  ix = threadIdx.x;  // Globaler x-Index 
   const unsigned  iy = blockIdx.x;   // Globaler y-Index 
   const unsigned  iz = blockIdx.y;   // Globaler z-Index 

   const unsigned nx = blockDim.x;
   const unsigned ny = gridDim.x;

   const unsigned k = nx*(ny*iz + iy) + ix;
   //////////////////////////////////////////////////////////////////////////

    //real press;
   real vx1,vx2,vx3;
   real drho_SWT,vx1_SWT,vx2_SWT,vx3_SWT;
   real drho_NWT,vx1_NWT,vx2_NWT,vx3_NWT;
   real drho_NET,vx1_NET,vx2_NET,vx3_NET;
   real drho_SET,vx1_SET,vx2_SET,vx3_SET;
   real drho_SWB,vx1_SWB,vx2_SWB,vx3_SWB;
   real drho_NWB,vx1_NWB,vx2_NWB,vx3_NWB;
   real drho_NEB,vx1_NEB,vx2_NEB,vx3_NEB;
   real drho_SEB,vx1_SEB,vx2_SEB,vx3_SEB;
   real f_E,f_W,f_N,f_S,f_T,f_B,f_NE,f_SW,f_SE,f_NW,f_TE,f_BW,f_BE,f_TW,f_TN,f_BS,f_BN,f_TS,f_ZERO,f_TNE, f_TSW, f_TSE, f_TNW, f_BNE, f_BSW, f_BSE, f_BNW;
   real *feC, *fwC, *fnC, *fsC, *ftC, *fbC, *fneC, *fswC, *fseC, *fnwC, *fteC, *fbwC, *fbeC, *ftwC, *ftnC, *fbsC, *fbnC, *ftsC, *fzeroC, *ftneC, *ftswC, *ftseC, *ftnwC, *fbneC, *fbswC, *fbseC, *fbnwC;
   real kxyFromfcNEQ_SWT, kyzFromfcNEQ_SWT, kxzFromfcNEQ_SWT, kxxMyyFromfcNEQ_SWT, kxxMzzFromfcNEQ_SWT;
   real kxyFromfcNEQ_NWT, kyzFromfcNEQ_NWT, kxzFromfcNEQ_NWT, kxxMyyFromfcNEQ_NWT, kxxMzzFromfcNEQ_NWT;
   real kxyFromfcNEQ_NET, kyzFromfcNEQ_NET, kxzFromfcNEQ_NET, kxxMyyFromfcNEQ_NET, kxxMzzFromfcNEQ_NET;
   real kxyFromfcNEQ_SET, kyzFromfcNEQ_SET, kxzFromfcNEQ_SET, kxxMyyFromfcNEQ_SET, kxxMzzFromfcNEQ_SET;
   real kxyFromfcNEQ_SWB, kyzFromfcNEQ_SWB, kxzFromfcNEQ_SWB, kxxMyyFromfcNEQ_SWB, kxxMzzFromfcNEQ_SWB;
   real kxyFromfcNEQ_NWB, kyzFromfcNEQ_NWB, kxzFromfcNEQ_NWB, kxxMyyFromfcNEQ_NWB, kxxMzzFromfcNEQ_NWB;
   real kxyFromfcNEQ_NEB, kyzFromfcNEQ_NEB, kxzFromfcNEQ_NEB, kxxMyyFromfcNEQ_NEB, kxxMzzFromfcNEQ_NEB;
   real kxyFromfcNEQ_SEB, kyzFromfcNEQ_SEB, kxzFromfcNEQ_SEB, kxxMyyFromfcNEQ_SEB, kxxMzzFromfcNEQ_SEB;
   real a0, ax, ay, az, axx, ayy, azz, axy, axz, ayz, b0, bx, by, bz, bxx, byy, bzz, bxy, bxz, byz, c0, cx, cy, cz, cxx, cyy, czz, cxy, cxz, cyz, axyz, bxyz, cxyz;
   //real d0, dx, dy, dz, dxy, dxz, dyz, dxyz;

   real x,y,z;

   if(k < numberOfParticles)
   {
		/////////////////////////////////////////////////////////////
	    unsigned int kTimeStep = k + (timestep * numberOfParticles);
		/////////////////////////////////////////////////////////////
		unsigned int kCellBaseID = cellBaseID[k];
	    unsigned int BC000  = bcMatD[kCellBaseID];
	    unsigned int BCx00  = bcMatD[neighborX[kCellBaseID]];
	    unsigned int BC0y0  = bcMatD[neighborY[kCellBaseID]];
	    unsigned int BC00z  = bcMatD[neighborZ[kCellBaseID]];
	    unsigned int BCxy0  = bcMatD[neighborY[neighborX[kCellBaseID]]];
	    unsigned int BCx0z  = bcMatD[neighborZ[neighborX[kCellBaseID]]];
	    unsigned int BC0yz  = bcMatD[neighborZ[neighborY[kCellBaseID]]];
	    unsigned int BCxyz  = bcMatD[neighborZ[neighborY[neighborX[kCellBaseID]]]];
		/////////////////////////////////////////////////////////////
   		if( (BC000 >= GEO_FLUID) || 
			(BCx00 >= GEO_FLUID) || 
			(BC0y0 >= GEO_FLUID) || 
			(BC00z >= GEO_FLUID) || 
			(BCxy0 >= GEO_FLUID) || 
			(BCx0z >= GEO_FLUID) || 
			(BC0yz >= GEO_FLUID) || 
			(BCxyz >= GEO_FLUID) )
		{
		   if (isEvenTimestep==true)
		   {
			  feC    = &DD[E   *size_Mat];
			  fwC    = &DD[W   *size_Mat];
			  fnC    = &DD[N   *size_Mat];
			  fsC    = &DD[S   *size_Mat];
			  ftC    = &DD[T   *size_Mat];
			  fbC    = &DD[B   *size_Mat];
			  fneC   = &DD[NE  *size_Mat];
			  fswC   = &DD[SW  *size_Mat];
			  fseC   = &DD[SE  *size_Mat];
			  fnwC   = &DD[NW  *size_Mat];
			  fteC   = &DD[TE  *size_Mat];
			  fbwC   = &DD[BW  *size_Mat];
			  fbeC   = &DD[BE  *size_Mat];
			  ftwC   = &DD[TW  *size_Mat];
			  ftnC   = &DD[TN  *size_Mat];
			  fbsC   = &DD[BS  *size_Mat];
			  fbnC   = &DD[BN  *size_Mat];
			  ftsC   = &DD[TS  *size_Mat];
			  fzeroC = &DD[dirREST*size_Mat];
			  ftneC  = &DD[TNE *size_Mat];
			  ftswC  = &DD[TSW *size_Mat];
			  ftseC  = &DD[TSE *size_Mat];
			  ftnwC  = &DD[TNW *size_Mat];
			  fbneC  = &DD[BNE *size_Mat];
			  fbswC  = &DD[BSW *size_Mat];
			  fbseC  = &DD[BSE *size_Mat];
			  fbnwC  = &DD[BNW *size_Mat];
		   } 			 
		   else			 
		   {			 
			  fwC    = &DD[E   *size_Mat];
			  feC    = &DD[W   *size_Mat];
			  fsC    = &DD[N   *size_Mat];
			  fnC    = &DD[S   *size_Mat];
			  fbC    = &DD[T   *size_Mat];
			  ftC    = &DD[B   *size_Mat];
			  fswC   = &DD[NE  *size_Mat];
			  fneC   = &DD[SW  *size_Mat];
			  fnwC   = &DD[SE  *size_Mat];
			  fseC   = &DD[NW  *size_Mat];
			  fbwC   = &DD[TE  *size_Mat];
			  fteC   = &DD[BW  *size_Mat];
			  ftwC   = &DD[BE  *size_Mat];
			  fbeC   = &DD[TW  *size_Mat];
			  fbsC   = &DD[TN  *size_Mat];
			  ftnC   = &DD[BS  *size_Mat];
			  ftsC   = &DD[BN  *size_Mat];
			  fbnC   = &DD[TS  *size_Mat];
			  fzeroC = &DD[dirREST*size_Mat];
			  fbswC  = &DD[TNE *size_Mat];
			  fbneC  = &DD[TSW *size_Mat];
			  fbnwC  = &DD[TSE *size_Mat];
			  fbseC  = &DD[TNW *size_Mat];
			  ftswC  = &DD[BNE *size_Mat];
			  ftneC  = &DD[BSW *size_Mat];
			  ftnwC  = &DD[BSE *size_Mat];
			  ftseC  = &DD[BNW *size_Mat];
		   }

			  //////////////////////////////////////////////////////////////////////////
			  //SWB//
			  //////////////////////////////////////////////////////////////////////////
			  //index 0
			  unsigned int k0zero= cellBaseID[k];
			  unsigned int k0w   = neighborX[k0zero];
			  unsigned int k0s   = neighborY[k0zero];
			  unsigned int k0b   = neighborZ[k0zero];
			  unsigned int k0sw  = neighborY[k0w];
			  unsigned int k0bw  = neighborZ[k0w];
			  unsigned int k0bs  = neighborZ[k0s];
			  unsigned int k0bsw = neighborZ[k0sw];
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  unsigned int kzero= k0zero;
			  unsigned int kw   = k0w;   
			  unsigned int ks   = k0s;   
			  unsigned int kb   = k0b;   
			  unsigned int ksw  = k0sw;  
			  unsigned int kbw  = k0bw;  
			  unsigned int kbs  = k0bs;  
			  unsigned int kbsw = k0bsw; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_SWB = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_SWB  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_SWB);
			  vx2_SWB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_SWB);
			  vx3_SWB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_SWB);

			  kxyFromfcNEQ_SWB    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_SWB) - ((vx1_SWB*vx2_SWB)));
			  kyzFromfcNEQ_SWB    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_SWB) - ((vx2_SWB*vx3_SWB)));
			  kxzFromfcNEQ_SWB    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_SWB) - ((vx1_SWB*vx3_SWB)));
			  kxxMyyFromfcNEQ_SWB = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_SWB) - ((vx1_SWB*vx1_SWB-vx2_SWB*vx2_SWB)));
			  kxxMzzFromfcNEQ_SWB = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_SWB) - ((vx1_SWB*vx1_SWB-vx3_SWB*vx3_SWB)));

			  //////////////////////////////////////////////////////////////////////////
			  //SWT//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= kb;
			  kw   = kbw;   
			  ks   = kbs;   
			  kb   = neighborZ[kb];   
			  ksw  = kbsw;  
			  kbw  = neighborZ[kbw];  
			  kbs  = neighborZ[kbs];  
			  kbsw = neighborZ[kbsw]; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_SWT = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_SWT  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_SWT);
			  vx2_SWT  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_SWT);
			  vx3_SWT  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_SWT);

			  kxyFromfcNEQ_SWT    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_SWT) - ((vx1_SWT*vx2_SWT)));
			  kyzFromfcNEQ_SWT    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_SWT) - ((vx2_SWT*vx3_SWT)));
			  kxzFromfcNEQ_SWT    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_SWT) - ((vx1_SWT*vx3_SWT)));
			  kxxMyyFromfcNEQ_SWT = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_SWT) - ((vx1_SWT*vx1_SWT-vx2_SWT*vx2_SWT)));
			  kxxMzzFromfcNEQ_SWT = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_SWT) - ((vx1_SWT*vx1_SWT-vx3_SWT*vx3_SWT)));

			  //////////////////////////////////////////////////////////////////////////
			  //SET//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= kw;
			  kw   = neighborX[kw];   
			  ks   = ksw;   
			  kb   = kbw;   
			  ksw  = neighborX[ksw];  
			  kbw  = neighborX[kbw];  
			  kbs  = kbsw;  
			  kbsw = neighborX[kbsw]; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_SET = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_SET  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_SET);
			  vx2_SET  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_SET);
			  vx3_SET  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_SET);

			  kxyFromfcNEQ_SET    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_SET) - ((vx1_SET*vx2_SET)));
			  kyzFromfcNEQ_SET    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_SET) - ((vx2_SET*vx3_SET)));
			  kxzFromfcNEQ_SET    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_SET) - ((vx1_SET*vx3_SET)));
			  kxxMyyFromfcNEQ_SET = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_SET) - ((vx1_SET*vx1_SET-vx2_SET*vx2_SET)));
			  kxxMzzFromfcNEQ_SET = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_SET) - ((vx1_SET*vx1_SET-vx3_SET*vx3_SET)));

			  //////////////////////////////////////////////////////////////////////////
			  //SEB//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kb   = kzero;   
			  kbw  = kw;  
			  kbs  = ks;  
			  kbsw = ksw; 
			  kzero= k0w;
			  kw   = neighborX[k0w];   
			  ks   = k0sw;   
			  ksw  = neighborX[k0sw];  
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_SEB = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_SEB  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_SEB);
			  vx2_SEB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_SEB);
			  vx3_SEB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_SEB);

			  kxyFromfcNEQ_SEB    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_SEB) - ((vx1_SEB*vx2_SEB)));
			  kyzFromfcNEQ_SEB    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_SEB) - ((vx2_SEB*vx3_SEB)));
			  kxzFromfcNEQ_SEB    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_SEB) - ((vx1_SEB*vx3_SEB)));
			  kxxMyyFromfcNEQ_SEB = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_SEB) - ((vx1_SEB*vx1_SEB-vx2_SEB*vx2_SEB)));
			  kxxMzzFromfcNEQ_SEB = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_SEB) - ((vx1_SEB*vx1_SEB-vx3_SEB*vx3_SEB)));

			  //////////////////////////////////////////////////////////////////////////
			  //NWB//
			  //////////////////////////////////////////////////////////////////////////
			  //index 0
			  k0zero= k0s;
			  k0w   = k0sw;
			  k0s   = neighborY[k0s];
			  k0b   = k0bs;
			  k0sw  = neighborY[k0sw];
			  k0bw  = k0bsw;
			  k0bs  = neighborY[k0bs];
			  k0bsw = neighborY[k0bsw];
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= k0zero;
			  kw   = k0w;   
			  ks   = k0s;   
			  kb   = k0b;   
			  ksw  = k0sw;  
			  kbw  = k0bw;  
			  kbs  = k0bs;  
			  kbsw = k0bsw; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_NWB = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_NWB  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_NWB);
			  vx2_NWB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_NWB);
			  vx3_NWB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_NWB);

			  kxyFromfcNEQ_NWB    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_NWB) - ((vx1_NWB*vx2_NWB)));
			  kyzFromfcNEQ_NWB    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_NWB) - ((vx2_NWB*vx3_NWB)));
			  kxzFromfcNEQ_NWB    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_NWB) - ((vx1_NWB*vx3_NWB)));
			  kxxMyyFromfcNEQ_NWB = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_NWB) - ((vx1_NWB*vx1_NWB-vx2_NWB*vx2_NWB)));
			  kxxMzzFromfcNEQ_NWB = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_NWB) - ((vx1_NWB*vx1_NWB-vx3_NWB*vx3_NWB)));

			  //////////////////////////////////////////////////////////////////////////
			  //NWT//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= kb;
			  kw   = kbw;   
			  ks   = kbs;   
			  kb   = neighborZ[kb];   
			  ksw  = kbsw;  
			  kbw  = neighborZ[kbw];  
			  kbs  = neighborZ[kbs];  
			  kbsw = neighborZ[kbsw]; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_NWT = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_NWT  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_NWT);
			  vx2_NWT  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_NWT);
			  vx3_NWT  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_NWT);

			  kxyFromfcNEQ_NWT    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_NWT) - ((vx1_NWT*vx2_NWT)));
			  kyzFromfcNEQ_NWT    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_NWT) - ((vx2_NWT*vx3_NWT)));
			  kxzFromfcNEQ_NWT    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_NWT) - ((vx1_NWT*vx3_NWT)));
			  kxxMyyFromfcNEQ_NWT = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_NWT) - ((vx1_NWT*vx1_NWT-vx2_NWT*vx2_NWT)));
			  kxxMzzFromfcNEQ_NWT = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_NWT) - ((vx1_NWT*vx1_NWT-vx3_NWT*vx3_NWT)));

			  //////////////////////////////////////////////////////////////////////////
			  //NET//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= kw;
			  kw   = neighborX[kw];   
			  ks   = ksw;   
			  kb   = kbw;   
			  ksw  = neighborX[ksw];  
			  kbw  = neighborX[kbw];  
			  kbs  = kbsw;  
			  kbsw = neighborX[kbsw]; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_NET = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_NET  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_NET);
			  vx2_NET  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_NET);
			  vx3_NET  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_NET);

			  kxyFromfcNEQ_NET    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_NET) - ((vx1_NET*vx2_NET)));
			  kyzFromfcNEQ_NET    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_NET) - ((vx2_NET*vx3_NET)));
			  kxzFromfcNEQ_NET    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_NET) - ((vx1_NET*vx3_NET)));
			  kxxMyyFromfcNEQ_NET = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_NET) - ((vx1_NET*vx1_NET-vx2_NET*vx2_NET)));
			  kxxMzzFromfcNEQ_NET = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_NET) - ((vx1_NET*vx1_NET-vx3_NET*vx3_NET)));

			  //////////////////////////////////////////////////////////////////////////
			  //NEB//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kb   = kzero;   
			  kbw  = kw;  
			  kbs  = ks;  
			  kbsw = ksw; 
			  kzero= k0w;
			  kw   = neighborX[k0w];   
			  ks   = k0sw;   
			  ksw  = neighborX[k0sw];  
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_NEB = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_NEB  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_NEB);
			  vx2_NEB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_NEB);
			  vx3_NEB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_NEB);

			  kxyFromfcNEQ_NEB    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_NEB) - ((vx1_NEB*vx2_NEB)));
			  kyzFromfcNEQ_NEB    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_NEB) - ((vx2_NEB*vx3_NEB)));
			  kxzFromfcNEQ_NEB    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_NEB) - ((vx1_NEB*vx3_NEB)));
			  kxxMyyFromfcNEQ_NEB = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_NEB) - ((vx1_NEB*vx1_NEB-vx2_NEB*vx2_NEB)));
			  kxxMzzFromfcNEQ_NEB = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_NEB) - ((vx1_NEB*vx1_NEB-vx3_NEB*vx3_NEB)));

			  //////////////////////////////////////////////////////////////////////////
			  //interpolate
			  //////////////////////////////////////////////////////////////////////////
			  a0 = (-kxxMyyFromfcNEQ_NEB - kxxMyyFromfcNEQ_NET + kxxMyyFromfcNEQ_NWB + kxxMyyFromfcNEQ_NWT - 
				 kxxMyyFromfcNEQ_SEB - kxxMyyFromfcNEQ_SET + kxxMyyFromfcNEQ_SWB + kxxMyyFromfcNEQ_SWT - 
				 kxxMzzFromfcNEQ_NEB - kxxMzzFromfcNEQ_NET + kxxMzzFromfcNEQ_NWB + kxxMzzFromfcNEQ_NWT - 
				 kxxMzzFromfcNEQ_SEB - kxxMzzFromfcNEQ_SET + kxxMzzFromfcNEQ_SWB + kxxMzzFromfcNEQ_SWT - 
				 c2o1*kxyFromfcNEQ_NEB - c2o1*kxyFromfcNEQ_NET - c2o1*kxyFromfcNEQ_NWB - c2o1*kxyFromfcNEQ_NWT + 
				 c2o1*kxyFromfcNEQ_SEB + c2o1*kxyFromfcNEQ_SET + c2o1*kxyFromfcNEQ_SWB + c2o1*kxyFromfcNEQ_SWT + 
				 c2o1*kxzFromfcNEQ_NEB - c2o1*kxzFromfcNEQ_NET + c2o1*kxzFromfcNEQ_NWB - c2o1*kxzFromfcNEQ_NWT + 
				 c2o1*kxzFromfcNEQ_SEB - c2o1*kxzFromfcNEQ_SET + c2o1*kxzFromfcNEQ_SWB - c2o1*kxzFromfcNEQ_SWT + 
				 c8o1*vx1_NEB + c8o1*vx1_NET + c8o1*vx1_NWB + c8o1*vx1_NWT + c8o1*vx1_SEB + 
				 c8o1*vx1_SET + c8o1*vx1_SWB + c8o1*vx1_SWT + c2o1*vx2_NEB + c2o1*vx2_NET - 
				 c2o1*vx2_NWB - c2o1*vx2_NWT - c2o1*vx2_SEB - c2o1*vx2_SET + c2o1*vx2_SWB + 
				 c2o1*vx2_SWT - c2o1*vx3_NEB + c2o1*vx3_NET + c2o1*vx3_NWB - c2o1*vx3_NWT - 
				 c2o1*vx3_SEB + c2o1*vx3_SET + c2o1*vx3_SWB - c2o1*vx3_SWT)/c64o1;
			  b0 = (c2o1*kxxMyyFromfcNEQ_NEB + c2o1*kxxMyyFromfcNEQ_NET + c2o1*kxxMyyFromfcNEQ_NWB + c2o1*kxxMyyFromfcNEQ_NWT - 
				 c2o1*kxxMyyFromfcNEQ_SEB - c2o1*kxxMyyFromfcNEQ_SET - c2o1*kxxMyyFromfcNEQ_SWB - c2o1*kxxMyyFromfcNEQ_SWT - 
				 kxxMzzFromfcNEQ_NEB - kxxMzzFromfcNEQ_NET - kxxMzzFromfcNEQ_NWB - kxxMzzFromfcNEQ_NWT + 
				 kxxMzzFromfcNEQ_SEB + kxxMzzFromfcNEQ_SET + kxxMzzFromfcNEQ_SWB + kxxMzzFromfcNEQ_SWT - 
				 c2o1*kxyFromfcNEQ_NEB - c2o1*kxyFromfcNEQ_NET + c2o1*kxyFromfcNEQ_NWB + c2o1*kxyFromfcNEQ_NWT - 
				 c2o1*kxyFromfcNEQ_SEB - c2o1*kxyFromfcNEQ_SET + c2o1*kxyFromfcNEQ_SWB + c2o1*kxyFromfcNEQ_SWT + 
				 c2o1*kyzFromfcNEQ_NEB - c2o1*kyzFromfcNEQ_NET + c2o1*kyzFromfcNEQ_NWB - c2o1*kyzFromfcNEQ_NWT + 
				 c2o1*kyzFromfcNEQ_SEB - c2o1*kyzFromfcNEQ_SET + c2o1*kyzFromfcNEQ_SWB - c2o1*kyzFromfcNEQ_SWT + 
				 c2o1*vx1_NEB + c2o1*vx1_NET - c2o1*vx1_NWB - c2o1*vx1_NWT - 
				 c2o1*vx1_SEB - c2o1*vx1_SET + c2o1*vx1_SWB + c2o1*vx1_SWT + 
				 c8o1*vx2_NEB + c8o1*vx2_NET + c8o1*vx2_NWB + c8o1*vx2_NWT + 
				 c8o1*vx2_SEB + c8o1*vx2_SET + c8o1*vx2_SWB + c8o1*vx2_SWT - 
				 c2o1*vx3_NEB + c2o1*vx3_NET - c2o1*vx3_NWB + c2o1*vx3_NWT + 
				 c2o1*vx3_SEB - c2o1*vx3_SET + c2o1*vx3_SWB - c2o1*vx3_SWT)/c64o1;

			  //b0 = ((eight*vx2_NEB + eight*vx2_SWT) + (eight*vx2_NET + eight*vx2_SWB) + (eight*vx2_NWB + eight*vx2_SET) + (eight*vx2_NWT + eight*vx2_SEB))/sixtyfour;

			  c0 = (kxxMyyFromfcNEQ_NEB - kxxMyyFromfcNEQ_NET + kxxMyyFromfcNEQ_NWB - kxxMyyFromfcNEQ_NWT + 
				 kxxMyyFromfcNEQ_SEB - kxxMyyFromfcNEQ_SET + kxxMyyFromfcNEQ_SWB - kxxMyyFromfcNEQ_SWT - 
				 c2o1*kxxMzzFromfcNEQ_NEB + c2o1*kxxMzzFromfcNEQ_NET - c2o1*kxxMzzFromfcNEQ_NWB + c2o1*kxxMzzFromfcNEQ_NWT - 
				 c2o1*kxxMzzFromfcNEQ_SEB + c2o1*kxxMzzFromfcNEQ_SET - c2o1*kxxMzzFromfcNEQ_SWB + c2o1*kxxMzzFromfcNEQ_SWT - 
				 c2o1*kxzFromfcNEQ_NEB - c2o1*kxzFromfcNEQ_NET + c2o1*kxzFromfcNEQ_NWB + c2o1*kxzFromfcNEQ_NWT - 
				 c2o1*kxzFromfcNEQ_SEB - c2o1*kxzFromfcNEQ_SET + c2o1*kxzFromfcNEQ_SWB + c2o1*kxzFromfcNEQ_SWT - 
				 c2o1*kyzFromfcNEQ_NEB - c2o1*kyzFromfcNEQ_NET - c2o1*kyzFromfcNEQ_NWB - c2o1*kyzFromfcNEQ_NWT + 
				 c2o1*kyzFromfcNEQ_SEB + c2o1*kyzFromfcNEQ_SET + c2o1*kyzFromfcNEQ_SWB + c2o1*kyzFromfcNEQ_SWT - 
				 c2o1*vx1_NEB + c2o1*vx1_NET + c2o1*vx1_NWB - c2o1*vx1_NWT - 
				 c2o1*vx1_SEB + c2o1*vx1_SET + c2o1*vx1_SWB - c2o1*vx1_SWT - 
				 c2o1*vx2_NEB + c2o1*vx2_NET - c2o1*vx2_NWB + c2o1*vx2_NWT + 
				 c2o1*vx2_SEB - c2o1*vx2_SET + c2o1*vx2_SWB - c2o1*vx2_SWT + 
				 c8o1*vx3_NEB + c8o1*vx3_NET + c8o1*vx3_NWB + c8o1*vx3_NWT + 
				 c8o1*vx3_SEB + c8o1*vx3_SET + c8o1*vx3_SWB + c8o1*vx3_SWT)/c64o1;
			  ax = (vx1_NEB + vx1_NET - vx1_NWB - vx1_NWT + vx1_SEB + vx1_SET - vx1_SWB - vx1_SWT)/c4o1;
			  bx = (vx2_NEB + vx2_NET - vx2_NWB - vx2_NWT + vx2_SEB + vx2_SET - vx2_SWB - vx2_SWT)/c4o1;
			  //bx = ((vx2_NEB - vx2_SWT) + (vx2_NET - vx2_SWB) + (vx2_SET - vx2_NWB) + (vx2_SEB - vx2_NWT))/four;
			  cx = (vx3_NEB + vx3_NET - vx3_NWB - vx3_NWT + vx3_SEB + vx3_SET - vx3_SWB - vx3_SWT)/c4o1;
			  axx= (kxxMyyFromfcNEQ_NEB + kxxMyyFromfcNEQ_NET - kxxMyyFromfcNEQ_NWB - kxxMyyFromfcNEQ_NWT + 
				 kxxMyyFromfcNEQ_SEB + kxxMyyFromfcNEQ_SET - kxxMyyFromfcNEQ_SWB - kxxMyyFromfcNEQ_SWT + 
				 kxxMzzFromfcNEQ_NEB + kxxMzzFromfcNEQ_NET - kxxMzzFromfcNEQ_NWB - kxxMzzFromfcNEQ_NWT + 
				 kxxMzzFromfcNEQ_SEB + kxxMzzFromfcNEQ_SET - kxxMzzFromfcNEQ_SWB - kxxMzzFromfcNEQ_SWT + 
				 c2o1*vx2_NEB + c2o1*vx2_NET - c2o1*vx2_NWB - c2o1*vx2_NWT - 
				 c2o1*vx2_SEB - c2o1*vx2_SET + c2o1*vx2_SWB + c2o1*vx2_SWT - 
				 c2o1*vx3_NEB + c2o1*vx3_NET + c2o1*vx3_NWB - c2o1*vx3_NWT - 
				 c2o1*vx3_SEB + c2o1*vx3_SET + c2o1*vx3_SWB - c2o1*vx3_SWT)/c16o1;
			  bxx= (kxyFromfcNEQ_NEB + kxyFromfcNEQ_NET - kxyFromfcNEQ_NWB - kxyFromfcNEQ_NWT + 
				 kxyFromfcNEQ_SEB + kxyFromfcNEQ_SET - kxyFromfcNEQ_SWB - kxyFromfcNEQ_SWT - 
				 c2o1*vx1_NEB - c2o1*vx1_NET + c2o1*vx1_NWB + c2o1*vx1_NWT + 
				 c2o1*vx1_SEB + c2o1*vx1_SET - c2o1*vx1_SWB - c2o1*vx1_SWT)/c8o1;
			  cxx= (kxzFromfcNEQ_NEB + kxzFromfcNEQ_NET - kxzFromfcNEQ_NWB - kxzFromfcNEQ_NWT + 
				 kxzFromfcNEQ_SEB + kxzFromfcNEQ_SET - kxzFromfcNEQ_SWB - kxzFromfcNEQ_SWT + 
				 c2o1*vx1_NEB - c2o1*vx1_NET - c2o1*vx1_NWB + c2o1*vx1_NWT + 
				 c2o1*vx1_SEB - c2o1*vx1_SET - c2o1*vx1_SWB + c2o1*vx1_SWT)/c8o1;
			  ay = (vx1_NEB + vx1_NET + vx1_NWB + vx1_NWT - vx1_SEB - vx1_SET - vx1_SWB - vx1_SWT)/c4o1;
			  by = (vx2_NEB + vx2_NET + vx2_NWB + vx2_NWT - vx2_SEB - vx2_SET - vx2_SWB - vx2_SWT)/c4o1;
			  cy = (vx3_NEB + vx3_NET + vx3_NWB + vx3_NWT - vx3_SEB - vx3_SET - vx3_SWB - vx3_SWT)/c4o1;
			  ayy= (kxyFromfcNEQ_NEB + kxyFromfcNEQ_NET + kxyFromfcNEQ_NWB + kxyFromfcNEQ_NWT - 
				 kxyFromfcNEQ_SEB - kxyFromfcNEQ_SET - kxyFromfcNEQ_SWB - kxyFromfcNEQ_SWT - 
				 c2o1*vx2_NEB - c2o1*vx2_NET + c2o1*vx2_NWB + c2o1*vx2_NWT + 
				 c2o1*vx2_SEB + c2o1*vx2_SET - c2o1*vx2_SWB - c2o1*vx2_SWT)/c8o1;
			  byy= (-c2o1*kxxMyyFromfcNEQ_NEB - c2o1*kxxMyyFromfcNEQ_NET - c2o1*kxxMyyFromfcNEQ_NWB - c2o1*kxxMyyFromfcNEQ_NWT + 
				 c2o1*kxxMyyFromfcNEQ_SEB + c2o1*kxxMyyFromfcNEQ_SET + c2o1*kxxMyyFromfcNEQ_SWB + c2o1*kxxMyyFromfcNEQ_SWT + 
				 kxxMzzFromfcNEQ_NEB + kxxMzzFromfcNEQ_NET + kxxMzzFromfcNEQ_NWB + kxxMzzFromfcNEQ_NWT - 
				 kxxMzzFromfcNEQ_SEB - kxxMzzFromfcNEQ_SET - kxxMzzFromfcNEQ_SWB - kxxMzzFromfcNEQ_SWT + 
				 c2o1*vx1_NEB + c2o1*vx1_NET - c2o1*vx1_NWB - c2o1*vx1_NWT - 
				 c2o1*vx1_SEB - c2o1*vx1_SET + c2o1*vx1_SWB + c2o1*vx1_SWT - 
				 c2o1*vx3_NEB + c2o1*vx3_NET - c2o1*vx3_NWB + c2o1*vx3_NWT + 
				 c2o1*vx3_SEB - c2o1*vx3_SET + c2o1*vx3_SWB - c2o1*vx3_SWT)/c16o1;
			  cyy= (kyzFromfcNEQ_NEB + kyzFromfcNEQ_NET + kyzFromfcNEQ_NWB + kyzFromfcNEQ_NWT - 
				 kyzFromfcNEQ_SEB - kyzFromfcNEQ_SET - kyzFromfcNEQ_SWB - kyzFromfcNEQ_SWT + 
				 c2o1*vx2_NEB - c2o1*vx2_NET + c2o1*vx2_NWB - c2o1*vx2_NWT - 
				 c2o1*vx2_SEB + c2o1*vx2_SET - c2o1*vx2_SWB + c2o1*vx2_SWT)/c8o1;
			  az = (-vx1_NEB + vx1_NET - vx1_NWB + vx1_NWT - vx1_SEB + vx1_SET - vx1_SWB + vx1_SWT)/c4o1;
			  //bz = (-vx2_NEB + vx2_NET - vx2_NWB + vx2_NWT - vx2_SEB + vx2_SET - vx2_SWB + vx2_SWT)/four;
			  bz = ((vx2_SWT - vx2_NEB) + (vx2_NET - vx2_SWB) + (vx2_SET - vx2_NWB) + (vx2_NWT - vx2_SEB))/c4o1;
			  cz = (-vx3_NEB + vx3_NET - vx3_NWB + vx3_NWT - vx3_SEB + vx3_SET - vx3_SWB + vx3_SWT)/c4o1;
			  azz= (-kxzFromfcNEQ_NEB + kxzFromfcNEQ_NET - kxzFromfcNEQ_NWB + kxzFromfcNEQ_NWT - 
				 kxzFromfcNEQ_SEB + kxzFromfcNEQ_SET - kxzFromfcNEQ_SWB + kxzFromfcNEQ_SWT + 
				 c2o1*vx3_NEB - c2o1*vx3_NET - c2o1*vx3_NWB + c2o1*vx3_NWT + 
				 c2o1*vx3_SEB - c2o1*vx3_SET - c2o1*vx3_SWB + c2o1*vx3_SWT)/c8o1;
			  bzz= (-kyzFromfcNEQ_NEB + kyzFromfcNEQ_NET - kyzFromfcNEQ_NWB + kyzFromfcNEQ_NWT - 
				 kyzFromfcNEQ_SEB + kyzFromfcNEQ_SET - kyzFromfcNEQ_SWB + kyzFromfcNEQ_SWT + 
				 c2o1*vx3_NEB - c2o1*vx3_NET + c2o1*vx3_NWB - c2o1*vx3_NWT - 
				 c2o1*vx3_SEB + c2o1*vx3_SET - c2o1*vx3_SWB + c2o1*vx3_SWT)/c8o1;
			  czz= (-kxxMyyFromfcNEQ_NEB + kxxMyyFromfcNEQ_NET - kxxMyyFromfcNEQ_NWB + kxxMyyFromfcNEQ_NWT - 
				 kxxMyyFromfcNEQ_SEB + kxxMyyFromfcNEQ_SET - kxxMyyFromfcNEQ_SWB + kxxMyyFromfcNEQ_SWT + 
				 c2o1*kxxMzzFromfcNEQ_NEB - c2o1*kxxMzzFromfcNEQ_NET + c2o1*kxxMzzFromfcNEQ_NWB - c2o1*kxxMzzFromfcNEQ_NWT + 
				 c2o1*kxxMzzFromfcNEQ_SEB - c2o1*kxxMzzFromfcNEQ_SET + c2o1*kxxMzzFromfcNEQ_SWB - c2o1*kxxMzzFromfcNEQ_SWT - 
				 c2o1*vx1_NEB + c2o1*vx1_NET + c2o1*vx1_NWB - c2o1*vx1_NWT - 
				 c2o1*vx1_SEB + c2o1*vx1_SET + c2o1*vx1_SWB - c2o1*vx1_SWT - 
				 c2o1*vx2_NEB + c2o1*vx2_NET - c2o1*vx2_NWB + c2o1*vx2_NWT + 
				 c2o1*vx2_SEB - c2o1*vx2_SET + c2o1*vx2_SWB - c2o1*vx2_SWT)/c16o1;
			  axy= (vx1_NEB + vx1_NET - vx1_NWB - vx1_NWT - vx1_SEB - vx1_SET + vx1_SWB + vx1_SWT)/c2o1;
			  bxy= (vx2_NEB + vx2_NET - vx2_NWB - vx2_NWT - vx2_SEB - vx2_SET + vx2_SWB + vx2_SWT)/c2o1;
			  cxy= (vx3_NEB + vx3_NET - vx3_NWB - vx3_NWT - vx3_SEB - vx3_SET + vx3_SWB + vx3_SWT)/c2o1;
			  axz= (-vx1_NEB + vx1_NET + vx1_NWB - vx1_NWT - vx1_SEB + vx1_SET + vx1_SWB - vx1_SWT)/c2o1;
			  bxz= (-vx2_NEB + vx2_NET + vx2_NWB - vx2_NWT - vx2_SEB + vx2_SET + vx2_SWB - vx2_SWT)/c2o1;
			  cxz= (-vx3_NEB + vx3_NET + vx3_NWB - vx3_NWT - vx3_SEB + vx3_SET + vx3_SWB - vx3_SWT)/c2o1;
			  ayz= (-vx1_NEB + vx1_NET - vx1_NWB + vx1_NWT + vx1_SEB - vx1_SET + vx1_SWB - vx1_SWT)/c2o1;
			  byz= (-vx2_NEB + vx2_NET - vx2_NWB + vx2_NWT + vx2_SEB - vx2_SET + vx2_SWB - vx2_SWT)/c2o1;
			  cyz= (-vx3_NEB + vx3_NET - vx3_NWB + vx3_NWT + vx3_SEB - vx3_SET + vx3_SWB - vx3_SWT)/c2o1;
			  axyz=-vx1_NEB + vx1_NET + vx1_NWB - vx1_NWT + vx1_SEB - vx1_SET - vx1_SWB + vx1_SWT;
			  bxyz=-vx2_NEB + vx2_NET + vx2_NWB - vx2_NWT + vx2_SEB - vx2_SET - vx2_SWB + vx2_SWT;
			  cxyz=-vx3_NEB + vx3_NET + vx3_NWB - vx3_NWT + vx3_SEB - vx3_SET - vx3_SWB + vx3_SWT;
			  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  //drho
			//   d0   = ( drho_NEB + drho_NET + drho_NWB + drho_NWT + drho_SEB + drho_SET + drho_SWB + drho_SWT) * c1o8;
			//   dx   = ( drho_NEB + drho_NET - drho_NWB - drho_NWT + drho_SEB + drho_SET - drho_SWB - drho_SWT) * c1o4;
			//   dy   = ( drho_NEB + drho_NET + drho_NWB + drho_NWT - drho_SEB - drho_SET - drho_SWB - drho_SWT) * c1o4;
			//   dz   = (-drho_NEB + drho_NET - drho_NWB + drho_NWT - drho_SEB + drho_SET - drho_SWB + drho_SWT) * c1o4;
			//   dxy  = ( drho_NEB + drho_NET - drho_NWB - drho_NWT - drho_SEB - drho_SET + drho_SWB + drho_SWT) * c1o2;
			//   dxz  = (-drho_NEB + drho_NET + drho_NWB - drho_NWT - drho_SEB + drho_SET + drho_SWB - drho_SWT) * c1o2;
			//   dyz  = (-drho_NEB + drho_NET - drho_NWB + drho_NWT + drho_SEB - drho_SET + drho_SWB - drho_SWT) * c1o2;
			//   dxyz =  -drho_NEB + drho_NET + drho_NWB - drho_NWT + drho_SEB - drho_SET - drho_SWB + drho_SWT;
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  unsigned int kTimeStepOld = kTimeStep - numberOfParticles;
			  real localX = coordParticleXlocal[kTimeStepOld];
			  real localY = coordParticleYlocal[kTimeStepOld];
			  real localZ = coordParticleZlocal[kTimeStepOld];

			  x = (localX * (real)(pow((double)c2o1, (double)level))) - c1o2; //-c1o4;
              y = (localY * (real)(pow((double)c2o1, (double)level))) - c1o2; //-c1o4;
              z = (localZ * (real)(pow((double)c2o1, (double)level))) - c1o2; //-c1o4;
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  //press = d0 + x*dx + y*dy + z*dz + x*y*dxy + x*z*dxz + y*z*dyz + x*y*z*dxyz;
			  vx1 = (a0 + x*ax + y*ay + z*az + x*x*axx + y*y*ayy + z*z*azz + x*y*axy + x*z*axz + y*z*ayz + x*y*z*axyz);
			  vx2 = (b0 + x*bx + y*by + z*bz + x*x*bxx + y*y*byy + z*z*bzz + x*y*bxy + x*z*bxz + y*z*byz + x*y*z*bxyz);
			  vx3 = (c0 + x*cx + y*cy + z*cz + x*x*cxx + y*y*cyy + z*z*czz + x*y*cxy + x*z*cxz + y*z*cyz + x*y*z*cxyz);

			  real veloPreX = veloParticleX[kTimeStepOld];
			  real veloPreY = veloParticleY[kTimeStepOld];
			  real veloPreZ = veloParticleZ[kTimeStepOld];

			  real veloPostX = (veloPreX + vx1) * c1o2;
			  real veloPostY = (veloPreY + vx2) * c1o2;
			  real veloPostZ = (veloPreZ + vx3) * c1o2;

			  //real veloPostX = vx1;
			  //real veloPostY = vx2;
			  //real veloPostZ = vx3;

			  veloParticleX[kTimeStep] = veloPostX;
			  veloParticleY[kTimeStep] = veloPostY;
			  veloParticleZ[kTimeStep] = veloPostZ;
			  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  x = x + veloPostX;
			  //x = x + c1o3;
			  y = y + veloPostY;
			  z = z + veloPostZ;

			  unsigned int cbID = cellBaseID[k];
			  bool negativeDirection = false;

			  if (x >  c1o2)
			  {
				  cbID = neighborX[cbID]; 
				  x = x - c1o1;
			  }
			  if (y >  c1o2)
  			  {
				  cbID = neighborY[cbID]; 
				  y = y - c1o1;
			  }
			  if (z >  c1o2)
			  {
				  cbID = neighborZ[cbID]; 
				  z = z - c1o1;
			  }

			  real tempX = x;
			  real tempY = y;
			  real tempZ = z;

			  if ((x < -c1o2) || (y < -c1o2) || (z < -c1o2))
			  {
				  cbID = neighborWSB[cbID];
				  negativeDirection = true;
				  tempX = x + c1o1;
				  tempY = y + c1o1;
				  tempZ = z + c1o1;
			  }
			  if ((x >= -c1o2) && (negativeDirection == true))
			  {
				  cbID = neighborX[cbID]; 
				  tempX = x;
			  }
			  if ((y >= -c1o2) && (negativeDirection == true))
			  {
				  cbID = neighborY[cbID]; 
				  tempY = y;
			  }
			  if ((z >= -c1o2) && (negativeDirection == true))
			  { 
				  cbID = neighborZ[cbID]; 
				  tempZ = z;
			  }

			  x = tempX;
			  y = tempY;
			  z = tempZ;

			  localX                         = (x + c1o2) / (real)(pow((double)c2o1, (double)level));
              localY                         = (y + c1o2) / (real)(pow((double)c2o1, (double)level));
              localZ                         = (z + c1o2) / (real)(pow((double)c2o1, (double)level));
			  coordParticleXlocal[kTimeStep] = localX;
			  coordParticleYlocal[kTimeStep] = localY;
			  coordParticleZlocal[kTimeStep] = localZ;
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  real globalX = localX + coordX[cbID];
			  real globalY = localY + coordY[cbID];
			  real globalZ = localZ + coordZ[cbID];
			  coordParticleXglobal[kTimeStep] = globalX;
			  coordParticleYglobal[kTimeStep] = globalY;
			  coordParticleZglobal[kTimeStep] = globalZ;
			  //coordParticleXglobal[kTimeStep] = coordParticleXglobal[kTimeStepOld];
			  //coordParticleYglobal[kTimeStep] = coordParticleYglobal[kTimeStepOld];
			  //coordParticleZglobal[kTimeStep] = coordParticleZglobal[kTimeStepOld];
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  cellBaseID[k] = cbID;
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	  
		}
   }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






































//////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void MoveParticlesWithoutBCs(   real* coordX,
													  real* coordY,
													  real* coordZ, 
													  real* coordParticleXlocal,
													  real* coordParticleYlocal,
													  real* coordParticleZlocal,
													  real* coordParticleXglobal,
													  real* coordParticleYglobal,
													  real* coordParticleZglobal,
													  real* veloParticleX,
													  real* veloParticleY,
													  real* veloParticleZ,
													  real* DD,
													  real  omega,
													  unsigned int* particleID,
													  unsigned int* cellBaseID,
													  unsigned int* bcMatD,
													  unsigned int* neighborX,
													  unsigned int* neighborY,
													  unsigned int* neighborZ,
													  unsigned int* neighborWSB,
													  int level,
													  unsigned int timestep, 
													  unsigned int numberOfTimesteps, 
													  unsigned int numberOfParticles, 
													  unsigned int size_Mat,
													  bool isEvenTimestep)
{
   ////////////////////////////////////////////////////////////////////////////////
   const unsigned  ix = threadIdx.x;  // Globaler x-Index 
   const unsigned  iy = blockIdx.x;   // Globaler y-Index 
   const unsigned  iz = blockIdx.y;   // Globaler z-Index 

   const unsigned nx = blockDim.x;
   const unsigned ny = gridDim.x;

   const unsigned k = nx*(ny*iz + iy) + ix;
   //////////////////////////////////////////////////////////////////////////

   //real press;
   real vx1,vx2,vx3;
   real drho_SWT,vx1_SWT,vx2_SWT,vx3_SWT;
   real drho_NWT,vx1_NWT,vx2_NWT,vx3_NWT;
   real drho_NET,vx1_NET,vx2_NET,vx3_NET;
   real drho_SET,vx1_SET,vx2_SET,vx3_SET;
   real drho_SWB,vx1_SWB,vx2_SWB,vx3_SWB;
   real drho_NWB,vx1_NWB,vx2_NWB,vx3_NWB;
   real drho_NEB,vx1_NEB,vx2_NEB,vx3_NEB;
   real drho_SEB,vx1_SEB,vx2_SEB,vx3_SEB;
   real f_E,f_W,f_N,f_S,f_T,f_B,f_NE,f_SW,f_SE,f_NW,f_TE,f_BW,f_BE,f_TW,f_TN,f_BS,f_BN,f_TS,f_ZERO,f_TNE, f_TSW, f_TSE, f_TNW, f_BNE, f_BSW, f_BSE, f_BNW;
   real *feC, *fwC, *fnC, *fsC, *ftC, *fbC, *fneC, *fswC, *fseC, *fnwC, *fteC, *fbwC, *fbeC, *ftwC, *ftnC, *fbsC, *fbnC, *ftsC, *fzeroC, *ftneC, *ftswC, *ftseC, *ftnwC, *fbneC, *fbswC, *fbseC, *fbnwC;
   real kxyFromfcNEQ_SWT, kyzFromfcNEQ_SWT, kxzFromfcNEQ_SWT, kxxMyyFromfcNEQ_SWT, kxxMzzFromfcNEQ_SWT;
   real kxyFromfcNEQ_NWT, kyzFromfcNEQ_NWT, kxzFromfcNEQ_NWT, kxxMyyFromfcNEQ_NWT, kxxMzzFromfcNEQ_NWT;
   real kxyFromfcNEQ_NET, kyzFromfcNEQ_NET, kxzFromfcNEQ_NET, kxxMyyFromfcNEQ_NET, kxxMzzFromfcNEQ_NET;
   real kxyFromfcNEQ_SET, kyzFromfcNEQ_SET, kxzFromfcNEQ_SET, kxxMyyFromfcNEQ_SET, kxxMzzFromfcNEQ_SET;
   real kxyFromfcNEQ_SWB, kyzFromfcNEQ_SWB, kxzFromfcNEQ_SWB, kxxMyyFromfcNEQ_SWB, kxxMzzFromfcNEQ_SWB;
   real kxyFromfcNEQ_NWB, kyzFromfcNEQ_NWB, kxzFromfcNEQ_NWB, kxxMyyFromfcNEQ_NWB, kxxMzzFromfcNEQ_NWB;
   real kxyFromfcNEQ_NEB, kyzFromfcNEQ_NEB, kxzFromfcNEQ_NEB, kxxMyyFromfcNEQ_NEB, kxxMzzFromfcNEQ_NEB;
   real kxyFromfcNEQ_SEB, kyzFromfcNEQ_SEB, kxzFromfcNEQ_SEB, kxxMyyFromfcNEQ_SEB, kxxMzzFromfcNEQ_SEB;
   real a0, ax, ay, az, axx, ayy, azz, axy, axz, ayz, b0, bx, by, bz, bxx, byy, bzz, bxy, bxz, byz, c0, cx, cy, cz, cxx, cyy, czz, cxy, cxz, cyz, axyz, bxyz, cxyz;
   //real d0, dx, dy, dz, dxy, dxz, dyz, dxyz;

   real x,y,z;

   if(k < numberOfParticles)
   {
		/////////////////////////////////////////////////////////////
	    unsigned int kTimeStep = k + (timestep * numberOfParticles);
		/////////////////////////////////////////////////////////////
		unsigned int kCellBaseID = cellBaseID[k];
	    unsigned int BC000  = bcMatD[kCellBaseID];
	    unsigned int BCx00  = bcMatD[neighborX[kCellBaseID]];
	    unsigned int BC0y0  = bcMatD[neighborY[kCellBaseID]];
	    unsigned int BC00z  = bcMatD[neighborZ[kCellBaseID]];
	    unsigned int BCxy0  = bcMatD[neighborY[neighborX[kCellBaseID]]];
	    unsigned int BCx0z  = bcMatD[neighborZ[neighborX[kCellBaseID]]];
	    unsigned int BC0yz  = bcMatD[neighborZ[neighborY[kCellBaseID]]];
	    unsigned int BCxyz  = bcMatD[neighborZ[neighborY[neighborX[kCellBaseID]]]];
		/////////////////////////////////////////////////////////////
   		if( (BC000 == GEO_FLUID) || (BCx00 == GEO_FLUID) || (BC0y0 == GEO_FLUID) || (BC00z == GEO_FLUID) || 
			(BCxy0 == GEO_FLUID) || (BCx0z == GEO_FLUID) || (BC0yz == GEO_FLUID) || (BCxyz == GEO_FLUID) )
		{
		   if (isEvenTimestep==true)
		   {
			  feC    = &DD[E   *size_Mat];
			  fwC    = &DD[W   *size_Mat];
			  fnC    = &DD[N   *size_Mat];
			  fsC    = &DD[S   *size_Mat];
			  ftC    = &DD[T   *size_Mat];
			  fbC    = &DD[B   *size_Mat];
			  fneC   = &DD[NE  *size_Mat];
			  fswC   = &DD[SW  *size_Mat];
			  fseC   = &DD[SE  *size_Mat];
			  fnwC   = &DD[NW  *size_Mat];
			  fteC   = &DD[TE  *size_Mat];
			  fbwC   = &DD[BW  *size_Mat];
			  fbeC   = &DD[BE  *size_Mat];
			  ftwC   = &DD[TW  *size_Mat];
			  ftnC   = &DD[TN  *size_Mat];
			  fbsC   = &DD[BS  *size_Mat];
			  fbnC   = &DD[BN  *size_Mat];
			  ftsC   = &DD[TS  *size_Mat];
			  fzeroC = &DD[dirREST*size_Mat];
			  ftneC  = &DD[TNE *size_Mat];
			  ftswC  = &DD[TSW *size_Mat];
			  ftseC  = &DD[TSE *size_Mat];
			  ftnwC  = &DD[TNW *size_Mat];
			  fbneC  = &DD[BNE *size_Mat];
			  fbswC  = &DD[BSW *size_Mat];
			  fbseC  = &DD[BSE *size_Mat];
			  fbnwC  = &DD[BNW *size_Mat];
		   } 			 
		   else			 
		   {			 
			  fwC    = &DD[E   *size_Mat];
			  feC    = &DD[W   *size_Mat];
			  fsC    = &DD[N   *size_Mat];
			  fnC    = &DD[S   *size_Mat];
			  fbC    = &DD[T   *size_Mat];
			  ftC    = &DD[B   *size_Mat];
			  fswC   = &DD[NE  *size_Mat];
			  fneC   = &DD[SW  *size_Mat];
			  fnwC   = &DD[SE  *size_Mat];
			  fseC   = &DD[NW  *size_Mat];
			  fbwC   = &DD[TE  *size_Mat];
			  fteC   = &DD[BW  *size_Mat];
			  ftwC   = &DD[BE  *size_Mat];
			  fbeC   = &DD[TW  *size_Mat];
			  fbsC   = &DD[TN  *size_Mat];
			  ftnC   = &DD[BS  *size_Mat];
			  ftsC   = &DD[BN  *size_Mat];
			  fbnC   = &DD[TS  *size_Mat];
			  fzeroC = &DD[dirREST*size_Mat];
			  fbswC  = &DD[TNE *size_Mat];
			  fbneC  = &DD[TSW *size_Mat];
			  fbnwC  = &DD[TSE *size_Mat];
			  fbseC  = &DD[TNW *size_Mat];
			  ftswC  = &DD[BNE *size_Mat];
			  ftneC  = &DD[BSW *size_Mat];
			  ftnwC  = &DD[BSE *size_Mat];
			  ftseC  = &DD[BNW *size_Mat];
		   }

			  //////////////////////////////////////////////////////////////////////////
			  //SWB//
			  //////////////////////////////////////////////////////////////////////////
			  //index 0
			  unsigned int k0zero= cellBaseID[k];
			  unsigned int k0w   = neighborX[k0zero];
			  unsigned int k0s   = neighborY[k0zero];
			  unsigned int k0b   = neighborZ[k0zero];
			  unsigned int k0sw  = neighborY[k0w];
			  unsigned int k0bw  = neighborZ[k0w];
			  unsigned int k0bs  = neighborZ[k0s];
			  unsigned int k0bsw = neighborZ[k0sw];
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  unsigned int kzero= k0zero;
			  unsigned int kw   = k0w;   
			  unsigned int ks   = k0s;   
			  unsigned int kb   = k0b;   
			  unsigned int ksw  = k0sw;  
			  unsigned int kbw  = k0bw;  
			  unsigned int kbs  = k0bs;  
			  unsigned int kbsw = k0bsw; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_SWB = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_SWB  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_SWB);
			  vx2_SWB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_SWB);
			  vx3_SWB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_SWB);

			  kxyFromfcNEQ_SWB    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_SWB) - ((vx1_SWB*vx2_SWB)));
			  kyzFromfcNEQ_SWB    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_SWB) - ((vx2_SWB*vx3_SWB)));
			  kxzFromfcNEQ_SWB    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_SWB) - ((vx1_SWB*vx3_SWB)));
			  kxxMyyFromfcNEQ_SWB = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_SWB) - ((vx1_SWB*vx1_SWB-vx2_SWB*vx2_SWB)));
			  kxxMzzFromfcNEQ_SWB = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_SWB) - ((vx1_SWB*vx1_SWB-vx3_SWB*vx3_SWB)));

			  //////////////////////////////////////////////////////////////////////////
			  //SWT//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= kb;
			  kw   = kbw;   
			  ks   = kbs;   
			  kb   = neighborZ[kb];   
			  ksw  = kbsw;  
			  kbw  = neighborZ[kbw];  
			  kbs  = neighborZ[kbs];  
			  kbsw = neighborZ[kbsw]; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_SWT = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_SWT  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_SWT);
			  vx2_SWT  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_SWT);
			  vx3_SWT  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_SWT);

			  kxyFromfcNEQ_SWT    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_SWT) - ((vx1_SWT*vx2_SWT)));
			  kyzFromfcNEQ_SWT    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_SWT) - ((vx2_SWT*vx3_SWT)));
			  kxzFromfcNEQ_SWT    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_SWT) - ((vx1_SWT*vx3_SWT)));
			  kxxMyyFromfcNEQ_SWT = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_SWT) - ((vx1_SWT*vx1_SWT-vx2_SWT*vx2_SWT)));
			  kxxMzzFromfcNEQ_SWT = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_SWT) - ((vx1_SWT*vx1_SWT-vx3_SWT*vx3_SWT)));

			  //////////////////////////////////////////////////////////////////////////
			  //SET//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= kw;
			  kw   = neighborX[kw];   
			  ks   = ksw;   
			  kb   = kbw;   
			  ksw  = neighborX[ksw];  
			  kbw  = neighborX[kbw];  
			  kbs  = kbsw;  
			  kbsw = neighborX[kbsw]; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_SET = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_SET  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_SET);
			  vx2_SET  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_SET);
			  vx3_SET  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_SET);

			  kxyFromfcNEQ_SET    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_SET) - ((vx1_SET*vx2_SET)));
			  kyzFromfcNEQ_SET    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_SET) - ((vx2_SET*vx3_SET)));
			  kxzFromfcNEQ_SET    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_SET) - ((vx1_SET*vx3_SET)));
			  kxxMyyFromfcNEQ_SET = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_SET) - ((vx1_SET*vx1_SET-vx2_SET*vx2_SET)));
			  kxxMzzFromfcNEQ_SET = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_SET) - ((vx1_SET*vx1_SET-vx3_SET*vx3_SET)));

			  //////////////////////////////////////////////////////////////////////////
			  //SEB//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kb   = kzero;   
			  kbw  = kw;  
			  kbs  = ks;  
			  kbsw = ksw; 
			  kzero= k0w;
			  kw   = neighborX[k0w];   
			  ks   = k0sw;   
			  ksw  = neighborX[k0sw];  
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_SEB = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_SEB  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_SEB);
			  vx2_SEB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_SEB);
			  vx3_SEB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_SEB);

			  kxyFromfcNEQ_SEB    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_SEB) - ((vx1_SEB*vx2_SEB)));
			  kyzFromfcNEQ_SEB    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_SEB) - ((vx2_SEB*vx3_SEB)));
			  kxzFromfcNEQ_SEB    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_SEB) - ((vx1_SEB*vx3_SEB)));
			  kxxMyyFromfcNEQ_SEB = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_SEB) - ((vx1_SEB*vx1_SEB-vx2_SEB*vx2_SEB)));
			  kxxMzzFromfcNEQ_SEB = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_SEB) - ((vx1_SEB*vx1_SEB-vx3_SEB*vx3_SEB)));

			  //////////////////////////////////////////////////////////////////////////
			  //NWB//
			  //////////////////////////////////////////////////////////////////////////
			  //index 0
			  k0zero= k0s;
			  k0w   = k0sw;
			  k0s   = neighborY[k0s];
			  k0b   = k0bs;
			  k0sw  = neighborY[k0sw];
			  k0bw  = k0bsw;
			  k0bs  = neighborY[k0bs];
			  k0bsw = neighborY[k0bsw];
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= k0zero;
			  kw   = k0w;   
			  ks   = k0s;   
			  kb   = k0b;   
			  ksw  = k0sw;  
			  kbw  = k0bw;  
			  kbs  = k0bs;  
			  kbsw = k0bsw; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_NWB = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_NWB  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_NWB);
			  vx2_NWB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_NWB);
			  vx3_NWB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_NWB);

			  kxyFromfcNEQ_NWB    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_NWB) - ((vx1_NWB*vx2_NWB)));
			  kyzFromfcNEQ_NWB    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_NWB) - ((vx2_NWB*vx3_NWB)));
			  kxzFromfcNEQ_NWB    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_NWB) - ((vx1_NWB*vx3_NWB)));
			  kxxMyyFromfcNEQ_NWB = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_NWB) - ((vx1_NWB*vx1_NWB-vx2_NWB*vx2_NWB)));
			  kxxMzzFromfcNEQ_NWB = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_NWB) - ((vx1_NWB*vx1_NWB-vx3_NWB*vx3_NWB)));

			  //////////////////////////////////////////////////////////////////////////
			  //NWT//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= kb;
			  kw   = kbw;   
			  ks   = kbs;   
			  kb   = neighborZ[kb];   
			  ksw  = kbsw;  
			  kbw  = neighborZ[kbw];  
			  kbs  = neighborZ[kbs];  
			  kbsw = neighborZ[kbsw]; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_NWT = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_NWT  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_NWT);
			  vx2_NWT  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_NWT);
			  vx3_NWT  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_NWT);

			  kxyFromfcNEQ_NWT    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_NWT) - ((vx1_NWT*vx2_NWT)));
			  kyzFromfcNEQ_NWT    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_NWT) - ((vx2_NWT*vx3_NWT)));
			  kxzFromfcNEQ_NWT    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_NWT) - ((vx1_NWT*vx3_NWT)));
			  kxxMyyFromfcNEQ_NWT = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_NWT) - ((vx1_NWT*vx1_NWT-vx2_NWT*vx2_NWT)));
			  kxxMzzFromfcNEQ_NWT = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_NWT) - ((vx1_NWT*vx1_NWT-vx3_NWT*vx3_NWT)));

			  //////////////////////////////////////////////////////////////////////////
			  //NET//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kzero= kw;
			  kw   = neighborX[kw];   
			  ks   = ksw;   
			  kb   = kbw;   
			  ksw  = neighborX[ksw];  
			  kbw  = neighborX[kbw];  
			  kbs  = kbsw;  
			  kbsw = neighborX[kbsw]; 
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_NET = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_NET  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_NET);
			  vx2_NET  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_NET);
			  vx3_NET  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_NET);

			  kxyFromfcNEQ_NET    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_NET) - ((vx1_NET*vx2_NET)));
			  kyzFromfcNEQ_NET    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_NET) - ((vx2_NET*vx3_NET)));
			  kxzFromfcNEQ_NET    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_NET) - ((vx1_NET*vx3_NET)));
			  kxxMyyFromfcNEQ_NET = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_NET) - ((vx1_NET*vx1_NET-vx2_NET*vx2_NET)));
			  kxxMzzFromfcNEQ_NET = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_NET) - ((vx1_NET*vx1_NET-vx3_NET*vx3_NET)));

			  //////////////////////////////////////////////////////////////////////////
			  //NEB//
			  //////////////////////////////////////////////////////////////////////////
			  //index 
			  kb   = kzero;   
			  kbw  = kw;  
			  kbs  = ks;  
			  kbsw = ksw; 
			  kzero= k0w;
			  kw   = neighborX[k0w];   
			  ks   = k0sw;   
			  ksw  = neighborX[k0sw];  
			  ////////////////////////////////////////////////////////////////////////////////
			  f_E    = feC[kzero];
			  f_W    = fwC[kw];
			  f_N    = fnC[kzero];
			  f_S    = fsC[ks];
			  f_T    = ftC[kzero];
			  f_B    = fbC[kb];
			  f_NE   = fneC[kzero];
			  f_SW   = fswC[ksw];
			  f_SE   = fseC[ks];
			  f_NW   = fnwC[kw];
			  f_TE   = fteC[kzero];
			  f_BW   = fbwC[kbw];
			  f_BE   = fbeC[kb];
			  f_TW   = ftwC[kw];
			  f_TN   = ftnC[kzero];
			  f_BS   = fbsC[kbs];
			  f_BN   = fbnC[kb];
			  f_TS   = ftsC[ks];
			  f_ZERO = fzeroC[kzero];
			  f_TNE  = ftneC[kzero];
			  f_TSW  = ftswC[ksw];
			  f_TSE  = ftseC[ks];
			  f_TNW  = ftnwC[kw];
			  f_BNE  = fbneC[kb];
			  f_BSW  = fbswC[kbsw];
			  f_BSE  = fbseC[kbs];
			  f_BNW  = fbnwC[kbw];

			  drho_NEB = f_E+f_W+f_N+f_S+f_T+f_B+f_NE+f_SW+f_SE+f_NW+f_TE+f_BW+f_BE+f_TW+f_TN+f_BS+f_BN+f_TS+f_ZERO+f_TNE+f_TSW+f_TSE+f_TNW+f_BNE+f_BSW+f_BSE+f_BNW;
			  vx1_NEB  = (((f_TNE-f_BSW)+(f_TSE-f_BNW)+(f_BNE-f_TSW)+(f_BSE-f_TNW)) + (((f_NE-f_SW)+(f_TE-f_BW))+((f_SE-f_NW)+(f_BE-f_TW))) + (f_E-f_W))/(c1o1 + drho_NEB);
			  vx2_NEB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_BNE-f_TSW)+(f_BNW-f_TSE)) + (((f_NE-f_SW)+(f_TN-f_BS))+((f_BN-f_TS)+(f_NW-f_SE))) + (f_N-f_S))/(c1o1 + drho_NEB);
			  vx3_NEB  = (((f_TNE-f_BSW)+(f_TNW-f_BSE)+(f_TSE-f_BNW)+(f_TSW-f_BNE)) + (((f_TE-f_BW)+(f_TN-f_BS))+((f_TW-f_BE)+(f_TS-f_BN))) + (f_T-f_B))/(c1o1 + drho_NEB);

			  kxyFromfcNEQ_NEB    = -c3o1*omega*((f_SW+f_BSW+f_TSW-f_NW-f_BNW-f_TNW-f_SE-f_BSE-f_TSE+f_NE+f_BNE+f_TNE ) / (c1o1 + drho_NEB) - ((vx1_NEB*vx2_NEB)));
			  kyzFromfcNEQ_NEB    = -c3o1*omega*((f_BS+f_BSE+f_BSW-f_TS-f_TSE-f_TSW-f_BN-f_BNE-f_BNW+f_TN+f_TNE+f_TNW ) / (c1o1 + drho_NEB) - ((vx2_NEB*vx3_NEB)));
			  kxzFromfcNEQ_NEB    = -c3o1*omega*((f_BW+f_BSW+f_BNW-f_TW-f_TSW-f_TNW-f_BE-f_BSE-f_BNE+f_TE+f_TSE+f_TNE ) / (c1o1 + drho_NEB) - ((vx1_NEB*vx3_NEB)));
			  kxxMyyFromfcNEQ_NEB = -c3o2*omega *((f_BW+f_W+f_TW-f_BS-f_S-f_TS-f_BN-f_N-f_TN+f_BE+f_E+f_TE             ) / (c1o1 + drho_NEB) - ((vx1_NEB*vx1_NEB-vx2_NEB*vx2_NEB)));
			  kxxMzzFromfcNEQ_NEB = -c3o2*omega *((f_SW+f_W+f_NW-f_BS-f_TS-f_B-f_T-f_BN-f_TN+f_SE+f_E+f_NE             ) / (c1o1 + drho_NEB) - ((vx1_NEB*vx1_NEB-vx3_NEB*vx3_NEB)));

			  //////////////////////////////////////////////////////////////////////////
			  //interpolate
			  //////////////////////////////////////////////////////////////////////////
			  a0 = (-kxxMyyFromfcNEQ_NEB - kxxMyyFromfcNEQ_NET + kxxMyyFromfcNEQ_NWB + kxxMyyFromfcNEQ_NWT - 
				 kxxMyyFromfcNEQ_SEB - kxxMyyFromfcNEQ_SET + kxxMyyFromfcNEQ_SWB + kxxMyyFromfcNEQ_SWT - 
				 kxxMzzFromfcNEQ_NEB - kxxMzzFromfcNEQ_NET + kxxMzzFromfcNEQ_NWB + kxxMzzFromfcNEQ_NWT - 
				 kxxMzzFromfcNEQ_SEB - kxxMzzFromfcNEQ_SET + kxxMzzFromfcNEQ_SWB + kxxMzzFromfcNEQ_SWT - 
				 c2o1*kxyFromfcNEQ_NEB - c2o1*kxyFromfcNEQ_NET - c2o1*kxyFromfcNEQ_NWB - c2o1*kxyFromfcNEQ_NWT + 
				 c2o1*kxyFromfcNEQ_SEB + c2o1*kxyFromfcNEQ_SET + c2o1*kxyFromfcNEQ_SWB + c2o1*kxyFromfcNEQ_SWT + 
				 c2o1*kxzFromfcNEQ_NEB - c2o1*kxzFromfcNEQ_NET + c2o1*kxzFromfcNEQ_NWB - c2o1*kxzFromfcNEQ_NWT + 
				 c2o1*kxzFromfcNEQ_SEB - c2o1*kxzFromfcNEQ_SET + c2o1*kxzFromfcNEQ_SWB - c2o1*kxzFromfcNEQ_SWT + 
				 c8o1*vx1_NEB + c8o1*vx1_NET + c8o1*vx1_NWB + c8o1*vx1_NWT + c8o1*vx1_SEB + 
				 c8o1*vx1_SET + c8o1*vx1_SWB + c8o1*vx1_SWT + c2o1*vx2_NEB + c2o1*vx2_NET - 
				 c2o1*vx2_NWB - c2o1*vx2_NWT - c2o1*vx2_SEB - c2o1*vx2_SET + c2o1*vx2_SWB + 
				 c2o1*vx2_SWT - c2o1*vx3_NEB + c2o1*vx3_NET + c2o1*vx3_NWB - c2o1*vx3_NWT - 
				 c2o1*vx3_SEB + c2o1*vx3_SET + c2o1*vx3_SWB - c2o1*vx3_SWT)/c64o1;
			  b0 = (c2o1*kxxMyyFromfcNEQ_NEB + c2o1*kxxMyyFromfcNEQ_NET + c2o1*kxxMyyFromfcNEQ_NWB + c2o1*kxxMyyFromfcNEQ_NWT - 
				 c2o1*kxxMyyFromfcNEQ_SEB - c2o1*kxxMyyFromfcNEQ_SET - c2o1*kxxMyyFromfcNEQ_SWB - c2o1*kxxMyyFromfcNEQ_SWT - 
				 kxxMzzFromfcNEQ_NEB - kxxMzzFromfcNEQ_NET - kxxMzzFromfcNEQ_NWB - kxxMzzFromfcNEQ_NWT + 
				 kxxMzzFromfcNEQ_SEB + kxxMzzFromfcNEQ_SET + kxxMzzFromfcNEQ_SWB + kxxMzzFromfcNEQ_SWT - 
				 c2o1*kxyFromfcNEQ_NEB - c2o1*kxyFromfcNEQ_NET + c2o1*kxyFromfcNEQ_NWB + c2o1*kxyFromfcNEQ_NWT - 
				 c2o1*kxyFromfcNEQ_SEB - c2o1*kxyFromfcNEQ_SET + c2o1*kxyFromfcNEQ_SWB + c2o1*kxyFromfcNEQ_SWT + 
				 c2o1*kyzFromfcNEQ_NEB - c2o1*kyzFromfcNEQ_NET + c2o1*kyzFromfcNEQ_NWB - c2o1*kyzFromfcNEQ_NWT + 
				 c2o1*kyzFromfcNEQ_SEB - c2o1*kyzFromfcNEQ_SET + c2o1*kyzFromfcNEQ_SWB - c2o1*kyzFromfcNEQ_SWT + 
				 c2o1*vx1_NEB + c2o1*vx1_NET - c2o1*vx1_NWB - c2o1*vx1_NWT - 
				 c2o1*vx1_SEB - c2o1*vx1_SET + c2o1*vx1_SWB + c2o1*vx1_SWT + 
				 c8o1*vx2_NEB + c8o1*vx2_NET + c8o1*vx2_NWB + c8o1*vx2_NWT + 
				 c8o1*vx2_SEB + c8o1*vx2_SET + c8o1*vx2_SWB + c8o1*vx2_SWT - 
				 c2o1*vx3_NEB + c2o1*vx3_NET - c2o1*vx3_NWB + c2o1*vx3_NWT + 
				 c2o1*vx3_SEB - c2o1*vx3_SET + c2o1*vx3_SWB - c2o1*vx3_SWT)/c64o1;

			  //b0 = ((eight*vx2_NEB + eight*vx2_SWT) + (eight*vx2_NET + eight*vx2_SWB) + (eight*vx2_NWB + eight*vx2_SET) + (eight*vx2_NWT + eight*vx2_SEB))/sixtyfour;

			  c0 = (kxxMyyFromfcNEQ_NEB - kxxMyyFromfcNEQ_NET + kxxMyyFromfcNEQ_NWB - kxxMyyFromfcNEQ_NWT + 
				 kxxMyyFromfcNEQ_SEB - kxxMyyFromfcNEQ_SET + kxxMyyFromfcNEQ_SWB - kxxMyyFromfcNEQ_SWT - 
				 c2o1*kxxMzzFromfcNEQ_NEB + c2o1*kxxMzzFromfcNEQ_NET - c2o1*kxxMzzFromfcNEQ_NWB + c2o1*kxxMzzFromfcNEQ_NWT - 
				 c2o1*kxxMzzFromfcNEQ_SEB + c2o1*kxxMzzFromfcNEQ_SET - c2o1*kxxMzzFromfcNEQ_SWB + c2o1*kxxMzzFromfcNEQ_SWT - 
				 c2o1*kxzFromfcNEQ_NEB - c2o1*kxzFromfcNEQ_NET + c2o1*kxzFromfcNEQ_NWB + c2o1*kxzFromfcNEQ_NWT - 
				 c2o1*kxzFromfcNEQ_SEB - c2o1*kxzFromfcNEQ_SET + c2o1*kxzFromfcNEQ_SWB + c2o1*kxzFromfcNEQ_SWT - 
				 c2o1*kyzFromfcNEQ_NEB - c2o1*kyzFromfcNEQ_NET - c2o1*kyzFromfcNEQ_NWB - c2o1*kyzFromfcNEQ_NWT + 
				 c2o1*kyzFromfcNEQ_SEB + c2o1*kyzFromfcNEQ_SET + c2o1*kyzFromfcNEQ_SWB + c2o1*kyzFromfcNEQ_SWT - 
				 c2o1*vx1_NEB + c2o1*vx1_NET + c2o1*vx1_NWB - c2o1*vx1_NWT - 
				 c2o1*vx1_SEB + c2o1*vx1_SET + c2o1*vx1_SWB - c2o1*vx1_SWT - 
				 c2o1*vx2_NEB + c2o1*vx2_NET - c2o1*vx2_NWB + c2o1*vx2_NWT + 
				 c2o1*vx2_SEB - c2o1*vx2_SET + c2o1*vx2_SWB - c2o1*vx2_SWT + 
				 c8o1*vx3_NEB + c8o1*vx3_NET + c8o1*vx3_NWB + c8o1*vx3_NWT + 
				 c8o1*vx3_SEB + c8o1*vx3_SET + c8o1*vx3_SWB + c8o1*vx3_SWT)/c64o1;
			  ax = (vx1_NEB + vx1_NET - vx1_NWB - vx1_NWT + vx1_SEB + vx1_SET - vx1_SWB - vx1_SWT)/c4o1;
			  bx = (vx2_NEB + vx2_NET - vx2_NWB - vx2_NWT + vx2_SEB + vx2_SET - vx2_SWB - vx2_SWT)/c4o1;
			  //bx = ((vx2_NEB - vx2_SWT) + (vx2_NET - vx2_SWB) + (vx2_SET - vx2_NWB) + (vx2_SEB - vx2_NWT))/four;
			  cx = (vx3_NEB + vx3_NET - vx3_NWB - vx3_NWT + vx3_SEB + vx3_SET - vx3_SWB - vx3_SWT)/c4o1;
			  axx= (kxxMyyFromfcNEQ_NEB + kxxMyyFromfcNEQ_NET - kxxMyyFromfcNEQ_NWB - kxxMyyFromfcNEQ_NWT + 
				 kxxMyyFromfcNEQ_SEB + kxxMyyFromfcNEQ_SET - kxxMyyFromfcNEQ_SWB - kxxMyyFromfcNEQ_SWT + 
				 kxxMzzFromfcNEQ_NEB + kxxMzzFromfcNEQ_NET - kxxMzzFromfcNEQ_NWB - kxxMzzFromfcNEQ_NWT + 
				 kxxMzzFromfcNEQ_SEB + kxxMzzFromfcNEQ_SET - kxxMzzFromfcNEQ_SWB - kxxMzzFromfcNEQ_SWT + 
				 c2o1*vx2_NEB + c2o1*vx2_NET - c2o1*vx2_NWB - c2o1*vx2_NWT - 
				 c2o1*vx2_SEB - c2o1*vx2_SET + c2o1*vx2_SWB + c2o1*vx2_SWT - 
				 c2o1*vx3_NEB + c2o1*vx3_NET + c2o1*vx3_NWB - c2o1*vx3_NWT - 
				 c2o1*vx3_SEB + c2o1*vx3_SET + c2o1*vx3_SWB - c2o1*vx3_SWT)/c16o1;
			  bxx= (kxyFromfcNEQ_NEB + kxyFromfcNEQ_NET - kxyFromfcNEQ_NWB - kxyFromfcNEQ_NWT + 
				 kxyFromfcNEQ_SEB + kxyFromfcNEQ_SET - kxyFromfcNEQ_SWB - kxyFromfcNEQ_SWT - 
				 c2o1*vx1_NEB - c2o1*vx1_NET + c2o1*vx1_NWB + c2o1*vx1_NWT + 
				 c2o1*vx1_SEB + c2o1*vx1_SET - c2o1*vx1_SWB - c2o1*vx1_SWT)/c8o1;
			  cxx= (kxzFromfcNEQ_NEB + kxzFromfcNEQ_NET - kxzFromfcNEQ_NWB - kxzFromfcNEQ_NWT + 
				 kxzFromfcNEQ_SEB + kxzFromfcNEQ_SET - kxzFromfcNEQ_SWB - kxzFromfcNEQ_SWT + 
				 c2o1*vx1_NEB - c2o1*vx1_NET - c2o1*vx1_NWB + c2o1*vx1_NWT + 
				 c2o1*vx1_SEB - c2o1*vx1_SET - c2o1*vx1_SWB + c2o1*vx1_SWT)/c8o1;
			  ay = (vx1_NEB + vx1_NET + vx1_NWB + vx1_NWT - vx1_SEB - vx1_SET - vx1_SWB - vx1_SWT)/c4o1;
			  by = (vx2_NEB + vx2_NET + vx2_NWB + vx2_NWT - vx2_SEB - vx2_SET - vx2_SWB - vx2_SWT)/c4o1;
			  cy = (vx3_NEB + vx3_NET + vx3_NWB + vx3_NWT - vx3_SEB - vx3_SET - vx3_SWB - vx3_SWT)/c4o1;
			  ayy= (kxyFromfcNEQ_NEB + kxyFromfcNEQ_NET + kxyFromfcNEQ_NWB + kxyFromfcNEQ_NWT - 
				 kxyFromfcNEQ_SEB - kxyFromfcNEQ_SET - kxyFromfcNEQ_SWB - kxyFromfcNEQ_SWT - 
				 c2o1*vx2_NEB - c2o1*vx2_NET + c2o1*vx2_NWB + c2o1*vx2_NWT + 
				 c2o1*vx2_SEB + c2o1*vx2_SET - c2o1*vx2_SWB - c2o1*vx2_SWT)/c8o1;
			  byy= (-c2o1*kxxMyyFromfcNEQ_NEB - c2o1*kxxMyyFromfcNEQ_NET - c2o1*kxxMyyFromfcNEQ_NWB - c2o1*kxxMyyFromfcNEQ_NWT + 
				 c2o1*kxxMyyFromfcNEQ_SEB + c2o1*kxxMyyFromfcNEQ_SET + c2o1*kxxMyyFromfcNEQ_SWB + c2o1*kxxMyyFromfcNEQ_SWT + 
				 kxxMzzFromfcNEQ_NEB + kxxMzzFromfcNEQ_NET + kxxMzzFromfcNEQ_NWB + kxxMzzFromfcNEQ_NWT - 
				 kxxMzzFromfcNEQ_SEB - kxxMzzFromfcNEQ_SET - kxxMzzFromfcNEQ_SWB - kxxMzzFromfcNEQ_SWT + 
				 c2o1*vx1_NEB + c2o1*vx1_NET - c2o1*vx1_NWB - c2o1*vx1_NWT - 
				 c2o1*vx1_SEB - c2o1*vx1_SET + c2o1*vx1_SWB + c2o1*vx1_SWT - 
				 c2o1*vx3_NEB + c2o1*vx3_NET - c2o1*vx3_NWB + c2o1*vx3_NWT + 
				 c2o1*vx3_SEB - c2o1*vx3_SET + c2o1*vx3_SWB - c2o1*vx3_SWT)/c16o1;
			  cyy= (kyzFromfcNEQ_NEB + kyzFromfcNEQ_NET + kyzFromfcNEQ_NWB + kyzFromfcNEQ_NWT - 
				 kyzFromfcNEQ_SEB - kyzFromfcNEQ_SET - kyzFromfcNEQ_SWB - kyzFromfcNEQ_SWT + 
				 c2o1*vx2_NEB - c2o1*vx2_NET + c2o1*vx2_NWB - c2o1*vx2_NWT - 
				 c2o1*vx2_SEB + c2o1*vx2_SET - c2o1*vx2_SWB + c2o1*vx2_SWT)/c8o1;
			  az = (-vx1_NEB + vx1_NET - vx1_NWB + vx1_NWT - vx1_SEB + vx1_SET - vx1_SWB + vx1_SWT)/c4o1;
			  //bz = (-vx2_NEB + vx2_NET - vx2_NWB + vx2_NWT - vx2_SEB + vx2_SET - vx2_SWB + vx2_SWT)/four;
			  bz = ((vx2_SWT - vx2_NEB) + (vx2_NET - vx2_SWB) + (vx2_SET - vx2_NWB) + (vx2_NWT - vx2_SEB))/c4o1;
			  cz = (-vx3_NEB + vx3_NET - vx3_NWB + vx3_NWT - vx3_SEB + vx3_SET - vx3_SWB + vx3_SWT)/c4o1;
			  azz= (-kxzFromfcNEQ_NEB + kxzFromfcNEQ_NET - kxzFromfcNEQ_NWB + kxzFromfcNEQ_NWT - 
				 kxzFromfcNEQ_SEB + kxzFromfcNEQ_SET - kxzFromfcNEQ_SWB + kxzFromfcNEQ_SWT + 
				 c2o1*vx3_NEB - c2o1*vx3_NET - c2o1*vx3_NWB + c2o1*vx3_NWT + 
				 c2o1*vx3_SEB - c2o1*vx3_SET - c2o1*vx3_SWB + c2o1*vx3_SWT)/c8o1;
			  bzz= (-kyzFromfcNEQ_NEB + kyzFromfcNEQ_NET - kyzFromfcNEQ_NWB + kyzFromfcNEQ_NWT - 
				 kyzFromfcNEQ_SEB + kyzFromfcNEQ_SET - kyzFromfcNEQ_SWB + kyzFromfcNEQ_SWT + 
				 c2o1*vx3_NEB - c2o1*vx3_NET + c2o1*vx3_NWB - c2o1*vx3_NWT - 
				 c2o1*vx3_SEB + c2o1*vx3_SET - c2o1*vx3_SWB + c2o1*vx3_SWT)/c8o1;
			  czz= (-kxxMyyFromfcNEQ_NEB + kxxMyyFromfcNEQ_NET - kxxMyyFromfcNEQ_NWB + kxxMyyFromfcNEQ_NWT - 
				 kxxMyyFromfcNEQ_SEB + kxxMyyFromfcNEQ_SET - kxxMyyFromfcNEQ_SWB + kxxMyyFromfcNEQ_SWT + 
				 c2o1*kxxMzzFromfcNEQ_NEB - c2o1*kxxMzzFromfcNEQ_NET + c2o1*kxxMzzFromfcNEQ_NWB - c2o1*kxxMzzFromfcNEQ_NWT + 
				 c2o1*kxxMzzFromfcNEQ_SEB - c2o1*kxxMzzFromfcNEQ_SET + c2o1*kxxMzzFromfcNEQ_SWB - c2o1*kxxMzzFromfcNEQ_SWT - 
				 c2o1*vx1_NEB + c2o1*vx1_NET + c2o1*vx1_NWB - c2o1*vx1_NWT - 
				 c2o1*vx1_SEB + c2o1*vx1_SET + c2o1*vx1_SWB - c2o1*vx1_SWT - 
				 c2o1*vx2_NEB + c2o1*vx2_NET - c2o1*vx2_NWB + c2o1*vx2_NWT + 
				 c2o1*vx2_SEB - c2o1*vx2_SET + c2o1*vx2_SWB - c2o1*vx2_SWT)/c16o1;
			  axy= (vx1_NEB + vx1_NET - vx1_NWB - vx1_NWT - vx1_SEB - vx1_SET + vx1_SWB + vx1_SWT)/c2o1;
			  bxy= (vx2_NEB + vx2_NET - vx2_NWB - vx2_NWT - vx2_SEB - vx2_SET + vx2_SWB + vx2_SWT)/c2o1;
			  cxy= (vx3_NEB + vx3_NET - vx3_NWB - vx3_NWT - vx3_SEB - vx3_SET + vx3_SWB + vx3_SWT)/c2o1;
			  axz= (-vx1_NEB + vx1_NET + vx1_NWB - vx1_NWT - vx1_SEB + vx1_SET + vx1_SWB - vx1_SWT)/c2o1;
			  bxz= (-vx2_NEB + vx2_NET + vx2_NWB - vx2_NWT - vx2_SEB + vx2_SET + vx2_SWB - vx2_SWT)/c2o1;
			  cxz= (-vx3_NEB + vx3_NET + vx3_NWB - vx3_NWT - vx3_SEB + vx3_SET + vx3_SWB - vx3_SWT)/c2o1;
			  ayz= (-vx1_NEB + vx1_NET - vx1_NWB + vx1_NWT + vx1_SEB - vx1_SET + vx1_SWB - vx1_SWT)/c2o1;
			  byz= (-vx2_NEB + vx2_NET - vx2_NWB + vx2_NWT + vx2_SEB - vx2_SET + vx2_SWB - vx2_SWT)/c2o1;
			  cyz= (-vx3_NEB + vx3_NET - vx3_NWB + vx3_NWT + vx3_SEB - vx3_SET + vx3_SWB - vx3_SWT)/c2o1;
			  axyz=-vx1_NEB + vx1_NET + vx1_NWB - vx1_NWT + vx1_SEB - vx1_SET - vx1_SWB + vx1_SWT;
			  bxyz=-vx2_NEB + vx2_NET + vx2_NWB - vx2_NWT + vx2_SEB - vx2_SET - vx2_SWB + vx2_SWT;
			  cxyz=-vx3_NEB + vx3_NET + vx3_NWB - vx3_NWT + vx3_SEB - vx3_SET - vx3_SWB + vx3_SWT;
			  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  //drho
			//   d0   = ( drho_NEB + drho_NET + drho_NWB + drho_NWT + drho_SEB + drho_SET + drho_SWB + drho_SWT) * c1o8;
			//   dx   = ( drho_NEB + drho_NET - drho_NWB - drho_NWT + drho_SEB + drho_SET - drho_SWB - drho_SWT) * c1o4;
			//   dy   = ( drho_NEB + drho_NET + drho_NWB + drho_NWT - drho_SEB - drho_SET - drho_SWB - drho_SWT) * c1o4;
			//   dz   = (-drho_NEB + drho_NET - drho_NWB + drho_NWT - drho_SEB + drho_SET - drho_SWB + drho_SWT) * c1o4;
			//   dxy  = ( drho_NEB + drho_NET - drho_NWB - drho_NWT - drho_SEB - drho_SET + drho_SWB + drho_SWT) * c1o2;
			//   dxz  = (-drho_NEB + drho_NET + drho_NWB - drho_NWT - drho_SEB + drho_SET + drho_SWB - drho_SWT) * c1o2;
			//   dyz  = (-drho_NEB + drho_NET - drho_NWB + drho_NWT + drho_SEB - drho_SET + drho_SWB - drho_SWT) * c1o2;
			//   dxyz =  -drho_NEB + drho_NET + drho_NWB - drho_NWT + drho_SEB - drho_SET - drho_SWB + drho_SWT;
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  unsigned int kTimeStepOld = kTimeStep - numberOfParticles;
			  real localX = coordParticleXlocal[kTimeStepOld];
			  real localY = coordParticleYlocal[kTimeStepOld];
			  real localZ = coordParticleZlocal[kTimeStepOld];

			  x = (localX * (real)(pow((double)c2o1, (double)level))) - c1o2; //-c1o4;
              y = (localY * (real)(pow((double)c2o1, (double)level))) - c1o2; //-c1o4;
              z = (localZ * (real)(pow((double)c2o1, (double)level))) - c1o2; //-c1o4;
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  //press = d0 + x*dx + y*dy + z*dz + x*y*dxy + x*z*dxz + y*z*dyz + x*y*z*dxyz;
			  vx1 = (a0 + x*ax + y*ay + z*az + x*x*axx + y*y*ayy + z*z*azz + x*y*axy + x*z*axz + y*z*ayz + x*y*z*axyz);
			  vx2 = (b0 + x*bx + y*by + z*bz + x*x*bxx + y*y*byy + z*z*bzz + x*y*bxy + x*z*bxz + y*z*byz + x*y*z*bxyz);
			  vx3 = (c0 + x*cx + y*cy + z*cz + x*x*cxx + y*y*cyy + z*z*czz + x*y*cxy + x*z*cxz + y*z*cyz + x*y*z*cxyz);

			  real veloPreX = veloParticleX[kTimeStepOld];
			  real veloPreY = veloParticleY[kTimeStepOld];
			  real veloPreZ = veloParticleZ[kTimeStepOld];

			  real veloPostX = (veloPreX + vx1) * c1o2;
			  real veloPostY = (veloPreY + vx2) * c1o2;
			  real veloPostZ = (veloPreZ + vx3) * c1o2;

			  //real veloPostX = vx1;
			  //real veloPostY = vx2;
			  //real veloPostZ = vx3;

			  veloParticleX[kTimeStep] = veloPostX;
			  veloParticleY[kTimeStep] = veloPostY;
			  veloParticleZ[kTimeStep] = veloPostZ;
			  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  x = x + veloPostX;
			  //x = x + c1o3;
			  y = y + veloPostY;
			  z = z + veloPostZ;

			  unsigned int cbID = cellBaseID[k];
			  bool negativeDirection = false;

			  if (x >  c1o2)
			  {
				  cbID = neighborX[cbID]; 
				  x = x - c1o1;
			  }
			  if (y >  c1o2)
  			  {
				  cbID = neighborY[cbID]; 
				  y = y - c1o1;
			  }
			  if (z >  c1o2)
			  {
				  cbID = neighborZ[cbID]; 
				  z = z - c1o1;
			  }

			  real tempX = x;
			  real tempY = y;
			  real tempZ = z;

			  if ((x < -c1o2) || (y < -c1o2) || (z < -c1o2))
			  {
				  cbID = neighborWSB[cbID];
				  negativeDirection = true;
				  tempX = x + c1o1;
				  tempY = y + c1o1;
				  tempZ = z + c1o1;
			  }
			  if ((x >= -c1o2) && (negativeDirection == true))
			  {
				  cbID = neighborX[cbID]; 
				  tempX = x;
			  }
			  if ((y >= -c1o2) && (negativeDirection == true))
			  {
				  cbID = neighborY[cbID]; 
				  tempY = y;
			  }
			  if ((z >= -c1o2) && (negativeDirection == true))
			  { 
				  cbID = neighborZ[cbID]; 
				  tempZ = z;
			  }

			  x = tempX;
			  y = tempY;
			  z = tempZ;

			  localX                         = (x + c1o2) / (real)(pow((double)c2o1, (double)level));
              localY                         = (y + c1o2) / (real)(pow((double)c2o1, (double)level));
              localZ                         = (z + c1o2) / (real)(pow((double)c2o1, (double)level));
			  coordParticleXlocal[kTimeStep] = localX;
			  coordParticleYlocal[kTimeStep] = localY;
			  coordParticleZlocal[kTimeStep] = localZ;
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  real globalX = localX + coordX[cbID];
			  real globalY = localY + coordY[cbID];
			  real globalZ = localZ + coordZ[cbID];
			  coordParticleXglobal[kTimeStep] = globalX;
			  coordParticleYglobal[kTimeStep] = globalY;
			  coordParticleZglobal[kTimeStep] = globalZ;
			  //coordParticleXglobal[kTimeStep] = coordParticleXglobal[kTimeStepOld];
			  //coordParticleYglobal[kTimeStep] = coordParticleYglobal[kTimeStepOld];
			  //coordParticleZglobal[kTimeStep] = coordParticleZglobal[kTimeStepOld];
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			  cellBaseID[k] = cbID;
			  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	  
		}
   }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






































//////////////////////////////////////////////////////////////////////////////
extern "C" __global__ void ParticleNoSlipDeviceComp27(real* coordX,
													  real* coordY,
													  real* coordZ, 
													  real* coordParticleXlocal,
													  real* coordParticleYlocal,
													  real* coordParticleZlocal,
													  real* coordParticleXglobal,
													  real* coordParticleYglobal,
													  real* coordParticleZglobal,
													  real* veloParticleX,
													  real* veloParticleY,
													  real* veloParticleZ,
													  real* randArray,
													  real* DD,
													  real  omega,
													  unsigned int* particleID,
													  unsigned int* cellBaseID,
													  unsigned int* bcMatD,
													  unsigned int* neighborX,
													  unsigned int* neighborY,
													  unsigned int* neighborZ,
													  unsigned int* neighborWSB,
													  int level,
													  unsigned int numberOfTimesteps, 
													  unsigned int timestep, 
													  unsigned int numberOfParticles, 
													  int* k_Q, 
													  real* QQ,
													  unsigned int  numberOfBCnodes,
													  real* NormalX,
													  real* NormalY,
													  real* NormalZ,
													  unsigned int size_Mat, 
													  bool isEvenTimestep)
{

	//TODO: What is this function for???

   //Distributions27 D;
   //if (isEvenTimestep==true)
   //{
   //   D.f[E   ] = &DD[E   *size_Mat];
   //   D.f[W   ] = &DD[W   *size_Mat];
   //   D.f[N   ] = &DD[N   *size_Mat];
   //   D.f[S   ] = &DD[S   *size_Mat];
   //   D.f[T   ] = &DD[T   *size_Mat];
   //   D.f[B   ] = &DD[B   *size_Mat];
   //   D.f[NE  ] = &DD[NE  *size_Mat];
   //   D.f[SW  ] = &DD[SW  *size_Mat];
   //   D.f[SE  ] = &DD[SE  *size_Mat];
   //   D.f[NW  ] = &DD[NW  *size_Mat];
   //   D.f[TE  ] = &DD[TE  *size_Mat];
   //   D.f[BW  ] = &DD[BW  *size_Mat];
   //   D.f[BE  ] = &DD[BE  *size_Mat];
   //   D.f[TW  ] = &DD[TW  *size_Mat];
   //   D.f[TN  ] = &DD[TN  *size_Mat];
   //   D.f[BS  ] = &DD[BS  *size_Mat];
   //   D.f[BN  ] = &DD[BN  *size_Mat];
   //   D.f[TS  ] = &DD[TS  *size_Mat];
   //   D.f[dirREST] = &DD[dirREST*size_Mat];
   //   D.f[TNE ] = &DD[TNE *size_Mat];
   //   D.f[TSW ] = &DD[TSW *size_Mat];
   //   D.f[TSE ] = &DD[TSE *size_Mat];
   //   D.f[TNW ] = &DD[TNW *size_Mat];
   //   D.f[BNE ] = &DD[BNE *size_Mat];
   //   D.f[BSW ] = &DD[BSW *size_Mat];
   //   D.f[BSE ] = &DD[BSE *size_Mat];
   //   D.f[BNW ] = &DD[BNW *size_Mat];
   //} 
   //else
   //{
   //   D.f[W   ] = &DD[E   *size_Mat];
   //   D.f[E   ] = &DD[W   *size_Mat];
   //   D.f[S   ] = &DD[N   *size_Mat];
   //   D.f[N   ] = &DD[S   *size_Mat];
   //   D.f[B   ] = &DD[T   *size_Mat];
   //   D.f[T   ] = &DD[B   *size_Mat];
   //   D.f[SW  ] = &DD[NE  *size_Mat];
   //   D.f[NE  ] = &DD[SW  *size_Mat];
   //   D.f[NW  ] = &DD[SE  *size_Mat];
   //   D.f[SE  ] = &DD[NW  *size_Mat];
   //   D.f[BW  ] = &DD[TE  *size_Mat];
   //   D.f[TE  ] = &DD[BW  *size_Mat];
   //   D.f[TW  ] = &DD[BE  *size_Mat];
   //   D.f[BE  ] = &DD[TW  *size_Mat];
   //   D.f[BS  ] = &DD[TN  *size_Mat];
   //   D.f[TN  ] = &DD[BS  *size_Mat];
   //   D.f[TS  ] = &DD[BN  *size_Mat];
   //   D.f[BN  ] = &DD[TS  *size_Mat];
   //   D.f[dirREST] = &DD[dirREST*size_Mat];
   //   D.f[TNE ] = &DD[BSW *size_Mat];
   //   D.f[TSW ] = &DD[BNE *size_Mat];
   //   D.f[TSE ] = &DD[BNW *size_Mat];
   //   D.f[TNW ] = &DD[BSE *size_Mat];
   //   D.f[BNE ] = &DD[TSW *size_Mat];
   //   D.f[BSW ] = &DD[TNE *size_Mat];
   //   D.f[BSE ] = &DD[TNW *size_Mat];
   //   D.f[BNW ] = &DD[TSE *size_Mat];
   //}
   //////////////////////////////////////////////////////////////////////////////////
   //const unsigned  x = threadIdx.x;  // Globaler x-Index 
   //const unsigned  y = blockIdx.x;   // Globaler y-Index 
   //const unsigned  z = blockIdx.y;   // Globaler z-Index 

   //const unsigned nx = blockDim.x;
   //const unsigned ny = gridDim.x;

   //const unsigned k = nx*(ny*z + y) + x;
   ////////////////////////////////////////////////////////////////////////////

   //if(k <  numberOfBCnodes)
   //{
   //   ////////////////////////////////////////////////////////////////////////////////
   //   real *q_dirW, *q_dirS, *q_dirB;
   // //   real *q_dirE,   *q_dirW,   *q_dirN,   *q_dirS,   *q_dirT,   *q_dirB, 
   // //         *q_dirNE,  *q_dirSW,  *q_dirSE,  *q_dirNW,  *q_dirTE,  *q_dirBW,
   // //         *q_dirBE,  *q_dirTW,  *q_dirTN,  *q_dirBS,  *q_dirBN,  *q_dirTS,
   // //         *q_dirTNE, *q_dirTSW, *q_dirTSE, *q_dirTNW, *q_dirBNE, *q_dirBSW,
   // //         *q_dirBSE, *q_dirBNW; 
   // //   q_dirE   = &QQ[E   * numberOfBCnodes];
   //    q_dirW   = &QQ[W   * numberOfBCnodes];
   // //   q_dirN   = &QQ[N   * numberOfBCnodes];
   //    q_dirS   = &QQ[S   * numberOfBCnodes];
   // //   q_dirT   = &QQ[T   * numberOfBCnodes];
   //    q_dirB   = &QQ[B   * numberOfBCnodes];
   // //   q_dirNE  = &QQ[NE  * numberOfBCnodes];
   // //   q_dirSW  = &QQ[SW  * numberOfBCnodes];
   // //   q_dirSE  = &QQ[SE  * numberOfBCnodes];
   // //   q_dirNW  = &QQ[NW  * numberOfBCnodes];
   // //   q_dirTE  = &QQ[TE  * numberOfBCnodes];
   // //   q_dirBW  = &QQ[BW  * numberOfBCnodes];
   // //   q_dirBE  = &QQ[BE  * numberOfBCnodes];
   // //   q_dirTW  = &QQ[TW  * numberOfBCnodes];
   // //   q_dirTN  = &QQ[TN  * numberOfBCnodes];
   // //   q_dirBS  = &QQ[BS  * numberOfBCnodes];
   // //   q_dirBN  = &QQ[BN  * numberOfBCnodes];
   // //   q_dirTS  = &QQ[TS  * numberOfBCnodes];
   // //   q_dirTNE = &QQ[TNE * numberOfBCnodes];
   // //   q_dirTSW = &QQ[TSW * numberOfBCnodes];
   // //   q_dirTSE = &QQ[TSE * numberOfBCnodes];
   // //   q_dirTNW = &QQ[TNW * numberOfBCnodes];
   // //   q_dirBNE = &QQ[BNE * numberOfBCnodes];
   // //   q_dirBSW = &QQ[BSW * numberOfBCnodes];
   // //   q_dirBSE = &QQ[BSE * numberOfBCnodes];
   // //   q_dirBNW = &QQ[BNW * numberOfBCnodes];
   //   ////////////////////////////////////////////////////////////////////////////////
   // //   real *nx_dirE,   *nx_dirW,   *nx_dirN,   *nx_dirS,   *nx_dirT,   *nx_dirB, 
   // //           *nx_dirNE,  *nx_dirSW,  *nx_dirSE,  *nx_dirNW,  *nx_dirTE,  *nx_dirBW,
   // //           *nx_dirBE,  *nx_dirTW,  *nx_dirTN,  *nx_dirBS,  *nx_dirBN,  *nx_dirTS,
   // //           *nx_dirTNE, *nx_dirTSW, *nx_dirTSE, *nx_dirTNW, *nx_dirBNE, *nx_dirBSW,
   // //           *nx_dirBSE, *nx_dirBNW; 
   // //   nx_dirE   = &NormalX[E   * numberOfBCnodes];
   // //   nx_dirW   = &NormalX[W   * numberOfBCnodes];
   // //   nx_dirN   = &NormalX[N   * numberOfBCnodes];
   // //   nx_dirS   = &NormalX[S   * numberOfBCnodes];
   // //   nx_dirT   = &NormalX[T   * numberOfBCnodes];
   // //   nx_dirB   = &NormalX[B   * numberOfBCnodes];
   // //   nx_dirNE  = &NormalX[NE  * numberOfBCnodes];
   // //   nx_dirSW  = &NormalX[SW  * numberOfBCnodes];
   // //   nx_dirSE  = &NormalX[SE  * numberOfBCnodes];
   // //   nx_dirNW  = &NormalX[NW  * numberOfBCnodes];
   // //   nx_dirTE  = &NormalX[TE  * numberOfBCnodes];
   // //   nx_dirBW  = &NormalX[BW  * numberOfBCnodes];
   // //   nx_dirBE  = &NormalX[BE  * numberOfBCnodes];
   // //   nx_dirTW  = &NormalX[TW  * numberOfBCnodes];
   // //   nx_dirTN  = &NormalX[TN  * numberOfBCnodes];
   // //   nx_dirBS  = &NormalX[BS  * numberOfBCnodes];
   // //   nx_dirBN  = &NormalX[BN  * numberOfBCnodes];
   // //   nx_dirTS  = &NormalX[TS  * numberOfBCnodes];
   // //   nx_dirTNE = &NormalX[TNE * numberOfBCnodes];
   // //   nx_dirTSW = &NormalX[TSW * numberOfBCnodes];
   // //   nx_dirTSE = &NormalX[TSE * numberOfBCnodes];
   // //   nx_dirTNW = &NormalX[TNW * numberOfBCnodes];
   // //   nx_dirBNE = &NormalX[BNE * numberOfBCnodes];
   // //   nx_dirBSW = &NormalX[BSW * numberOfBCnodes];
   // //   nx_dirBSE = &NormalX[BSE * numberOfBCnodes];
   // //   nx_dirBNW = &NormalX[BNW * numberOfBCnodes];
   //   ////////////////////////////////////////////////////////////////////////////////
   // //   real *ny_dirE,   *ny_dirW,   *ny_dirN,   *ny_dirS,   *ny_dirT,   *ny_dirB, 
   // //           *ny_dirNE,  *ny_dirSW,  *ny_dirSE,  *ny_dirNW,  *ny_dirTE,  *ny_dirBW,
   // //           *ny_dirBE,  *ny_dirTW,  *ny_dirTN,  *ny_dirBS,  *ny_dirBN,  *ny_dirTS,
   // //           *ny_dirTNE, *ny_dirTSW, *ny_dirTSE, *ny_dirTNW, *ny_dirBNE, *ny_dirBSW,
   // //           *ny_dirBSE, *ny_dirBNW; 
   // //   ny_dirE   = &NormalY[E   * numberOfBCnodes];
   // //   ny_dirW   = &NormalY[W   * numberOfBCnodes];
   // //   ny_dirN   = &NormalY[N   * numberOfBCnodes];
   // //   ny_dirS   = &NormalY[S   * numberOfBCnodes];
   // //   ny_dirT   = &NormalY[T   * numberOfBCnodes];
   // //   ny_dirB   = &NormalY[B   * numberOfBCnodes];
   // //   ny_dirNE  = &NormalY[NE  * numberOfBCnodes];
   // //   ny_dirSW  = &NormalY[SW  * numberOfBCnodes];
   // //   ny_dirSE  = &NormalY[SE  * numberOfBCnodes];
   // //   ny_dirNW  = &NormalY[NW  * numberOfBCnodes];
   // //   ny_dirTE  = &NormalY[TE  * numberOfBCnodes];
   // //   ny_dirBW  = &NormalY[BW  * numberOfBCnodes];
   // //   ny_dirBE  = &NormalY[BE  * numberOfBCnodes];
   // //   ny_dirTW  = &NormalY[TW  * numberOfBCnodes];
   // //   ny_dirTN  = &NormalY[TN  * numberOfBCnodes];
   // //   ny_dirBS  = &NormalY[BS  * numberOfBCnodes];
   // //   ny_dirBN  = &NormalY[BN  * numberOfBCnodes];
   // //   ny_dirTS  = &NormalY[TS  * numberOfBCnodes];
   // //   ny_dirTNE = &NormalY[TNE * numberOfBCnodes];
   // //   ny_dirTSW = &NormalY[TSW * numberOfBCnodes];
   // //   ny_dirTSE = &NormalY[TSE * numberOfBCnodes];
   // //   ny_dirTNW = &NormalY[TNW * numberOfBCnodes];
   // //   ny_dirBNE = &NormalY[BNE * numberOfBCnodes];
   // //   ny_dirBSW = &NormalY[BSW * numberOfBCnodes];
   // //   ny_dirBSE = &NormalY[BSE * numberOfBCnodes];
   // //   ny_dirBNW = &NormalY[BNW * numberOfBCnodes];
   //   ////////////////////////////////////////////////////////////////////////////////
   // //   real *nz_dirE,   *nz_dirW,   *nz_dirN,   *nz_dirS,   *nz_dirT,   *nz_dirB, 
   // //           *nz_dirNE,  *nz_dirSW,  *nz_dirSE,  *nz_dirNW,  *nz_dirTE,  *nz_dirBW,
   // //           *nz_dirBE,  *nz_dirTW,  *nz_dirTN,  *nz_dirBS,  *nz_dirBN,  *nz_dirTS,
   // //           *nz_dirTNE, *nz_dirTSW, *nz_dirTSE, *nz_dirTNW, *nz_dirBNE, *nz_dirBSW,
   // //           *nz_dirBSE, *nz_dirBNW; 
   // //   nz_dirE   = &NormalZ[E   * numberOfBCnodes];
   // //   nz_dirW   = &NormalZ[W   * numberOfBCnodes];
   // //   nz_dirN   = &NormalZ[N   * numberOfBCnodes];
   // //   nz_dirS   = &NormalZ[S   * numberOfBCnodes];
   // //   nz_dirT   = &NormalZ[T   * numberOfBCnodes];
   // //   nz_dirB   = &NormalZ[B   * numberOfBCnodes];
   // //   nz_dirNE  = &NormalZ[NE  * numberOfBCnodes];
   // //   nz_dirSW  = &NormalZ[SW  * numberOfBCnodes];
   // //   nz_dirSE  = &NormalZ[SE  * numberOfBCnodes];
   // //   nz_dirNW  = &NormalZ[NW  * numberOfBCnodes];
   // //   nz_dirTE  = &NormalZ[TE  * numberOfBCnodes];
   // //   nz_dirBW  = &NormalZ[BW  * numberOfBCnodes];
   // //   nz_dirBE  = &NormalZ[BE  * numberOfBCnodes];
   // //   nz_dirTW  = &NormalZ[TW  * numberOfBCnodes];
   // //   nz_dirTN  = &NormalZ[TN  * numberOfBCnodes];
   // //   nz_dirBS  = &NormalZ[BS  * numberOfBCnodes];
   // //   nz_dirBN  = &NormalZ[BN  * numberOfBCnodes];
   // //   nz_dirTS  = &NormalZ[TS  * numberOfBCnodes];
   // //   nz_dirTNE = &NormalZ[TNE * numberOfBCnodes];
   // //   nz_dirTSW = &NormalZ[TSW * numberOfBCnodes];
   // //   nz_dirTSE = &NormalZ[TSE * numberOfBCnodes];
   // //   nz_dirTNW = &NormalZ[TNW * numberOfBCnodes];
   // //   nz_dirBNE = &NormalZ[BNE * numberOfBCnodes];
   // //   nz_dirBSW = &NormalZ[BSW * numberOfBCnodes];
   // //   nz_dirBSE = &NormalZ[BSE * numberOfBCnodes];
   // //   nz_dirBNW = &NormalZ[BNW * numberOfBCnodes];
   //   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //bool changeCell = false;
   //   unsigned int KQK  = k_Q[k];
   //   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //if( q_dirW[k] > c0o1 || q_dirS[k] > c0o1 || q_dirB[k] > c0o1 ) {
		 // KQK = neighborWSB[KQK];
		 // changeCell = true;
	  //}
	  //if( q_dirW[k] == c0o1 && changeCell == true ) {
		 // KQK = neighborX[KQK];
	  //}
	  //if( q_dirS[k] == c0o1 && changeCell == true ) {
		 // KQK = neighborY[KQK];
	  //}
	  //if( q_dirB[k] == c0o1 && changeCell == true ) {
		 // KQK = neighborZ[KQK];
	  //}

	  ////for(int i = 0; i < numberOfParticles; i++){
		 //// //push back?
	  ////}

   //   ////////////////////////////////////////////////////////////////////////////////
   //   //index
   //   //unsigned int KQK  = k_Q[k];
   //   unsigned int kzero= KQK;
   //   unsigned int ke   = KQK;
   //   unsigned int kw   = neighborX[KQK];
   //   unsigned int kn   = KQK;
   //   unsigned int ks   = neighborY[KQK];
   //   unsigned int kt   = KQK;
   //   unsigned int kb   = neighborZ[KQK];
   //   unsigned int ksw  = neighborY[kw];
   //   unsigned int kne  = KQK;
   //   unsigned int kse  = ks;
   //   unsigned int knw  = kw;
   //   unsigned int kbw  = neighborZ[kw];
   //   unsigned int kte  = KQK;
   //   unsigned int kbe  = kb;
   //   unsigned int ktw  = kw;
   //   unsigned int kbs  = neighborZ[ks];
   //   unsigned int ktn  = KQK;
   //   unsigned int kbn  = kb;
   //   unsigned int kts  = ks;
   //   unsigned int ktse = ks;
   //   unsigned int kbnw = kbw;
   //   unsigned int ktnw = kw;
   //   unsigned int kbse = kbs;
   //   unsigned int ktsw = ksw;
   //   unsigned int kbne = kb;
   //   unsigned int ktne = KQK;
   //   unsigned int kbsw = neighborZ[ksw];
   //   ////////////////////////////////////////////////////////////////////////////////
   //   real f_W    = (D.f[E   ])[ke   ];
   //   real f_E    = (D.f[W   ])[kw   ];
   //   real f_S    = (D.f[N   ])[kn   ];
   //   real f_N    = (D.f[S   ])[ks   ];
   //   real f_B    = (D.f[T   ])[kt   ];
   //   real f_T    = (D.f[B   ])[kb   ];
   //   real f_SW   = (D.f[NE  ])[kne  ];
   //   real f_NE   = (D.f[SW  ])[ksw  ];
   //   real f_NW   = (D.f[SE  ])[kse  ];
   //   real f_SE   = (D.f[NW  ])[knw  ];
   //   real f_BW   = (D.f[TE  ])[kte  ];
   //   real f_TE   = (D.f[BW  ])[kbw  ];
   //   real f_TW   = (D.f[BE  ])[kbe  ];
   //   real f_BE   = (D.f[TW  ])[ktw  ];
   //   real f_BS   = (D.f[TN  ])[ktn  ];
   //   real f_TN   = (D.f[BS  ])[kbs  ];
   //   real f_TS   = (D.f[BN  ])[kbn  ];
   //   real f_BN   = (D.f[TS  ])[kts  ];
   //   real f_BSW  = (D.f[TNE ])[ktne ];
   //   real f_BNE  = (D.f[TSW ])[ktsw ];
   //   real f_BNW  = (D.f[TSE ])[ktse ];
   //   real f_BSE  = (D.f[TNW ])[ktnw ];
   //   real f_TSW  = (D.f[BNE ])[kbne ];
   //   real f_TNE  = (D.f[BSW ])[kbsw ];
   //   real f_TNW  = (D.f[BSE ])[kbse ];
   //   real f_TSE  = (D.f[BNW ])[kbnw ];
   //   ////////////////////////////////////////////////////////////////////////////////
   //   // real feq, q;
   //   real vx1, vx2, vx3, drho;
   //   drho   =  f_TSE + f_TNW + f_TNE + f_TSW + f_BSE + f_BNW + f_BNE + f_BSW +
   //             f_BN + f_TS + f_TN + f_BS + f_BE + f_TW + f_TE + f_BW + f_SE + f_NW + f_NE + f_SW + 
   //             f_T + f_B + f_N + f_S + f_E + f_W + ((D.f[dirREST])[kzero]); 

   //   vx1    =  (((f_TSE - f_BNW) - (f_TNW - f_BSE)) + ((f_TNE - f_BSW) - (f_TSW - f_BNE)) +
   //             ((f_BE - f_TW)   + (f_TE - f_BW))   + ((f_SE - f_NW)   + (f_NE - f_SW)) +
   //             (f_E - f_W)) / (c1o1 + drho); 
   //      

   //   vx2    =   ((-(f_TSE - f_BNW) + (f_TNW - f_BSE)) + ((f_TNE - f_BSW) - (f_TSW - f_BNE)) +
   //              ((f_BN - f_TS)   + (f_TN - f_BS))    + (-(f_SE - f_NW)  + (f_NE - f_SW)) +
   //              (f_N - f_S)) / (c1o1 + drho); 

   //   vx3    =   (((f_TSE - f_BNW) + (f_TNW - f_BSE)) + ((f_TNE - f_BSW) + (f_TSW - f_BNE)) +
   //              (-(f_BN - f_TS)  + (f_TN - f_BS))   + ((f_TE - f_BW)   - (f_BE - f_TW)) +
   //              (f_T - f_B)) / (c1o1 + drho); 

   //   //real cu_sq=c3o2*(vx1*vx1+vx2*vx2+vx3*vx3) * (c1o1 + drho);

   //   //////////////////////////////////////////////////////////////////////////
   //   if (isEvenTimestep==false)
   //   {
   //      D.f[E   ] = &DD[E   *size_Mat];
   //      D.f[W   ] = &DD[W   *size_Mat];
   //      D.f[N   ] = &DD[N   *size_Mat];
   //      D.f[S   ] = &DD[S   *size_Mat];
   //      D.f[T   ] = &DD[T   *size_Mat];
   //      D.f[B   ] = &DD[B   *size_Mat];
   //      D.f[NE  ] = &DD[NE  *size_Mat];
   //      D.f[SW  ] = &DD[SW  *size_Mat];
   //      D.f[SE  ] = &DD[SE  *size_Mat];
   //      D.f[NW  ] = &DD[NW  *size_Mat];
   //      D.f[TE  ] = &DD[TE  *size_Mat];
   //      D.f[BW  ] = &DD[BW  *size_Mat];
   //      D.f[BE  ] = &DD[BE  *size_Mat];
   //      D.f[TW  ] = &DD[TW  *size_Mat];
   //      D.f[TN  ] = &DD[TN  *size_Mat];
   //      D.f[BS  ] = &DD[BS  *size_Mat];
   //      D.f[BN  ] = &DD[BN  *size_Mat];
   //      D.f[TS  ] = &DD[TS  *size_Mat];
   //      D.f[dirREST] = &DD[dirREST*size_Mat];
   //      D.f[TNE ] = &DD[TNE *size_Mat];
   //      D.f[TSW ] = &DD[TSW *size_Mat];
   //      D.f[TSE ] = &DD[TSE *size_Mat];
   //      D.f[TNW ] = &DD[TNW *size_Mat];
   //      D.f[BNE ] = &DD[BNE *size_Mat];
   //      D.f[BSW ] = &DD[BSW *size_Mat];
   //      D.f[BSE ] = &DD[BSE *size_Mat];
   //      D.f[BNW ] = &DD[BNW *size_Mat];
   //   } 
   //   else
   //   {
   //      D.f[W   ] = &DD[E   *size_Mat];
   //      D.f[E   ] = &DD[W   *size_Mat];
   //      D.f[S   ] = &DD[N   *size_Mat];
   //      D.f[N   ] = &DD[S   *size_Mat];
   //      D.f[B   ] = &DD[T   *size_Mat];
   //      D.f[T   ] = &DD[B   *size_Mat];
   //      D.f[SW  ] = &DD[NE  *size_Mat];
   //      D.f[NE  ] = &DD[SW  *size_Mat];
   //      D.f[NW  ] = &DD[SE  *size_Mat];
   //      D.f[SE  ] = &DD[NW  *size_Mat];
   //      D.f[BW  ] = &DD[TE  *size_Mat];
   //      D.f[TE  ] = &DD[BW  *size_Mat];
   //      D.f[TW  ] = &DD[BE  *size_Mat];
   //      D.f[BE  ] = &DD[TW  *size_Mat];
   //      D.f[BS  ] = &DD[TN  *size_Mat];
   //      D.f[TN  ] = &DD[BS  *size_Mat];
   //      D.f[TS  ] = &DD[BN  *size_Mat];
   //      D.f[BN  ] = &DD[TS  *size_Mat];
   //      D.f[dirREST] = &DD[dirREST*size_Mat];
   //      D.f[TNE ] = &DD[BSW *size_Mat];
   //      D.f[TSW ] = &DD[BNE *size_Mat];
   //      D.f[TSE ] = &DD[BNW *size_Mat];
   //      D.f[TNW ] = &DD[BSE *size_Mat];
   //      D.f[BNE ] = &DD[TSW *size_Mat];
   //      D.f[BSW ] = &DD[TNE *size_Mat];
   //      D.f[BSE ] = &DD[TNW *size_Mat];
   //      D.f[BNW ] = &DD[TSE *size_Mat];
   //   }
   //}
}
