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
#include "pcryptohash-sha2-512.h"

struct PHashSHA2_512_ {
	union buf_ {
		puchar	buf[128];
		puint64	buf_w[16];
	} buf;
	puint64		hash[8];

	puint64		len_high;
	puint64		len_low;

	pboolean	is384;
};

static const puchar pp_crypto_hash_sha2_512_pad[128] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const puint64 pp_crypto_hash_sha2_512_K[] = {
	0x428A2F98D728AE22ULL,  0x7137449123EF65CDULL,
	0xB5C0FBCFEC4D3B2FULL,  0xE9B5DBA58189DBBCULL,
	0x3956C25BF348B538ULL,  0x59F111F1B605D019ULL,
	0x923F82A4AF194F9BULL,  0xAB1C5ED5DA6D8118ULL,
	0xD807AA98A3030242ULL,  0x12835B0145706FBEULL,
	0x243185BE4EE4B28CULL,  0x550C7DC3D5FFB4E2ULL,
	0x72BE5D74F27B896FULL,  0x80DEB1FE3B1696B1ULL,
	0x9BDC06A725C71235ULL,  0xC19BF174CF692694ULL,
	0xE49B69C19EF14AD2ULL,  0xEFBE4786384F25E3ULL,
	0x0FC19DC68B8CD5B5ULL,  0x240CA1CC77AC9C65ULL,
	0x2DE92C6F592B0275ULL,  0x4A7484AA6EA6E483ULL,
	0x5CB0A9DCBD41FBD4ULL,  0x76F988DA831153B5ULL,
	0x983E5152EE66DFABULL,  0xA831C66D2DB43210ULL,
	0xB00327C898FB213FULL,  0xBF597FC7BEEF0EE4ULL,
	0xC6E00BF33DA88FC2ULL,  0xD5A79147930AA725ULL,
	0x06CA6351E003826FULL,  0x142929670A0E6E70ULL,
	0x27B70A8546D22FFCULL,  0x2E1B21385C26C926ULL,
	0x4D2C6DFC5AC42AEDULL,  0x53380D139D95B3DFULL,
	0x650A73548BAF63DEULL,  0x766A0ABB3C77B2A8ULL,
	0x81C2C92E47EDAEE6ULL,  0x92722C851482353BULL,
	0xA2BFE8A14CF10364ULL,  0xA81A664BBC423001ULL,
	0xC24B8B70D0F89791ULL,  0xC76C51A30654BE30ULL,
	0xD192E819D6EF5218ULL,  0xD69906245565A910ULL,
	0xF40E35855771202AULL,  0x106AA07032BBD1B8ULL,
	0x19A4C116B8D2D0C8ULL,  0x1E376C085141AB53ULL,
	0x2748774CDF8EEB99ULL,  0x34B0BCB5E19B48A8ULL,
	0x391C0CB3C5C95A63ULL,  0x4ED8AA4AE3418ACBULL,
	0x5B9CCA4F7763E373ULL,  0x682E6FF3D6B2B8A3ULL,
	0x748F82EE5DEFB2FCULL,  0x78A5636F43172F60ULL,
	0x84C87814A1F0AB72ULL,  0x8CC702081A6439ECULL,
	0x90BEFFFA23631E28ULL,  0xA4506CEBDE82BDE9ULL,
	0xBEF9A3F7B2C67915ULL,  0xC67178F2E372532BULL,
	0xCA273ECEEA26619CULL,  0xD186B8C721C0C207ULL,
	0xEADA7DD6CDE0EB1EULL,  0xF57D4F7FEE6ED178ULL,
	0x06F067AA72176FBAULL,  0x0A637DC5A2C898A6ULL,
	0x113F9804BEF90DAEULL,  0x1B710B35131C471BULL,
	0x28DB77F523047D84ULL,  0x32CAAB7B40C72493ULL,
	0x3C9EBE0A15C9BEBCULL,  0x431D67C49C100D4CULL,
	0x4CC5D4BECB3E42B6ULL,  0x597F299CFC657E2AULL,
	0x5FCB6FAB3AD6FAECULL,  0x6C44198C4A475817ULL
};

static void pp_crypto_hash_sha2_512_swap_bytes (puint64 *data, puint words);
static void pp_crypto_hash_sha2_512_process (PHashSHA2_512 *ctx, const puint64 data[16]);
static PHashSHA2_512 * pp_crypto_hash_sha2_512_new_internal (pboolean is384);

#define P_SHA2_512_SHR(val, shift) ((val) >> (shift))
#define P_SHA2_512_ROTR(val, shift) (P_SHA2_512_SHR(val, shift) | ((val) << (64 - (shift))))

#define P_SHA2_512_S0(x) (P_SHA2_512_ROTR (x, 1)  ^ P_SHA2_512_ROTR (x, 8)  ^ P_SHA2_512_SHR  (x, 7))
#define P_SHA2_512_S1(x) (P_SHA2_512_ROTR (x, 19) ^ P_SHA2_512_ROTR (x, 61) ^ P_SHA2_512_SHR  (x, 6))
#define P_SHA2_512_S2(x) (P_SHA2_512_ROTR (x, 28) ^ P_SHA2_512_ROTR (x, 34) ^ P_SHA2_512_ROTR (x, 39))
#define P_SHA2_512_S3(x) (P_SHA2_512_ROTR (x, 14) ^ P_SHA2_512_ROTR (x, 18) ^ P_SHA2_512_ROTR (x, 41))

#define P_SHA2_512_F0(x, y, z) ((x & y) | (z & (x | y)))
#define P_SHA2_512_F1(x, y, z) (z ^ (x & (y ^ z)))

#define P_SHA2_512_P(a, b, c, d, e, f, g, h, x, K)				\
{										\
	tmp_sum1 = h + P_SHA2_512_S3 (e) + P_SHA2_512_F1 (e, f, g) + K + x;	\
	tmp_sum2 = P_SHA2_512_S2 (a) + P_SHA2_512_F0 (a, b, c);			\
	d += tmp_sum1;								\
	h = tmp_sum1 + tmp_sum2;						\
}

static void
pp_crypto_hash_sha2_512_swap_bytes (puint64	*data,
				    puint	words)
{
#ifdef PLIBSYS_IS_BIGENDIAN
	P_UNUSED (data);
	P_UNUSED (words);
#else
	while (words-- > 0) {
		*data = PUINT64_TO_BE (*data);
		++data;
	}
#endif
}

static void
pp_crypto_hash_sha2_512_process (PHashSHA2_512	*ctx,
				 const puint64	data[16])
{
	puint64	tmp_sum1, tmp_sum2;
	puint64 W[80];
	puint64	A[8];
	puint	i;

	for (i = 0; i < 8; ++i)
		A[i] = ctx->hash[i];

	memcpy (W, data, 128);

	for (i = 16; i < 80; ++i)
		W[i] = P_SHA2_512_S1 (W[i -  2]) + W[i -  7] + P_SHA2_512_S0 (W[i - 15]) + W[i - 16];

	for (i = 0; i < 80; i += 8) {
		P_SHA2_512_P (A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], W[i + 0], pp_crypto_hash_sha2_512_K[i + 0]);
		P_SHA2_512_P (A[7], A[0], A[1], A[2], A[3], A[4], A[5], A[6], W[i + 1], pp_crypto_hash_sha2_512_K[i + 1]);
		P_SHA2_512_P (A[6], A[7], A[0], A[1], A[2], A[3], A[4], A[5], W[i + 2], pp_crypto_hash_sha2_512_K[i + 2]);
		P_SHA2_512_P (A[5], A[6], A[7], A[0], A[1], A[2], A[3], A[4], W[i + 3], pp_crypto_hash_sha2_512_K[i + 3]);
		P_SHA2_512_P (A[4], A[5], A[6], A[7], A[0], A[1], A[2], A[3], W[i + 4], pp_crypto_hash_sha2_512_K[i + 4]);
		P_SHA2_512_P (A[3], A[4], A[5], A[6], A[7], A[0], A[1], A[2], W[i + 5], pp_crypto_hash_sha2_512_K[i + 5]);
		P_SHA2_512_P (A[2], A[3], A[4], A[5], A[6], A[7], A[0], A[1], W[i + 6], pp_crypto_hash_sha2_512_K[i + 6]);
		P_SHA2_512_P (A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[0], W[i + 7], pp_crypto_hash_sha2_512_K[i + 7]);
	}

	for (i = 0; i < 8; ++i)
		ctx->hash[i] += A[i];
}

static PHashSHA2_512 *
pp_crypto_hash_sha2_512_new_internal (pboolean is384)
{
	PHashSHA2_512 *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PHashSHA2_512))) == NULL))
		return NULL;

	ret->is384 = is384;

	p_crypto_hash_sha2_512_reset (ret);

	return ret;
}

void
p_crypto_hash_sha2_512_reset (PHashSHA2_512 *ctx)
{
	memset (ctx->buf.buf, 0, 128);

	ctx->len_low = 0;
	ctx->len_high = 0;

	if (ctx->is384 == FALSE) {
		/* SHA2-512 */
		ctx->hash[0] = 0x6A09E667F3BCC908ULL;
		ctx->hash[1] = 0xBB67AE8584CAA73BULL;
		ctx->hash[2] = 0x3C6EF372FE94F82BULL;
		ctx->hash[3] = 0xA54FF53A5F1D36F1ULL;
		ctx->hash[4] = 0x510E527FADE682D1ULL;
		ctx->hash[5] = 0x9B05688C2B3E6C1FULL;
		ctx->hash[6] = 0x1F83D9ABFB41BD6BULL;
		ctx->hash[7] = 0x5BE0CD19137E2179ULL;
	} else {
		/* SHA2-384 */
		ctx->hash[0] = 0xCBBB9D5DC1059ED8ULL;
		ctx->hash[1] = 0x629A292A367CD507ULL;
		ctx->hash[2] = 0x9159015A3070DD17ULL;
		ctx->hash[3] = 0x152FECD8F70E5939ULL;
		ctx->hash[4] = 0x67332667FFC00B31ULL;
		ctx->hash[5] = 0x8EB44A8768581511ULL;
		ctx->hash[6] = 0xDB0C2E0D64F98FA7ULL;
		ctx->hash[7] = 0x47B5481DBEFA4FA4ULL;
	}
}

PHashSHA2_512 *
p_crypto_hash_sha2_512_new (void)
{
	return pp_crypto_hash_sha2_512_new_internal (FALSE);
}

PHashSHA2_512 *
p_crypto_hash_sha2_384_new (void)
{
	return pp_crypto_hash_sha2_512_new_internal (TRUE);
}

void
p_crypto_hash_sha2_512_update (PHashSHA2_512	*ctx,
			       const puchar	*data,
			       psize		len)
{
	puint32	left, to_fill;

	left = (puint32) (ctx->len_low & 0x7F);
	to_fill = 128 - left;

	ctx->len_low += (puint64) len;

	if (ctx->len_low < (puint64) len)
		++ctx->len_high;

	if (left && (puint64) len >= to_fill) {
		memcpy (ctx->buf.buf + left, data, to_fill);
		pp_crypto_hash_sha2_512_swap_bytes (ctx->buf.buf_w, 16);
		pp_crypto_hash_sha2_512_process (ctx, ctx->buf.buf_w);

		data += to_fill;
		len -= to_fill;
		left = 0;
	}

	while (len >= 128) {
		memcpy (ctx->buf.buf, data, 128);
		pp_crypto_hash_sha2_512_swap_bytes (ctx->buf.buf_w, 16);
		pp_crypto_hash_sha2_512_process (ctx, ctx->buf.buf_w);

		data += 128;
		len -= 128;
	}

	if (len > 0)
		memcpy (ctx->buf.buf + left, data, len);
}

void
p_crypto_hash_sha2_512_finish (PHashSHA2_512 *ctx)
{
	puint64	high, low;
	pint	left, last;

	left = (pint) (ctx->len_low & 0x7F);
	last = (left < 112) ? (112 - left) : (240 - left);

	low = ctx->len_low << 3;
	high = ctx->len_high << 3
	     | ctx->len_low >> 61;

	if (last > 0)
		p_crypto_hash_sha2_512_update (ctx, pp_crypto_hash_sha2_512_pad, (psize) last);

	ctx->buf.buf_w[14] = high;
	ctx->buf.buf_w[15] = low;

	pp_crypto_hash_sha2_512_swap_bytes (ctx->buf.buf_w, 14);
	pp_crypto_hash_sha2_512_process (ctx, ctx->buf.buf_w);

	pp_crypto_hash_sha2_512_swap_bytes (ctx->hash, ctx->is384 == FALSE ? 8 : 6);
}

const puchar *
p_crypto_hash_sha2_512_digest (PHashSHA2_512 *ctx)
{
	return (const puchar *) ctx->hash;
}

void
p_crypto_hash_sha2_512_free (PHashSHA2_512 *ctx)
{
	p_free (ctx);
}
