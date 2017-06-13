/*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Sun Nov  7 20:45:10 EST 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -hc2hc-backward 4 */

/*
 * This function contains 34 FP additions, 18 FP multiplications,
 * (or, 28 additions, 12 multiplications, 6 fused multiply/add),
 * 15 stack variables, and 32 memory accesses
 */
static const fftw_real K1_414213562 = FFTW_KONST(+1.414213562373095048801688724209698078569671875);
static const fftw_real K2_000000000 = FFTW_KONST(+2.000000000000000000000000000000000000000000000);

/*
 * Generator Id's : 
 * $Id: fhb_4.c,v 1.1.1.1 2003/07/10 19:14:44 eml Exp $
 * $Id: fhb_4.c,v 1.1.1.1 2003/07/10 19:14:44 eml Exp $
 * $Id: fhb_4.c,v 1.1.1.1 2003/07/10 19:14:44 eml Exp $
 */

void fftw_hc2hc_backward_4(fftw_real *A, const fftw_complex *W, int iostride, int m, int dist)
{
     int i;
     fftw_real *X;
     fftw_real *Y;
     X = A;
     Y = A + (4 * iostride);
     {
	  fftw_real tmp39;
	  fftw_real tmp42;
	  fftw_real tmp37;
	  fftw_real tmp40;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp38;
	       fftw_real tmp41;
	       fftw_real tmp35;
	       fftw_real tmp36;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp38 = X[iostride];
	       tmp39 = K2_000000000 * tmp38;
	       tmp41 = Y[-iostride];
	       tmp42 = K2_000000000 * tmp41;
	       tmp35 = X[0];
	       tmp36 = X[2 * iostride];
	       tmp37 = tmp35 + tmp36;
	       tmp40 = tmp35 - tmp36;
	  }
	  X[2 * iostride] = tmp37 - tmp39;
	  X[0] = tmp37 + tmp39;
	  X[3 * iostride] = tmp40 + tmp42;
	  X[iostride] = tmp40 - tmp42;
     }
     X = X + dist;
     Y = Y - dist;
     for (i = 2; i < m; i = i + 2, X = X + dist, Y = Y - dist, W = W + 3) {
	  fftw_real tmp9;
	  fftw_real tmp28;
	  fftw_real tmp18;
	  fftw_real tmp25;
	  fftw_real tmp12;
	  fftw_real tmp24;
	  fftw_real tmp21;
	  fftw_real tmp29;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp7;
	       fftw_real tmp8;
	       fftw_real tmp16;
	       fftw_real tmp17;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp7 = X[0];
	       tmp8 = Y[-2 * iostride];
	       tmp9 = tmp7 + tmp8;
	       tmp28 = tmp7 - tmp8;
	       tmp16 = Y[0];
	       tmp17 = X[2 * iostride];
	       tmp18 = tmp16 - tmp17;
	       tmp25 = tmp16 + tmp17;
	  }
	  {
	       fftw_real tmp10;
	       fftw_real tmp11;
	       fftw_real tmp19;
	       fftw_real tmp20;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp10 = X[iostride];
	       tmp11 = Y[-3 * iostride];
	       tmp12 = tmp10 + tmp11;
	       tmp24 = tmp10 - tmp11;
	       tmp19 = Y[-iostride];
	       tmp20 = X[3 * iostride];
	       tmp21 = tmp19 - tmp20;
	       tmp29 = tmp19 + tmp20;
	  }
	  X[0] = tmp9 + tmp12;
	  {
	       fftw_real tmp14;
	       fftw_real tmp22;
	       fftw_real tmp13;
	       fftw_real tmp15;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp14 = tmp9 - tmp12;
	       tmp22 = tmp18 - tmp21;
	       tmp13 = c_re(W[1]);
	       tmp15 = c_im(W[1]);
	       X[2 * iostride] = (tmp13 * tmp14) + (tmp15 * tmp22);
	       Y[-iostride] = (tmp13 * tmp22) - (tmp15 * tmp14);
	  }
	  Y[-3 * iostride] = tmp18 + tmp21;
	  {
	       fftw_real tmp26;
	       fftw_real tmp30;
	       fftw_real tmp23;
	       fftw_real tmp27;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp26 = tmp24 + tmp25;
	       tmp30 = tmp28 - tmp29;
	       tmp23 = c_re(W[0]);
	       tmp27 = c_im(W[0]);
	       Y[-2 * iostride] = (tmp23 * tmp26) - (tmp27 * tmp30);
	       X[iostride] = (tmp27 * tmp26) + (tmp23 * tmp30);
	  }
	  {
	       fftw_real tmp32;
	       fftw_real tmp34;
	       fftw_real tmp31;
	       fftw_real tmp33;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp32 = tmp25 - tmp24;
	       tmp34 = tmp28 + tmp29;
	       tmp31 = c_re(W[2]);
	       tmp33 = c_im(W[2]);
	       Y[0] = (tmp31 * tmp32) - (tmp33 * tmp34);
	       X[3 * iostride] = (tmp33 * tmp32) + (tmp31 * tmp34);
	  }
     }
     if (i == m) {
	  fftw_real tmp1;
	  fftw_real tmp2;
	  fftw_real tmp3;
	  fftw_real tmp4;
	  fftw_real tmp5;
	  fftw_real tmp6;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = X[0];
	  tmp2 = X[iostride];
	  tmp3 = tmp1 - tmp2;
	  tmp4 = Y[0];
	  tmp5 = Y[-iostride];
	  tmp6 = tmp4 + tmp5;
	  X[0] = K2_000000000 * (tmp1 + tmp2);
	  X[2 * iostride] = -(K2_000000000 * (tmp4 - tmp5));
	  X[iostride] = K1_414213562 * (tmp3 - tmp6);
	  X[3 * iostride] = -(K1_414213562 * (tmp3 + tmp6));
     }
}

static const int twiddle_order[] =
{1, 2, 3};
fftw_codelet_desc fftw_hc2hc_backward_4_desc =
{
     "fftw_hc2hc_backward_4",
     (void (*)()) fftw_hc2hc_backward_4,
     4,
     FFTW_BACKWARD,
     FFTW_HC2HC,
     102,
     3,
     twiddle_order,
};
