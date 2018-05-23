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

#include "pmem.h"
#include "pcryptohash-md5.h"

#include <string.h>
#include <stdlib.h>

struct PHashMD5_ {
	union buf_ {
		puchar	buf[64];
		puint32	buf_w[16];
	} buf;
	puint32		hash[4];

	puint32		len_high;
	puint32		len_low;
};

static const puchar pp_crypto_hash_md5_pad[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void pp_crypto_hash_md5_swap_bytes (puint32 *data, puint words);
static void pp_crypto_hash_md5_process (PHashMD5 *ctx, const puint32 data[16]);

#define P_MD5_ROTL(val, shift) ((val) << (shift) |  (val) >> (32 - (shift)))

#define P_MD5_F(x, y, z) (z ^ (x & (y ^ z)))
#define P_MD5_G(x, y, z) P_MD5_F (z, x, y)
#define P_MD5_H(x, y, z) (x ^ y ^ z)
#define P_MD5_I(x, y, z) (y ^ (x | (~z)))

#define P_MD5_ROUND_0(a, b, c, d, k, i, s)				\
	a += P_MD5_F (b, c, d) + data[k] + i, a = P_MD5_ROTL (a, s) + b

#define P_MD5_ROUND_1(a, b, c, d, k, i, s)				\
	a += P_MD5_G (b, c, d) + data[k] + i, a = P_MD5_ROTL (a, s) + b


#define P_MD5_ROUND_2(a, b, c, d, k, i, s)				\
	a += P_MD5_H (b, c, d) + data[k] + i, a = P_MD5_ROTL (a, s) + b

#define P_MD5_ROUND_3(a, b, c, d, k, i, s)				\
	a += P_MD5_I (b, c, d) + data[k] + i, a = P_MD5_ROTL (a, s) + b

static void
pp_crypto_hash_md5_swap_bytes (puint32	*data,
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

static void
pp_crypto_hash_md5_process (PHashMD5		*ctx,
			    const puint32	data[16])
{
	puint32	A, B, C, D;

	A = ctx->hash[0];
	B = ctx->hash[1];
	C = ctx->hash[2];
	D = ctx->hash[3];

	P_MD5_ROUND_0 (A, B, C, D, 0,  0xD76AA478,  7);
	P_MD5_ROUND_0 (D, A, B, C, 1,  0xE8C7B756, 12);
	P_MD5_ROUND_0 (C, D, A, B, 2,  0x242070DB, 17);
	P_MD5_ROUND_0 (B, C, D, A, 3,  0xC1BDCEEE, 22);
	P_MD5_ROUND_0 (A, B, C, D, 4,  0xF57C0FAF,  7);
	P_MD5_ROUND_0 (D, A, B, C, 5,  0x4787C62A, 12);
	P_MD5_ROUND_0 (C, D, A, B, 6,  0xA8304613, 17);
	P_MD5_ROUND_0 (B, C, D, A, 7,  0xFD469501, 22);
	P_MD5_ROUND_0 (A, B, C, D, 8,  0x698098D8,  7);
	P_MD5_ROUND_0 (D, A, B, C, 9,  0x8B44F7AF, 12);
	P_MD5_ROUND_0 (C, D, A, B, 10, 0xFFFF5BB1, 17);
	P_MD5_ROUND_0 (B, C, D, A, 11, 0x895CD7BE, 22);
	P_MD5_ROUND_0 (A, B, C, D, 12, 0x6B901122,  7);
	P_MD5_ROUND_0 (D, A, B, C, 13, 0xFD987193, 12);
	P_MD5_ROUND_0 (C, D, A, B, 14, 0xA679438E, 17);
	P_MD5_ROUND_0 (B, C, D, A, 15, 0x49B40821, 22);

	P_MD5_ROUND_1 (A, B, C, D, 1,  0xF61E2562,  5);
	P_MD5_ROUND_1 (D, A, B, C, 6,  0xC040B340,  9);
	P_MD5_ROUND_1 (C, D, A, B, 11, 0x265E5A51, 14);
	P_MD5_ROUND_1 (B, C, D, A, 0,  0xE9B6C7AA, 20);
	P_MD5_ROUND_1 (A, B, C, D, 5,  0xD62F105D,  5);
	P_MD5_ROUND_1 (D, A, B, C, 10, 0x02441453,  9);
	P_MD5_ROUND_1 (C, D, A, B, 15, 0xD8A1E681, 14);
	P_MD5_ROUND_1 (B, C, D, A, 4,  0xE7D3FBC8, 20);
	P_MD5_ROUND_1 (A, B, C, D, 9,  0x21E1CDE6,  5);
	P_MD5_ROUND_1 (D, A, B, C, 14, 0xC33707D6,  9);
	P_MD5_ROUND_1 (C, D, A, B, 3,  0xF4D50D87, 14);
	P_MD5_ROUND_1 (B, C, D, A, 8,  0x455A14ED, 20);
	P_MD5_ROUND_1 (A, B, C, D, 13, 0xA9E3E905,  5);
	P_MD5_ROUND_1 (D, A, B, C, 2,  0xFCEFA3F8,  9);
	P_MD5_ROUND_1 (C, D, A, B, 7,  0x676F02D9, 14);
	P_MD5_ROUND_1 (B, C, D, A, 12, 0x8D2A4C8A, 20);

	P_MD5_ROUND_2 (A, B, C, D, 5,  0xFFFA3942,  4);
	P_MD5_ROUND_2 (D, A, B, C, 8,  0x8771F681, 11);
	P_MD5_ROUND_2 (C, D, A, B, 11, 0x6D9D6122, 16);
	P_MD5_ROUND_2 (B, C, D, A, 14, 0xFDE5380C, 23);
	P_MD5_ROUND_2 (A, B, C, D, 1,  0xA4BEEA44,  4);
	P_MD5_ROUND_2 (D, A, B, C, 4,  0x4BDECFA9, 11);
	P_MD5_ROUND_2 (C, D, A, B, 7,  0xF6BB4B60, 16);
	P_MD5_ROUND_2 (B, C, D, A, 10, 0xBEBFBC70, 23);
	P_MD5_ROUND_2 (A, B, C, D, 13, 0x289B7EC6,  4);
	P_MD5_ROUND_2 (D, A, B, C, 0,  0xEAA127FA, 11);
	P_MD5_ROUND_2 (C, D, A, B, 3,  0xD4EF3085, 16);
	P_MD5_ROUND_2 (B, C, D, A, 6,  0x04881D05, 23);
	P_MD5_ROUND_2 (A, B, C, D, 9,  0xD9D4D039,  4);
	P_MD5_ROUND_2 (D, A, B, C, 12, 0xE6DB99E5, 11);
	P_MD5_ROUND_2 (C, D, A, B, 15, 0x1FA27CF8, 16);
	P_MD5_ROUND_2 (B, C, D, A, 2,  0xC4AC5665, 23);

	P_MD5_ROUND_3 (A, B, C, D, 0,  0xF4292244,  6);
	P_MD5_ROUND_3 (D, A, B, C, 7,  0x432AFF97, 10);
	P_MD5_ROUND_3 (C, D, A, B, 14, 0xAB9423A7, 15);
	P_MD5_ROUND_3 (B, C, D, A, 5,  0xFC93A039, 21);
	P_MD5_ROUND_3 (A, B, C, D, 12, 0x655B59C3,  6);
	P_MD5_ROUND_3 (D, A, B, C, 3,  0x8F0CCC92, 10);
	P_MD5_ROUND_3 (C, D, A, B, 10, 0xFFEFF47D, 15);
	P_MD5_ROUND_3 (B, C, D, A, 1,  0x85845DD1, 21);
	P_MD5_ROUND_3 (A, B, C, D, 8,  0x6FA87E4F,  6);
	P_MD5_ROUND_3 (D, A, B, C, 15, 0xFE2CE6E0, 10);
	P_MD5_ROUND_3 (C, D, A, B, 6,  0xA3014314, 15);
	P_MD5_ROUND_3 (B, C, D, A, 13, 0x4E0811A1, 21);
	P_MD5_ROUND_3 (A, B, C, D, 4,  0xF7537E82,  6);
	P_MD5_ROUND_3 (D, A, B, C, 11, 0xBD3AF235, 10);
	P_MD5_ROUND_3 (C, D, A, B, 2,  0x2AD7D2BB, 15);
	P_MD5_ROUND_3 (B, C, D, A, 9,  0xEB86D391, 21);

	ctx->hash[0] += A;
	ctx->hash[1] += B;
	ctx->hash[2] += C;
	ctx->hash[3] += D;
}

void
p_crypto_hash_md5_reset (PHashMD5 *ctx)
{
	memset (ctx->buf.buf, 0, 64);

	ctx->len_low = 0;
	ctx->len_high = 0;

	ctx->hash[0] = 0x67452301;
	ctx->hash[1] = 0xEFCDAB89;
	ctx->hash[2] = 0x98BADCFE;
	ctx->hash[3] = 0x10325476;
}

PHashMD5 *
p_crypto_hash_md5_new (void)
{
	PHashMD5 *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PHashMD5))) == NULL))
		return NULL;

	p_crypto_hash_md5_reset (ret);

	return ret;
}

void
p_crypto_hash_md5_update (PHashMD5	*ctx,
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
		pp_crypto_hash_md5_swap_bytes (ctx->buf.buf_w, 16);
		pp_crypto_hash_md5_process (ctx, ctx->buf.buf_w);

		data += to_fill;
		len -= to_fill;
		left = 0;
	}

	while (len >= 64) {
		memcpy (ctx->buf.buf, data, 64);
		pp_crypto_hash_md5_swap_bytes (ctx->buf.buf_w, 16);
		pp_crypto_hash_md5_process (ctx, ctx->buf.buf_w);

		data += 64;
		len -= 64;
	}

	if (len > 0)
		memcpy (ctx->buf.buf + left, data, len);
}

void
p_crypto_hash_md5_finish (PHashMD5 *ctx)
{
	puint32	high, low;
	pint	left, last;

	left = ctx->len_low & 0x3F;
	last = (left < 56) ? (56 - left) : (120 - left);

	low = ctx->len_low << 3;
	high = ctx->len_high << 3
	     | ctx->len_low >> 29;

	if (last > 0)
		p_crypto_hash_md5_update (ctx, pp_crypto_hash_md5_pad, (psize) last);

	ctx->buf.buf_w[14] = low;
	ctx->buf.buf_w[15] = high;

	pp_crypto_hash_md5_swap_bytes (ctx->buf.buf_w, 14);
	pp_crypto_hash_md5_process (ctx, ctx->buf.buf_w);

	pp_crypto_hash_md5_swap_bytes (ctx->hash, 4);
}

const puchar *
p_crypto_hash_md5_digest (PHashMD5 *ctx)
{
	return (const puchar *) ctx->hash;
}

void
p_crypto_hash_md5_free (PHashMD5 *ctx)
{
	p_free (ctx);
}
