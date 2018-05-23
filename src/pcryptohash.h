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

/**
 * @file pcryptohash.h
 * @brief Cryptographic hash function
 * @author Alexander Saprykin
 *
 * A cryptographic hash function is an algorithm which performs a transformation
 * of the income data to a hash value.
 *
 * One of the main requirements to all of the cryptographic hashing algorithms
 * is that any (even a considerably small) change in the input data must lead to
 * notable changes in the result hash value. It is the so called avalanche
 * effect. It helps to avoid collisions (the same hash value for different
 * input arrays).
 *
 * The cryptographic hash function is designed to be a one-way so you couldn't
 * revert the output hash value to the input data back. The length of the
 * resulting hash is a constant value depending on the algorithm being used.
 *
 * A cryptographic hash works with the incoming data using fixed length blocks
 * so it is possible to feed as many data as required.
 *
 * The cryptographic hash module supports the following hash functions:
 * - MD5;
 * - SHA-1;
 * - SHA-2/224;
 * - SHA-2/256;
 * - SHA-2/384;
 * - SHA-2/512;
 * - SHA-3/224;
 * - SHA-3/256;
 * - SHA-3/384;
 * - SHA-3/512;
 * - GOST (R 34.11-94).
 *
 * Use p_crypto_hash_new() to initialize a new hash context with one of the
 * mentioned above types. Data for hashing can be added in several chunks using
 * the p_crypto_hash_update() routine. You can add more chunks as long as the
 * hash context is open.
 *
 * The hash context becomes close in two cases: p_crypto_hash_get_string() or
 * p_crypto_hash_get_digest() was called. After that you can only get a hash in
 * a hexidemical string or in a raw representation.
 *
 * A hashing algorithm couldn't be changed after the context initialization.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PCRYPTOHASH_H
#define PLIBSYS_HEADER_PCRYPTOHASH_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Opaque data structure for handling a cryptographic hash context. */
typedef struct PCryptoHash_ PCryptoHash;

/** Cryptographic hash function types for #PCryptoHash. */
typedef enum PCryptoHashType_ {
	P_CRYPTO_HASH_TYPE_MD5		= 0, /**< MD5 hash function.			@since 0.0.1	*/
	P_CRYPTO_HASH_TYPE_SHA1		= 1, /**< SHA-1 hash function.			@since 0.0.1	*/
	P_CRYPTO_HASH_TYPE_SHA2_224	= 2, /**< SHA-2/224 hash function.		@since 0.0.2	*/
	P_CRYPTO_HASH_TYPE_SHA2_256	= 3, /**< SHA-2/256 hash function.		@since 0.0.2	*/
	P_CRYPTO_HASH_TYPE_SHA2_384	= 4, /**< SHA-2/384 hash function.		@since 0.0.2	*/
	P_CRYPTO_HASH_TYPE_SHA2_512	= 5, /**< SHA-2/512 hash function.		@since 0.0.2	*/
	P_CRYPTO_HASH_TYPE_SHA3_224	= 6, /**< SHA-2/224 hash function.		@since 0.0.2	*/
	P_CRYPTO_HASH_TYPE_SHA3_256	= 7, /**< SHA-2/256 hash function.		@since 0.0.2	*/
	P_CRYPTO_HASH_TYPE_SHA3_384	= 8, /**< SHA-2/384 hash function.		@since 0.0.2	*/
	P_CRYPTO_HASH_TYPE_SHA3_512	= 9, /**< SHA-3/512 hash function.		@since 0.0.2	*/
	P_CRYPTO_HASH_TYPE_GOST		= 10 /**< GOST (R 34.11-94) hash function.	@since 0.0.1	*/
} PCryptoHashType;

/**
 * @brief Initializes a new #PCryptoHash context.
 * @param type Hash function type to use, can't be changed later.
 * @return Newly initialized #PCryptoHash context in case of success, NULL
 * otherwise.
 * @since 0.0.1
 */
P_LIB_API PCryptoHash *		p_crypto_hash_new		(PCryptoHashType	type);

/**
 * @brief Adds a new chunk of data for hashing.
 * @param hash #PCryptoHash context to add @a data to.
 * @param data Data to add for hashing.
 * @param len Data length, in bytes.
 * @note After calling p_crypto_hash_get_string() or p_crypto_hash_get_digest()
 * the hash couldn't be updated anymore as it becomes close.
 * @since 0.0.1
 */
P_LIB_API void			p_crypto_hash_update		(PCryptoHash		*hash,
								 const puchar		*data,
								 psize			len);

/**
 * @brief Resets a hash state.
 * @param hash #PCryptoHash context to reset.
 * @since 0.0.1
 *
 * After a reset the hash context becomes open for updating, but all previously
 * added data will be lost. A hash function type couldn't be changed during or
 * after the resets.
 */
P_LIB_API void			p_crypto_hash_reset		(PCryptoHash		*hash);

/**
 * @brief Gets a hash in a hexidemical representation.
 * @param hash #PCryptoHash context to get a string from.
 * @return NULL-terminated string with the hexidemical representation of a hash
 * state in case of success, NULL otherwise. The string should be freed with
 * p_free() after using it.
 * @note Before returning the string the hash context will be closed for further
 * updates.
 * @since 0.0.1
 */
P_LIB_API pchar *		p_crypto_hash_get_string	(PCryptoHash		*hash);

/**
 * @brief Gets a hash in a raw representation.
 * @param hash #PCryptoHash context to get a digest from.
 * @param buf Buffer to store the digest with the hash raw representation.
 * @param[in,out] len Size of @a buf when calling, count of written bytes
 * after.
 * @note Before getting the raw digest the hash context will be closed for
 * further updates.
 * @since 0.0.1
 */
P_LIB_API void			p_crypto_hash_get_digest	(PCryptoHash		*hash,
								 puchar			*buf,
								 psize			*len);

/**
 * @brief Gets a hash digest length depending on its type.
 * @param hash #PCryptoHash context to get the length for.
 * @return Length (in bytes) of the given hash depending on its type in case of
 * success, -1 otherwise.
 * @note This length doesn't match a string hash representation.
 * @since 0.0.1
 */
P_LIB_API pssize		p_crypto_hash_get_length	(const PCryptoHash	*hash);

/**
 * @brief Gets a hash function type.
 * @param hash #PCryptoHash context to get the type for.
 * @return Hash function type used in the given context.
 * @since 0.0.1
 */
P_LIB_API PCryptoHashType	p_crypto_hash_get_type		(const PCryptoHash	*hash);

/**
 * @brief Frees a previously initialized hash context.
 * @param hash #PCryptoHash context to free.
 * @since 0.0.1
 */
P_LIB_API void			p_crypto_hash_free		(PCryptoHash		*hash);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PCRYPTOHASH_H */
