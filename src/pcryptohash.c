/* 
 * Copyright (C) 2010-2013 Alexander Saprykin <xelfium@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

/* TODO: Handle NULL-terminated strings */
/* TODO: Proper hex converting */

#include "pmem.h"
#include "pcryptohash.h"
#include "pgost3411.h"
#include "pmd5.h"
#include "psha1.h"

#include <stdio.h>
#include <string.h>

/* Keep in sync with hash algorithms */
#define P_MAX_HASH_LENGTH	32

#define P_HASH_FUNCS(ctx, type) \
	ctx->new = (void * (*) (void)) p_##type##_new;					\
	ctx->update = (void (*) (void *, const puchar *, psize)) p_##type##_update;	\
	ctx->finish = (void (*) (void *)) p_##type##_finish;				\
	ctx->digest = (const puchar * (*) (void *)) p_##type##_digest;			\
	ctx->reset = (void (*) (void *)) p_##type##_reset;				\
	ctx->free = (void (*) (void *)) p_##type##_free;

struct _PCryptoHash {
	PCryptoHashType	type;
	void		*context;
	puint		hash_len;
	pboolean	closed;
	pboolean	reseted;
	void *		(*new)		(void);
	void		(*update)	(void *hash, const puchar *data, psize len);
	void		(*finish)	(void *hash);
	const puchar *	(*digest)	(void *hash);
	void		(*reset)	(void *hash);
	void		(*free)		(void *hash);
};

P_LIB_API PCryptoHash *
p_crypto_hash_new (PCryptoHashType type)
{
	PCryptoHash *ret;

	if ((ret = p_malloc0 (sizeof (PCryptoHash))) == NULL)
		return NULL;

	switch (type) {
	case P_CRYPTO_HASH_TYPE_MD5:
		P_HASH_FUNCS (ret, md5);
		ret->hash_len = 16;
		break;
	case P_CRYPTO_HASH_TYPE_SHA1:
		P_HASH_FUNCS (ret, sha1);
		ret->hash_len = 20;
		break;
	case P_CRYPTO_HASH_TYPE_GOST:
		P_HASH_FUNCS (ret, gost3411);
		ret->hash_len = 32;
		break;
	default:
		p_free (ret);
		return NULL;
	}

	ret->type = type;
	ret->closed = FALSE;
	ret->reseted = TRUE;

	if ((ret->context = ret->new ()) == NULL) {
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API void
p_crypto_hash_update (PCryptoHash *hash, const puchar *data, psize len)
{
	if (hash == NULL || data == NULL || len == 0)
		return;

	if (hash->closed)
		return;

	hash->update (hash->context, data, len);

	hash->reseted = FALSE;
}

P_LIB_API void
p_crypto_hash_reset (PCryptoHash *hash)
{
	if (hash == NULL)
		return;

	hash->reset (hash->context);
	hash->closed = FALSE;
	hash->reseted = TRUE;
}

P_LIB_API pchar *
p_crypto_hash_get_string (PCryptoHash *hash)
{
	pchar		*ret;
	puint		i;
	const puchar	*digest;

	if (hash == NULL || hash->reseted)
		return NULL;

	if (!hash->closed) {
		hash->finish (hash->context);
		hash->closed = TRUE;
	}

	if ((digest = hash->digest (hash->context)) == NULL)
		return NULL;

	if ((ret = p_malloc0 (hash->hash_len * 2 + 1)) == NULL)
		return NULL;

	for (i = 0; i < hash->hash_len; ++i)
		sprintf (ret + i * 2, "%02x", digest[i]);

	return ret;
}

P_LIB_API void
p_crypto_hash_get_digest (PCryptoHash *hash, puchar *buf, psize *len)
{
	const puchar *digest;

	if (len == NULL)
		return;

	if (hash == NULL || buf == NULL) {
		*len = 0;
		return;
	}

	if (hash->reseted) {
		*len = 0;
		return;
	}

	if (hash->hash_len < *len) {
		*len = 0;
		return;
	}

	if (!hash->closed) {
		hash->finish (hash->context);
		hash->closed = TRUE;
	}

	if ((digest = hash->digest (hash->context)) == NULL) {
		*len = 0;
		return;
	}

	memcpy (buf, digest, hash->hash_len);
	*len = hash->hash_len;
}

P_LIB_API pssize
p_crypto_hash_get_length (PCryptoHash *hash)
{
	if (hash == NULL)
		return -1;

	return hash->hash_len;
}

P_LIB_API PCryptoHashType
p_crypto_hash_get_type (PCryptoHash *hash)
{
	if (hash == NULL)
		return -1;

	return hash->type;
}

P_LIB_API void
p_crypto_hash_free (PCryptoHash *hash)
{
	if (hash == NULL)
		return;

	hash->free (hash->context);
	p_free (hash);
}

