/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/* SHA-3 (Keccak) interface implementation for #PCryptoHash */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PCRYPTOHASHSHA3_H
#define PLIBSYS_HEADER_PCRYPTOHASHSHA3_H

#include "ptypes.h"
#include "pmacros.h"

P_BEGIN_DECLS

typedef struct PHashSHA3_ PHashSHA3;

void		p_crypto_hash_sha3_update	(PHashSHA3 *ctx, const puchar *data, psize len);
void		p_crypto_hash_sha3_finish	(PHashSHA3 *ctx);
const puchar *	p_crypto_hash_sha3_digest	(PHashSHA3 *ctx);
void		p_crypto_hash_sha3_reset	(PHashSHA3 *ctx);
void		p_crypto_hash_sha3_free		(PHashSHA3 *ctx);

PHashSHA3 *	p_crypto_hash_sha3_224_new	(void);
PHashSHA3 *	p_crypto_hash_sha3_256_new	(void);
PHashSHA3 *	p_crypto_hash_sha3_384_new	(void);
PHashSHA3 *	p_crypto_hash_sha3_512_new	(void);

#define p_crypto_hash_sha3_224_update p_crypto_hash_sha3_update
#define p_crypto_hash_sha3_224_finish p_crypto_hash_sha3_finish
#define p_crypto_hash_sha3_224_digest p_crypto_hash_sha3_digest
#define p_crypto_hash_sha3_224_reset  p_crypto_hash_sha3_reset
#define p_crypto_hash_sha3_224_free   p_crypto_hash_sha3_free

#define p_crypto_hash_sha3_256_update p_crypto_hash_sha3_update
#define p_crypto_hash_sha3_256_finish p_crypto_hash_sha3_finish
#define p_crypto_hash_sha3_256_digest p_crypto_hash_sha3_digest
#define p_crypto_hash_sha3_256_reset  p_crypto_hash_sha3_reset
#define p_crypto_hash_sha3_256_free   p_crypto_hash_sha3_free

#define p_crypto_hash_sha3_384_update p_crypto_hash_sha3_update
#define p_crypto_hash_sha3_384_finish p_crypto_hash_sha3_finish
#define p_crypto_hash_sha3_384_digest p_crypto_hash_sha3_digest
#define p_crypto_hash_sha3_384_reset  p_crypto_hash_sha3_reset
#define p_crypto_hash_sha3_384_free   p_crypto_hash_sha3_free

#define p_crypto_hash_sha3_512_update p_crypto_hash_sha3_update
#define p_crypto_hash_sha3_512_finish p_crypto_hash_sha3_finish
#define p_crypto_hash_sha3_512_digest p_crypto_hash_sha3_digest
#define p_crypto_hash_sha3_512_reset  p_crypto_hash_sha3_reset
#define p_crypto_hash_sha3_512_free   p_crypto_hash_sha3_free

P_END_DECLS

#endif /* PLIBSYS_HEADER_PCRYPTOHASHSHA3_H */
