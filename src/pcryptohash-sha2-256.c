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
#include "pcryptohash-sha2-256.h"

struct PHashSHA2_256_ {
	union buf_ {
		puchar	buf[64];
		puint32	buf_w[16];
	} buf;
	puint32		hash[8];

	puint32		len_high;
	puint32		len_low;

	pboolean	is224;
};

static const puchar pp_crypto_hash_sha2_256_pad[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const puint32 pp_crypto_hash_sha2_256_K[] = {
	0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
	0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
	0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
	0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
	0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
	0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
	0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
	0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
	0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
	0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
	0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
	0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
	0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
	0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
	0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
	0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

static void pp_crypto_hash_sha2_256_swap_bytes (puint32 *data, puint words);
static void pp_crypto_hash_sha2_256_process (PHashSHA2_256 *ctx, const puint32 data[16]);
static PHashSHA2_256 * pp_crypto_hash_sha2_256_new_internal (pboolean is224);

#define P_SHA2_256_SHR(val, shift) (((val) & 0xFFFFFFFF) >> (shift))
#define P_SHA2_256_ROTR(val, shift) (P_SHA2_256_SHR(val, shift) | ((val) << (32 - (shift))))

#define P_SHA2_256_S0(x) (P_SHA2_256_ROTR (x, 7)  ^ P_SHA2_256_ROTR (x, 18) ^ P_SHA2_256_SHR  (x, 3))
#define P_SHA2_256_S1(x) (P_SHA2_256_ROTR (x, 17) ^ P_SHA2_256_ROTR (x, 19) ^ P_SHA2_256_SHR  (x, 10))
#define P_SHA2_256_S2(x) (P_SHA2_256_ROTR (x, 2)  ^ P_SHA2_256_ROTR (x, 13) ^ P_SHA2_256_ROTR (x, 22))
#define P_SHA2_256_S3(x) (P_SHA2_256_ROTR (x, 6)  ^ P_SHA2_256_ROTR (x, 11) ^ P_SHA2_256_ROTR (x, 25))

#define P_SHA2_256_F0(x, y, z) ((x & y) | (z & (x | y)))
#define P_SHA2_256_F1(x, y, z) (z ^ (x & (y ^ z)))

#define P_SHA2_256_R(t)								\
(										\
	W[t] = P_SHA2_256_S1 (W[t -  2]) + W[t -  7] +				\
	       P_SHA2_256_S0 (W[t - 15]) + W[t - 16]				\
)

#define P_SHA2_256_P(a, b, c, d, e, f, g, h, x, K)				\
{										\
	tmp_sum1 = h + P_SHA2_256_S3 (e) + P_SHA2_256_F1 (e, f, g) + K + x;	\
	tmp_sum2 = P_SHA2_256_S2 (a) + P_SHA2_256_F0 (a, b, c);			\
	d += tmp_sum1;								\
	h = tmp_sum1 + tmp_sum2;						\
}

static void
pp_crypto_hash_sha2_256_swap_bytes (puint32	*data,
				    puint	words)
{
#ifdef PLIBSYS_IS_BIGENDIAN
	P_UNUSED (data);
	P_UNUSED (words);
#else
	while (words-- > 0) {
		*data = PUINT32_TO_BE (*data);
		++data;
	}
#endif
}

static void
pp_crypto_hash_sha2_256_process (PHashSHA2_256	*ctx,
				 const puint32	data[16])
{
	puint32	tmp_sum1, tmp_sum2;
	puint32 W[64];
	puint32	A[8];
	puint	i;

	for (i = 0; i < 8; i++)
		A[i] = ctx->hash[i];

	memcpy (W, data, 64);

	for (i = 0; i < 16; i += 8) {
		P_SHA2_256_P (A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], W[i + 0], pp_crypto_hash_sha2_256_K[i + 0]);
		P_SHA2_256_P (A[7], A[0], A[1], A[2], A[3], A[4], A[5], A[6], W[i + 1], pp_crypto_hash_sha2_256_K[i + 1]);
		P_SHA2_256_P (A[6], A[7], A[0], A[1], A[2], A[3], A[4], A[5], W[i + 2], pp_crypto_hash_sha2_256_K[i + 2]);
		P_SHA2_256_P (A[5], A[6], A[7], A[0], A[1], A[2], A[3], A[4], W[i + 3], pp_crypto_hash_sha2_256_K[i + 3]);
		P_SHA2_256_P (A[4], A[5], A[6], A[7], A[0], A[1], A[2], A[3], W[i + 4], pp_crypto_hash_sha2_256_K[i + 4]);
		P_SHA2_256_P (A[3], A[4], A[5], A[6], A[7], A[0], A[1], A[2], W[i + 5], pp_crypto_hash_sha2_256_K[i + 5]);
		P_SHA2_256_P (A[2], A[3], A[4], A[5], A[6], A[7], A[0], A[1], W[i + 6], pp_crypto_hash_sha2_256_K[i + 6]);
		P_SHA2_256_P (A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[0], W[i + 7], pp_crypto_hash_sha2_256_K[i + 7]);
	}

	for (i = 16; i < 64; i += 8) {
		P_SHA2_256_P (A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], P_SHA2_256_R (i + 0), pp_crypto_hash_sha2_256_K[i + 0]);
		P_SHA2_256_P (A[7], A[0], A[1], A[2], A[3], A[4], A[5], A[6], P_SHA2_256_R (i + 1), pp_crypto_hash_sha2_256_K[i + 1]);
		P_SHA2_256_P (A[6], A[7], A[0], A[1], A[2], A[3], A[4], A[5], P_SHA2_256_R (i + 2), pp_crypto_hash_sha2_256_K[i + 2]);
		P_SHA2_256_P (A[5], A[6], A[7], A[0], A[1], A[2], A[3], A[4], P_SHA2_256_R (i + 3), pp_crypto_hash_sha2_256_K[i + 3]);
		P_SHA2_256_P (A[4], A[5], A[6], A[7], A[0], A[1], A[2], A[3], P_SHA2_256_R (i + 4), pp_crypto_hash_sha2_256_K[i + 4]);
		P_SHA2_256_P (A[3], A[4], A[5], A[6], A[7], A[0], A[1], A[2], P_SHA2_256_R (i + 5), pp_crypto_hash_sha2_256_K[i + 5]);
		P_SHA2_256_P (A[2], A[3], A[4], A[5], A[6], A[7], A[0], A[1], P_SHA2_256_R (i + 6), pp_crypto_hash_sha2_256_K[i + 6]);
		P_SHA2_256_P (A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[0], P_SHA2_256_R (i + 7), pp_crypto_hash_sha2_256_K[i + 7]);
	}

	for (i = 0; i < 8; i++)
		ctx->hash[i] += A[i];
}

static PHashSHA2_256 *
pp_crypto_hash_sha2_256_new_internal (pboolean is224)
{
	PHashSHA2_256 *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PHashSHA2_256))) == NULL))
		return NULL;

	ret->is224 = is224;

	p_crypto_hash_sha2_256_reset (ret);

	return ret;
}

void
p_crypto_hash_sha2_256_reset (PHashSHA2_256 *ctx)
{
	memset (ctx->buf.buf, 0, 64);

	ctx->len_low = 0;
	ctx->len_high = 0;

	if (ctx->is224 == FALSE) {
		/* SHA2-256 */
		ctx->hash[0] = 0x6A09E667;
		ctx->hash[1] = 0xBB67AE85;
		ctx->hash[2] = 0x3C6EF372;
		ctx->hash[3] = 0xA54FF53A;
		ctx->hash[4] = 0x510E527F;
		ctx->hash[5] = 0x9B05688C;
		ctx->hash[6] = 0x1F83D9AB;
		ctx->hash[7] = 0x5BE0CD19;
	} else {
		/* SHA2-224 */
		ctx->hash[0] = 0xC1059ED8;
		ctx->hash[1] = 0x367CD507;
		ctx->hash[2] = 0x3070DD17;
		ctx->hash[3] = 0xF70E5939;
		ctx->hash[4] = 0xFFC00B31;
		ctx->hash[5] = 0x68581511;
		ctx->hash[6] = 0x64F98FA7;
		ctx->hash[7] = 0xBEFA4FA4;
	}
}

PHashSHA2_256 *
p_crypto_hash_sha2_256_new (void)
{
	return pp_crypto_hash_sha2_256_new_internal (FALSE);
}

PHashSHA2_256 *
p_crypto_hash_sha2_224_new (void)
{
	return pp_crypto_hash_sha2_256_new_internal (TRUE);
}

void
p_crypto_hash_sha2_256_update (PHashSHA2_256	*ctx,
			       const puchar	*data,
			       psize		len)
{
	puint32	left, to_fill;

	left = ctx->len_low & 0x3F;
	to_fill = 64 - left;

	ctx->len_low += (puint32) len;

	if (ctx->len_low < (puint32) len)
		++ctx->len_high;

	if (left && (puint32) len >= to_fill) {
		memcpy (ctx->buf.buf + left, data, to_fill);
		pp_crypto_hash_sha2_256_swap_bytes (ctx->buf.buf_w, 16);
		pp_crypto_hash_sha2_256_process (ctx, ctx->buf.buf_w);

		data += to_fill;
		len -= to_fill;
		left = 0;
	}

	while (len >= 64) {
		memcpy (ctx->buf.buf, data, 64);
		pp_crypto_hash_sha2_256_swap_bytes (ctx->buf.buf_w, 16);
		pp_crypto_hash_sha2_256_process (ctx, ctx->buf.buf_w);

		data += 64;
		len -= 64;
	}

	if (len > 0)
		memcpy (ctx->buf.buf + left, data, len);
}

void
p_crypto_hash_sha2_256_finish (PHashSHA2_256 *ctx)
{
	puint32	high, low;
	pint	left, last;

	left = ctx->len_low & 0x3F;
	last = (left < 56) ? (56 - left) : (120 - left);

	low = ctx->len_low << 3;
	high = ctx->len_high << 3
	     | ctx->len_low >> 29;

	if (last > 0)
		p_crypto_hash_sha2_256_update (ctx, pp_crypto_hash_sha2_256_pad, (psize) last);

	ctx->buf.buf_w[14] = high;
	ctx->buf.buf_w[15] = low;

	pp_crypto_hash_sha2_256_swap_bytes (ctx->buf.buf_w, 14);
	pp_crypto_hash_sha2_256_process (ctx, ctx->buf.buf_w);

	pp_crypto_hash_sha2_256_swap_bytes (ctx->hash, ctx->is224 == FALSE ? 8 : 7);
}

const puchar *
p_crypto_hash_sha2_256_digest (PHashSHA2_256 *ctx)
{
	return (const puchar *) ctx->hash;
}

void
p_crypto_hash_sha2_256_free (PHashSHA2_256 *ctx)
{
	p_free (ctx);
}
