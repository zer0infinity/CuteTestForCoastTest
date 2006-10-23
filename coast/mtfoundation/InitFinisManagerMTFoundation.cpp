/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "InitFinisManagerMTFoundation.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------

//--- c-modules used -----------------------------------------------------------

//---- InitFinisManagerMTFoundation ----------------------------------------------------------------
InitFinisManagerMTFoundation *InitFinisManagerMTFoundation::fgpInitFinisManagerMTFoundation = 0;

InitFinisManagerMTFoundation::InitFinisManagerMTFoundation(unsigned int uiPriority, const String name)
	: InitFinisManager(uiPriority, name)
{
	// ensure library specific instance - do not enter if we are in construction of the singleton object
	if ( Instance() && fgpInitFinisManagerMTFoundation != (InitFinisManagerMTFoundation *)0x123 ) {
		Add(fgpInitFinisManagerMTFoundation, this);
	}
}

InitFinisManagerMTFoundation *InitFinisManagerMTFoundation::Instance()
{
	if ( fgpInitFinisManagerMTFoundation == 0 && fgpInitFinisManagerMTFoundation != (InitFinisManagerMTFoundation *)0x123 ) {
		// ensure we do not loop while creating the singleton
		IFMTrace("creating InitFinisManagerMTFoundation instance\n");
		fgpInitFinisManagerMTFoundation = (InitFinisManagerMTFoundation *)0x123;
		fgpInitFinisManagerMTFoundation = new InitFinisManagerMTFoundation(0, "InitFinisManagerMTFoundation");
	}
	return fgpInitFinisManagerMTFoundation;
}
