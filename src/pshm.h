/*
 * The MIT License
 *
 * Copyright (C) 2010-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

/**
 * @file pshm.h
 * @brief Shared memory
 * @author Alexander Saprykin
 *
 * Shared memory is a memory segment which can be accessed from several threads
 * or processes. It provides an efficient way to transfer large blocks of data
 * between processes. It can be used as any other regular memory segment in an
 * application.
 *
 * Shared memory acts like an inter-process communication method. This memory
 * exchange implementation is process-wide so you can transfer data not only
 * between the threads. But it makes this IPC method (actually like any other
 * IPC method, as well) relatively heavy. Consider using other approaches
 * instead if you do not need to cross the process boundary.
 *
 * A shared memory segment doesn't provide any synchronization primitives itself
 * which means that several processes or threads can concurrently write and read
 * from it. This can lead to data consistency problems. To avoid such situations
 * a locking mechanism is provided: use p_shm_lock() before entering a critical
 * section on the memory segment and p_shm_unlock() when leaving this section.
 * The locking mechanism is working across the process boundary.
 *
 * A process-wide shared memory segment is identified by its name across the
 * system, thus it is also called a named memory segment. Use p_shm_new() to
 * open the named shared memory segment and p_shm_free() to close it.
 *
 * Please note the following platform specific differences:
 *
 * - Windows and OS/2 don't own IPC objects (processes own them), which means
 * that a shared memory segment will be removed after the last process or thread
 * detaches (or after terminating all the processes and threads attached to the
 * segment) it.
 *
 * - UNIX systems own IPC objects. Because of that UNIX IPC objects can survive
 * an application crash: the attached shared memory segment can contain data
 * from the previous working session. This could happen if you have not detached
 * from all the shared memory segments explicitly before terminating the
 * application.
 *
 * - HP-UX has limitations due to its MPAS/MGAS features, so you couldn't attach
 * to the same memory segment twice from the same process.
 *
 * - IRIX allows to open several instances of the same buffer within the single
 * process, but it will close the object after the first close call from any of
 * the threads within the process.
 *
 * - OpenVMS (as of 8.4 release) has broken implementation of process-wide named
 * semaphores which leads to the broken shared memory also.
 *
 * - Syllable lacks support for process-wide named semaphores which leads to the
 * absence of shared memory.
 *
 * - BeOS lacks support for process-wide named semaphores which leads to the
 * absence of shared memory.
 *
 * You can take ownership of the shared memory segment with
 * p_shm_take_ownership() to explicitly remove it from the system after closing.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PSHM_H
#define PLIBSYS_HEADER_PSHM_H

#include <pmacros.h>
#include <ptypes.h>
#include <perror.h>

P_BEGIN_DECLS

/** Enum with shared memory access permissions. */
typedef enum PShmAccessPerms_ {
	P_SHM_ACCESS_READONLY	= 0,	/**< Read-only access.	*/
	P_SHM_ACCESS_READWRITE	= 1	/**< Read/write access.	*/
} PShmAccessPerms;

/** Shared memory opaque data structure. */
typedef struct PShm_ PShm;

/**
 * @brief Creates a new #PShm object.
 * @param name Shared memory name.
 * @param size Size of the memory segment in bytes, can't be changed later.
 * @param perms Memory segment permissions, see #PShmAccessPerms.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to a newly created #PShm object in case of success, NULL
 * otherwise.
 * @since 0.0.1
 */
P_LIB_API PShm *	p_shm_new		(const pchar		*name,
						 psize			size,
						 PShmAccessPerms	perms,
						 PError			**error);

/**
 * @brief Takes ownership of a shared memory segment.
 * @param shm Shared memory segment.
 * @since 0.0.1
 *
 * If you take ownership of the shared memory object, p_shm_free() will try to
 * completely unlink it and remove from the system. This is useful on UNIX
 * systems where shared memory can survive an application crash. On the Windows
 * and OS/2 platforms this call has no effect.
 *
 * The common usage of this call is upon application startup to ensure that the
 * memory segment from the previous crash will be unlinked from the system. To
 * do that, call p_shm_new() and check if its condition is normal (the segment
 * size, the data). If not, take ownership of the shared memory object and
 * remove it with the p_shm_free() call. After that, create it again.
 */
P_LIB_API void		p_shm_take_ownership	(PShm			*shm);

/**
 * @brief Frees #PShm object.
 * @param shm #PShm to free.
 * @since 0.0.1
 *
 * It doesn't unlock a given shared memory segment, be careful to not to make a
 * deadlock or a segfault while freeing the memory segment which is under usage.
 */
P_LIB_API void		p_shm_free		(PShm			*shm);

/**
 * @brief Locks #PShm object for usage.
 * @param shm #PShm to lock.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * If the object is already locked then the thread will be suspended until the
 * object becomes unlocked.
 */
P_LIB_API pboolean	p_shm_lock		(PShm			*shm,
						 PError			**error);

/**
 * @brief Unlocks #PShm object.
 * @param shm #PShm to unlock.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_shm_unlock		(PShm			*shm,
						 PError			**error);

/**
 * @brief Gets a starting address of a #PShm memory segment.
 * @param shm #PShm to get the address for.
 * @return Pointer to the starting address in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_shm_get_address	(const PShm		*shm);

/**
 * @brief Gets the size of a #PShm memory segment.
 * @param shm #PShm to get the size for.
 * @return Size of the given memory segment in case of success, 0 otherwise.
 * @since 0.0.1
 *
 * Note that the returned size would be a slightly larger than specified during
 * the p_shm_new() call due to service information stored inside.
 */
P_LIB_API psize		p_shm_get_size		(const PShm		*shm);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PSHM_H */
