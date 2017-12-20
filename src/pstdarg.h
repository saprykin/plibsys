/*
 * Copyright (C) 2017 Jean-Damien Durand <jeandamiendurand@free.fr>
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

/**
 * @file pstdarg.h
 * @brief Variable arguments
 * @author Jean-Damien Durand
 *
 * Functions declared with a variable number of arguments use macros to step
 * through them.
 *
 * The #p_va_list type must be declared in such function, #p_va_start,
 * #p_va_arg and #p_va_end are used to initialize, to step through and to end
 * the navigation, respectively.
 *
 * A variable number of arguments can be propagated to another function which
 * accepts a #p_va_list parameter, using #p_va_copy.
 *
 * Any use of #p_va_start or #p_va_copy must have a corresponding #p_va_end.
 *
 * Using a variable number of parameters requires a known-in-advance contract
 * between the caller and the callee on the number of parameters and their types.
 * Note, that this mechanism is a weakly typed: the compiler will always apply
 * default type promotion, regardless if you explicitely typecast an argument in
 * the stack, i.e.:
 * - Integer arguments of types lower than #pint are always promoted to #pint,
 *   or #puint if #pint is not enough.
 * - Arguments of type @a float are always promoted to @a double.
 * - Arrays are always promoted to pointers.
 *
 * You need to be very careful when using variable arguments. Improper usage may
 * lead to program crash. In order to avoid type casting mistakes, consider
 * using macros for variable arguments with explicit type casting provided below.
 * Though you still can use #p_va_arg if you know what are you doing.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PSTDARG_H
#define PLIBSYS_HEADER_PSTDARG_H

#include <pmacros.h>
#include <ptypes.h>

#include <stdio.h>
#include <stdarg.h>
#ifndef PLIBSYS_VA_COPY
#  include <string.h>
#endif

/**
 * @brief Opaque type holding variable number of arguments navigation.
 * @since 0.0.4
 */
#define p_va_list va_list

/**
 * @brief Initializes navigation through a variable number of arguments.
 * @param ap Declared object of type #p_va_list.
 * @param last Name of the last argument before @a ap.
 * @since 0.0.4
 * @note There must be a corresponding call to #p_va_end(ap) afterwards.
 */
#define p_va_start(ap, last) va_start(ap, last)

/**
 * @brief Gets the next argument in the list.
 * @param ap Declared object of type va_list, previously initalized with
 * #p_va_start.
 * @param type Type of the next argument.
 * @return Value of the next argument.
 * @since 0.0.4
 */

#define p_va_arg(ap, type) (type) va_arg(ap, type)

/**
 * @brief Ends the navigation.
 * @param ap Declared object of type #p_va_list.
 * @since 0.0.4
 * @note There must be a corresponding call to #p_va_start(ap) before.
 */
#define p_va_end(ap) va_end(ap)

/**
 * @brief Copies a navigation object.
 * @param dst Destination object of type #p_va_list.
 * @param src Source object of type #p_va_list.
 * @since 0.0.4
 * @note The state of @a src is copied as well, @a dst is initialized as if
 * #p_va_start(dst) would have been called. There must be a corresponding call
 * to #p_va_end(dst) afterwards.
 */
#ifdef PLIBSYS_VA_COPY
#  define p_va_copy(dst, src) PLIBSYS_VA_COPY(dst, src)
#else
#  define p_va_copy(dst, src) ((void) memcpy (&(dst), &(src), sizeof (va_list)))
#endif

/**
 * @def pint8_va_arg
 * @brief Unstacks a #pint8 variable.
 * @param ap #p_va_list stack.
 * @return #pint8 value.
 * @since 0.0.4
 */
#define pint8_va_arg(ap)		((pint8) p_va_arg(ap, pint))

/**
 * @def puint8_va_arg
 * @brief Unstacks a #puint8 variable.
 * @param ap #p_va_list stack.
 * @return #puint8 value.
 * @since 0.0.4
 */
#define puint8_va_arg(ap)		((puint8) p_va_arg(ap, puint))

/**
 * @def pint16_va_arg
 * @brief Unstacks a #pint16 variable.
 * @param ap #p_va_list stack.
 * @return #pint16 value.
 * @since 0.0.4
 */
#define pint16_va_arg(ap)		((pint16) p_va_arg(ap, pint))

/**
 * @def puint16_va_arg
 * @brief Unstacks a #puint16 variable.
 * @param ap #p_va_list stack.
 * @return #puint16 value.
 * @since 0.0.4
 */
#define puint16_va_arg(ap)		((puint16) p_va_arg(ap, puint))

/**
 * @def pint32_va_arg
 * @brief Unstacks a #pint32 variable.
 * @param ap #p_va_list stack.
 * @return #pint32 value.
 * @since 0.0.4
 */
#define pint32_va_arg(ap)		((pint32) p_va_arg(ap, pint))

/**
 * @def puint32_va_arg
 * @brief Unstacks a #puint32 variable.
 * @param ap #p_va_list stack.
 * @return #puint32 value.
 * @since 0.0.4
 */
#define puint32_va_arg(ap)		((puint32) p_va_arg(ap, puint))

/**
 * @def pint64_va_arg
 * @brief Unstacks a #pint64 variable.
 * @param ap #p_va_list stack.
 * @return #pint64 value.
 * @since 0.0.4
 */
#define pint64_va_arg(ap)		(p_va_arg(ap, pint64))

/**
 * @def puint64_va_arg
 * @brief Unstacks a #puint64 variable.
 * @param ap #p_va_list stack.
 * @return #puint64 value.
 * @since 0.0.4
 */
#define puint64_va_arg(ap)		(p_va_arg(ap, puint64))

/**
 * @def ppointer_va_arg
 * @brief Unstacks a #ppointer variable.
 * @param ap #p_va_list stack.
 * @return #ppointer value.
 * @since 0.0.4
 */
#define ppointer_va_arg(ap)		(p_va_arg(ap, ppointer))

/**
 * @def pconstpointer_va_arg
 * @brief Unstacks a #pconstpointer variable.
 * @param ap #p_va_list stack.
 * @return #pconstpointer value.
 * @since 0.0.4
 */
#define pconstpointer_va_arg(ap)	(p_va_arg(ap, pconstpointer))

/**
 * @def pboolean_va_arg
 * @brief Unstacks a #pboolean variable.
 * @param ap #p_va_list stack.
 * @return #pboolean value.
 * @since 0.0.4
 */
#define pboolean_va_arg(ap)		((pboolean) p_va_arg(ap, pint))

/**
 * @def pchar_va_arg
 * @brief Unstacks a #pchar variable.
 * @param ap #p_va_list stack.
 * @return #pchar value.
 * @since 0.0.4
 */
#define pchar_va_arg(ap)		((pchar) p_va_arg(ap, pint))

/**
 * @def pshort_va_arg
 * @brief Unstacks a #pshort variable.
 * @param ap #p_va_list stack.
 * @return #pshort value.
 * @since 0.0.4
 */
#define pshort_va_arg(ap)		((pshort) p_va_arg(ap, pint))

/**
 * @def pint_va_arg
 * @brief Unstacks a #pint variable.
 * @param ap #p_va_list stack.
 * @return #pint value.
 * @since 0.0.4
 */
#define pint_va_arg(ap)			(p_va_arg(ap, pint))

/**
 * @def plong_va_arg
 * @brief Unstacks a #plong variable.
 * @param ap #p_va_list stack.
 * @return #plong value.
 * @since 0.0.4
 */
#define plong_va_arg(ap)		(p_va_arg(ap, plong))

/**
 * @def puchar_va_arg
 * @brief Unstacks a #puchar variable.
 * @param ap #p_va_list stack.
 * @return #puchar value.
 * @since 0.0.4
 */
#define puchar_va_arg(ap)		((puchar) p_va_arg(ap, puint))

/**
 * @def pushort_va_arg
 * @brief Unstacks a #pushort variable.
 * @param ap #p_va_list stack.
 * @return #pushort value.
 * @since 0.0.4
 */
#define pushort_va_arg(ap)		((pushort) p_va_arg(ap, puint))

/**
 * @def puint_va_arg
 * @brief Unstacks a #puint variable.
 * @param ap #p_va_list stack.
 * @return #puint value.
 * @since 0.0.4
 */
#define puint_va_arg(ap)		(p_va_arg(ap, puint))

/**
 * @def pulong_va_arg
 * @brief Unstacks a #pulong variable.
 * @param ap #p_va_list stack.
 * @return #pulong value.
 * @since 0.0.4
 */
#define pulong_va_arg(ap)		(p_va_arg(ap, pulong))

/**
 * @def pfloat_va_arg
 * @brief Unstacks a #pfloat variable.
 * @param ap #p_va_list stack.
 * @return #pfloat value.
 * @since 0.0.4
 */
#define pfloat_va_arg(ap)		((pfloat) p_va_arg(ap, pdouble))

/**
 * @def pdouble_va_arg
 * @brief Unstacks a #pdouble variable.
 * @param ap #p_va_list stack.
 * @return #pdouble value.
 * @since 0.0.4
 */
#define pdouble_va_arg(ap)		(p_va_arg(ap, pdouble))

#endif /* PLIBSYS_HEADER_PSTDARG_H */
