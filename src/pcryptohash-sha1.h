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

/* SHA1 interface implementation for #PCryptoHash */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PCRYPTOHASHSHA1_H
#define PLIBSYS_HEADER_PCRYPTOHASHSHA1_H

#include "ptypes.h"
#include "pmacros.h"

P_BEGIN_DECLS

typedef struct PHashSHA1_ PHashSHA1;

PHashSHA1 *	p_crypto_hash_sha1_new		(void);
void		p_crypto_hash_sha1_update	(PHashSHA1 *ctx, const puchar *data, psize len);
void		p_crypto_hash_sha1_finish	(PHashSHA1 *ctx);
const puchar *	p_crypto_hash_sha1_digest	(PHashSHA1 *ctx);
void		p_crypto_hash_sha1_reset	(PHashSHA1 *ctx);
void		p_crypto_hash_sha1_free		(PHashSHA1 *ctx);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PCRYPTOHASHSHA1_H */
