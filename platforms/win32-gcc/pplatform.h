#ifndef __PPLATFORM_H__
#define __PPLATFORM_H__

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <memory.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct _PMutex* PStaticMutex;
#define P_STATIC_MUTEX_INIT NULL
extern P_LIB_API PStaticMutex p_static_mutex_get_mutex_impl (PStaticMutex *mutex);
#define p_static_mutex_get_mutex(mutex)	(p_static_mutex_get_mutex_impl (mutex))

#endif /* __PPLATFORM_H__ */
