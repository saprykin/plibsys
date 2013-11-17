#ifndef __PPLATFORM_H__
#define __PPLATFORM_H__

#include <malloc.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct _PMutex* PStaticMutex;
#define P_STATIC_MUTEX_INIT NULL
extern P_LIB_API PStaticMutex p_static_mutex_get_mutex_impl (PStaticMutex *mutex);
#define p_static_mutex_get_mutex(mutex)	(p_static_mutex_get_mutex_impl (mutex))

#endif /* __PPLATFORM_H__ */
