/*
 * The MIT License
 *
 * Copyright (C) 2010-2017 Alexander Saprykin <saprykin.spb@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "pmem.h"
#include "pcryptohash-gost3411.h"

#include <string.h>
#include <stdlib.h>

struct PHashGOST3411_ {
	puint32	buf[8];  /* Buffer to handle incoming data. */
	puint32	hash[8]; /* State of calculated hash.       */
	puint32	len[8];  /* Length of hashed data, in bits. */
	puint32	sum[8];  /* 256-bit sum of hashed data.     */
};

static void pp_crypto_hash_gost3411_swap_bytes (puint32 *data, puint words);
static void pp_crypto_hash_gost3411_sum_256 (puint32 a[8], const puint32 b[8]);
static void pp_crypto_hash_gost3411_process (PHashGOST3411 *ctx, const puint32 data[8]);

/* K block data from RFC4357 for GOST 28147-89 */
/* static const puchar pp_crypto_hash_gost3411_K_block[8][16] = {
		{0x9, 0x6, 0x3, 0x2, 0x8, 0xB, 0x1, 0x7, 0xA, 0x4, 0xE, 0xF, 0xC, 0x0, 0xD, 0x5},
		{0x3, 0x7, 0xE, 0x9, 0x8, 0xA, 0xF, 0x0, 0x5, 0x2, 0x6, 0xC, 0xB, 0x4, 0xD, 0x1},
		{0xE, 0x4, 0x6, 0x2, 0xB, 0x3, 0xD, 0x8, 0xC, 0xF, 0x5, 0xA, 0x0, 0x7, 0x1, 0x9},
		{0xE, 0x7, 0xA, 0xC, 0xD, 0x1, 0x3, 0x9, 0x0, 0x2, 0xB, 0x4, 0xF, 0x8, 0x5, 0x6},
		{0xB, 0x5, 0x1, 0x9, 0x8, 0xD, 0xF, 0x0, 0xE, 0x4, 0x2, 0x3, 0xC, 0x7, 0xA, 0x6},
		{0x3, 0xA, 0xD, 0xC, 0x1, 0x2, 0x0, 0xB, 0x7, 0x5, 0x9, 0x4, 0x8, 0xF, 0xE, 0x6},
		{0x1, 0xD, 0x2, 0x9, 0x7, 0xA, 0x6, 0x0, 0x8, 0xC, 0x4, 0x5, 0xF, 0x3, 0xB, 0xE},
		{0xB, 0xA, 0xF, 0x5, 0x0, 0xC, 0xE, 0x8, 0x6, 0x2, 0x3, 0x9, 0x1, 0x7, 0xD, 0x4}
}; */

/* K block data used by Russian Central Bank (see RFC 4357, sec. 11.2) */
/* static const puchar pp_crypto_hash_gost3411_K_block[8][16] = {
		{0x4, 0xA, 0x9, 0x2, 0xD, 0x8, 0x0, 0xE, 0x6, 0xB, 0x1, 0xC, 0x7, 0xF, 0x5, 0x3},
		{0xE, 0xB, 0x4, 0xC, 0x6, 0xD, 0xF, 0xA, 0x2, 0x3, 0x8, 0x1, 0x0, 0x7, 0x5, 0x9},
		{0x5, 0x8, 0x1, 0xD, 0xA, 0x3, 0x4, 0x2, 0xE, 0xF, 0xC, 0x7, 0x6, 0x0, 0x9, 0xB},
		{0x7, 0xD, 0xA, 0x1, 0x0, 0x8, 0x9, 0xF, 0xE, 0x4, 0x6, 0xC, 0xB, 0x2, 0x5, 0x3},
		{0x6, 0xC, 0x7, 0x1, 0x5, 0xF, 0xD, 0x8, 0x4, 0xA, 0x9, 0xE, 0x0, 0x3, 0xB, 0x2},
		{0x4, 0xB, 0xA, 0x0, 0x7, 0x2, 0x1, 0xD, 0x3, 0x6, 0x8, 0x5, 0x9, 0xC, 0xF, 0xE},
		{0xD, 0xB, 0x4, 0x1, 0x3, 0xF, 0x5, 0x9, 0x0, 0xA, 0xE, 0x7, 0x6, 0x8, 0x2, 0xC},
		{0x1, 0xF, 0xD, 0x0, 0x5, 0x7, 0xA, 0x4, 0x9, 0x2, 0x3, 0xE, 0x6, 0xB, 0x8, 0xC}
}; */

 /* K block data id-GostR3411-94-CryptoProParamSet (see RFC 4357, sec. 11.2) */
  static const puchar pp_crypto_hash_gost3411_K_block[8][16] = {
		{0xA, 0x4, 0x5, 0x6, 0x8, 0x1, 0x3, 0x7, 0xD, 0xC, 0xE, 0x0, 0x9, 0x2, 0xB, 0xF},
		{0x5, 0xF, 0x4, 0x0, 0x2, 0xD, 0xB, 0x9, 0x1, 0x7, 0x6, 0x3, 0xC, 0xE, 0xA, 0x8},
		{0x7, 0xF, 0xC, 0xE, 0x9, 0x4, 0x1, 0x0, 0x3, 0xB, 0x5, 0x2, 0x6, 0xA, 0x8, 0xD},
		{0x4, 0xA, 0x7, 0xC, 0x0, 0xF, 0x2, 0x8, 0xE, 0x1, 0x6, 0x5, 0xD, 0xB, 0x9, 0x3},
		{0x7, 0x6, 0x4, 0xB, 0x9, 0xC, 0x2, 0xA, 0x1, 0x8, 0x0, 0xE, 0xF, 0xD, 0x3, 0x5},
		{0x7, 0x6, 0x2, 0x4, 0xD, 0x9, 0xF, 0x0, 0xA, 0x1, 0x5, 0xB, 0x8, 0xE, 0xC, 0x3},
		{0xD, 0xE, 0x4, 0x1, 0x7, 0x0, 0x5, 0xA, 0x3, 0xC, 0x8, 0xF, 0x6, 0x2, 0x9, 0xB},
		{0x1, 0x3, 0xA, 0x9, 0x5, 0xB, 0x4, 0xF, 0x8, 0x6, 0x7, 0xE, 0xD, 0x0, 0x2, 0xC}
 };

/* GOST 28147-89 transformation to generate keys */
#define P_GOST_28147_ROUND(N, key)							\
{											\
	puint32 CM1;									\
											\
	CM1 = (N)[0] + (key);								\
											\
	CM1 = ((puint32) pp_crypto_hash_gost3411_K_block [0][CM1 & 0xF]			\
	    |  (puint32) pp_crypto_hash_gost3411_K_block [1][(CM1 >> 4)  & 0xF] << 4	\
	    |  (puint32) pp_crypto_hash_gost3411_K_block [2][(CM1 >> 8)  & 0xF] << 8	\
	    |  (puint32) pp_crypto_hash_gost3411_K_block [3][(CM1 >> 12) & 0xF] << 12	\
	    |  (puint32) pp_crypto_hash_gost3411_K_block [4][(CM1 >> 16) & 0xF] << 16	\
	    |  (puint32) pp_crypto_hash_gost3411_K_block [5][(CM1 >> 20) & 0xF] << 20	\
	    |  (puint32) pp_crypto_hash_gost3411_K_block [6][(CM1 >> 24) & 0xF] << 24	\
	    |  (puint32) pp_crypto_hash_gost3411_K_block [7][(CM1 >> 28) & 0xF] << 28);	\
											\
	CM1 = ((CM1 << 11) | (CM1 >> 21)) ^ (N)[1];					\
	(N)[1] = (N)[0];								\
	(N)[0] = CM1;									\
}

/* Core GOST 28147-89 transformation */
#define P_GOST_28147_E(data, key, out)					\
{									\
	puint32 N[2];							\
									\
	memcpy (N, data, 8);						\
									\
	P_GOST_28147_ROUND (N, (key)[0]);				\
	P_GOST_28147_ROUND (N, (key)[1]);				\
	P_GOST_28147_ROUND (N, (key)[2]);				\
	P_GOST_28147_ROUND (N, (key)[3]);				\
	P_GOST_28147_ROUND (N, (key)[4]);				\
	P_GOST_28147_ROUND (N, (key)[5]);				\
	P_GOST_28147_ROUND (N, (key)[6]);				\
	P_GOST_28147_ROUND (N, (key)[7]);				\
									\
	P_GOST_28147_ROUND (N, (key)[0]);				\
	P_GOST_28147_ROUND (N, (key)[1]);				\
	P_GOST_28147_ROUND (N, (key)[2]);				\
	P_GOST_28147_ROUND (N, (key)[3]);				\
	P_GOST_28147_ROUND (N, (key)[4]);				\
	P_GOST_28147_ROUND (N, (key)[5]);				\
	P_GOST_28147_ROUND (N, (key)[6]);				\
	P_GOST_28147_ROUND (N, (key)[7]);				\
									\
	P_GOST_28147_ROUND (N, (key)[0]);				\
	P_GOST_28147_ROUND (N, (key)[1]);				\
	P_GOST_28147_ROUND (N, (key)[2]);				\
	P_GOST_28147_ROUND (N, (key)[3]);				\
	P_GOST_28147_ROUND (N, (key)[4]);				\
	P_GOST_28147_ROUND (N, (key)[5]);				\
	P_GOST_28147_ROUND (N, (key)[6]);				\
	P_GOST_28147_ROUND (N, (key)[7]);				\
									\
	P_GOST_28147_ROUND (N, (key)[7]);				\
	P_GOST_28147_ROUND (N, (key)[6]);				\
	P_GOST_28147_ROUND (N, (key)[5]);				\
	P_GOST_28147_ROUND (N, (key)[4]);				\
	P_GOST_28147_ROUND (N, (key)[3]);				\
	P_GOST_28147_ROUND (N, (key)[2]);				\
	P_GOST_28147_ROUND (N, (key)[1]);				\
	P_GOST_28147_ROUND (N, (key)[0]);				\
									\
	(out)[0] = N[1];						\
	(out)[1] = N[0];						\
}

/* P transformation from GOST R 34.11-94 */
#define P_GOST_3411_P(data, out)					\
{									\
	(out)[0] = ((data) [0] & 0x000000FF)				\
		 | (((data)[2] << 8)	& 0x0000FF00)			\
		 | (((data)[4] << 16)	& 0x00FF0000)			\
		 | (((data)[6] << 24)	& 0xFF000000);			\
	(out)[1] = (((data)[0] >> 8)	& 0x000000FF)			\
		 | ((data) [2]		& 0x0000FF00)			\
		 | (((data)[4] << 8)	& 0x00FF0000)			\
		 | (((data)[6] << 16)	& 0xFF000000);			\
	(out)[2] = (((data)[0] >> 16)	& 0x000000FF)			\
		 | (((data)[2] >> 8)	& 0x0000FF00)			\
		 | ((data) [4]		& 0x00FF0000)			\
		 | (((data)[6] << 8)	& 0xFF000000);			\
	(out)[3] = (((data)[0] >> 24)	& 0x000000FF)			\
		 | (((data)[2] >> 16)	& 0x0000FF00)			\
		 | (((data)[4] >> 8)	& 0x00FF0000)			\
		 | ((data) [6]		& 0xFF000000);			\
	(out)[4] = ((data) [1]		& 0x000000FF)			\
		 | (((data)[3] << 8)	& 0x0000FF00)			\
		 | (((data)[5] << 16)	& 0x00FF0000)			\
		 | (((data)[7] << 24)	& 0xFF000000);			\
	(out)[5] = (((data)[1] >> 8)	& 0x000000FF)			\
		 | ((data) [3]		& 0x0000FF00)			\
		 | (((data)[5] << 8)	& 0x00FF0000)			\
		 | (((data)[7] << 16)	& 0xFF000000);			\
	(out)[6] = (((data)[1] >> 16)	& 0x000000FF)			\
		 | (((data)[3] >> 8)	& 0x0000FF00)			\
		 | ((data) [5]		& 0x00FF0000)			\
		 | (((data)[7] << 8)	& 0xFF000000);			\
	(out)[7] = (((data)[1] >> 24)	& 0x000000FF)			\
		 | (((data)[3] >> 16)	& 0x0000FF00)			\
		 | (((data)[5] >> 8)	& 0x00FF0000)			\
		 | ((data) [7]		& 0xFF000000);			\
}

static void
pp_crypto_hash_gost3411_swap_bytes (puint32	*data,
				    puint	words)
{
#ifndef PLIBSYS_IS_BIGENDIAN
	P_UNUSED (data);
	P_UNUSED (words);
#else
	while (words-- > 0) {
		*data = PUINT32_TO_LE (*data);
		++data;
	}
#endif
}

/* 256-bit sum */
static void
pp_crypto_hash_gost3411_sum_256 (puint32	a[8],
				 const puint32	b[8])
{
	puint		i;
	puint32		old;
	pboolean	carry;

	carry = FALSE;
	for (i = 0; i < 8; ++i) {
		old = a[i];
		a[i] = a[i] + b[i] + (carry ? 1 : 0);
		carry = (a[i] < old || a[i] < b[i]) ? TRUE : FALSE;
	}
}

/* Core GOST R 34.11-94 transformation */
static void pp_crypto_hash_gost3411_process (PHashGOST3411	*ctx,
					     const puint32	data[8])
{
	puint32 U[8], V[8], W[8], S[8], K[4][8];

	memcpy (U, ctx->hash, 32);
	memcpy (V, data, 32);

	/* Generate first key: P (U xor V) */
	W[0] = U[0] ^ V[0];
	W[1] = U[1] ^ V[1];
	W[2] = U[2] ^ V[2];
	W[3] = U[3] ^ V[3];
	W[4] = U[4] ^ V[4];
	W[5] = U[5] ^ V[5];
	W[6] = U[6] ^ V[6];
	W[7] = U[7] ^ V[7];

	P_GOST_3411_P (W, K[0]);

	/* Generate second key: P (A (U) xor A^2 (V)) */
	W[0] = U[2] ^ V[4];
	W[1] = U[3] ^ V[5];
	W[2] = U[4] ^ V[6];
	W[3] = U[5] ^ V[7];
	W[4] = U[6] ^ (V[0] ^= V[2]);
	W[5] = U[7] ^ (V[1] ^= V[3]);
	W[6] = (U[0] ^= U[2]) ^ (V[2] ^= V[4]);
	W[7] = (U[1] ^= U[3]) ^ (V[3] ^= V[5]);

	P_GOST_3411_P (W, K[1]);

	/* Generate third key: P ((A^2 (U) + C3) xor A^4 (V)) */
	/* C3 = FF00FFFF 000000FF FF0000FF 00FFFF00 00FF00FF 00FF00FF FF00FF00 FF00FF00 */
	U[2] ^= U[4] ^ 0x000000FF;
	U[3] ^= U[5] ^ 0xFF00FFFF;
	U[4] ^= 0xFF00FF00;
	U[5] ^= 0xFF00FF00;
	U[6] ^= 0x00FF00FF;
	U[7] ^= 0x00FF00FF;
	U[0] ^= 0x00FFFF00;
	U[1] ^= 0xFF0000FF;

	W[0] = U[4] ^ V[0];
	W[2] = U[6] ^ V[2];
	W[4] = U[0] ^ (V[4] ^= V[6]);
	W[6] = U[2] ^ (V[6] ^= V[0]);
	W[1] = U[5] ^ V[1];
	W[3] = U[7] ^ V[3];
	W[5] = U[1] ^ (V[5] ^= V[7]);
	W[7] = U[3] ^ (V[7] ^= V[1]);

	P_GOST_3411_P (W, K[2]);

	/* Generate forth key: P (A (A^2 (U) xor C3) xor A^6 (V)) */
	W[0] = U[6] ^ V[4];
	W[1] = U[7] ^ V[5];
	W[2] = U[0] ^ V[6];
	W[3] = U[1] ^ V[7];
	W[4] = U[2] ^ (V[0] ^= V[2]);
	W[5] = U[3] ^ (V[1] ^= V[3]);
	W[6] = (U[4] ^= U[6]) ^ (V[2] ^= V[4]);
	W[7] = (U[5] ^= U[7]) ^ (V[3] ^= V[5]);

	P_GOST_3411_P (W, K[3]);

	/* Perform GOST 28147-89 encryption */
	P_GOST_28147_E (ctx->hash,      K[0], S);
	P_GOST_28147_E (ctx->hash + 2,  K[1], S + 2);
	P_GOST_28147_E (ctx->hash + 4,  K[2], S + 4);
	P_GOST_28147_E (ctx->hash + 6,  K[3], S + 6);

	/* Step hash function: H (M, Hprev) = PSI^61 (Hprev xor PSI (M xor PSI^12 (S))) */

	/* (12 rounds of LFSR) xor M */
	U[0] = data[0] ^ S[6];

	U[1] = data[1] ^ S[7];

	U[2] = data[2] ^ (S[0] & 0x0000FFFF)	^ (S[0] >> 16) ^ (S[0] << 16)
		       ^ (S[1] & 0x0000FFFF)	^ (S[1] >> 16) ^ (S[2] << 16)
		       ^ (S[7] & 0xFFFF0000)	^ (S[6] << 16) ^ (S[7] >> 16)
		       ^  S[6];

	U[3] = data[3] ^ (S[0] & 0x0000FFFF)	^ (S[0] << 16) ^ (S[2] << 16)
		       ^ (S[1] & 0x0000FFFF)	^ (S[1] << 16) ^ (S[1] >> 16)
		       ^ (S[7] & 0x0000FFFF)	^ (S[2] >> 16) ^ (S[3] << 16)
		       ^ (S[6] << 16)		^ (S[6] >> 16) ^ (S[7] << 16)
		       ^ (S[7] >> 16)		^ S[6];

	U[4] = data[4] ^ (S[0] & 0xFFFF0000)	^ (S[0] << 16) ^ (S[0] >> 16)
		       ^ (S[1] & 0xFFFF0000)	^ (S[1] >> 16) ^ (S[2] << 16)
		       ^ (S[7] & 0x0000FFFF)	^ (S[3] << 16) ^ (S[3] >> 16)
		       ^ (S[4] << 16)		^ (S[6] << 16) ^ (S[6] >> 16)
		       ^ (S[2] >> 16)		^ (S[7] << 16) ^ (S[7] >> 16);

	U[5] = data[5] ^ (S[0] & 0xFFFF0000)	^ (S[0] >> 16) ^ (S[0] << 16)
		       ^ (S[1] & 0x0000FFFF)	^ (S[7] >> 16) ^ (S[2] >> 16)
		       ^ (S[7] & 0xFFFF0000)	^ (S[3] >> 16) ^ (S[4] << 16)
		       ^ (S[4] >> 16)		^ (S[5] << 16) ^ (S[6] << 16)
		       ^ (S[6] >> 16)		^ (S[3] << 16) ^ (S[7] << 16)
		       ^ S[2];

	U[6] = data[6] ^ (S[4] >> 16)		^ (S[1] >> 16) ^ (S[2] << 16)
		       ^ (S[7] << 16)		^ (S[3] >> 16) ^ (S[4] << 16)
		       ^ (S[5] << 16)		^ (S[5] >> 16) ^ (S[6] << 16)
		       ^ (S[6] >> 16)		^ S[6]	       ^ S[0]
		       ^ S[3];

	U[7] = data[7] ^ (S[0] & 0xFFFF0000)	^ (S[0] << 16) ^ (S[1] << 16)
		       ^ (S[1] & 0x0000FFFF)	^ (S[2] >> 16) ^ (S[3] << 16)
		       ^ (S[7] & 0x0000FFFF)	^ (S[4] >> 16) ^ (S[5] << 16)
		       ^ (S[5] >> 16)		^ (S[6] >> 16) ^ (S[7] << 16)
		       ^ (S[7] >> 16)		^ S[4];

	/* (1 round of LFSR) xor Hprev */
	V[0] = ctx->hash[0] ^ (U[1] << 16) ^ (U[0] >> 16);
	V[1] = ctx->hash[1] ^ (U[2] << 16) ^ (U[1] >> 16);
	V[2] = ctx->hash[2] ^ (U[3] << 16) ^ (U[2] >> 16);
	V[3] = ctx->hash[3] ^ (U[4] << 16) ^ (U[3] >> 16);
	V[4] = ctx->hash[4] ^ (U[5] << 16) ^ (U[4] >> 16);
	V[5] = ctx->hash[5] ^ (U[6] << 16) ^ (U[5] >> 16);
	V[6] = ctx->hash[6] ^ (U[7] << 16) ^ (U[6] >> 16);
	V[7] = ctx->hash[7] ^ (U[7] >> 16)
	     ^ (U[0] << 16) ^ (U[1] & 0xFFFF0000)
	     ^ (U[1] << 16) ^ (U[7] & 0xFFFF0000)
	     ^ (U[6] << 16) ^ (U[0] & 0xFFFF0000);

	/* Final 61 rounds of LFSR */
	ctx->hash[0] = (V[0] & 0xFFFF0000)	^ (V[0] << 16)	^ (V[0] >> 16)
		     ^ (V[1] & 0xFFFF0000)	^ (V[1] >> 16)	^ (V[2] << 16)
		     ^ (V[7] & 0x0000FFFF)	^ (V[3] >> 16)	^ (V[4] << 16)
		     ^ (V[5] >> 16)		^ (V[6] >> 16)	^ (V[7] << 16)
		     ^ (V[7] >> 16)		^ V[5];
	ctx->hash[1] = (V[0] & 0xFFFF0000)	^ (V[0] << 16)	^ (V[0] >> 16)
		     ^ (V[1] & 0x0000FFFF)	^ (V[2] >> 16)	^ (V[3] << 16)
		     ^ (V[7] & 0xFFFF0000)	^ (V[4] >> 16)	^ (V[5] << 16)
		     ^ (V[6] << 16)		^ (V[7] >> 16)	^ V[6]
		     ^ V[2] ;
	ctx->hash[2] = (V[0] & 0x0000FFFF)	^ (V[0] << 16)	^ (V[1] << 16)
		     ^ (V[7] & 0x0000FFFF)	^ (V[1] >> 16)	^ (V[2] << 16)
		     ^ (V[1] & 0xFFFF0000)	^ (V[3] >> 16)  ^ (V[4] << 16)
		     ^ (V[5] >> 16)		^ (V[6] >> 16)	^ (V[7] << 16)
		     ^ (V[7] >> 16)		^ V[3]		^ V[6];
	ctx->hash[3] = (V[0] & 0xFFFF0000)	^ (V[0] << 16)	^ (V[0] >> 16)
		     ^ (V[1] & 0xFFFF0000)	^ (V[1] >> 16)	^ (V[2] << 16)
		     ^ (V[7] & 0x0000FFFF)	^ (V[2] >> 16)	^ (V[3] << 16)
		     ^ (V[4] >> 16)		^ (V[5] << 16)	^ (V[6] << 16)
		     ^ (V[7] >> 16)		^ V[2]		^ V[4];
	ctx->hash[4] = (V[0] >> 16)		^ (V[1] << 16)  ^ (V[2] >> 16)
		     ^ (V[3] << 16)		^ (V[3] >> 16)	^ (V[4] << 16)
		     ^ (V[5] >> 16)		^ (V[6] << 16)	^ (V[6] >> 16)
		     ^ (V[7] << 16)		^ V[1]		^ V[2]
		     ^ V[3]			^ V[5];
	ctx->hash[5] = (V[0] & 0xFFFF0000)	^ (V[0] << 16)	^ (V[1] << 16)
		     ^ (V[1] & 0xFFFF0000) 	^ (V[1] >> 16)	^ (V[2] << 16)
		     ^ (V[7] & 0xFFFF0000)	^ (V[3] >> 16)  ^ (V[4] << 16)
		     ^ (V[4] >> 16)		^ (V[5] << 16)	^ (V[6] << 16)
		     ^ (V[6] >> 16)		^ (V[7] << 16)	^ (V[7] >> 16)
		     ^ V[2]			^ V[3]		^ V[4]
		     ^ V[6];
	ctx->hash[6] = (V[2] >> 16)		^ (V[3] << 16)	^ (V[4] >> 16)
		     ^ (V[5] << 16)		^ (V[5] >> 16)	^ (V[6] << 16)
		     ^ (V[6] >> 16)		^ (V[7] << 16)	^ V[7]
		     ^ V[0]			^ V[2]		^ V[3]
		     ^ V[4]			^ V[5]		^ V[6];
	ctx->hash[7] = (V[0] >> 16)		^ (V[1] << 16)	^ (V[1] >> 16)
		     ^ (V[2] << 16)		^ (V[3] >> 16)	^ (V[4] << 16)
		     ^ (V[5] >> 16)		^ (V[6] << 16)	^ (V[6] >> 16)
		     ^ (V[7] << 16)		^ V[7]		^ V[0]
		     ^ V[3]			^ V[4]		^ V[5];
}

PHashGOST3411 *
p_crypto_hash_gost3411_new (void)
{
	PHashGOST3411 *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PHashGOST3411))) == NULL))
		return NULL;

	p_crypto_hash_gost3411_reset (ret);

	return ret;
}

void
p_crypto_hash_gost3411_update (PHashGOST3411	*ctx,
			       const puchar	*data,
			       psize		len)
{
	puint32	left, to_fill, len256[8];

	left = (ctx->len[0] & 0xFF) >> 3;
	to_fill = 32 - left;

	memset (len256, 0, 32);
	len256[0] = (puint32) (len << 3);
	len256[1] = (puint32) (len >> 29);

	pp_crypto_hash_gost3411_sum_256 (ctx->len, len256);

	if (left && (puint32) len >= to_fill) {
		memcpy ((pchar *) ctx->buf + left, data, to_fill);
		pp_crypto_hash_gost3411_swap_bytes (ctx->buf, 8);
		pp_crypto_hash_gost3411_process (ctx, ctx->buf);
		pp_crypto_hash_gost3411_sum_256 (ctx->sum, ctx->buf);

		data += to_fill;
		len -= to_fill;
		left = 0;
	}

	while (len >= 32) {
		memcpy (ctx->buf, data, 32);
		pp_crypto_hash_gost3411_swap_bytes (ctx->buf, 8);
		pp_crypto_hash_gost3411_process (ctx, ctx->buf);
		pp_crypto_hash_gost3411_sum_256 (ctx->sum, ctx->buf);

		data += 32;
		len -= 32;
	}

	if (len > 0)
		memcpy ((pchar *) ctx->buf + left, data, len);
}

void
p_crypto_hash_gost3411_finish (PHashGOST3411 *ctx)
{
	puint32 left, last;

	left = ctx->len[0] & 0xFF;
	last = 32 - (left >> 3);

	if (last % 32 != 0) {
		memset ((pchar *) ctx->buf + (left >> 3), 0, last);
		pp_crypto_hash_gost3411_swap_bytes (ctx->buf, 8);
		pp_crypto_hash_gost3411_process (ctx, ctx->buf);
		pp_crypto_hash_gost3411_sum_256 (ctx->sum, ctx->buf);
	}

	pp_crypto_hash_gost3411_process (ctx, ctx->len);
	pp_crypto_hash_gost3411_process (ctx, ctx->sum);

	pp_crypto_hash_gost3411_swap_bytes (ctx->hash, 8);
}

const puchar *
p_crypto_hash_gost3411_digest (PHashGOST3411 *ctx)
{
	return (const puchar *) ctx->hash;
}

void
p_crypto_hash_gost3411_reset (PHashGOST3411 *ctx)
{
	memset (ctx->buf, 0, 32);
	memset (ctx->hash, 0, 32);
	memset (ctx->len, 0, 32);
	memset (ctx->sum, 0, 32);
}

void
p_crypto_hash_gost3411_free (PHashGOST3411 *ctx)
{
	p_free (ctx);
}
