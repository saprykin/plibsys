#ifndef __PPLATFORM_H__
#define __PPLATFORM_H__

#include <unistd.h>
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

/* Byte contents of pmutex */
#define PLIB_BYTE_CONTENTS_PMUTEX -89,-85,-86,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

/* The size of pmutex, as computed by sizeof. */
#define PLIB_SIZEOF_PMUTEX 64

typedef struct _PStaticMutex
{
	union {
	char	pad[64];
	double	dummy_double;
	long	dummy_long;
	void	*dummy_pointer;
	} static_mutex;
} PStaticMutex;

#define	P_STATIC_MUTEX_INIT		{ { { -89,-85,-86,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} } }

#define	p_static_mutex_get_mutex(mutex)	((PMutex*) ((ppointer) ((mutex)->static_mutex.pad)))

#endif /* __PPLATFORM_H__ */
