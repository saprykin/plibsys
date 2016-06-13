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

/* GOST R 34.11-94 interface implementation for #PCryptoHash */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PCRYPTOHASHGOST3411_H
#define PLIBSYS_HEADER_PCRYPTOHASHGOST3411_H

#include "ptypes.h"
#include "pmacros.h"

P_BEGIN_DECLS

typedef struct PHashGOST3411_ PHashGOST3411;

PHashGOST3411 *	p_crypto_hash_gost3411_new	(void);
void		p_crypto_hash_gost3411_update	(PHashGOST3411		*ctx,
						 const puchar		*data,
						 psize			len);
void		p_crypto_hash_gost3411_finish	(PHashGOST3411		*ctx);
const puchar *	p_crypto_hash_gost3411_digest	(PHashGOST3411		*ctx);
void		p_crypto_hash_gost3411_reset	(PHashGOST3411		*ctx);
void		p_crypto_hash_gost3411_free	(PHashGOST3411		*ctx);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PCRYPTOHASHGOST3411_H */
