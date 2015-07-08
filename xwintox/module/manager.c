#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"

#include "module/manager.h"

ModuleManager_t *pmmManager;

ModuleManager_t *ModuleManager_getInstance()
{
	static ModuleManager_t mmManager;

	return &mmManager;
}

void ModuleManager_init()
{
	pmmManager =ModuleManager_getInstance();

	pmmManager->lstpmodModules =List_new();
	pmmManager->lstpobjWildcards =List_new();
	pmmManager->dictpobjObjects =Dictionary_new();

	pmmManager->psrvServices.uiVersion =1;
	pmmManager->psrvServices.fnRegisterObj =ModuleManager_registerObject;
	pmmManager->psrvServices.fnCall =ModuleManager_call;
}

int ModuleManager_loadDynamicModule(const char *pszPath)
{
	XWF_Module_t *pmodNew =malloc(sizeof(XWF_Module_t));
	const char *pszDLError;
	XWF_Init_f fnInit;

	pmodNew->enModtype =XWF_Dynamic;
	pmodNew->hdlLib =dlopen(pszPath, RTLD_LAZY | RTLD_GLOBAL);
	if ((pszDLError =dlerror()) != NULL) goto dlerror;

	fnInit =dlsym(pmodNew->hdlLib, "XWF_init");
	if ((pszDLError =dlerror()) != NULL) goto dlerror;

	if (ModuleManager_initialiseModule(pmodNew, fnInit) != 0) goto error;

	return 0;

dlerror:
	dbg("Failed to load dynamic module (%s)\n", pszDLError);
error:
	dbg("Unloading dynamic module (%s) due to error.\n", pszPath);
	free(pmodNew);
	return -1;
}

int ModuleManager_initialiseModule(XWF_Module_t *pmodNew, XWF_Init_f fnInit)
{
	int iRet =fnInit(pmodNew, &pmmManager->psrvServices);

	if(!iRet)
	{
		dbg("Module <%s> loaded successfully.\n", pmodNew->pszName);
		return 0;
	}
	else if(iRet == 1)
	{
		dbg("Module <%s> loaded with errors.\n", pmodNew->pszName);
		return 0;
	}
	else
	{
		dbg("Module failed to load.\n");
		return -1;
	}
}

int ModuleManager_registerObject(const XWF_Object_t *pobjRegistered)
{
	if(strcmp(pobjRegistered->pszType, "*") == 0)
	{
		dbg("Adding new wildcard object\n");
		List_add(pmmManager->lstpobjWildcards, (void*)pobjRegistered);
		return 0;
	}
	else if(Dictionary_get(pmmManager->dictpobjObjects, pobjRegistered->pszType))
	{
		dbg("Object already exists: %s\n", pobjRegistered->pszType);
		return -1;
	}
	else
	{
		dbg("Adding new object: %s\n", pobjRegistered->pszType);
		return 0;
	}
}

int ModuleManager_call(const char *pszService, void *pvParams)
{
	if(strncmp(pszService, "SYSTEM", 6) == 0)
	{
		dbg("System call: %s\n", pszService);
		return 0;
	}

	return -1;
}
