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
/* Generated on Sun Nov  7 20:44:16 EST 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -notwiddleinv 32 */

/*
 * This function contains 372 FP additions, 84 FP multiplications,
 * (or, 340 additions, 52 multiplications, 32 fused multiply/add),
 * 92 stack variables, and 128 memory accesses
 */
static const fftw_real K195090322 = FFTW_KONST(+0.195090322016128267848284868477022240927691618);
static const fftw_real K980785280 = FFTW_KONST(+0.980785280403230449126182236134239036973933731);
static const fftw_real K831469612 = FFTW_KONST(+0.831469612302545237078788377617905756738560812);
static const fftw_real K555570233 = FFTW_KONST(+0.555570233019602224742830813948532874374937191);
static const fftw_real K382683432 = FFTW_KONST(+0.382683432365089771728459984030398866761344562);
static const fftw_real K923879532 = FFTW_KONST(+0.923879532511286756128183189396788286822416626);
static const fftw_real K707106781 = FFTW_KONST(+0.707106781186547524400844362104849039284835938);

/*
 * Generator Id's : 
 * $Id: fni_32.c,v 1.1.1.1 2003/07/10 19:14:45 eml Exp $
 * $Id: fni_32.c,v 1.1.1.1 2003/07/10 19:14:45 eml Exp $
 * $Id: fni_32.c,v 1.1.1.1 2003/07/10 19:14:45 eml Exp $
 */

void fftwi_no_twiddle_32(const fftw_complex *input, fftw_complex *output, int istride, int ostride)
{
     fftw_real tmp7;
     fftw_real tmp339;
     fftw_real tmp70;
     fftw_real tmp313;
     fftw_real tmp97;
     fftw_real tmp215;
     fftw_real tmp179;
     fftw_real tmp241;
     fftw_real tmp14;
     fftw_real tmp314;
     fftw_real tmp77;
     fftw_real tmp340;
     fftw_real tmp182;
     fftw_real tmp216;
     fftw_real tmp104;
     fftw_real tmp242;
     fftw_real tmp153;
     fftw_real tmp236;
     fftw_real tmp53;
     fftw_real tmp60;
     fftw_real tmp287;
     fftw_real tmp336;
     fftw_real tmp360;
     fftw_real tmp290;
     fftw_real tmp293;
     fftw_real tmp294;
     fftw_real tmp170;
     fftw_real tmp233;
     fftw_real tmp333;
     fftw_real tmp359;
     fftw_real tmp164;
     fftw_real tmp234;
     fftw_real tmp173;
     fftw_real tmp237;
     fftw_real tmp22;
     fftw_real tmp318;
     fftw_real tmp343;
     fftw_real tmp85;
     fftw_real tmp112;
     fftw_real tmp185;
     fftw_real tmp220;
     fftw_real tmp245;
     fftw_real tmp29;
     fftw_real tmp321;
     fftw_real tmp342;
     fftw_real tmp92;
     fftw_real tmp119;
     fftw_real tmp184;
     fftw_real tmp223;
     fftw_real tmp244;
     fftw_real tmp126;
     fftw_real tmp229;
     fftw_real tmp38;
     fftw_real tmp45;
     fftw_real tmp278;
     fftw_real tmp329;
     fftw_real tmp357;
     fftw_real tmp281;
     fftw_real tmp284;
     fftw_real tmp285;
     fftw_real tmp143;
     fftw_real tmp226;
     fftw_real tmp326;
     fftw_real tmp356;
     fftw_real tmp137;
     fftw_real tmp227;
     fftw_real tmp146;
     fftw_real tmp230;
     ASSERT_ALIGNED_DOUBLE;
     {
	  fftw_real tmp3;
	  fftw_real tmp177;
	  fftw_real tmp66;
	  fftw_real tmp96;
	  fftw_real tmp6;
	  fftw_real tmp95;
	  fftw_real tmp69;
	  fftw_real tmp178;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp1;
	       fftw_real tmp2;
	       fftw_real tmp64;
	       fftw_real tmp65;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp1 = c_re(input[0]);
	       tmp2 = c_re(input[16 * istride]);
	       tmp3 = tmp1 + tmp2;
	       tmp177 = tmp1 - tmp2;
	       tmp64 = c_im(input[0]);
	       tmp65 = c_im(input[16 * istride]);
	       tmp66 = tmp64 + tmp65;
	       tmp96 = tmp64 - tmp65;
	  }
	  {
	       fftw_real tmp4;
	       fftw_real tmp5;
	       fftw_real tmp67;
	       fftw_real tmp68;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp4 = c_re(input[8 * istride]);
	       tmp5 = c_re(input[24 * istride]);
	       tmp6 = tmp4 + tmp5;
	       tmp95 = tmp4 - tmp5;
	       tmp67 = c_im(input[8 * istride]);
	       tmp68 = c_im(input[24 * istride]);
	       tmp69 = tmp67 + tmp68;
	       tmp178 = tmp67 - tmp68;
	  }
	  tmp7 = tmp3 + tmp6;
	  tmp339 = tmp3 - tmp6;
	  tmp70 = tmp66 + tmp69;
	  tmp313 = tmp66 - tmp69;
	  tmp97 = tmp95 + tmp96;
	  tmp215 = tmp96 - tmp95;
	  tmp179 = tmp177 - tmp178;
	  tmp241 = tmp177 + tmp178;
     }
     {
	  fftw_real tmp10;
	  fftw_real tmp98;
	  fftw_real tmp73;
	  fftw_real tmp99;
	  fftw_real tmp13;
	  fftw_real tmp102;
	  fftw_real tmp76;
	  fftw_real tmp101;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp8;
	       fftw_real tmp9;
	       fftw_real tmp71;
	       fftw_real tmp72;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp8 = c_re(input[4 * istride]);
	       tmp9 = c_re(input[20 * istride]);
	       tmp10 = tmp8 + tmp9;
	       tmp98 = tmp8 - tmp9;
	       tmp71 = c_im(input[4 * istride]);
	       tmp72 = c_im(input[20 * istride]);
	       tmp73 = tmp71 + tmp72;
	       tmp99 = tmp71 - tmp72;
	  }
	  {
	       fftw_real tmp11;
	       fftw_real tmp12;
	       fftw_real tmp74;
	       fftw_real tmp75;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp11 = c_re(input[28 * istride]);
	       tmp12 = c_re(input[12 * istride]);
	       tmp13 = tmp11 + tmp12;
	       tmp102 = tmp11 - tmp12;
	       tmp74 = c_im(input[28 * istride]);
	       tmp75 = c_im(input[12 * istride]);
	       tmp76 = tmp74 + tmp75;
	       tmp101 = tmp74 - tmp75;
	  }
	  tmp14 = tmp10 + tmp13;
	  tmp314 = tmp10 - tmp13;
	  tmp77 = tmp73 + tmp76;
	  tmp340 = tmp76 - tmp73;
	  {
	       fftw_real tmp180;
	       fftw_real tmp181;
	       fftw_real tmp100;
	       fftw_real tmp103;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp180 = tmp98 - tmp99;
	       tmp181 = tmp102 + tmp101;
	       tmp182 = K707106781 * (tmp180 + tmp181);
	       tmp216 = K707106781 * (tmp180 - tmp181);
	       tmp100 = tmp98 + tmp99;
	       tmp103 = tmp101 - tmp102;
	       tmp104 = K707106781 * (tmp100 + tmp103);
	       tmp242 = K707106781 * (tmp103 - tmp100);
	  }
     }
     {
	  fftw_real tmp49;
	  fftw_real tmp149;
	  fftw_real tmp169;
	  fftw_real tmp288;
	  fftw_real tmp52;
	  fftw_real tmp166;
	  fftw_real tmp152;
	  fftw_real tmp289;
	  fftw_real tmp56;
	  fftw_real tmp154;
	  fftw_real tmp157;
	  fftw_real tmp291;
	  fftw_real tmp59;
	  fftw_real tmp159;
	  fftw_real tmp162;
	  fftw_real tmp292;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp47;
	       fftw_real tmp48;
	       fftw_real tmp167;
	       fftw_real tmp168;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp47 = c_re(input[31 * istride]);
	       tmp48 = c_re(input[15 * istride]);
	       tmp49 = tmp47 + tmp48;
	       tmp149 = tmp47 - tmp48;
	       tmp167 = c_im(input[31 * istride]);
	       tmp168 = c_im(input[15 * istride]);
	       tmp169 = tmp167 - tmp168;
	       tmp288 = tmp167 + tmp168;
	  }
	  {
	       fftw_real tmp50;
	       fftw_real tmp51;
	       fftw_real tmp150;
	       fftw_real tmp151;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp50 = c_re(input[7 * istride]);
	       tmp51 = c_re(input[23 * istride]);
	       tmp52 = tmp50 + tmp51;
	       tmp166 = tmp50 - tmp51;
	       tmp150 = c_im(input[7 * istride]);
	       tmp151 = c_im(input[23 * istride]);
	       tmp152 = tmp150 - tmp151;
	       tmp289 = tmp150 + tmp151;
	  }
	  {
	       fftw_real tmp54;
	       fftw_real tmp55;
	       fftw_real tmp155;
	       fftw_real tmp156;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp54 = c_re(input[3 * istride]);
	       tmp55 = c_re(input[19 * istride]);
	       tmp56 = tmp54 + tmp55;
	       tmp154 = tmp54 - tmp55;
	       tmp155 = c_im(input[3 * istride]);
	       tmp156 = c_im(input[19 * istride]);
	       tmp157 = tmp155 - tmp156;
	       tmp291 = tmp155 + tmp156;
	  }
	  {
	       fftw_real tmp57;
	       fftw_real tmp58;
	       fftw_real tmp160;
	       fftw_real tmp161;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp57 = c_re(input[27 * istride]);
	       tmp58 = c_re(input[11 * istride]);
	       tmp59 = tmp57 + tmp58;
	       tmp159 = tmp57 - tmp58;
	       tmp160 = c_im(input[27 * istride]);
	       tmp161 = c_im(input[11 * istride]);
	       tmp162 = tmp160 - tmp161;
	       tmp292 = tmp160 + tmp161;
	  }
	  {
	       fftw_real tmp334;
	       fftw_real tmp335;
	       fftw_real tmp331;
	       fftw_real tmp332;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp153 = tmp149 - tmp152;
	       tmp236 = tmp149 + tmp152;
	       tmp53 = tmp49 + tmp52;
	       tmp60 = tmp56 + tmp59;
	       tmp287 = tmp53 - tmp60;
	       tmp334 = tmp49 - tmp52;
	       tmp335 = tmp292 - tmp291;
	       tmp336 = tmp334 - tmp335;
	       tmp360 = tmp334 + tmp335;
	       tmp290 = tmp288 + tmp289;
	       tmp293 = tmp291 + tmp292;
	       tmp294 = tmp290 - tmp293;
	       tmp170 = tmp166 + tmp169;
	       tmp233 = tmp169 - tmp166;
	       tmp331 = tmp288 - tmp289;
	       tmp332 = tmp56 - tmp59;
	       tmp333 = tmp331 - tmp332;
	       tmp359 = tmp332 + tmp331;
	       {
		    fftw_real tmp158;
		    fftw_real tmp163;
		    fftw_real tmp171;
		    fftw_real tmp172;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp158 = tmp154 - tmp157;
		    tmp163 = tmp159 + tmp162;
		    tmp164 = K707106781 * (tmp158 + tmp163);
		    tmp234 = K707106781 * (tmp158 - tmp163);
		    tmp171 = tmp154 + tmp157;
		    tmp172 = tmp162 - tmp159;
		    tmp173 = K707106781 * (tmp171 + tmp172);
		    tmp237 = K707106781 * (tmp172 - tmp171);
	       }
	  }
     }
     {
	  fftw_real tmp18;
	  fftw_real tmp106;
	  fftw_real tmp81;
	  fftw_real tmp110;
	  fftw_real tmp21;
	  fftw_real tmp109;
	  fftw_real tmp84;
	  fftw_real tmp107;
	  fftw_real tmp316;
	  fftw_real tmp317;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp16;
	       fftw_real tmp17;
	       fftw_real tmp79;
	       fftw_real tmp80;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp16 = c_re(input[2 * istride]);
	       tmp17 = c_re(input[18 * istride]);
	       tmp18 = tmp16 + tmp17;
	       tmp106 = tmp16 - tmp17;
	       tmp79 = c_im(input[2 * istride]);
	       tmp80 = c_im(input[18 * istride]);
	       tmp81 = tmp79 + tmp80;
	       tmp110 = tmp79 - tmp80;
	  }
	  {
	       fftw_real tmp19;
	       fftw_real tmp20;
	       fftw_real tmp82;
	       fftw_real tmp83;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp19 = c_re(input[10 * istride]);
	       tmp20 = c_re(input[26 * istride]);
	       tmp21 = tmp19 + tmp20;
	       tmp109 = tmp19 - tmp20;
	       tmp82 = c_im(input[10 * istride]);
	       tmp83 = c_im(input[26 * istride]);
	       tmp84 = tmp82 + tmp83;
	       tmp107 = tmp82 - tmp83;
	  }
	  tmp22 = tmp18 + tmp21;
	  tmp316 = tmp18 - tmp21;
	  tmp317 = tmp81 - tmp84;
	  tmp318 = tmp316 - tmp317;
	  tmp343 = tmp316 + tmp317;
	  tmp85 = tmp81 + tmp84;
	  {
	       fftw_real tmp108;
	       fftw_real tmp111;
	       fftw_real tmp218;
	       fftw_real tmp219;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp108 = tmp106 - tmp107;
	       tmp111 = tmp109 + tmp110;
	       tmp112 = (K923879532 * tmp108) - (K382683432 * tmp111);
	       tmp185 = (K923879532 * tmp111) + (K382683432 * tmp108);
	       tmp218 = tmp106 + tmp107;
	       tmp219 = tmp110 - tmp109;
	       tmp220 = (K382683432 * tmp218) - (K923879532 * tmp219);
	       tmp245 = (K382683432 * tmp219) + (K923879532 * tmp218);
	  }
     }
     {
	  fftw_real tmp25;
	  fftw_real tmp116;
	  fftw_real tmp88;
	  fftw_real tmp114;
	  fftw_real tmp28;
	  fftw_real tmp113;
	  fftw_real tmp91;
	  fftw_real tmp117;
	  fftw_real tmp319;
	  fftw_real tmp320;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp23;
	       fftw_real tmp24;
	       fftw_real tmp86;
	       fftw_real tmp87;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp23 = c_re(input[30 * istride]);
	       tmp24 = c_re(input[14 * istride]);
	       tmp25 = tmp23 + tmp24;
	       tmp116 = tmp23 - tmp24;
	       tmp86 = c_im(input[30 * istride]);
	       tmp87 = c_im(input[14 * istride]);
	       tmp88 = tmp86 + tmp87;
	       tmp114 = tmp86 - tmp87;
	  }
	  {
	       fftw_real tmp26;
	       fftw_real tmp27;
	       fftw_real tmp89;
	       fftw_real tmp90;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp26 = c_re(input[6 * istride]);
	       tmp27 = c_re(input[22 * istride]);
	       tmp28 = tmp26 + tmp27;
	       tmp113 = tmp26 - tmp27;
	       tmp89 = c_im(input[6 * istride]);
	       tmp90 = c_im(input[22 * istride]);
	       tmp91 = tmp89 + tmp90;
	       tmp117 = tmp89 - tmp90;
	  }
	  tmp29 = tmp25 + tmp28;
	  tmp319 = tmp25 - tmp28;
	  tmp320 = tmp88 - tmp91;
	  tmp321 = tmp319 + tmp320;
	  tmp342 = tmp320 - tmp319;
	  tmp92 = tmp88 + tmp91;
	  {
	       fftw_real tmp115;
	       fftw_real tmp118;
	       fftw_real tmp221;
	       fftw_real tmp222;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp115 = tmp113 + tmp114;
	       tmp118 = tmp116 - tmp117;
	       tmp119 = (K382683432 * tmp115) + (K923879532 * tmp118);
	       tmp184 = (K923879532 * tmp115) - (K382683432 * tmp118);
	       tmp221 = tmp114 - tmp113;
	       tmp222 = tmp116 + tmp117;
	       tmp223 = (K923879532 * tmp221) + (K382683432 * tmp222);
	       tmp244 = (K382683432 * tmp221) - (K923879532 * tmp222);
	  }
     }
     {
	  fftw_real tmp34;
	  fftw_real tmp122;
	  fftw_real tmp142;
	  fftw_real tmp279;
	  fftw_real tmp37;
	  fftw_real tmp139;
	  fftw_real tmp125;
	  fftw_real tmp280;
	  fftw_real tmp41;
	  fftw_real tmp127;
	  fftw_real tmp130;
	  fftw_real tmp282;
	  fftw_real tmp44;
	  fftw_real tmp132;
	  fftw_real tmp135;
	  fftw_real tmp283;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp32;
	       fftw_real tmp33;
	       fftw_real tmp140;
	       fftw_real tmp141;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp32 = c_re(input[istride]);
	       tmp33 = c_re(input[17 * istride]);
	       tmp34 = tmp32 + tmp33;
	       tmp122 = tmp32 - tmp33;
	       tmp140 = c_im(input[istride]);
	       tmp141 = c_im(input[17 * istride]);
	       tmp142 = tmp140 - tmp141;
	       tmp279 = tmp140 + tmp141;
	  }
	  {
	       fftw_real tmp35;
	       fftw_real tmp36;
	       fftw_real tmp123;
	       fftw_real tmp124;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp35 = c_re(input[9 * istride]);
	       tmp36 = c_re(input[25 * istride]);
	       tmp37 = tmp35 + tmp36;
	       tmp139 = tmp35 - tmp36;
	       tmp123 = c_im(input[9 * istride]);
	       tmp124 = c_im(input[25 * istride]);
	       tmp125 = tmp123 - tmp124;
	       tmp280 = tmp123 + tmp124;
	  }
	  {
	       fftw_real tmp39;
	       fftw_real tmp40;
	       fftw_real tmp128;
	       fftw_real tmp129;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp39 = c_re(input[5 * istride]);
	       tmp40 = c_re(input[21 * istride]);
	       tmp41 = tmp39 + tmp40;
	       tmp127 = tmp39 - tmp40;
	       tmp128 = c_im(input[5 * istride]);
	       tmp129 = c_im(input[21 * istride]);
	       tmp130 = tmp128 - tmp129;
	       tmp282 = tmp128 + tmp129;
	  }
	  {
	       fftw_real tmp42;
	       fftw_real tmp43;
	       fftw_real tmp133;
	       fftw_real tmp134;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp42 = c_re(input[29 * istride]);
	       tmp43 = c_re(input[13 * istride]);
	       tmp44 = tmp42 + tmp43;
	       tmp132 = tmp42 - tmp43;
	       tmp133 = c_im(input[29 * istride]);
	       tmp134 = c_im(input[13 * istride]);
	       tmp135 = tmp133 - tmp134;
	       tmp283 = tmp133 + tmp134;
	  }
	  {
	       fftw_real tmp327;
	       fftw_real tmp328;
	       fftw_real tmp324;
	       fftw_real tmp325;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp126 = tmp122 - tmp125;
	       tmp229 = tmp122 + tmp125;
	       tmp38 = tmp34 + tmp37;
	       tmp45 = tmp41 + tmp44;
	       tmp278 = tmp38 - tmp45;
	       tmp327 = tmp34 - tmp37;
	       tmp328 = tmp283 - tmp282;
	       tmp329 = tmp327 - tmp328;
	       tmp357 = tmp327 + tmp328;
	       tmp281 = tmp279 + tmp280;
	       tmp284 = tmp282 + tmp283;
	       tmp285 = tmp281 - tmp284;
	       tmp143 = tmp139 + tmp142;
	       tmp226 = tmp142 - tmp139;
	       tmp324 = tmp279 - tmp280;
	       tmp325 = tmp41 - tmp44;
	       tmp326 = tmp324 - tmp325;
	       tmp356 = tmp325 + tmp324;
	       {
		    fftw_real tmp131;
		    fftw_real tmp136;
		    fftw_real tmp144;
		    fftw_real tmp145;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp131 = tmp127 - tmp130;
		    tmp136 = tmp132 + tmp135;
		    tmp137 = K707106781 * (tmp131 + tmp136);
		    tmp227 = K707106781 * (tmp131 - tmp136);
		    tmp144 = tmp127 + tmp130;
		    tmp145 = tmp135 - tmp132;
		    tmp146 = K707106781 * (tmp144 + tmp145);
		    tmp230 = K707106781 * (tmp145 - tmp144);
	       }
	  }
     }
     {
	  fftw_real tmp277;
	  fftw_real tmp301;
	  fftw_real tmp304;
	  fftw_real tmp306;
	  fftw_real tmp296;
	  fftw_real tmp300;
	  fftw_real tmp299;
	  fftw_real tmp305;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp275;
	       fftw_real tmp276;
	       fftw_real tmp302;
	       fftw_real tmp303;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp275 = tmp70 - tmp77;
	       tmp276 = tmp22 - tmp29;
	       tmp277 = tmp275 - tmp276;
	       tmp301 = tmp276 + tmp275;
	       tmp302 = tmp278 + tmp285;
	       tmp303 = tmp294 - tmp287;
	       tmp304 = K707106781 * (tmp302 + tmp303);
	       tmp306 = K707106781 * (tmp303 - tmp302);
	  }
	  {
	       fftw_real tmp286;
	       fftw_real tmp295;
	       fftw_real tmp297;
	       fftw_real tmp298;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp286 = tmp278 - tmp285;
	       tmp295 = tmp287 + tmp294;
	       tmp296 = K707106781 * (tmp286 - tmp295);
	       tmp300 = K707106781 * (tmp286 + tmp295);
	       tmp297 = tmp7 - tmp14;
	       tmp298 = tmp92 - tmp85;
	       tmp299 = tmp297 + tmp298;
	       tmp305 = tmp297 - tmp298;
	  }
	  c_im(output[28 * ostride]) = tmp277 - tmp296;
	  c_im(output[12 * ostride]) = tmp277 + tmp296;
	  c_re(output[20 * ostride]) = tmp299 - tmp300;
	  c_re(output[4 * ostride]) = tmp299 + tmp300;
	  c_im(output[20 * ostride]) = tmp301 - tmp304;
	  c_im(output[4 * ostride]) = tmp301 + tmp304;
	  c_re(output[28 * ostride]) = tmp305 - tmp306;
	  c_re(output[12 * ostride]) = tmp305 + tmp306;
     }
     {
	  fftw_real tmp31;
	  fftw_real tmp311;
	  fftw_real tmp310;
	  fftw_real tmp312;
	  fftw_real tmp62;
	  fftw_real tmp63;
	  fftw_real tmp94;
	  fftw_real tmp307;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp15;
	       fftw_real tmp30;
	       fftw_real tmp308;
	       fftw_real tmp309;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp15 = tmp7 + tmp14;
	       tmp30 = tmp22 + tmp29;
	       tmp31 = tmp15 + tmp30;
	       tmp311 = tmp15 - tmp30;
	       tmp308 = tmp281 + tmp284;
	       tmp309 = tmp290 + tmp293;
	       tmp310 = tmp308 + tmp309;
	       tmp312 = tmp309 - tmp308;
	  }
	  {
	       fftw_real tmp46;
	       fftw_real tmp61;
	       fftw_real tmp78;
	       fftw_real tmp93;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp46 = tmp38 + tmp45;
	       tmp61 = tmp53 + tmp60;
	       tmp62 = tmp46 + tmp61;
	       tmp63 = tmp46 - tmp61;
	       tmp78 = tmp70 + tmp77;
	       tmp93 = tmp85 + tmp92;
	       tmp94 = tmp78 - tmp93;
	       tmp307 = tmp78 + tmp93;
	  }
	  c_re(output[16 * ostride]) = tmp31 - tmp62;
	  c_re(output[0]) = tmp31 + tmp62;
	  c_im(output[8 * ostride]) = tmp63 + tmp94;
	  c_im(output[24 * ostride]) = tmp94 - tmp63;
	  c_im(output[16 * ostride]) = tmp307 - tmp310;
	  c_im(output[0]) = tmp307 + tmp310;
	  c_re(output[24 * ostride]) = tmp311 - tmp312;
	  c_re(output[8 * ostride]) = tmp311 + tmp312;
     }
     {
	  fftw_real tmp121;
	  fftw_real tmp189;
	  fftw_real tmp187;
	  fftw_real tmp193;
	  fftw_real tmp148;
	  fftw_real tmp190;
	  fftw_real tmp175;
	  fftw_real tmp191;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp105;
	       fftw_real tmp120;
	       fftw_real tmp183;
	       fftw_real tmp186;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp105 = tmp97 - tmp104;
	       tmp120 = tmp112 - tmp119;
	       tmp121 = tmp105 - tmp120;
	       tmp189 = tmp105 + tmp120;
	       tmp183 = tmp179 - tmp182;
	       tmp186 = tmp184 - tmp185;
	       tmp187 = tmp183 + tmp186;
	       tmp193 = tmp183 - tmp186;
	  }
	  {
	       fftw_real tmp138;
	       fftw_real tmp147;
	       fftw_real tmp165;
	       fftw_real tmp174;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp138 = tmp126 - tmp137;
	       tmp147 = tmp143 - tmp146;
	       tmp148 = (K555570233 * tmp138) - (K831469612 * tmp147);
	       tmp190 = (K831469612 * tmp138) + (K555570233 * tmp147);
	       tmp165 = tmp153 - tmp164;
	       tmp174 = tmp170 - tmp173;
	       tmp175 = (K555570233 * tmp165) + (K831469612 * tmp174);
	       tmp191 = (K555570233 * tmp174) - (K831469612 * tmp165);
	  }
	  {
	       fftw_real tmp176;
	       fftw_real tmp188;
	       fftw_real tmp192;
	       fftw_real tmp194;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp176 = tmp148 - tmp175;
	       c_im(output[29 * ostride]) = tmp121 - tmp176;
	       c_im(output[13 * ostride]) = tmp121 + tmp176;
	       tmp188 = tmp148 + tmp175;
	       c_re(output[21 * ostride]) = tmp187 - tmp188;
	       c_re(output[5 * ostride]) = tmp187 + tmp188;
	       tmp192 = tmp190 + tmp191;
	       c_im(output[21 * ostride]) = tmp189 - tmp192;
	       c_im(output[5 * ostride]) = tmp189 + tmp192;
	       tmp194 = tmp191 - tmp190;
	       c_re(output[29 * ostride]) = tmp193 - tmp194;
	       c_re(output[13 * ostride]) = tmp193 + tmp194;
	  }
     }
     {
	  fftw_real tmp197;
	  fftw_real tmp209;
	  fftw_real tmp207;
	  fftw_real tmp213;
	  fftw_real tmp200;
	  fftw_real tmp210;
	  fftw_real tmp203;
	  fftw_real tmp211;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp195;
	       fftw_real tmp196;
	       fftw_real tmp205;
	       fftw_real tmp206;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp195 = tmp97 + tmp104;
	       tmp196 = tmp185 + tmp184;
	       tmp197 = tmp195 - tmp196;
	       tmp209 = tmp195 + tmp196;
	       tmp205 = tmp179 + tmp182;
	       tmp206 = tmp112 + tmp119;
	       tmp207 = tmp205 + tmp206;
	       tmp213 = tmp205 - tmp206;
	  }
	  {
	       fftw_real tmp198;
	       fftw_real tmp199;
	       fftw_real tmp201;
	       fftw_real tmp202;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp198 = tmp126 + tmp137;
	       tmp199 = tmp143 + tmp146;
	       tmp200 = (K980785280 * tmp198) - (K195090322 * tmp199);
	       tmp210 = (K195090322 * tmp198) + (K980785280 * tmp199);
	       tmp201 = tmp153 + tmp164;
	       tmp202 = tmp170 + tmp173;
	       tmp203 = (K980785280 * tmp201) + (K195090322 * tmp202);
	       tmp211 = (K980785280 * tmp202) - (K195090322 * tmp201);
	  }
	  {
	       fftw_real tmp204;
	       fftw_real tmp208;
	       fftw_real tmp212;
	       fftw_real tmp214;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp204 = tmp200 - tmp203;
	       c_im(output[25 * ostride]) = tmp197 - tmp204;
	       c_im(output[9 * ostride]) = tmp197 + tmp204;
	       tmp208 = tmp200 + tmp203;
	       c_re(output[17 * ostride]) = tmp207 - tmp208;
	       c_re(output[ostride]) = tmp207 + tmp208;
	       tmp212 = tmp210 + tmp211;
	       c_im(output[17 * ostride]) = tmp209 - tmp212;
	       c_im(output[ostride]) = tmp209 + tmp212;
	       tmp214 = tmp211 - tmp210;
	       c_re(output[25 * ostride]) = tmp213 - tmp214;
	       c_re(output[9 * ostride]) = tmp213 + tmp214;
	  }
     }
     {
	  fftw_real tmp323;
	  fftw_real tmp347;
	  fftw_real tmp350;
	  fftw_real tmp352;
	  fftw_real tmp338;
	  fftw_real tmp346;
	  fftw_real tmp345;
	  fftw_real tmp351;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp315;
	       fftw_real tmp322;
	       fftw_real tmp348;
	       fftw_real tmp349;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp315 = tmp313 - tmp314;
	       tmp322 = K707106781 * (tmp318 - tmp321);
	       tmp323 = tmp315 + tmp322;
	       tmp347 = tmp315 - tmp322;
	       tmp348 = (K382683432 * tmp329) - (K923879532 * tmp326);
	       tmp349 = (K923879532 * tmp333) + (K382683432 * tmp336);
	       tmp350 = tmp348 - tmp349;
	       tmp352 = tmp348 + tmp349;
	  }
	  {
	       fftw_real tmp330;
	       fftw_real tmp337;
	       fftw_real tmp341;
	       fftw_real tmp344;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp330 = (K382683432 * tmp326) + (K923879532 * tmp329);
	       tmp337 = (K382683432 * tmp333) - (K923879532 * tmp336);
	       tmp338 = tmp330 + tmp337;
	       tmp346 = tmp337 - tmp330;
	       tmp341 = tmp339 - tmp340;
	       tmp344 = K707106781 * (tmp342 - tmp343);
	       tmp345 = tmp341 - tmp344;
	       tmp351 = tmp341 + tmp344;
	  }
	  c_im(output[22 * ostride]) = tmp323 - tmp338;
	  c_im(output[6 * ostride]) = tmp323 + tmp338;
	  c_re(output[30 * ostride]) = tmp345 - tmp346;
	  c_re(output[14 * ostride]) = tmp345 + tmp346;
	  c_im(output[30 * ostride]) = tmp347 - tmp350;
	  c_im(output[14 * ostride]) = tmp347 + tmp350;
	  c_re(output[22 * ostride]) = tmp351 - tmp352;
	  c_re(output[6 * ostride]) = tmp351 + tmp352;
     }
     {
	  fftw_real tmp355;
	  fftw_real tmp367;
	  fftw_real tmp370;
	  fftw_real tmp372;
	  fftw_real tmp362;
	  fftw_real tmp366;
	  fftw_real tmp365;
	  fftw_real tmp371;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp353;
	       fftw_real tmp354;
	       fftw_real tmp368;
	       fftw_real tmp369;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp353 = tmp314 + tmp313;
	       tmp354 = K707106781 * (tmp343 + tmp342);
	       tmp355 = tmp353 + tmp354;
	       tmp367 = tmp353 - tmp354;
	       tmp368 = (K923879532 * tmp357) - (K382683432 * tmp356);
	       tmp369 = (K382683432 * tmp359) + (K923879532 * tmp360);
	       tmp370 = tmp368 - tmp369;
	       tmp372 = tmp368 + tmp369;
	  }
	  {
	       fftw_real tmp358;
	       fftw_real tmp361;
	       fftw_real tmp363;
	       fftw_real tmp364;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp358 = (K923879532 * tmp356) + (K382683432 * tmp357);
	       tmp361 = (K923879532 * tmp359) - (K382683432 * tmp360);
	       tmp362 = tmp358 + tmp361;
	       tmp366 = tmp361 - tmp358;
	       tmp363 = tmp339 + tmp340;
	       tmp364 = K707106781 * (tmp318 + tmp321);
	       tmp365 = tmp363 - tmp364;
	       tmp371 = tmp363 + tmp364;
	  }
	  c_im(output[18 * ostride]) = tmp355 - tmp362;
	  c_im(output[2 * ostride]) = tmp355 + tmp362;
	  c_re(output[26 * ostride]) = tmp365 - tmp366;
	  c_re(output[10 * ostride]) = tmp365 + tmp366;
	  c_im(output[26 * ostride]) = tmp367 - tmp370;
	  c_im(output[10 * ostride]) = tmp367 + tmp370;
	  c_re(output[18 * ostride]) = tmp371 - tmp372;
	  c_re(output[2 * ostride]) = tmp371 + tmp372;
     }
     {
	  fftw_real tmp225;
	  fftw_real tmp249;
	  fftw_real tmp247;
	  fftw_real tmp253;
	  fftw_real tmp232;
	  fftw_real tmp250;
	  fftw_real tmp239;
	  fftw_real tmp251;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp217;
	       fftw_real tmp224;
	       fftw_real tmp243;
	       fftw_real tmp246;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp217 = tmp215 - tmp216;
	       tmp224 = tmp220 - tmp223;
	       tmp225 = tmp217 + tmp224;
	       tmp249 = tmp217 - tmp224;
	       tmp243 = tmp241 - tmp242;
	       tmp246 = tmp244 - tmp245;
	       tmp247 = tmp243 - tmp246;
	       tmp253 = tmp243 + tmp246;
	  }
	  {
	       fftw_real tmp228;
	       fftw_real tmp231;
	       fftw_real tmp235;
	       fftw_real tmp238;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp228 = tmp226 - tmp227;
	       tmp231 = tmp229 - tmp230;
	       tmp232 = (K195090322 * tmp228) + (K980785280 * tmp231);
	       tmp250 = (K195090322 * tmp231) - (K980785280 * tmp228);
	       tmp235 = tmp233 - tmp234;
	       tmp238 = tmp236 - tmp237;
	       tmp239 = (K195090322 * tmp235) - (K980785280 * tmp238);
	       tmp251 = (K980785280 * tmp235) + (K195090322 * tmp238);
	  }
	  {
	       fftw_real tmp240;
	       fftw_real tmp248;
	       fftw_real tmp252;
	       fftw_real tmp254;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp240 = tmp232 + tmp239;
	       c_im(output[23 * ostride]) = tmp225 - tmp240;
	       c_im(output[7 * ostride]) = tmp225 + tmp240;
	       tmp248 = tmp239 - tmp232;
	       c_re(output[31 * ostride]) = tmp247 - tmp248;
	       c_re(output[15 * ostride]) = tmp247 + tmp248;
	       tmp252 = tmp250 - tmp251;
	       c_im(output[31 * ostride]) = tmp249 - tmp252;
	       c_im(output[15 * ostride]) = tmp249 + tmp252;
	       tmp254 = tmp250 + tmp251;
	       c_re(output[23 * ostride]) = tmp253 - tmp254;
	       c_re(output[7 * ostride]) = tmp253 + tmp254;
	  }
     }
     {
	  fftw_real tmp257;
	  fftw_real tmp269;
	  fftw_real tmp267;
	  fftw_real tmp273;
	  fftw_real tmp260;
	  fftw_real tmp270;
	  fftw_real tmp263;
	  fftw_real tmp271;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp255;
	       fftw_real tmp256;
	       fftw_real tmp265;
	       fftw_real tmp266;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp255 = tmp215 + tmp216;
	       tmp256 = tmp245 + tmp244;
	       tmp257 = tmp255 + tmp256;
	       tmp269 = tmp255 - tmp256;
	       tmp265 = tmp241 + tmp242;
	       tmp266 = tmp220 + tmp223;
	       tmp267 = tmp265 - tmp266;
	       tmp273 = tmp265 + tmp266;
	  }
	  {
	       fftw_real tmp258;
	       fftw_real tmp259;
	       fftw_real tmp261;
	       fftw_real tmp262;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp258 = tmp226 + tmp227;
	       tmp259 = tmp229 + tmp230;
	       tmp260 = (K831469612 * tmp258) + (K555570233 * tmp259);
	       tmp270 = (K831469612 * tmp259) - (K555570233 * tmp258);
	       tmp261 = tmp233 + tmp234;
	       tmp262 = tmp236 + tmp237;
	       tmp263 = (K831469612 * tmp261) - (K555570233 * tmp262);
	       tmp271 = (K555570233 * tmp261) + (K831469612 * tmp262);
	  }
	  {
	       fftw_real tmp264;
	       fftw_real tmp268;
	       fftw_real tmp272;
	       fftw_real tmp274;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp264 = tmp260 + tmp263;
	       c_im(output[19 * ostride]) = tmp257 - tmp264;
	       c_im(output[3 * ostride]) = tmp257 + tmp264;
	       tmp268 = tmp263 - tmp260;
	       c_re(output[27 * ostride]) = tmp267 - tmp268;
	       c_re(output[11 * ostride]) = tmp267 + tmp268;
	       tmp272 = tmp270 - tmp271;
	       c_im(output[27 * ostride]) = tmp269 - tmp272;
	       c_im(output[11 * ostride]) = tmp269 + tmp272;
	       tmp274 = tmp270 + tmp271;
	       c_re(output[19 * ostride]) = tmp273 - tmp274;
	       c_re(output[3 * ostride]) = tmp273 + tmp274;
	  }
     }
}

fftw_codelet_desc fftwi_no_twiddle_32_desc =
{
     "fftwi_no_twiddle_32",
     (void (*)()) fftwi_no_twiddle_32,
     32,
     FFTW_BACKWARD,
     FFTW_NOTW,
     716,
     0,
     (const int *) 0,
};