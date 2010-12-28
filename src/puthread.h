/* 
 * 15.09.2010
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

#ifndef __PUTHREAD_H__
#define __PUTHREAD_H__

#include <pmacros.h>
#include <ptypes.h>

typedef ppointer (*PUThreadFunc) (ppointer arg);

typedef struct _PUThread PUThread;

P_BEGIN_DECLS

P_LIB_API PUThread *		p_uthread_create	(PUThreadFunc	func,
							 ppointer	data,
							 pboolean	joinable);
P_LIB_API P_NO_RETURN void	p_uthread_exit		(pint		code);
P_LIB_API pint			p_uthread_join		(PUThread	*thread);
P_LIB_API void			p_uthread_free		(PUThread	*thread);
P_LIB_API pint			p_uthread_sleep		(puint32	msec);

P_END_DECLS

#endif /* __PUTHREAD_H__ */

