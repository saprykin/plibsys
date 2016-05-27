/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file pcryptohash.h
 * @brief Cryptographic hash function
 * @author Alexander Saprykin
 *
 * Cryptographic hash function is an algorithm which performs a transformation
 * of the income data to the hash value.
 *
 * One of the main requirements to all of the cryptographic hashing algorithms
 * is that any (even considerably small) change in the input data must lead to
 * notable changes in the result hash value. It is the so called avalanche
 * effect. It helps to avoid collisions (the same hash value for different
 * input arrays).
 *
 * Cryptographic hash function is designed to be one-way so you couldn't revert
 * the output hash value to the input data back. The length of resulting hash is
 * a constant value depending on the algorithm used.
 *
 * Cryptographic hash works with incoming data using fixed length blocks so it
 * is possible to feed as many data as required.
 *
 * Cryptographic hash module supports the following hash functions:
 * - MD5;
 * - SHA-1;
 * - GOST (R 34.11-94).
 *
 * Use p_crypto_hash_new() to initialize a new hash context with one of the any
 * mentioned above types. Data for hashing can be added in several chunks using
 * p_crypto_hash_update() routine. You can add more chunks as long as a hash
 * context is opened.
 *
 * Hash context becomes closed in two cases: p_crypto_hash_get_string() or
 * p_crypto_hash_get_digest() was called. After that you can only get hash in
 * hexidemical string or in a raw representation.
 *
 * Hashing algorithm couldn't be changed after the context initialization.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PCRYPTOHASH_H
#define PLIBSYS_HEADER_PCRYPTOHASH_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Opaque data structure for handling cryptographic hash context. */
typedef struct PCryptoHash_ PCryptoHash;

/** Cryptographic hash function types for #PCryptoHash. */
typedef enum PCryptoHashType_ {
	P_CRYPTO_HASH_TYPE_MD5	= 0, /**< MD5 hash function.			*/
	P_CRYPTO_HASH_TYPE_SHA1	= 1, /**< SHA-1 hash function.			*/
	P_CRYPTO_HASH_TYPE_GOST	= 2  /**< GOST (R 34.11-94) hash function.	*/
} PCryptoHashType;

/**
 * @brief Initializes new #PCryptoHash context.
 * @param type Hash function type to use, can't be changed later.
 * @return Newly initialized #PCryptoHash context in case of success,
 * NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PCryptoHash *		p_crypto_hash_new		(PCryptoHashType	type);

/**
 * @brief Adds new chunk of data for hashing.
 * @param hash #PCryptoHash context to add @a data to.
 * @param data Data to add for hashing.
 * @param len Data length, in bytes.
 * @note After calling p_crypto_hash_get_string() or p_crypto_hash_get_digest()
 * hash couldn't be updated anymore as it becomes closed.
 * @since 0.0.1
 */
P_LIB_API void			p_crypto_hash_update		(PCryptoHash		*hash,
								 const puchar		*data,
								 psize			len);

/**
 * @brief Resets hash state.
 * @param hash #PCryptoHash context to reset.
 * @since 0.0.1
 *
 * After reset hash context becomes opened for updating, but all previously
 * added data will be lost. Hash function type couldn't be changed during or
 * after resets.
 */
P_LIB_API void			p_crypto_hash_reset		(PCryptoHash		*hash);

/**
 * @brief Gets hash in a hexidemical representation.
 * @param hash #PCryptoHash context to get string from.
 * @return NULL-terminated string with a hexidemical representation of the hash
 * state in case of success, NULL otherwise. String should be freed with
 * p_free() after using it.
 * @note Before returning the string hash context will be closed for further
 * updates.
 * @since 0.0.1
 */
P_LIB_API pchar *		p_crypto_hash_get_string	(PCryptoHash		*hash);

/**
 * @brief Gets hash in a raw representation.
 * @param hash #PCryptoHash context to get digest from.
 * @param buf Buffer to store digest with a hash raw representation.
 * @param[in, out] len Size of @a buf when calling, count of written bytes after.
 * @note Before getting raw digest hash context will be closed for further
 * updates.
 * @since 0.0.1
 */
P_LIB_API void			p_crypto_hash_get_digest	(PCryptoHash		*hash,
								 puchar			*buf,
								 psize			*len);

/**
 * @brief Gets hash digest length depending on its type.
 * @param hash #PCryptoHash context to get length for.
 * @return Length (in bytes) of the given hash depending on its type in
 * case of success, -1 otherwise.
 * @note This length doesn't match a string hash representation.
 * @since 0.0.1
 */
P_LIB_API pssize		p_crypto_hash_get_length	(const PCryptoHash	*hash);

/**
 * @brief Gets hash function type.
 * @param hash #PCryptoHash context to get type for.
 * @return Hash function type used in the given context.
 * @since 0.0.1
 */
P_LIB_API PCryptoHashType	p_crypto_hash_get_type		(const PCryptoHash	*hash);

/**
 * @brief Frees previously initialized hash context.
 * @param hash #PCryptoHash context to free.
 * @since 0.0.1
 */
P_LIB_API void			p_crypto_hash_free		(PCryptoHash		*hash);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PCRYPTOHASH_H */
