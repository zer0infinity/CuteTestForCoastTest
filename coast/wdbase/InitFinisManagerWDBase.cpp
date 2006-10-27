/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "InitFinisManagerWDBase.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------

//--- c-modules used -----------------------------------------------------------

//---- InitFinisManagerWDBase ----------------------------------------------------------------
InitFinisManagerWDBase *InitFinisManagerWDBase::fgpInitFinisManagerWDBase = 0;

InitFinisManagerWDBase::InitFinisManagerWDBase(unsigned int uiPriority)
	: InitFinisManager(uiPriority)
{
	// ensure library specific instance - do not enter if we are in construction of the singleton object
	if ( Instance() && fgpInitFinisManagerWDBase != (InitFinisManagerWDBase *)0x123 ) {
		IFMTrace("Adding this to fgpInitFinisManagerWDBase\n");
		Add(fgpInitFinisManagerWDBase, this);
	}
}

InitFinisManagerWDBase *InitFinisManagerWDBase::Instance()
{
	if ( fgpInitFinisManagerWDBase == 0 && fgpInitFinisManagerWDBase != (InitFinisManagerWDBase *)0x123 ) {
		// ensure we do not loop while creating the singleton
		IFMTrace("creating InitFinisManagerWDBase instance\n");
		fgpInitFinisManagerWDBase = (InitFinisManagerWDBase *)0x123;
		fgpInitFinisManagerWDBase = new InitFinisManagerWDBase(0);
	}
	return fgpInitFinisManagerWDBase;
}
