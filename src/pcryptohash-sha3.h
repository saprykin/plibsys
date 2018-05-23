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
