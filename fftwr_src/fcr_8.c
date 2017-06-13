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
/* Generated on Sun Nov  7 20:44:20 EST 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -hc2real 8 */

/*
 * This function contains 20 FP additions, 6 FP multiplications,
 * (or, 20 additions, 6 multiplications, 0 fused multiply/add),
 * 18 stack variables, and 16 memory accesses
 */
static const fftw_real K1_414213562 = FFTW_KONST(+1.414213562373095048801688724209698078569671875);
static const fftw_real K2_000000000 = FFTW_KONST(+2.000000000000000000000000000000000000000000000);

/*
 * Generator Id's : 
 * $Id: fcr_8.c,v 1.1.1.1 2003/07/10 19:14:44 eml Exp $
 * $Id: fcr_8.c,v 1.1.1.1 2003/07/10 19:14:44 eml Exp $
 * $Id: fcr_8.c,v 1.1.1.1 2003/07/10 19:14:44 eml Exp $
 */

void fftw_hc2real_8(const fftw_real *real_input, const fftw_real *imag_input, fftw_real *output, int real_istride, int imag_istride, int ostride)
{
     fftw_real tmp5;
     fftw_real tmp12;
     fftw_real tmp3;
     fftw_real tmp10;
     fftw_real tmp9;
     fftw_real tmp14;
     fftw_real tmp17;
     fftw_real tmp22;
     fftw_real tmp6;
     fftw_real tmp21;
     ASSERT_ALIGNED_DOUBLE;
     {
	  fftw_real tmp4;
	  fftw_real tmp11;
	  fftw_real tmp1;
	  fftw_real tmp2;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp4 = real_input[2 * real_istride];
	  tmp5 = K2_000000000 * tmp4;
	  tmp11 = imag_input[2 * imag_istride];
	  tmp12 = K2_000000000 * tmp11;
	  tmp1 = real_input[0];
	  tmp2 = real_input[4 * real_istride];
	  tmp3 = tmp1 + tmp2;
	  tmp10 = tmp1 - tmp2;
	  {
	       fftw_real tmp7;
	       fftw_real tmp8;
	       fftw_real tmp15;
	       fftw_real tmp16;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp7 = real_input[real_istride];
	       tmp8 = real_input[3 * real_istride];
	       tmp9 = K2_000000000 * (tmp7 + tmp8);
	       tmp14 = tmp7 - tmp8;
	       tmp15 = imag_input[3 * imag_istride];
	       tmp16 = imag_input[imag_istride];
	       tmp17 = tmp15 + tmp16;
	       tmp22 = K2_000000000 * (tmp16 - tmp15);
	  }
     }
     tmp6 = tmp3 + tmp5;
     output[4 * ostride] = tmp6 - tmp9;
     output[0] = tmp6 + tmp9;
     tmp21 = tmp3 - tmp5;
     output[2 * ostride] = tmp21 - tmp22;
     output[6 * ostride] = tmp21 + tmp22;
     {
	  fftw_real tmp13;
	  fftw_real tmp18;
	  fftw_real tmp19;
	  fftw_real tmp20;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp13 = tmp10 - tmp12;
	  tmp18 = K1_414213562 * (tmp14 - tmp17);
	  output[5 * ostride] = tmp13 - tmp18;
	  output[ostride] = tmp13 + tmp18;
	  tmp19 = tmp10 + tmp12;
	  tmp20 = K1_414213562 * (tmp14 + tmp17);
	  output[3 * ostride] = tmp19 - tmp20;
	  output[7 * ostride] = tmp19 + tmp20;
     }
}

fftw_codelet_desc fftw_hc2real_8_desc =
{
     "fftw_hc2real_8",
     (void (*)()) fftw_hc2real_8,
     8,
     FFTW_BACKWARD,
     FFTW_HC2REAL,
     191,
     0,
     (const int *) 0,
};
