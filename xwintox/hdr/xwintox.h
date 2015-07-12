#ifndef __XWINTOX_H__
#define __XWINTOX_H__

#include "AOM/IMComm.h"
#include "AOM/IGUI.h"

typedef struct XWT_s
{
	Dictionary_t *dictConfig;
	XWF_Object_Handle_t *pimcIM;
	XWF_Object_Handle_t *pguiGUI;
} XWT_t;

typedef enum XWMtypes_e
{
	/* Client related */
	clCon,

	/* Friends related */
	frCon,
	frName,
	frStatusMsg,
	frRequest,
	frAdded,

	/* Chatrooms related */
	crNew,
	crTitle,
	crNames,
	crMessage,
	crInvite,

	/* File transfers related */
	ftRequest,
	ftControl,
	ftData,
} XWMtypes_e;

typedef struct XWContact_s
{
	unsigned int wNum, wStatus, wConnected;
	char *pszName, *pszStatus, *pszID;
	/* these allow the GUI or MCOMM to store extra data */
	void *pMCHandle, *pGUIHandle;
} XWContact_t;


#endif
