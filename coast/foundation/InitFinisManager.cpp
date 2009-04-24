/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "InitFinisManager.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"

//--- c-modules used -----------------------------------------------------------
#include <cstring>
#if defined(__SUNPRO_CC)
#include <strings.h>
#include <stdlib.h>
#endif

//---- InitFinisManager ----------------------------------------------------------
InitFinisManager::InitFinisManager(unsigned int uiPriority)
	: fNext(0)
	, fPriority(uiPriority)
{
}

InitFinisManager::~InitFinisManager()
{
	delete fNext;
}

void InitFinisManager::Init()
{
	DoInit();
	if ( fNext ) {
		fNext->Init();
	}
}

void InitFinisManager::Finis()
{
	if ( fNext ) {
		fNext->Finis();
	}
	DoFinis();
}

void InitFinisManager::Add(InitFinisManager *pManager, InitFinisManager *pCleaner)
{
	// do not add manager itself to the list of destructible elements
	if ( pManager != 0 && pCleaner != 0 && pManager != pCleaner ) {
		IFMTrace("InitFinisManager::Add: adding object\n");
		InitFinisManager *t = pManager, *n = pManager->fNext;
		// find position where to insert the object based on its priority
		while ( n != 0 && pCleaner->fPriority >= n->fPriority ) {
			t = n;
			n = t->fNext;
		}
		t->fNext = pCleaner;
		pCleaner->fNext = n;
	}
}

void InitFinisManager::IFMTrace(const char *pMsg)
{
	static const char *pEnvVar = getenv("TRACE_INITFINIS");
	static bool bTrace = ( pEnvVar != 0 && strcmp(pEnvVar, "1") == 0 );
	if ( bTrace ) {
		SysLog::WriteToStderr(pMsg);
	}
}
