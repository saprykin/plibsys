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
#include "pcryptohash.h"
#include "pcryptohash-gost3411.h"
#include "pcryptohash-md5.h"
#include "pcryptohash-sha1.h"
#include "pcryptohash-sha2-256.h"
#include "pcryptohash-sha2-512.h"
#include "pcryptohash-sha3.h"

#include <string.h>

#define P_HASH_FUNCS(ctx, type) \
	ctx->create = (void * (*) (void)) p_crypto_hash_##type##_new;				\
	ctx->update = (void (*) (void *, const puchar *, psize)) p_crypto_hash_##type##_update;	\
	ctx->finish = (void (*) (void *)) p_crypto_hash_##type##_finish;			\
	ctx->digest = (const puchar * (*) (void *)) p_crypto_hash_##type##_digest;		\
	ctx->reset = (void (*) (void *)) p_crypto_hash_##type##_reset;				\
	ctx->free = (void (*) (void *)) p_crypto_hash_##type##_free;

struct PCryptoHash_ {
	PCryptoHashType	type;
	ppointer	context;
	puint		hash_len;
	pboolean	closed;
	ppointer	(*create)	(void);
	void		(*update)	(void *hash, const puchar *data, psize len);
	void		(*finish)	(void *hash);
	const puchar *	(*digest)	(void *hash);
	void		(*reset)	(void *hash);
	void		(*free)		(void *hash);
};

static pchar pp_crypto_hash_hex_str[]= "0123456789abcdef";

static void
pp_crypto_hash_digest_to_hex (const puchar *digest, puint len, pchar *out);

static void
pp_crypto_hash_digest_to_hex (const puchar *digest, puint len, pchar *out)
{
	puint i;

	for (i = 0; i < len; ++i) {
		*(out + (i << 1)    ) = pp_crypto_hash_hex_str[(digest[i] >> 4) & 0x0F];
		*(out + (i << 1) + 1) = pp_crypto_hash_hex_str[(digest[i]     ) & 0x0F];
	}
}

P_LIB_API PCryptoHash *
p_crypto_hash_new (PCryptoHashType type)
{
	PCryptoHash *ret;

	if (P_UNLIKELY (!(type >= P_CRYPTO_HASH_TYPE_MD5 && type <= P_CRYPTO_HASH_TYPE_GOST)))
		return NULL;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PCryptoHash))) == NULL)) {
		P_ERROR ("PCryptoHash::p_crypto_hash_new: failed to allocate memory");
		return NULL;
	}

	switch (type) {
	case P_CRYPTO_HASH_TYPE_MD5:
		P_HASH_FUNCS (ret, md5);
		ret->hash_len = 16;
		break;
	case P_CRYPTO_HASH_TYPE_SHA1:
		P_HASH_FUNCS (ret, sha1);
		ret->hash_len = 20;
		break;
	case P_CRYPTO_HASH_TYPE_SHA2_224:
		P_HASH_FUNCS (ret, sha2_224);
		ret->hash_len = 28;
		break;
	case P_CRYPTO_HASH_TYPE_SHA2_256:
		P_HASH_FUNCS (ret, sha2_256);
		ret->hash_len = 32;
		break;
	case P_CRYPTO_HASH_TYPE_SHA2_384:
		P_HASH_FUNCS (ret, sha2_384);
		ret->hash_len = 48;
		break;
	case P_CRYPTO_HASH_TYPE_SHA2_512:
		P_HASH_FUNCS (ret, sha2_512);
		ret->hash_len = 64;
		break;
	case P_CRYPTO_HASH_TYPE_SHA3_224:
		P_HASH_FUNCS (ret, sha3_224);
		ret->hash_len = 28;
		break;
	case P_CRYPTO_HASH_TYPE_SHA3_256:
		P_HASH_FUNCS (ret, sha3_256);
		ret->hash_len = 32;
		break;
	case P_CRYPTO_HASH_TYPE_SHA3_384:
		P_HASH_FUNCS (ret, sha3_384);
		ret->hash_len = 48;
		break;
	case P_CRYPTO_HASH_TYPE_SHA3_512:
		P_HASH_FUNCS (ret, sha3_512);
		ret->hash_len = 64;
		break;
	case P_CRYPTO_HASH_TYPE_GOST:
		P_HASH_FUNCS (ret, gost3411);
		ret->hash_len = 32;
		break;
	}

	ret->type   = type;
	ret->closed = FALSE;

	if (P_UNLIKELY ((ret->context = ret->create ()) == NULL)) {
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API void
p_crypto_hash_update (PCryptoHash *hash, const puchar *data, psize len)
{
	if (P_UNLIKELY (hash == NULL || data == NULL || len == 0))
		return;

	if (P_UNLIKELY (hash->closed))
		return;

	hash->update (hash->context, data, len);
}

P_LIB_API void
p_crypto_hash_reset (PCryptoHash *hash)
{
	if (P_UNLIKELY (hash == NULL))
		return;

	hash->reset (hash->context);
	hash->closed = FALSE;
}

P_LIB_API pchar *
p_crypto_hash_get_string (PCryptoHash *hash)
{
	pchar		*ret;
	const puchar	*digest;

	if (P_UNLIKELY (hash == NULL))
		return NULL;

	if (!hash->closed) {
		hash->finish (hash->context);
		hash->closed = TRUE;
	}

	if (P_UNLIKELY ((digest = hash->digest (hash->context)) == NULL))
		return NULL;

	if (P_UNLIKELY ((ret = p_malloc0 (hash->hash_len * 2 + 1)) == NULL))
		return NULL;

	pp_crypto_hash_digest_to_hex (digest, hash->hash_len, ret);

	return ret;
}

P_LIB_API void
p_crypto_hash_get_digest (PCryptoHash *hash, puchar *buf, psize *len)
{
	const puchar *digest;

	if (P_UNLIKELY (len == NULL))
		return;

	if (P_UNLIKELY (hash == NULL || buf == NULL)) {
		*len = 0;
		return;
	}

	if (P_UNLIKELY (hash->hash_len > *len)) {
		*len = 0;
		return;
	}

	if (!hash->closed) {
		hash->finish (hash->context);
		hash->closed = TRUE;
	}

	if (P_UNLIKELY ((digest = hash->digest (hash->context)) == NULL)) {
		*len = 0;
		return;
	}

	memcpy (buf, digest, hash->hash_len);
	*len = hash->hash_len;
}

P_LIB_API pssize
p_crypto_hash_get_length (const PCryptoHash *hash)
{
	if (P_UNLIKELY (hash == NULL))
		return 0;

	return hash->hash_len;
}

P_LIB_API PCryptoHashType
p_crypto_hash_get_type (const PCryptoHash *hash)
{
	if (P_UNLIKELY (hash == NULL))
		return (PCryptoHashType) -1;

	return hash->type;
}

P_LIB_API void
p_crypto_hash_free (PCryptoHash *hash)
{
	if (P_UNLIKELY (hash == NULL))
		return;

	hash->free (hash->context);
	p_free (hash);
}
