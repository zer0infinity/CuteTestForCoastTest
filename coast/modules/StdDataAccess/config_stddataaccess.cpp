/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */


#include "InitFinisManager.h"

namespace {
	void Init()
	{
		InitFinisManager::IFMTrace(">> stddataaccess::Init\n");
		InitFinisManager::IFMTrace("<< stddataaccess::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> stddataaccess::Finis\n");
		InitFinisManager::IFMTrace("<< stddataaccess::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) stddataaccess_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) stddataaccess_fini()
{
	Finis();
}
