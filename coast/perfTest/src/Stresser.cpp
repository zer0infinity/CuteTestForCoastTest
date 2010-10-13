/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Stresser.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "DiffTimer.h"
#include "SystemLog.h"
#include "Dbg.h"

//---- Stresser -----------------------------------------------------------
Stresser::Stresser(const char *StresserName)
	: ConfNamedObject(StresserName)
{
	StartTrace(Stresser.Stresser);
}

Stresser::~Stresser()
{
}

//---- registry interface
RegCacheImpl(Stresser);	// FindStresser()

Anything Stresser::RunStresser(const String &StresserName, long id)
{
	StartTrace1(Stresser.RunStresser, "StresserName [" << StresserName << "]");
	Stresser *stresserPrototyp = Stresser::FindStresser(StresserName);
	if (stresserPrototyp) {
		return stresserPrototyp->Run(id);
	}
	Trace("Stresser not found " << StresserName);
	Anything anyError, anyRet;
	anyError["Error"] = 1;
	anyError["ErrorMessageCtr"]["1"][0L] = String("Stresser not found [") << StresserName << "]";
	anyRet.Append(anyError);
	return anyRet;
}

bool Stresser::DoGetConfigName(const char *category, const char *objName, String &configFileName) const
{
	configFileName = "StresserMeta";
	return true;
}

bool Stresser::DoLoadConfig(const char *category)
{
	StartTrace1(Stresser.DoLoadConfig, "fName [" << fName << "]");

	if ( ConfNamedObject::DoLoadConfig(category) && fConfig.IsDefined(fName) ) {
		// trx impls use only a subset of the whole configuration file
		fConfig = fConfig[fName];
		TraceAny(fConfig, "Config:");
		return (!fConfig.IsNull());
	}
	fConfig = ROAnything();
	SystemLog::Info(String("Stresser::DoLoadConfig: no configuration entry for <") << fName << "> defined, still returning true");
	return true;
}

// Provide a dummy Stresser that only stresses itself to do some testing with it
class EXPORTDECL_PERFTEST DummyStresser: public Stresser
{
public:
	DummyStresser(const char *StresserName)
		: Stresser(StresserName) {
		StartTrace(DummyStresser.Ctor);
	};
	~DummyStresser() {
		StartTrace(DummyStresser.Destructor);
	};

	Anything Run(long id);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) DummyStresser(fName);
	}
};

RegisterStresser(DummyStresser);

Anything DummyStresser::Run(long id)
{
	StartTrace(DummyStresser.Run);

	int number(0);
	Anything result, retResult;
	DiffTimer timer;
	for (long i = 0; i < 50000; i++) {
		number++;
	}
	result["Sum"] = (long)timer.Diff();
	result["Nr"] = 1;
	result["Steps"] = number;
	result["Max"] = 1;
	result["Min"] = 1;
	result["Error"] = 0;
	retResult["Results"].Append(result);
	return retResult;
}

//---- StressersModule -----------------------------------------------------------
RegisterModule(StressersModule);

StressersModule::StressersModule(const char *name)
	: WDModule(name)
{
}

StressersModule::~StressersModule()
{
}

bool StressersModule::Init(const ROAnything config)
{
	StartTrace(StressersModule.Init);
	TraceAny(config, "Config");

	if (config.IsDefined("Stressers")) {
		AliasInstaller ai("Stresser");
		return RegisterableObject::Install(config["Stressers"], "Stresser", &ai);
	}
	return false;
}

bool StressersModule::Finis()
{
	return StdFinis("Stresser", "Stressers");
}
