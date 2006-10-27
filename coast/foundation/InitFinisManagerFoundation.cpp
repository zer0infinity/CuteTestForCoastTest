/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "InitFinisManagerFoundation.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------

//--- c-modules used -----------------------------------------------------------

//---- InitFinisManagerFoundation ----------------------------------------------------------------
InitFinisManagerFoundation *InitFinisManagerFoundation::fgpInitFinisManagerFoundation = 0;

InitFinisManagerFoundation::InitFinisManagerFoundation(unsigned int uiPriority)
	: InitFinisManager(uiPriority)
{
	// ensure library specific instance - do not enter if we are in construction of the singleton object
	if ( Instance() && fgpInitFinisManagerFoundation != (InitFinisManagerFoundation *)0x123 ) {
		Add(fgpInitFinisManagerFoundation, this);
	}
}

InitFinisManagerFoundation *InitFinisManagerFoundation::Instance()
{
	if ( fgpInitFinisManagerFoundation == 0 && fgpInitFinisManagerFoundation != (InitFinisManagerFoundation *)0x123 ) {
		// ensure we do not loop while creating the singleton
		IFMTrace("creating InitFinisManagerFoundation instance\n");
		fgpInitFinisManagerFoundation = (InitFinisManagerFoundation *)0x123;
		fgpInitFinisManagerFoundation = new InitFinisManagerFoundation(0);
	}
	return fgpInitFinisManagerFoundation;
}
