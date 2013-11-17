#ifndef __PPLATFORM_H__
#define __PPLATFORM_H__

#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <memory.h>
#include <netinet/in.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Byte contents of pmutex */
#define PLIB_BYTE_CONTENTS_PMUTEX 0,0,0,-128,-1,-1,-1,-1

/* The size of pmutex, as computed by sizeof. */
#define PLIB_SIZEOF_PMUTEX 8

typedef struct _PStaticMutex
{
	union {
	char	pad[8];
	double	dummy_double;
	long	dummy_long;
	void	*dummy_pointer;
	} static_mutex;
} PStaticMutex;

#define	P_STATIC_MUTEX_INIT		{ { { 0,0,0,-128,-1,-1,-1,-1} } }
#define	p_static_mutex_get_mutex(mutex)	((PMutex*) ((ppointer) ((mutex)->static_mutex.pad)))

#endif /* __PPLATFORM_H__ */
