#ifndef __IMCOMM__H__
#define __IMCOMM__H__

#include "dictionary.h"

struct IMComm_s;
typedef int (*Connect_f)(struct IMComm_s *);

/* The interface for an MComm.
 * An MComm communicates with an IM network.
 * This is the framework-public interface. The methods within are 
 * likely to be called from across different threads.
 * As such, it is wise to create a seperate thread 
 * (multiple, in fact) for actual IM network communication, and
 * make these public APIs message through to it, or use locking. */
typedef struct IMComm_s
{
	Connect_f fnConnect;

	char *pszName;
	char *pszStatus;

	char szConfigFile[255]; /* implementation-specific configuration file */
	Dictionary_t *dictConfig; /* store implementation-specific config here */
	void *pvPrivate; /* implementation-specific private storage */
} IMComm_t;

#endif
