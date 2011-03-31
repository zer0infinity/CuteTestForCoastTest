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
		InitFinisManager::IFMTrace(">> ssl::Init\n");
		InitFinisManager::IFMTrace("<< ssl::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> ssl::Finis\n");
		InitFinisManager::IFMTrace("<< ssl::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) ssl_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) ssl_fini()
{
	Finis();
}
