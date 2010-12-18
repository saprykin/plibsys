/* 
 * 22.08.2010
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

#ifndef __PSEMAPHORE_H__
#define __PSEMAPHORE_H__

#include <pmacros.h>
#include <ptypes.h>

#define P_SEM_ERROR_BUF_SIZE	255

typedef enum _PSemaphoreError {
	P_SEM_ERROR_NONE	= 0,
	P_SEM_ERROR_ACCESS	= 1,
	P_SEM_ERROR_RESOURSE	= 2,
	P_SEM_ERROR_EXISTS	= 3,
	P_SEM_ERROR_UNKNOWN	= 4
} PSemaphoreError;

typedef enum _PSemaphoreAccessMode {
	P_SEM_ACCESS_OPEN	= 0,
	P_SEM_ACCESS_CREATE	= 1
} PSemaphoreAccessMode;

typedef struct _PSemaphore PSemaphore;

#ifndef P_OS_WIN
extern pchar *	p_ipc_unix_get_temp_dir		(void);
extern pint	p_ipc_unix_create_key_file	(const pchar		*file_name);
extern pint	p_ipc_unix_get_ftok_key		(const pchar		*file_name);
#endif

extern pchar *	p_ipc_get_platform_key		(const pchar		*name,
						 pboolean		posix);

P_BEGIN_DECLS

P_LIB_API PSemaphore *	p_semaphore_new 	(const pchar		*name,
						 pint			init_val,
						 PSemaphoreAccessMode	mode);
P_LIB_API pboolean	p_semaphore_acquire 	(PSemaphore		*sem);
P_LIB_API pboolean	p_semaphore_release	(PSemaphore		*sem);
P_LIB_API void		p_semaphore_free	(PSemaphore		*sem);

P_END_DECLS

#endif /* __PSEMAPHORE_H__ */

