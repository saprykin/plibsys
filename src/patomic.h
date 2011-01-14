/* 
 * 30.12.2010
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

/*
 * This file was taken from GLib and adopted for using along with PLib.
 * See gatomic.h for other copyrights and details.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __P_ATOMIC_H__
#define __P_ATOMIC_H__

#include <ptypes.h>
#include <pmacros.h>

P_BEGIN_DECLS

pint		p_atomic_int_exchange_and_add		(volatile pint		*atomic,
							 pint			val);
void		p_atomic_int_add			(volatile pint		*atomic,
							 pint			val);
pboolean	p_atomic_int_compare_and_exchange	(volatile pint		*atomic,
							 pint			oldval,
							 pint			newval);
pboolean	p_atomic_pointer_compare_and_exchange	(volatile ppointer	*atomic, 
							 ppointer 		oldval, 
							 ppointer		newval);
pint		p_atomic_int_get			(volatile pint		*atomic);
void		p_atomic_int_set			(volatile pint		*atomic,
							 pint			newval);
ppointer	p_atomic_pointer_get			(volatile ppointer	*atomic);
void		p_atomic_pointer_set			(volatile ppointer	*atomic,
							 ppointer		newval);
void		p_atomic_memory_barrier			(void);

#ifndef P_ATOMIC_OP_MEMORY_BARRIER_NEEDED
#  define p_atomic_int_get(atomic) 		((pint)*(atomic))
#  define p_atomic_int_set(atomic, newval) 	((void) (*(atomic) = (newval)))
#  define p_atomic_pointer_get(atomic) 		((ppointer)*(atomic))
#  define p_atomic_pointer_set(atomic, newval)	((void) (*(atomic) = (newval)))
#else
#  define p_atomic_int_get(atomic) \
   ((void) sizeof (pchar [sizeof (*(atomic)) == sizeof (pint) ? 1 : -1]), \
   (p_atomic_int_get) ((volatile pint *) (void *) (atomic)))
#  define p_atomic_int_set(atomic, newval) \
   ((void) sizeof (pchar [sizeof (*(atomic)) == sizeof (pint) ? 1 : -1]), \
   (p_atomic_int_set) ((volatile pint *) (void *) (atomic), (newval)))
#  define p_atomic_pointer_get(atomic) \
   ((void) sizeof (pchar [sizeof (*(atomic)) == sizeof (ppointer) ? 1 : -1]), \
   (p_atomic_pointer_get) ((volatile ppointer *) (void *) (atomic)))
#  define p_atomic_pointer_set(atomic, newval) \
   ((void) sizeof (pchar [sizeof (*(atomic)) == sizeof (ppointer) ? 1 : -1]), \
   (p_atomic_pointer_set) ((volatile ppointer *) (void *) (atomic), (newval)))
#endif /* P_ATOMIC_OP_MEMORY_BARRIER_NEEDED */

#define p_atomic_int_inc(atomic) (p_atomic_int_add ((atomic), 1))
#define p_atomic_int_dec_and_test(atomic)				\
  (p_atomic_int_exchange_and_add ((atomic), -1) == 1)

P_END_DECLS

#endif /* __P_ATOMIC_H__ */

