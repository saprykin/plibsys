/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
#include "pcryptohash-sha1.h"

struct PHashSHA1_ {
	union buf_ {
		puchar	buf[64];
		puint32	buf_w[16];
	} buf;
	puint32		hash[5];

	puint32		len_high;
	puint32		len_low;
};

static const puchar pp_crypto_hash_sha1_pad[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void pp_crypto_hash_sha1_swap_bytes (puint32 *data, puint words);
static void pp_crypto_hash_sha1_process (PHashSHA1 *ctx, const puint32 data[16]);

#define P_SHA1_ROTL(val, shift) ((val) << (shift) |  (val) >> (32 - (shift)))

#define P_SHA1_F1(x, y, z) ((x & y) | ((~x) & z))
#define P_SHA1_F2(x, y, z) (x ^ y ^ z)
#define P_SHA1_F3(x, y, z) ((x & y) | (x & z) | (y & z))

#define P_SHA1_W(W, i) 					\
(							\
	(W)[i & 0x0F] = P_SHA1_ROTL (			\
				(W)[(i - 3)  & 0x0F]	\
			      ^ (W)[(i - 8)  & 0x0F]	\
			      ^ (W)[(i - 14) & 0x0F]	\
			      ^ (W)[(i - 16) & 0x0F],	\
			      1)			\
)

#define P_SHA1_ROUND_0(a, b, c, d, e, w)		\
{							\
	e += P_SHA1_ROTL (a, 5) + P_SHA1_F1 (b, c, d)	\
	   + 0x5A827999 + w;				\
	b = P_SHA1_ROTL (b, 30);			\
}

#define P_SHA1_ROUND_1(a, b, c, d, e, w)		\
{							\
	e += P_SHA1_ROTL (a, 5) + P_SHA1_F2 (b, c, d)	\
	   + 0x6ED9EBA1 + w;				\
	b = P_SHA1_ROTL (b, 30);			\
}

#define P_SHA1_ROUND_2(a, b, c, d, e, w)		\
{							\
	e += P_SHA1_ROTL (a, 5) + P_SHA1_F3 (b, c, d)	\
	   + 0x8F1BBCDC + w;				\
	b = P_SHA1_ROTL (b, 30);			\
}

#define P_SHA1_ROUND_3(a, b, c, d, e, w)		\
{							\
	e += P_SHA1_ROTL (a, 5) + P_SHA1_F2 (b, c, d)	\
	   + 0xCA62C1D6 + w;				\
	b = P_SHA1_ROTL (b, 30);			\
}

static void
pp_crypto_hash_sha1_swap_bytes (puint32	*data,
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
pp_crypto_hash_sha1_process (PHashSHA1		*ctx,
			     const puint32	data[16])
{
	puint32	W[16], A, B, C, D, E;

	if (P_UNLIKELY (ctx == NULL))
		return;

	memcpy (W, data, 64);

	A = ctx->hash[0];
	B = ctx->hash[1];
	C = ctx->hash[2];
	D = ctx->hash[3];
	E = ctx->hash[4];

	P_SHA1_ROUND_0 (A, B, C, D, E, W[0]);
	P_SHA1_ROUND_0 (E, A, B, C, D, W[1]);
	P_SHA1_ROUND_0 (D, E, A, B, C, W[2]);
	P_SHA1_ROUND_0 (C, D, E, A, B, W[3]);
	P_SHA1_ROUND_0 (B, C, D, E, A, W[4]);
	P_SHA1_ROUND_0 (A, B, C, D, E, W[5]);
	P_SHA1_ROUND_0 (E, A, B, C, D, W[6]);
	P_SHA1_ROUND_0 (D, E, A, B, C, W[7]);
	P_SHA1_ROUND_0 (C, D, E, A, B, W[8]);
	P_SHA1_ROUND_0 (B, C, D, E, A, W[9]);
	P_SHA1_ROUND_0 (A, B, C, D, E, W[10]);
	P_SHA1_ROUND_0 (E, A, B, C, D, W[11]);
	P_SHA1_ROUND_0 (D, E, A, B, C, W[12]);
	P_SHA1_ROUND_0 (C, D, E, A, B, W[13]);
	P_SHA1_ROUND_0 (B, C, D, E, A, W[14]);
	P_SHA1_ROUND_0 (A, B, C, D, E, W[15]);
	P_SHA1_ROUND_0 (E, A, B, C, D, P_SHA1_W (W, 16));
	P_SHA1_ROUND_0 (D, E, A, B, C, P_SHA1_W (W, 17));
	P_SHA1_ROUND_0 (C, D, E, A, B, P_SHA1_W (W, 18));
	P_SHA1_ROUND_0 (B, C, D, E, A, P_SHA1_W (W, 19));

	P_SHA1_ROUND_1 (A, B, C, D, E, P_SHA1_W (W, 20));
	P_SHA1_ROUND_1 (E, A, B, C, D, P_SHA1_W (W, 21));
	P_SHA1_ROUND_1 (D, E, A, B, C, P_SHA1_W (W, 22));
	P_SHA1_ROUND_1 (C, D, E, A, B, P_SHA1_W (W, 23));
	P_SHA1_ROUND_1 (B, C, D, E, A, P_SHA1_W (W, 24));
	P_SHA1_ROUND_1 (A, B, C, D, E, P_SHA1_W (W, 25));
	P_SHA1_ROUND_1 (E, A, B, C, D, P_SHA1_W (W, 26));
	P_SHA1_ROUND_1 (D, E, A, B, C, P_SHA1_W (W, 27));
	P_SHA1_ROUND_1 (C, D, E, A, B, P_SHA1_W (W, 28));
	P_SHA1_ROUND_1 (B, C, D, E, A, P_SHA1_W (W, 29));
	P_SHA1_ROUND_1 (A, B, C, D, E, P_SHA1_W (W, 30));
	P_SHA1_ROUND_1 (E, A, B, C, D, P_SHA1_W (W, 31));
	P_SHA1_ROUND_1 (D, E, A, B, C, P_SHA1_W (W, 32));
	P_SHA1_ROUND_1 (C, D, E, A, B, P_SHA1_W (W, 33));
	P_SHA1_ROUND_1 (B, C, D, E, A, P_SHA1_W (W, 34));
	P_SHA1_ROUND_1 (A, B, C, D, E, P_SHA1_W (W, 35));
	P_SHA1_ROUND_1 (E, A, B, C, D, P_SHA1_W (W, 36));
	P_SHA1_ROUND_1 (D, E, A, B, C, P_SHA1_W (W, 37));
	P_SHA1_ROUND_1 (C, D, E, A, B, P_SHA1_W (W, 38));
	P_SHA1_ROUND_1 (B, C, D, E, A, P_SHA1_W (W, 39));

	P_SHA1_ROUND_2 (A, B, C, D, E, P_SHA1_W (W, 40));
	P_SHA1_ROUND_2 (E, A, B, C, D, P_SHA1_W (W, 41));
	P_SHA1_ROUND_2 (D, E, A, B, C, P_SHA1_W (W, 42));
	P_SHA1_ROUND_2 (C, D, E, A, B, P_SHA1_W (W, 43));
	P_SHA1_ROUND_2 (B, C, D, E, A, P_SHA1_W (W, 44));
	P_SHA1_ROUND_2 (A, B, C, D, E, P_SHA1_W (W, 45));
	P_SHA1_ROUND_2 (E, A, B, C, D, P_SHA1_W (W, 46));
	P_SHA1_ROUND_2 (D, E, A, B, C, P_SHA1_W (W, 47));
	P_SHA1_ROUND_2 (C, D, E, A, B, P_SHA1_W (W, 48));
	P_SHA1_ROUND_2 (B, C, D, E, A, P_SHA1_W (W, 49));
	P_SHA1_ROUND_2 (A, B, C, D, E, P_SHA1_W (W, 50));
	P_SHA1_ROUND_2 (E, A, B, C, D, P_SHA1_W (W, 51));
	P_SHA1_ROUND_2 (D, E, A, B, C, P_SHA1_W (W, 52));
	P_SHA1_ROUND_2 (C, D, E, A, B, P_SHA1_W (W, 53));
	P_SHA1_ROUND_2 (B, C, D, E, A, P_SHA1_W (W, 54));
	P_SHA1_ROUND_2 (A, B, C, D, E, P_SHA1_W (W, 55));
	P_SHA1_ROUND_2 (E, A, B, C, D, P_SHA1_W (W, 56));
	P_SHA1_ROUND_2 (D, E, A, B, C, P_SHA1_W (W, 57));
	P_SHA1_ROUND_2 (C, D, E, A, B, P_SHA1_W (W, 58));
	P_SHA1_ROUND_2 (B, C, D, E, A, P_SHA1_W (W, 59));

	P_SHA1_ROUND_3 (A, B, C, D, E, P_SHA1_W (W, 60));
	P_SHA1_ROUND_3 (E, A, B, C, D, P_SHA1_W (W, 61));
	P_SHA1_ROUND_3 (D, E, A, B, C, P_SHA1_W (W, 62));
	P_SHA1_ROUND_3 (C, D, E, A, B, P_SHA1_W (W, 63));
	P_SHA1_ROUND_3 (B, C, D, E, A, P_SHA1_W (W, 64));
	P_SHA1_ROUND_3 (A, B, C, D, E, P_SHA1_W (W, 65));
	P_SHA1_ROUND_3 (E, A, B, C, D, P_SHA1_W (W, 66));
	P_SHA1_ROUND_3 (D, E, A, B, C, P_SHA1_W (W, 67));
	P_SHA1_ROUND_3 (C, D, E, A, B, P_SHA1_W (W, 68));
	P_SHA1_ROUND_3 (B, C, D, E, A, P_SHA1_W (W, 69));
	P_SHA1_ROUND_3 (A, B, C, D, E, P_SHA1_W (W, 70));
	P_SHA1_ROUND_3 (E, A, B, C, D, P_SHA1_W (W, 71));
	P_SHA1_ROUND_3 (D, E, A, B, C, P_SHA1_W (W, 72));
	P_SHA1_ROUND_3 (C, D, E, A, B, P_SHA1_W (W, 73));
	P_SHA1_ROUND_3 (B, C, D, E, A, P_SHA1_W (W, 74));
	P_SHA1_ROUND_3 (A, B, C, D, E, P_SHA1_W (W, 75));
	P_SHA1_ROUND_3 (E, A, B, C, D, P_SHA1_W (W, 76));
	P_SHA1_ROUND_3 (D, E, A, B, C, P_SHA1_W (W, 77));
	P_SHA1_ROUND_3 (C, D, E, A, B, P_SHA1_W (W, 78));
	P_SHA1_ROUND_3 (B, C, D, E, A, P_SHA1_W (W, 79));

	ctx->hash[0] += A;
	ctx->hash[1] += B;
	ctx->hash[2] += C;
	ctx->hash[3] += D;
	ctx->hash[4] += E;
}

void
p_crypto_hash_sha1_reset (PHashSHA1 *ctx)
{
	memset (ctx->buf.buf, 0, 64);

	ctx->len_low = 0;
	ctx->len_high = 0;

	ctx->hash[0] = 0x67452301;
	ctx->hash[1] = 0xEFCDAB89;
	ctx->hash[2] = 0x98BADCFE;
	ctx->hash[3] = 0x10325476;
	ctx->hash[4] = 0xC3D2E1F0;
}

PHashSHA1 *
p_crypto_hash_sha1_new (void)
{
	PHashSHA1 *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PHashSHA1))) == NULL))
		return NULL;

	p_crypto_hash_sha1_reset (ret);

	return ret;
}

void
p_crypto_hash_sha1_update (PHashSHA1	*ctx,
			   const puchar	*data,
			   psize	len)
{
	puint32	left, to_fill;

	left = ctx->len_low & 0x3F;
	to_fill = 64 - left;

	ctx->len_low += (puint32) len;

	if (ctx->len_low < (puint32) len)
		++ctx->len_high;

	if (left && (puint32) len >= to_fill) {
		memcpy (ctx->buf.buf + left, data, to_fill);
		pp_crypto_hash_sha1_swap_bytes (ctx->buf.buf_w, 16);
		pp_crypto_hash_sha1_process (ctx, ctx->buf.buf_w);

		data += to_fill;
		len -= to_fill;
		left = 0;
	}

	while (len >= 64) {
		memcpy (ctx->buf.buf, data, 64);
		pp_crypto_hash_sha1_swap_bytes (ctx->buf.buf_w, 16);
		pp_crypto_hash_sha1_process (ctx, ctx->buf.buf_w);

		data += 64;
		len -= 64;
	}

	if (len > 0)
		memcpy (ctx->buf.buf + left, data, len);
}

void
p_crypto_hash_sha1_finish (PHashSHA1 *ctx)
{
	puint32	high, low;
	pint	left, last;

	left = ctx->len_low & 0x3F;
	last = (left < 56) ? (56 - left) : (120 - left);

	low = ctx->len_low << 3;
	high = ctx->len_high << 3
	     | ctx->len_low >> 29;

	if (last > 0)
		p_crypto_hash_sha1_update (ctx, pp_crypto_hash_sha1_pad, (psize) last);

	ctx->buf.buf_w[14] = high;
	ctx->buf.buf_w[15] = low;

	pp_crypto_hash_sha1_swap_bytes (ctx->buf.buf_w, 14);
	pp_crypto_hash_sha1_process (ctx, ctx->buf.buf_w);

	pp_crypto_hash_sha1_swap_bytes (ctx->hash, 5);
}

const puchar *
p_crypto_hash_sha1_digest (PHashSHA1 *ctx)
{
	return (const puchar *) ctx->hash;
}

void
p_crypto_hash_sha1_free (PHashSHA1 *ctx)
{
	p_free (ctx);
}
