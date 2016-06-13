/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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
