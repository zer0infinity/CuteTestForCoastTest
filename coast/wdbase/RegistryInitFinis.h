/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------
#include "Registry.h"

//---- RegistryInitFinis -----------------------------------------------------------------------
class EXPORTDECL_WDBASE RegistryInitFinis : public InitFinisManagerWDBase
{
public:
	RegistryInitFinis(unsigned int uiPriority)
		: InitFinisManagerWDBase(uiPriority) {
		IFMTrace("RegistryInitFinis created\n");
		Registry::GetRegTable();
		Registry::GetRegROTable();
	}

	~RegistryInitFinis()
	{}

	virtual void DoInit() {
		IFMTrace("RegistryInitFinis::DoInit\n");
	}

	virtual void DoFinis() {
		IFMTrace("RegistryInitFinis::DoFinis\n");
		// do not create new registries anymore

		Registry::FinalizeRegArray(*Registry::fgRegistryArray);
		delete Registry::fgRegistryArray;
		Registry::fgRegistryArray = 0;

		delete Registry::fgRORegistryArray;
		Registry::fgRORegistryArray = 0;

		Registry::SetFinalize(true);
	}
};

Anything *Registry::fgRegistryArray = 0;
ROAnything *Registry::fgRORegistryArray = 0;
bool Registry::fgFinalize = false;
static RegistryInitFinis *psgRegistryInitCleanup = new RegistryInitFinis(0);
