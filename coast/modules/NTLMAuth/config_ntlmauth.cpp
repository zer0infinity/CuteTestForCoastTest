/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "InitFinisManager.h"

static void Init()
{
	InitFinisManager::IFMTrace(">> ntlmauth::Init\n");
	InitFinisManager::IFMTrace("<< ntlmauth::Init\n");
}

static void Finis()
{
	InitFinisManager::IFMTrace(">> ntlmauth::Finis\n");
	InitFinisManager::IFMTrace("<< ntlmauth::Finis\n");
}

extern "C" void __attribute__ ((constructor)) ntlmauth_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) ntlmauth_fini()
{
	Finis();
}
