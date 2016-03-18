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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

/*
 * This file was taken from GLib and adopted for using along with PLib.
 * See gatomic.c for other copyrights and details. All original comments
 * are presented.
 */

#if defined (P_ATOMIC_ARM)
#include <sched.h>
#endif

#include "patomic.h"
#include "pmutex.h"

#if defined (__GNUC__)
#  if defined (P_ATOMIC_I486)
/* Adapted from CVS version 1.10 of glibc's sysdeps/i386/i486/bits/atomic.h */
P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	pint result;

	__asm__ __volatile__ ("lock; xaddl %0,%1"
			      : "=r" (result), "=m" (*atomic)
			      : "0" (val), "m" (*atomic));
	return result;
}

P_LIB_API void
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	__asm__ __volatile__ ("lock; addl %1,%0"
			      : "=m" (*atomic)
			      : "ir" (val), "m" (*atomic));
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	pint result;

	__asm__ __volatile__ ("lock; cmpxchgl %2, %1"
			      : "=a" (result), "=m" (*atomic)
			      : "r" (newval), "m" (*atomic), "0" (oldval));

	return result == oldval;
}

/*
 * The same code as above, as on i386 ppointer is 32 bit as well.
 * Duplicating the code here seems more natural than casting the
 * arguments and calling the former function.
 */
P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	ppointer result;

	__asm__ __volatile__ ("lock; cmpxchgl %2, %1"
			      : "=a" (result), "=m" (*atomic)
			      : "r" (newval), "m" (*atomic), "0" (oldval));

	return result == oldval;
}

#  elif defined (P_ATOMIC_SPARCV9)
/* Adapted from CVS version 1.3 of glibc's sysdeps/sparc/sparc64/bits/atomic.h */
#    define ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)				\
     ({ 									\
	pint __result;								\
	__asm__ __volatile__ ("cas [%4], %2, %0"				\
			      : "=r" (__result), "=m" (*(atomic))		\
			      : "r" (oldval), "m" (*(atomic)), "r" (atomic),	\
			      "0" (newval));					\
	__result == oldval;							\
     })

#    if PLIB_SIZEOF_VOID_P == 4 /* 32-bit system */
P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	ppointer result;
	__asm__ __volatile__ ("cas [%4], %2, %0"
			      : "=r" (result), "=m" (*atomic)
			      : "r" (oldval), "m" (*atomic), "r" (atomic),
			      "0" (newval));
	return result == oldval;
}
#    elif PLIB_SIZEOF_VOID_P == 8 /* 64-bit system */
P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	ppointer result;
	ppointer *a = atomic;
	__asm__ __volatile__ ("casx [%4], %2, %0"
			      : "=r" (result), "=m" (*a)
			      : "r" (oldval), "m" (*a), "r" (a),
			      "0" (newval));
	return result == oldval;
}
#    else /* What's that */
#      error "Your system has an unsupported pointer size."
#    endif /* PLIB_SIZEOF_VOID_P */
#    define P_ATOMIC_MEMORY_BARRIER					\
     __asm__ __volatile__ ("membar #LoadLoad | #LoadStore"		\
			 " | #StoreLoad | #StoreStore" : : : "memory")

#  elif defined (P_ATOMIC_ALPHA)
/* Adapted from CVS version 1.3 of glibc's sysdeps/alpha/bits/atomic.h */
#    define ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)				\
	({ 									\
		pint __result;							\
		pint __prev;							\
		__asm__ __volatile__ (						\
			"       mb\n"						\
			"1:     ldl_l   %0,%2\n"				\
			"       cmpeq   %0,%3,%1\n"				\
			"       beq     %1,2f\n"				\
			"       mov     %4,%1\n"				\
			"       stl_c   %1,%2\n"				\
			"       beq     %1,1b\n"				\
			"       mb\n"						\
			"2:"							\
			: "=&r" (__prev), 					\
			"=&r" (__result)					\
			: "m" (*(atomic)),					\
			"Ir" (oldval),						\
			"Ir" (newval)						\
			: "memory");						\
		__result != 0;							\
	})
#    if PLIB_SIZEOF_VOID_P == 4 /* 32-bit system */
P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	pint result;
	ppointer prev;
	__asm__ __volatile__ (
		"       mb\n"
		"1:     ldl_l   %0,%2\n"
		"       cmpeq   %0,%3,%1\n"
		"       beq     %1,2f\n"
		"       mov     %4,%1\n"
		"       stl_c   %1,%2\n"
		"       beq     %1,1b\n"
		"       mb\n"
		"2:"
		: "=&r" (prev),
		"=&r" (result)
		: "m" (*atomic),
		  "Ir" (oldval),
		  "Ir" (newval)
		: "memory");
	return result != 0;
}
#    elif PLIB_SIZEOF_VOID_P == 8 /* 64-bit system */
P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	pint result;
	ppointer prev;
	__asm__ __volatile__ (
		"       mb\n"
		"1:     ldq_l   %0,%2\n"
		"       cmpeq   %0,%3,%1\n"
		"       beq     %1,2f\n"
		"       mov     %4,%1\n"
		"       stq_c   %1,%2\n"
		"       beq     %1,1b\n"
		"       mb\n"
		"2:"
		: "=&r" (prev),
		  "=&r" (result)
		: "m" (*atomic),
		  "Ir" (oldval),
		  "Ir" (newval)
		: "memory");
	return result != 0;
}
#    else /* What's that */
#      error "Your system has an unsupported pointer size."
#    endif /* PLIB_SIZEOF_VOID_P */
#    define P_ATOMIC_MEMORY_BARRIER  __asm__ ("mb" : : : "memory")
#  elif defined (P_ATOMIC_X86_64)
/* Adapted from CVS version 1.9 of glibc's sysdeps/x86_64/bits/atomic.h */
P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	pint result;

	__asm__ __volatile__ ("lock; xaddl %0,%1"
			      : "=r" (result), "=m" (*atomic)
			      : "0" (val), "m" (*atomic));
	return result;
}

P_LIB_API void
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	__asm__ __volatile__ ("lock; addl %1,%0"
			      : "=m" (*atomic)
			      : "ir" (val), "m" (*atomic));
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	pint result;

	__asm__ __volatile__ ("lock; cmpxchgl %2, %1"
			      : "=a" (result), "=m" (*atomic)
			      : "r" (newval), "m" (*atomic), "0" (oldval));

	return result == oldval;
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	ppointer result;

	__asm__ __volatile__ ("lock; cmpxchgq %q2, %1"
			      : "=a" (result), "=m" (*atomic)
			      : "r" (newval), "m" (*atomic), "0" (oldval));

	return result == oldval;
}

#  elif defined (P_ATOMIC_POWERPC)
/*
 * Adapted from CVS version 1.16 of glibc's sysdeps/powerpc/bits/atomic.h
 * and CVS version 1.4 of glibc's sysdeps/powerpc/powerpc32/bits/atomic.h
 * and CVS version 1.7 of glibc's sysdeps/powerpc/powerpc64/bits/atomic.h
 */
#    ifdef __OPTIMIZE__
/*
 * Non-optimizing compile bails on the following two asm statements
 * for reasons unknown to the author.
 */
P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	pint result, temp;
#if ASM_NUMERIC_LABELS
	__asm__ __volatile__ ("1:       lwarx   %0,0,%3\n"
			      "         add     %1,%0,%4\n"
			      "         stwcx.  %1,0,%3\n"
			      "         bne-    1b"
			      : "=&b" (result), "=&r" (temp), "=m" (*atomic)
			      : "b" (atomic), "r" (val), "m" (*atomic)
			      : "cr0", "memory");
#else
	__asm__ __volatile__ (".Lieaa%=: lwarx   %0,0,%3\n"
			      "          add     %1,%0,%4\n"
			      "          stwcx.  %1,0,%3\n"
			      "          bne-    .Lieaa%="
			      : "=&b" (result), "=&r" (temp), "=m" (*atomic)
			      : "b" (atomic), "r" (val), "m" (*atomic)
			      : "cr0", "memory");
#endif
	return result;
}

/* The same as above, to save a function call repeated here. */
P_LIB_API void
p_atomic_int_add (volatile pint		*atomic,
		  pint			val)
{
	pint result, temp;
#if ASM_NUMERIC_LABELS
	__asm__ __volatile__ ("1:       lwarx   %0,0,%3\n"
			      "         add     %1,%0,%4\n"
			      "         stwcx.  %1,0,%3\n"
			      "         bne-    1b"
			      : "=&b" (result), "=&r" (temp), "=m" (*atomic)
			      : "b" (atomic), "r" (val), "m" (*atomic)
			      : "cr0", "memory");
#else
	__asm__ __volatile__ (".Lia%=:  lwarx   %0,0,%3\n"
			      "         add     %1,%0,%4\n"
			      "         stwcx.  %1,0,%3\n"
			      "         bne-    .Lia%="
			      : "=&b" (result), "=&r" (temp), "=m" (*atomic)
			      : "b" (atomic), "r" (val), "m" (*atomic)
			      : "cr0", "memory");
#endif
}
#    else /* !__OPTIMIZE__ */
P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	pint result;

	do
		result = *atomic;
	while (!p_atomic_int_compare_and_exchange (atomic, result, result + val));

	return result;
}

P_LIB_API void
p_atomic_int_add (volatile pint		*atomic,
		  pint			val)
{
	pint result;

	do
		result = *atomic;
	while (!p_atomic_int_compare_and_exchange (atomic, result, result + val));
}
#    endif /* !__OPTIMIZE__ */

#    if PLIB_SIZEOF_VOID_P == 4 /* 32-bit system */
P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	pint result;
#if ASM_NUMERIC_LABELS
	__asm__ __volatile__ ("sync\n"
			      "1: lwarx   %0,0,%1\n"
			      "   subf.   %0,%2,%0\n"
			      "   bne     2f\n"
			      "   stwcx.  %3,0,%1\n"
			      "   bne-    1b\n"
			      "2: isync"
			      : "=&r" (result)
			      : "b" (atomic), "r" (oldval), "r" (newval)
			      : "cr0", "memory");
#else
	__asm__ __volatile__ ("sync\n"
			      ".L1icae%=: lwarx   %0,0,%1\n"
			      "   subf.   %0,%2,%0\n"
			      "   bne     .L2icae%=\n"
			      "   stwcx.  %3,0,%1\n"
			      "   bne-    .L1icae%=\n"
			      ".L2icae%=: isync"
			      : "=&r" (result)
			      : "b" (atomic), "r" (oldval), "r" (newval)
			      : "cr0", "memory");
#endif
	return result == 0;
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	ppointer result;
#if ASM_NUMERIC_LABELS
	__asm__ __volatile__ ("sync\n"
			      "1: lwarx   %0,0,%1\n"
			      "   subf.   %0,%2,%0\n"
			      "   bne     2f\n"
			      "   stwcx.  %3,0,%1\n"
			      "   bne-    1b\n"
			      "2: isync"
			      : "=&r" (result)
			      : "b" (atomic), "r" (oldval), "r" (newval)
			      : "cr0", "memory");
#else
	__asm__ __volatile__ ("sync\n"
			      ".L1pcae%=: lwarx   %0,0,%1\n"
			      "   subf.   %0,%2,%0\n"
			      "   bne     .L2pcae%=\n"
			      "   stwcx.  %3,0,%1\n"
			      "   bne-    .L1pcae%=\n"
			      ".L2pcae%=: isync"
			      : "=&r" (result)
			      : "b" (atomic), "r" (oldval), "r" (newval)
			      : "cr0", "memory");
#endif
	return result == 0;
}
#    elif PLIB_SIZEOF_VOID_P == 8 /* 64-bit system */
P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	ppointer result;
#if ASM_NUMERIC_LABELS
	__asm__ __volatile__ ("sync\n"
			      "1: lwarx   %0,0,%1\n"
			      "   extsw   %0,%0\n"
			      "   subf.   %0,%2,%0\n"
			      "   bne     2f\n"
			      "   stwcx.  %3,0,%1\n"
			      "   bne-    1b\n"
			      "2: isync"
			      : "=&r" (result)
			      : "b" (atomic), "r" (oldval), "r" (newval)
			      : "cr0", "memory");
#else
	__asm__ __volatile__ ("sync\n"
			      ".L1icae%=: lwarx   %0,0,%1\n"
			      "   extsw   %0,%0\n"
			      "   subf.   %0,%2,%0\n"
			      "   bne     .L2icae%=\n"
			      "   stwcx.  %3,0,%1\n"
			      "   bne-    .L1icae%=\n"
			      ".L2icae%=: isync"
			      : "=&r" (result)
			      : "b" (atomic), "r" (oldval), "r" (newval)
			      : "cr0", "memory");
#endif
	return result == 0;
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	ppointer result;
#if ASM_NUMERIC_LABELS
	__asm__ __volatile__ ("sync\n"
			      "1: ldarx   %0,0,%1\n"
			      "   subf.   %0,%2,%0\n"
			      "   bne     2f\n"
			      "   stdcx.  %3,0,%1\n"
			      "   bne-    1b\n"
			      "2: isync"
			      : "=&r" (result)
			      : "b" (atomic), "r" (oldval), "r" (newval)
			      : "cr0", "memory");
#else
	__asm__ __volatile__ ("sync\n"
			      ".L1pcae%=: ldarx   %0,0,%1\n"
			      "   subf.   %0,%2,%0\n"
			      "   bne     .L2pcae%=\n"
			      "   stdcx.  %3,0,%1\n"
			      "   bne-    .L1pcae%=\n"
			      ".L2pcae%=: isync"
			      : "=&r" (result)
			      : "b" (atomic), "r" (oldval), "r" (newval)
			      : "cr0", "memory");
#endif
	return result == 0;
}
#    else /* What's that */
#      error "Your system has an unsupported pointer size."
#    endif /* PLIB_SIZEOF_VOID_P */

#    define P_ATOMIC_MEMORY_BARRIER __asm__ ("sync" : : : "memory")

#  elif defined (P_ATOMIC_IA64)
/* Adapted from CVS version 1.8 of glibc's sysdeps/ia64/bits/atomic.h */
P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	return __sync_fetch_and_add (atomic, val);
}

P_LIB_API void
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	__sync_fetch_and_add (atomic, val);
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	return __sync_bool_compare_and_swap (atomic, oldval, newval);
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	return __sync_bool_compare_and_swap ((long *) atomic,
					     (long) oldval, (long) newval);
}

#    define P_ATOMIC_MEMORY_BARRIER __sync_synchronize ()
#  elif defined (P_ATOMIC_S390)
/* Adapted from glibc's sysdeps/s390/bits/atomic.h */
#    define ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)				\
	({ 									\
		pint __result = oldval;						\
		__asm__ __volatile__ ("cs %0, %2, %1"				\
				      : "+d" (__result), "=Q" (*(atomic))	\
				      : "d" (newval), "m" (*(atomic)) : "cc" );	\
		__result == oldval;						\
	})

#    if PLIB_SIZEOF_VOID_P == 4 /* 32-bit system */
P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	ppointer result = oldval;

	__asm__ __volatile__ ("cs %0, %2, %1"
			      : "+d" (result), "=Q" (*(atomic))
			      : "d" (newval), "m" (*(atomic)) : "cc" );
	return result == oldval;
}
#    elif PLIB_SIZEOF_VOID_P == 8 /* 64-bit system */
P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	ppointer result = oldval;
	ppointer *a = atomic;

	__asm__ __volatile__ ("csg %0, %2, %1"
			      : "+d" (result), "=Q" (*a)
			      : "d" ((long)(newval)), "m" (*a) : "cc" );
	return result == oldval;
}
#    else /* What's that */
#      error "Your system has an unsupported pointer size."
#    endif /* PLIB_SIZEOF_VOID_P */
#  elif defined (P_ATOMIC_ARM)
static volatile pint atomic_spin = 0;

static pint atomic_spin_trylock (void)
{
	pint result;

	asm volatile ("swp %0, %1, [%2]\n"
		      : "=&r,&r" (result)
		      : "r,0" (1), "r,r" (&atomic_spin)
		      : "memory");
	if (result == 0)
		return 0;
	else
		return -1;
}

static void atomic_spin_lock (void)
{
	while (atomic_spin_trylock ())
		sched_yield ();
}

static void atomic_spin_unlock (void)
{
	atomic_spin = 0;
}

P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	pint result;

	atomic_spin_lock ();
	result = *atomic;
	*atomic += val;
	atomic_spin_unlock ();

	return result;
}

P_LIB_API void
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	atomic_spin_lock ();
	*atomic += val;
	atomic_spin_unlock ();
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	pboolean result;

	atomic_spin_lock ();
	if (*atomic == oldval) {
		result = TRUE;
		*atomic = newval;
	} else
		result = FALSE;

	atomic_spin_unlock ();

	return result;
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	pboolean result;

	atomic_spin_lock ();
	if (*atomic == oldval) {
		result = TRUE;
		*atomic = newval;
	} else
		result = FALSE;

	atomic_spin_unlock ();

	return result;
}
#  elif defined (P_ATOMIC_CRIS) || defined (P_ATOMIC_CRISV32)
#    ifdef P_ATOMIC_CRIS
#      define CRIS_ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)			\
	({									\
		pboolean __result;						\
		__asm__ __volatile__ ("\n"					\
				      "0:\tclearf\n\t"				\
				      "cmp.d [%[Atomic]], %[OldVal]\n\t"	\
				      "bne 1f\n\t"				\
				      "ax\n\t"					\
				      "move.d %[NewVal], [%[Atomic]]\n\t"	\
				      "bwf 0b\n"				\
				      "1:\tseq %[Result]"			\
				      : [Result] "=&r" (__result),		\
				      "=m" (*(atomic))				\
				      : [Atomic] "r" (atomic),			\
				      [OldVal] "r" (oldval),			\
				      [NewVal] "r" (newval),			\
				      "g" (*(ppointer*) (atomic))		\
				     : "memory");				\
		__result;							\
  })
#    else
#      define CRIS_ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)			\
	({									\
		pboolean __result;						\
		__asm__ __volatile__ ("\n"					\
				      "0:\tclearf p\n\t"			\
				      "cmp.d [%[Atomic]], %[OldVal]\n\t"	\
				      "bne 1f\n\t"				\
				      "ax\n\t"					\
				      "move.d %[NewVal], [%[Atomic]]\n\t"	\
				      "bcs 0b\n"				\
				      "1:\tseq %[Result]"			\
				      : [Result] "=&r" (__result),		\
				      "=m" (*(atomic))				\
				      : [Atomic] "r" (atomic),			\
				      [OldVal] "r" (oldval),			\
				      [NewVal] "r" (newval),			\
				      "g" (*(ppointer*) (atomic))		\
				      : "memory");				\
		__result;							\
	})
#    endif

#    define CRIS_CACHELINE_SIZE 32
#    define CRIS_ATOMIC_BREAKS_CACHELINE(atomic) \
	(((pulong)(atomic) & (CRIS_CACHELINE_SIZE - 1)) > (CRIS_CACHELINE_SIZE - sizeof (atomic)))

pint     __p_atomic_int_exchange_and_add         (volatile pint		*atomic,
						  pint			val);
void     __p_atomic_int_add                      (volatile pint		*atomic,
						  pint			val);
pboolean __p_atomic_int_compare_and_exchange     (volatile pint		*atomic,
						  pint			oldval,
						  pint			newval);
pboolean __p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
						  ppointer		oldval,
						  ppointer		newval);

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	if (CRIS_ATOMIC_BREAKS_CACHELINE (atomic))
		return __p_atomic_pointer_compare_and_exchange (atomic, oldval, newval);

	return CRIS_ATOMIC_INT_CMP_XCHG (atomic, oldval, newval);
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	if (CRIS_ATOMIC_BREAKS_CACHELINE (atomic))
		return __p_atomic_int_compare_and_exchange (atomic, oldval, newval);

	return CRIS_ATOMIC_INT_CMP_XCHG (atomic, oldval, newval);
}

P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	pint result;

	if (CRIS_ATOMIC_BREAKS_CACHELINE (atomic))
		return __p_atomic_int_exchange_and_add (atomic, val);

	do
		result = *atomic;
	while (!CRIS_ATOMIC_INT_CMP_XCHG (atomic, result, result + val));

	return result;
}

P_LIB_API void
p_atomic_int_add (volatile pint		*atomic,
		  pint			val)
{
	pint result;

	if (CRIS_ATOMIC_BREAKS_CACHELINE (atomic))
		return __p_atomic_int_add (atomic, val);

	do
		result = *atomic;
	while (!CRIS_ATOMIC_INT_CMP_XCHG (atomic, result, result + val));
}

/*
 * We need the atomic mutex for atomic operations where the atomic variable
 * breaks the 32 byte cache line since the CRIS architecture does not support
 * atomic operations on such variables. Fortunately this should be rare.
 */
#    define DEFINE_WITH_MUTEXES
#    define p_atomic_int_exchange_and_add __p_atomic_int_exchange_and_add
#    define p_atomic_int_add __p_atomic_int_add
#    define p_atomic_int_compare_and_exchange __p_atomic_int_compare_and_exchange
#    define p_atomic_pointer_compare_and_exchange __p_atomic_pointer_compare_and_exchange

#  else /* !P_ATOMIC_* */
#    define DEFINE_WITH_MUTEXES
#  endif /* P_ATOMIC_* */
#else /* !__GNUC__ */
#  ifdef P_OS_WIN
#    define DEFINE_WITH_WIN32_INTERLOCKED
#  else
#    define DEFINE_WITH_MUTEXES
#  endif
#endif /* __GNUC__ */

#ifdef DEFINE_WITH_WIN32_INTERLOCKED
#include <winsock2.h>
#include <windows.h>

P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint32	*atomic,
			       pint32		val)
{
	return (pint) InterlockedExchangeAdd ((LONG volatile *) atomic, (LONG) val);
}

P_LIB_API void
p_atomic_int_add (volatile pint32	*atomic,
		  pint32		val)
{
	InterlockedExchangeAdd ((LONG volatile *) atomic, (LONG) val);
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint32	*atomic,
				   pint32		oldval,
				   pint32		newval)
{
	return InterlockedCompareExchange ((LONG volatile *) atomic,
					   (LONG) newval,
					   (LONG) oldval) == (LONG) oldval;
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
#  ifdef PLIB_HAS_INTERLOCKED_POINTER
	return InterlockedCompareExchangePointer ((PVOID volatile *) atomic,
						  (PVOID) newval,
						  (PVOID) oldval) == (PVOID) oldval;
#  else
#    if PLIB_SIZEOF_VOID_P != 4 /* no 32-bit system */
#      if defined(PLIB_HAS_INTERLOCKED_64)
	return InterlockedCompareExchange64 ((LONGLONG volatile *) atomic,
					      (LONGLONG) newval,
					      (LONGLONG) oldval) == (LONGLONG) oldval;
#      else
#        error "InterlockedCompareExchangePointer needed"
#      endif
#    else
	return InterlockedCompareExchange ((LONG volatile *) atomic,
					   (LONG) newval,
					   (LONG) oldval) == (LONG) oldval;
#    endif
#  endif
}
#endif /* DEFINE_WITH_WIN32_INTERLOCKED */

#ifdef DEFINE_WITH_MUTEXES
/* We have to use the slow, but safe locking method. */
static PMutex *p_atomic_mutex;

P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	pint result;

	p_mutex_lock (p_atomic_mutex);
	result = *atomic;
	*atomic += val;
	p_mutex_unlock (p_atomic_mutex);

	return result;
}

P_LIB_API void
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	p_mutex_lock (p_atomic_mutex);
	*atomic += val;
	p_mutex_unlock (p_atomic_mutex);
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	pboolean result;

	p_mutex_lock (p_atomic_mutex);
	if (*atomic == oldval) {
		result = TRUE;
		*atomic = newval;
	} else
		result = FALSE;
	p_mutex_unlock (p_atomic_mutex);

	return result;
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	pboolean result;

	p_mutex_lock (p_atomic_mutex);

	if (*atomic == oldval) {
		result = TRUE;
		*atomic = newval;
	} else
		result = FALSE;

	p_mutex_unlock (p_atomic_mutex);

	return result;
}

#  ifdef P_ATOMIC_OP_MEMORY_BARRIER_NEEDED
P_LIB_API pint
(p_atomic_int_get) (volatile pint *atomic)
{
	pint result;

	p_mutex_lock (p_atomic_mutex);
	result = *atomic;
	p_mutex_unlock (p_atomic_mutex);

	return result;
}

P_LIB_API void
(p_atomic_int_set) (volatile pint	*atomic,
		    pint		newval)
{
	p_mutex_lock (p_atomic_mutex);
	*atomic = newval;
	p_mutex_unlock (p_atomic_mutex);
}

P_LIB_API ppointer
(p_atomic_pointer_get) (volatile ppointer *atomic)
{
	ppointer result;

	p_mutex_lock (p_atomic_mutex);
	result = *atomic;
	p_mutex_unlock (p_atomic_mutex);

	return result;
}

P_LIB_API void
(p_atomic_pointer_set) (volatile ppointer	*atomic,
			ppointer		newval)
{
	p_mutex_lock (p_atomic_mutex);
	*atomic = newval;
	p_mutex_unlock (p_atomic_mutex);
}
#  endif /* P_ATOMIC_OP_MEMORY_BARRIER_NEEDED */
#elif defined (P_ATOMIC_OP_MEMORY_BARRIER_NEEDED)
P_LIB_API pint
(p_atomic_int_get) (volatile pint *atomic)
{
	P_ATOMIC_MEMORY_BARRIER;
	return *atomic;
}

P_LIB_API void
(p_atomic_int_set) (volatile pint	*atomic,
		    pint		newval)
{
	*atomic = newval;
	P_ATOMIC_MEMORY_BARRIER;
}

P_LIB_API ppointer
(p_atomic_pointer_get) (volatile ppointer *atomic)
{
	P_ATOMIC_MEMORY_BARRIER;
	return *atomic;
}

P_LIB_API void
(p_atomic_pointer_set) (volatile ppointer	*atomic,
			ppointer		newval)
{
	*atomic = newval;
	P_ATOMIC_MEMORY_BARRIER;
}
#endif /* DEFINE_WITH_MUTEXES || P_ATOMIC_OP_MEMORY_BARRIER_NEEDED */

#ifdef ATOMIC_INT_CMP_XCHG
P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	return ATOMIC_INT_CMP_XCHG (atomic, oldval, newval);
}

P_LIB_API pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	pint result;

	do
		result = *atomic;
	while (!ATOMIC_INT_CMP_XCHG (atomic, result, result + val));

	return result;
}

P_LIB_API void
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	pint result;

	do
		result = *atomic;
	while (!ATOMIC_INT_CMP_XCHG (atomic, result, result + val));
}
#endif /* ATOMIC_INT_CMP_XCHG */

#ifndef P_ATOMIC_OP_MEMORY_BARRIER_NEEDED
P_LIB_API pint
(p_atomic_int_get) (volatile pint *atomic)
{
	return p_atomic_int_get (atomic);
}

P_LIB_API void
(p_atomic_int_set) (volatile pint	*atomic,
		    pint		newval)
{
	p_atomic_int_set (atomic, newval);
}

P_LIB_API ppointer
(p_atomic_pointer_get) (volatile ppointer *atomic)
{
	return p_atomic_pointer_get (atomic);
}

P_LIB_API void
(p_atomic_pointer_set) (volatile ppointer	*atomic,
			ppointer		newval)
{
	p_atomic_pointer_set (atomic, newval);
}
#endif /* P_ATOMIC_OP_MEMORY_BARRIER_NEEDED */

P_LIB_API void
p_atomic_memory_barrier (void)
{
#ifdef P_ATOMIC_MEMORY_BARRIER
	P_ATOMIC_MEMORY_BARRIER;
#endif
}

void
__p_atomic_thread_init (void)
{
#ifdef DEFINE_WITH_MUTEXES
	p_atomic_mutex = p_mutex_new ();
#endif /* DEFINE_WITH_MUTEXES */
}

void
__p_atomic_thread_shutdown (void)
{
#ifdef DEFINE_WITH_MUTEXES
	if (p_atomic_mutex != NULL) {
		p_mutex_free (p_atomic_mutex);
		p_atomic_mutex = NULL;
	}
#endif /* DEFINE_WITH_MUTEXES */
}
