/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ConfiguredTestCase.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SysLog.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------

//--- c-library modules used ---------------------------------------------------

//---- ConfiguredTestCase ---------------------------------------------------------------
ConfiguredTestCase::ConfiguredTestCase(TString tname, TString configFileName)
	: TestCase(tname), fConfigFileName(configFileName)
{
	StartTrace(ConfiguredTestCase.Ctor);
}

ConfiguredTestCase::~ConfiguredTestCase()
{
	StartTrace(ConfiguredTestCase.Dtor);
}

// setup for this TestCase
void ConfiguredTestCase::setUp ()
{
	StartTrace(ConfiguredTestCase.setUp);

	fConfig = LoadConfigFile(fConfigFileName);
	TraceAny(fConfig, "whole Config of [" << fConfigFileName << "]");

	fTestCaseConfig = fConfig[name()];
	TraceAny(fTestCaseConfig, "config of TestCase [" << name() << "]");
} // setUp

void ConfiguredTestCase::tearDown ()
{
	StartTrace(ConfiguredTestCase.tearDown);

	// force deletion of Config
	Anything dummy;
	fTestCaseConfig = dummy;
	fConfig = dummy;
} // tearDown

Anything ConfiguredTestCase::LoadConfigFile(const TString &configFileName)
{
	StartTrace(ConfiguredTestCase.LoadConfigFile);

	iostream *fp = System::OpenStream(configFileName, "any");
	Trace("Loading file " << configFileName);

	Anything result;
	if (!fp) {
		SysLog::Error(String("Open ") << configFileName << ".any for reading failed");
	} else {
		t_assert(result.Import(*fp, configFileName));
	}
	delete fp;

	return result;
}

void ConfiguredTestCase::PutInStore(const Anything &source, Anything &dest)
{
	StartTrace(ConfiguredTestCase.PutInStore);

	long sz = source.GetSize();
	for (long i = 0; i < sz; i++) {
		dest[source.SlotName(i)] = source[i].DeepClone();
	}
}
