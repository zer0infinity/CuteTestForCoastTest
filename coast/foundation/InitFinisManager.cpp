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
#include "ITOString.h"

//--- c-modules used -----------------------------------------------------------

//---- InitFinisManager ----------------------------------------------------------
InitFinisManager::InitFinisManager(unsigned int uiPriority, const String name)
	: fNext(0)
	, fPriority(uiPriority)
	, fName(name)
{
}

InitFinisManager::~InitFinisManager()
{
	String msg;
	msg << "InitFinisManager::Finis: calling Delete:  [" << GetName() << "]\n";
	IFMTrace(msg);
	delete fNext;
}

void InitFinisManager::Init()
{
	String msg;
	msg << "InitFinisManager::Finis: calling DoInit:  [" << GetName() << "]\n";
	IFMTrace(msg);
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
	String msg;
	msg << "InitFinisManager::Finis: calling DoFinis: [" << GetName() << "]\n";
	IFMTrace(msg);
	DoFinis();
}

void InitFinisManager::Add(InitFinisManager *pManager, InitFinisManager *pCleaner)
{
	// do not add manager itself to the list of destructible elements
	if ( pManager != 0 && pCleaner != 0 && pManager != pCleaner ) {
		String msg;
		msg << "InitFinisManager::Add:    adding object: [" << pManager->GetName() << "]\n";
		IFMTrace(msg);
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

const String InitFinisManager::GetName()
{
	return fName;
}
