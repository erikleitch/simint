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
/* Generated on Sun Nov  7 20:43:55 EST 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -real2hc 10 */

/*
 * This function contains 34 FP additions, 12 FP multiplications,
 * (or, 28 additions, 6 multiplications, 6 fused multiply/add),
 * 22 stack variables, and 20 memory accesses
 */
static const fftw_real K587785252 = FFTW_KONST(+0.587785252292473129168705954639072768597652438);
static const fftw_real K951056516 = FFTW_KONST(+0.951056516295153572116439333379382143405698634);
static const fftw_real K250000000 = FFTW_KONST(+0.250000000000000000000000000000000000000000000);
static const fftw_real K559016994 = FFTW_KONST(+0.559016994374947424102293417182819058860154590);

/*
 * Generator Id's : 
 * $Id: frc_10.c,v 1.1.1.1 2003/07/10 19:14:45 eml Exp $
 * $Id: frc_10.c,v 1.1.1.1 2003/07/10 19:14:45 eml Exp $
 * $Id: frc_10.c,v 1.1.1.1 2003/07/10 19:14:45 eml Exp $
 */

void fftw_real2hc_10(const fftw_real *input, fftw_real *real_output, fftw_real *imag_output, int istride, int real_ostride, int imag_ostride)
{
     fftw_real tmp18;
     fftw_real tmp29;
     fftw_real tmp10;
     fftw_real tmp23;
     fftw_real tmp13;
     fftw_real tmp24;
     fftw_real tmp14;
     fftw_real tmp31;
     fftw_real tmp3;
     fftw_real tmp26;
     fftw_real tmp6;
     fftw_real tmp27;
     fftw_real tmp7;
     fftw_real tmp30;
     fftw_real tmp16;
     fftw_real tmp17;
     ASSERT_ALIGNED_DOUBLE;
     tmp16 = input[0];
     tmp17 = input[5 * istride];
     tmp18 = tmp16 - tmp17;
     tmp29 = tmp16 + tmp17;
     {
	  fftw_real tmp8;
	  fftw_real tmp9;
	  fftw_real tmp11;
	  fftw_real tmp12;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp8 = input[4 * istride];
	  tmp9 = input[9 * istride];
	  tmp10 = tmp8 - tmp9;
	  tmp23 = tmp8 + tmp9;
	  tmp11 = input[6 * istride];
	  tmp12 = input[istride];
	  tmp13 = tmp11 - tmp12;
	  tmp24 = tmp11 + tmp12;
     }
     tmp14 = tmp10 + tmp13;
     tmp31 = tmp23 + tmp24;
     {
	  fftw_real tmp1;
	  fftw_real tmp2;
	  fftw_real tmp4;
	  fftw_real tmp5;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = input[2 * istride];
	  tmp2 = input[7 * istride];
	  tmp3 = tmp1 - tmp2;
	  tmp26 = tmp1 + tmp2;
	  tmp4 = input[8 * istride];
	  tmp5 = input[3 * istride];
	  tmp6 = tmp4 - tmp5;
	  tmp27 = tmp4 + tmp5;
     }
     tmp7 = tmp3 + tmp6;
     tmp30 = tmp26 + tmp27;
     {
	  fftw_real tmp15;
	  fftw_real tmp19;
	  fftw_real tmp20;
	  fftw_real tmp34;
	  fftw_real tmp32;
	  fftw_real tmp33;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp15 = K559016994 * (tmp7 - tmp14);
	  tmp19 = tmp7 + tmp14;
	  tmp20 = tmp18 - (K250000000 * tmp19);
	  real_output[real_ostride] = tmp15 + tmp20;
	  real_output[3 * real_ostride] = tmp20 - tmp15;
	  real_output[5 * real_ostride] = tmp18 + tmp19;
	  tmp34 = K559016994 * (tmp30 - tmp31);
	  tmp32 = tmp30 + tmp31;
	  tmp33 = tmp29 - (K250000000 * tmp32);
	  real_output[2 * real_ostride] = tmp33 - tmp34;
	  real_output[4 * real_ostride] = tmp34 + tmp33;
	  real_output[0] = tmp29 + tmp32;
     }
     {
	  fftw_real tmp21;
	  fftw_real tmp22;
	  fftw_real tmp25;
	  fftw_real tmp28;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp21 = tmp3 - tmp6;
	  tmp22 = tmp10 - tmp13;
	  imag_output[imag_ostride] = -((K951056516 * tmp21) + (K587785252 * tmp22));
	  imag_output[3 * imag_ostride] = (K587785252 * tmp21) - (K951056516 * tmp22);
	  tmp25 = tmp23 - tmp24;
	  tmp28 = tmp26 - tmp27;
	  imag_output[2 * imag_ostride] = (K951056516 * tmp25) - (K587785252 * tmp28);
	  imag_output[4 * imag_ostride] = (K951056516 * tmp28) + (K587785252 * tmp25);
     }
}

fftw_codelet_desc fftw_real2hc_10_desc =
{
     "fftw_real2hc_10",
     (void (*)()) fftw_real2hc_10,
     10,
     FFTW_FORWARD,
     FFTW_REAL2HC,
     222,
     0,
     (const int *) 0,
};
