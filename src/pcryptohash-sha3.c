/*
 * The MIT License
 *
 * Copyright (C) 2016 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include <string.h>
#include <stdlib.h>

#include "pmem.h"
#include "pcryptohash-sha3.h"

struct PHashSHA3_ {
	union buf_ {
		puchar	buf[200];
		puint64	buf_w[25];
	} buf;
	puint64		hash[25];

	puint32		len;
	puint32		block_size;
};

static const puint64 pp_crypto_hash_sha3_K[] = {
	0x0000000000000001ULL, 0x0000000000008082ULL,
	0x800000000000808AULL, 0x8000000080008000ULL,
	0x000000000000808BULL, 0x0000000080000001ULL,
	0x8000000080008081ULL, 0x8000000000008009ULL,
	0x000000000000008AULL, 0x0000000000000088ULL,
	0x0000000080008009ULL, 0x000000008000000AULL,
	0x000000008000808BULL, 0x800000000000008BULL,
	0x8000000000008089ULL, 0x8000000000008003ULL,
	0x8000000000008002ULL, 0x8000000000000080ULL,
	0x000000000000800AULL, 0x800000008000000AULL,
	0x8000000080008081ULL, 0x8000000000008080ULL,
	0x0000000080000001ULL, 0x8000000080008008ULL
};

static void pp_crypto_hash_sha3_swap_bytes (puint64 *data, puint words);
static void pp_crypto_hash_sha3_keccak_theta (PHashSHA3 *ctx);
static void pp_crypto_hash_sha3_keccak_rho_pi (PHashSHA3 *ctx);
static void pp_crypto_hash_sha3_keccak_chi (PHashSHA3 *ctx);
static void pp_crypto_hash_sha3_keccak_permutate (PHashSHA3 *ctx);
static void pp_crypto_hash_sha3_process (PHashSHA3 *ctx, const puint64 *data);
static PHashSHA3 * pp_crypto_hash_sha3_new_internal (puint bits);

#define P_SHA3_SHL(val, shift) ((val) << (shift))
#define P_SHA3_ROTL(val, shift) (P_SHA3_SHL(val, shift) | ((val) >> (64 - (shift))))

static void
pp_crypto_hash_sha3_swap_bytes (puint64	*data,
				puint	words)
{
#ifndef PLIBSYS_IS_BIGENDIAN
	P_UNUSED (data);
	P_UNUSED (words);
#else
	while (words-- > 0) {
		*data = PUINT64_TO_LE (*data);
		++data;
	}
#endif
}

/* Theta step (see [Keccak Reference, Section 2.3.2]) */
static void
pp_crypto_hash_sha3_keccak_theta (PHashSHA3 *ctx)
{
	puint	i;
	puint64	C[5], D[5];

	/* Compute the parity of the columns */
	for (i = 0; i < 5; ++i)
		C[i] = ctx->hash[i] ^ ctx->hash[i + 5] ^ ctx->hash[i + 10] ^ ctx->hash[i + 15] ^ ctx->hash[i + 20];

	/* Compute the theta effect for a given column */
	D[0] = P_SHA3_ROTL (C[1], 1) ^ C[4];
	D[1] = P_SHA3_ROTL (C[2], 1) ^ C[0];
	D[2] = P_SHA3_ROTL (C[3], 1) ^ C[1];
	D[3] = P_SHA3_ROTL (C[4], 1) ^ C[2];
	D[4] = P_SHA3_ROTL (C[0], 1) ^ C[3];

	/* Add the theta effect to the whole column */
	for (i = 0; i < 5; ++i) {
		ctx->hash[i]      ^= D[i];
		ctx->hash[i + 5]  ^= D[i];
		ctx->hash[i + 10] ^= D[i];
		ctx->hash[i + 15] ^= D[i];
		ctx->hash[i + 20] ^= D[i];
	}
}

/* Rho and pi steps (see [Keccak Reference, Sections 2.3.3 and 2.3.4]) */
static void
pp_crypto_hash_sha3_keccak_rho_pi (PHashSHA3 *ctx)
{
	puint64 tmp_A;

	/* Unroll the loop over ((0 1)(2 3))^t * (1 0) for 0 ≤ t ≤ 23 */
	tmp_A = ctx->hash[1];
	ctx->hash[1]  = P_SHA3_ROTL (ctx->hash[6],  44);
	ctx->hash[6]  = P_SHA3_ROTL (ctx->hash[9],  20);
	ctx->hash[9]  = P_SHA3_ROTL (ctx->hash[22], 61);
	ctx->hash[22] = P_SHA3_ROTL (ctx->hash[14], 39);
	ctx->hash[14] = P_SHA3_ROTL (ctx->hash[20], 18);
	ctx->hash[20] = P_SHA3_ROTL (ctx->hash[2],  62);
	ctx->hash[2]  = P_SHA3_ROTL (ctx->hash[12], 43);
	ctx->hash[12] = P_SHA3_ROTL (ctx->hash[13], 25);
	ctx->hash[13] = P_SHA3_ROTL (ctx->hash[19],  8);
	ctx->hash[19] = P_SHA3_ROTL (ctx->hash[23], 56);
	ctx->hash[23] = P_SHA3_ROTL (ctx->hash[15], 41);
	ctx->hash[15] = P_SHA3_ROTL (ctx->hash[4],  27);
	ctx->hash[4]  = P_SHA3_ROTL (ctx->hash[24], 14);
	ctx->hash[24] = P_SHA3_ROTL (ctx->hash[21],  2);
	ctx->hash[21] = P_SHA3_ROTL (ctx->hash[8],  55);
	ctx->hash[8]  = P_SHA3_ROTL (ctx->hash[16], 45);
	ctx->hash[16] = P_SHA3_ROTL (ctx->hash[5],  36);
	ctx->hash[5]  = P_SHA3_ROTL (ctx->hash[3],  28);
	ctx->hash[3]  = P_SHA3_ROTL (ctx->hash[18], 21);
	ctx->hash[18] = P_SHA3_ROTL (ctx->hash[17], 15);
	ctx->hash[17] = P_SHA3_ROTL (ctx->hash[11], 10);
	ctx->hash[11] = P_SHA3_ROTL (ctx->hash[7],   6);
	ctx->hash[7]  = P_SHA3_ROTL (ctx->hash[10],  3);
	ctx->hash[10] = P_SHA3_ROTL (tmp_A, 1);
}

/* Chi step (see [Keccak Reference, Section 2.3.1]) */
static void
pp_crypto_hash_sha3_keccak_chi (PHashSHA3 *ctx)
{
	puint i;
	puint64	tmp_A1, tmp_A2;

	for (i = 0; i < 25; i += 5) {
		tmp_A1 = ctx->hash[i + 0];
		tmp_A2 = ctx->hash[i + 1];

		ctx->hash[i + 0] ^= ~tmp_A2 & ctx->hash[i + 2];
		ctx->hash[i + 1] ^= ~ctx->hash[i + 2] & ctx->hash[i + 3];
		ctx->hash[i + 2] ^= ~ctx->hash[i + 3] & ctx->hash[i + 4];
		ctx->hash[i + 3] ^= ~ctx->hash[i + 4] & tmp_A1;
		ctx->hash[i + 4] ^= ~tmp_A1 & tmp_A2;
	}
}

static void
pp_crypto_hash_sha3_keccak_permutate (PHashSHA3 *ctx)
{
	puint i;

	for (i = 0; i < 24; ++i) {
		pp_crypto_hash_sha3_keccak_theta (ctx);
		pp_crypto_hash_sha3_keccak_rho_pi (ctx);
		pp_crypto_hash_sha3_keccak_chi (ctx);

		/* Iota step (see [Keccak Reference, Section 2.3.5]) */
		ctx->hash[0] ^= pp_crypto_hash_sha3_K[i];
	}
}

static void
pp_crypto_hash_sha3_process (PHashSHA3		*ctx,
			     const puint64	*data)
{
	puint i;
	puint qwords = ctx->block_size / 8;

	for (i = 0; i < qwords; ++i)
		ctx->hash[i] ^= data[i];

	/* Make the Keccak permutation */
	pp_crypto_hash_sha3_keccak_permutate (ctx);
}

static PHashSHA3 *
pp_crypto_hash_sha3_new_internal (puint bits)
{
	PHashSHA3 *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PHashSHA3))) == NULL))
		return NULL;

	ret->block_size = (1600 - bits * 2) / 8;

	return ret;
}

void
p_crypto_hash_sha3_reset (PHashSHA3 *ctx)
{
	memset (ctx->buf.buf, 0, 200);
	memset (ctx->hash, 0, sizeof (ctx->hash));

	ctx->len = 0;
}

PHashSHA3 *
p_crypto_hash_sha3_224_new (void)
{
	return pp_crypto_hash_sha3_new_internal (224);
}

PHashSHA3 *
p_crypto_hash_sha3_256_new (void)
{
	return pp_crypto_hash_sha3_new_internal (256);
}

PHashSHA3 *
p_crypto_hash_sha3_384_new (void)
{
	return pp_crypto_hash_sha3_new_internal (384);
}

PHashSHA3 *
p_crypto_hash_sha3_512_new (void)
{
	return pp_crypto_hash_sha3_new_internal (512);
}

void
p_crypto_hash_sha3_update (PHashSHA3	*ctx,
			   const puchar	*data,
			   psize	len)
{
	puint32	left, to_fill;

	left     = ctx->len;
	to_fill  = ctx->block_size - left;
	ctx->len = (puint32) (((psize) ctx->len + len) % (psize) ctx->block_size);

	if (left && (puint64) len >= to_fill) {
		memcpy (ctx->buf.buf + left, data, to_fill);
		pp_crypto_hash_sha3_swap_bytes (ctx->buf.buf_w, ctx->block_size >> 3);
		pp_crypto_hash_sha3_process (ctx, ctx->buf.buf_w);

		data += to_fill;
		len -= to_fill;
		left = 0;
	}

	while (len >= ctx->block_size) {
		memcpy (ctx->buf.buf, data, ctx->block_size);
		pp_crypto_hash_sha3_swap_bytes (ctx->buf.buf_w, ctx->block_size >> 3);
		pp_crypto_hash_sha3_process (ctx, ctx->buf.buf_w);

		data += ctx->block_size;
		len -= ctx->block_size;
	}

	if (len > 0)
		memcpy (ctx->buf.buf + left, data, len);
}

void
p_crypto_hash_sha3_finish (PHashSHA3 *ctx)
{
	memset (ctx->buf.buf + ctx->len, 0, ctx->block_size - ctx->len);
	ctx->buf.buf[ctx->len]            |= 0x06;
	ctx->buf.buf[ctx->block_size - 1] |= 0x80;

	pp_crypto_hash_sha3_swap_bytes (ctx->buf.buf_w, ctx->block_size >> 3);
	pp_crypto_hash_sha3_process (ctx, ctx->buf.buf_w);

	pp_crypto_hash_sha3_swap_bytes (ctx->hash, (100 - (ctx->block_size >> 2)) >> 3);
}

const puchar *
p_crypto_hash_sha3_digest (PHashSHA3 *ctx)
{
	return (const puchar *) ctx->hash;
}

void
p_crypto_hash_sha3_free (PHashSHA3 *ctx)
{
	p_free (ctx);
}
