/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Sun Oct 29 08:17:34 EDT 2017 */

#include "dft/codelet-dft.h"

#if defined(ARCH_PREFERS_FMA) || defined(ISA_EXTENSION_PREFERS_FMA)

/* Generated by: ../../../genfft/gen_twiddle.native -fma -simd -compact -variables 4 -pipeline-latency 8 -twiddle-log3 -precompute-twiddles -n 4 -name t2sv_4 -include dft/simd/ts.h */

/*
 * This function contains 24 FP additions, 16 FP multiplications,
 * (or, 16 additions, 8 multiplications, 8 fused multiply/add),
 * 21 stack variables, 0 constants, and 16 memory accesses
 */
#include "dft/simd/ts.h"

static void t2sv_4(R *ri, R *ii, const R *W, stride rs, INT mb, INT me, INT ms)
{
     {
	  INT m;
	  for (m = mb, W = W + (mb * 4); m < me; m = m + (2 * VL), ri = ri + ((2 * VL) * ms), ii = ii + ((2 * VL) * ms), W = W + ((2 * VL) * 4), MAKE_VOLATILE_STRIDE(8, rs)) {
	       V T2, T6, T3, T5, T7, Tb, T4, Ta;
	       T2 = LDW(&(W[0]));
	       T6 = LDW(&(W[TWVL * 3]));
	       T3 = LDW(&(W[TWVL * 2]));
	       T4 = VMUL(T2, T3);
	       Ta = VMUL(T2, T6);
	       T5 = LDW(&(W[TWVL * 1]));
	       T7 = VFMA(T5, T6, T4);
	       Tb = VFNMS(T5, T3, Ta);
	       {
		    V T1, Tx, Td, Tw, Ti, Tq, Tm, Ts;
		    T1 = LD(&(ri[0]), ms, &(ri[0]));
		    Tx = LD(&(ii[0]), ms, &(ii[0]));
		    {
			 V T8, T9, Tc, Tv;
			 T8 = LD(&(ri[WS(rs, 2)]), ms, &(ri[0]));
			 T9 = VMUL(T7, T8);
			 Tc = LD(&(ii[WS(rs, 2)]), ms, &(ii[0]));
			 Tv = VMUL(T7, Tc);
			 Td = VFMA(Tb, Tc, T9);
			 Tw = VFNMS(Tb, T8, Tv);
		    }
		    {
			 V Tf, Tg, Th, Tp;
			 Tf = LD(&(ri[WS(rs, 1)]), ms, &(ri[WS(rs, 1)]));
			 Tg = VMUL(T2, Tf);
			 Th = LD(&(ii[WS(rs, 1)]), ms, &(ii[WS(rs, 1)]));
			 Tp = VMUL(T2, Th);
			 Ti = VFMA(T5, Th, Tg);
			 Tq = VFNMS(T5, Tf, Tp);
		    }
		    {
			 V Tj, Tk, Tl, Tr;
			 Tj = LD(&(ri[WS(rs, 3)]), ms, &(ri[WS(rs, 1)]));
			 Tk = VMUL(T3, Tj);
			 Tl = LD(&(ii[WS(rs, 3)]), ms, &(ii[WS(rs, 1)]));
			 Tr = VMUL(T3, Tl);
			 Tm = VFMA(T6, Tl, Tk);
			 Ts = VFNMS(T6, Tj, Tr);
		    }
		    {
			 V Te, Tn, Tu, Ty;
			 Te = VADD(T1, Td);
			 Tn = VADD(Ti, Tm);
			 ST(&(ri[WS(rs, 2)]), VSUB(Te, Tn), ms, &(ri[0]));
			 ST(&(ri[0]), VADD(Te, Tn), ms, &(ri[0]));
			 Tu = VADD(Tq, Ts);
			 Ty = VADD(Tw, Tx);
			 ST(&(ii[0]), VADD(Tu, Ty), ms, &(ii[0]));
			 ST(&(ii[WS(rs, 2)]), VSUB(Ty, Tu), ms, &(ii[0]));
		    }
		    {
			 V To, Tt, Tz, TA;
			 To = VSUB(T1, Td);
			 Tt = VSUB(Tq, Ts);
			 ST(&(ri[WS(rs, 3)]), VSUB(To, Tt), ms, &(ri[WS(rs, 1)]));
			 ST(&(ri[WS(rs, 1)]), VADD(To, Tt), ms, &(ri[WS(rs, 1)]));
			 Tz = VSUB(Tx, Tw);
			 TA = VSUB(Ti, Tm);
			 ST(&(ii[WS(rs, 1)]), VSUB(Tz, TA), ms, &(ii[WS(rs, 1)]));
			 ST(&(ii[WS(rs, 3)]), VADD(TA, Tz), ms, &(ii[WS(rs, 1)]));
		    }
	       }
	  }
     }
     VLEAVE();
}

static const tw_instr twinstr[] = {
     VTW(0, 1),
     VTW(0, 3),
     {TW_NEXT, (2 * VL), 0}
};

static const ct_desc desc = { 4, XSIMD_STRING("t2sv_4"), twinstr, &GENUS, {16, 8, 8, 0}, 0, 0, 0 };

void XSIMD(codelet_t2sv_4) (planner *p) {
     X(kdft_dit_register) (p, t2sv_4, &desc);
}
#else

/* Generated by: ../../../genfft/gen_twiddle.native -simd -compact -variables 4 -pipeline-latency 8 -twiddle-log3 -precompute-twiddles -n 4 -name t2sv_4 -include dft/simd/ts.h */

/*
 * This function contains 24 FP additions, 16 FP multiplications,
 * (or, 16 additions, 8 multiplications, 8 fused multiply/add),
 * 21 stack variables, 0 constants, and 16 memory accesses
 */
#include "dft/simd/ts.h"

static void t2sv_4(R *ri, R *ii, const R *W, stride rs, INT mb, INT me, INT ms)
{
     {
	  INT m;
	  for (m = mb, W = W + (mb * 4); m < me; m = m + (2 * VL), ri = ri + ((2 * VL) * ms), ii = ii + ((2 * VL) * ms), W = W + ((2 * VL) * 4), MAKE_VOLATILE_STRIDE(8, rs)) {
	       V T2, T4, T3, T5, T6, T8;
	       T2 = LDW(&(W[0]));
	       T4 = LDW(&(W[TWVL * 1]));
	       T3 = LDW(&(W[TWVL * 2]));
	       T5 = LDW(&(W[TWVL * 3]));
	       T6 = VFMA(T2, T3, VMUL(T4, T5));
	       T8 = VFNMS(T4, T3, VMUL(T2, T5));
	       {
		    V T1, Tp, Ta, To, Te, Tk, Th, Tl, T7, T9;
		    T1 = LD(&(ri[0]), ms, &(ri[0]));
		    Tp = LD(&(ii[0]), ms, &(ii[0]));
		    T7 = LD(&(ri[WS(rs, 2)]), ms, &(ri[0]));
		    T9 = LD(&(ii[WS(rs, 2)]), ms, &(ii[0]));
		    Ta = VFMA(T6, T7, VMUL(T8, T9));
		    To = VFNMS(T8, T7, VMUL(T6, T9));
		    {
			 V Tc, Td, Tf, Tg;
			 Tc = LD(&(ri[WS(rs, 1)]), ms, &(ri[WS(rs, 1)]));
			 Td = LD(&(ii[WS(rs, 1)]), ms, &(ii[WS(rs, 1)]));
			 Te = VFMA(T2, Tc, VMUL(T4, Td));
			 Tk = VFNMS(T4, Tc, VMUL(T2, Td));
			 Tf = LD(&(ri[WS(rs, 3)]), ms, &(ri[WS(rs, 1)]));
			 Tg = LD(&(ii[WS(rs, 3)]), ms, &(ii[WS(rs, 1)]));
			 Th = VFMA(T3, Tf, VMUL(T5, Tg));
			 Tl = VFNMS(T5, Tf, VMUL(T3, Tg));
		    }
		    {
			 V Tb, Ti, Tn, Tq;
			 Tb = VADD(T1, Ta);
			 Ti = VADD(Te, Th);
			 ST(&(ri[WS(rs, 2)]), VSUB(Tb, Ti), ms, &(ri[0]));
			 ST(&(ri[0]), VADD(Tb, Ti), ms, &(ri[0]));
			 Tn = VADD(Tk, Tl);
			 Tq = VADD(To, Tp);
			 ST(&(ii[0]), VADD(Tn, Tq), ms, &(ii[0]));
			 ST(&(ii[WS(rs, 2)]), VSUB(Tq, Tn), ms, &(ii[0]));
		    }
		    {
			 V Tj, Tm, Tr, Ts;
			 Tj = VSUB(T1, Ta);
			 Tm = VSUB(Tk, Tl);
			 ST(&(ri[WS(rs, 3)]), VSUB(Tj, Tm), ms, &(ri[WS(rs, 1)]));
			 ST(&(ri[WS(rs, 1)]), VADD(Tj, Tm), ms, &(ri[WS(rs, 1)]));
			 Tr = VSUB(Tp, To);
			 Ts = VSUB(Te, Th);
			 ST(&(ii[WS(rs, 1)]), VSUB(Tr, Ts), ms, &(ii[WS(rs, 1)]));
			 ST(&(ii[WS(rs, 3)]), VADD(Ts, Tr), ms, &(ii[WS(rs, 1)]));
		    }
	       }
	  }
     }
     VLEAVE();
}

static const tw_instr twinstr[] = {
     VTW(0, 1),
     VTW(0, 3),
     {TW_NEXT, (2 * VL), 0}
};

static const ct_desc desc = { 4, XSIMD_STRING("t2sv_4"), twinstr, &GENUS, {16, 8, 8, 0}, 0, 0, 0 };

void XSIMD(codelet_t2sv_4) (planner *p) {
     X(kdft_dit_register) (p, t2sv_4, &desc);
}
#endif
