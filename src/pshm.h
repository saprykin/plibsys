/* 
 * 24.08.2010
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PSHM_H__
#define __PSHM_H__

#include <pmacros.h>
#include <ptypes.h>

typedef enum _PShmAccessPerms {
	P_SHM_ACCESS_READONLY	= 0,
	P_SHM_ACCESS_READWRITE	= 1
} PShmAccessPerms;

typedef struct _PShm PShm;

P_BEGIN_DECLS

P_LIB_API PShm *	p_shm_new		(const pchar		*name,
						 psize			size,
						 PShmAccessPerms	perms);
P_LIB_API void		p_shm_free		(PShm *shm);
P_LIB_API pboolean	p_shm_lock		(PShm *shm);
P_LIB_API pboolean	p_shm_unlock		(PShm *shm);
P_LIB_API ppointer	p_shm_get_address	(PShm *shm);
P_LIB_API psize		p_shm_get_size		(PShm *shm);

P_END_DECLS

#endif /* __PSHM_H__ */

