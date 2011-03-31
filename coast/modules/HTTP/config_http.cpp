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
		InitFinisManager::IFMTrace(">> http::Init\n");
		InitFinisManager::IFMTrace("<< http::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> http::Finis\n");
		InitFinisManager::IFMTrace("<< http::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) http_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) http_fini()
{
	Finis();
}
