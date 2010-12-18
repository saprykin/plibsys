/* 
 * 07.10.2010
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

#ifndef __PNETWORK_H__
#define __PNETWORK_H__

#include <pmacros.h>
#include <ptypes.h>

#ifndef P_OS_WIN
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

P_BEGIN_DECLS

P_LIB_API const char *	inet_ntop (int af, const void *addr, char *buf, socklen_t len);
P_LIB_API int		inet_pton (int af, const char *src, void *dst);

P_END_DECLS
#endif /* !P_OS_WIN */

#endif /* __PNETWORK_H__ */

