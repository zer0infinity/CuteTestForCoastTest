/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------
#include "config_bases.h"

#if defined(WIN32)
#include "TestAction.h"
#include "TestRunnerApplication.h"
void RegisterBasesObjs()
{
	RegisterAction(TestAction);
	RegisterAction(ConfiguredTestAction);
	RegisterApplication(TestRunnerApplication);
}
#endif	// WIN32
