/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LocalizedStrings.h"
#include "SystemLog.h"
#include "SystemFile.h"

//---- LocalizedStrings ---------------------------------------------------------
LocalizedStrings *LocalizedStrings::fgLocStrings = 0;
Mutex LocalizedStrings::fgLocMutex("LocalizedStrings");

LocalizedStrings::LocalizedStrings()
{
}

LocalizedStrings::~LocalizedStrings()
{
}

LocalizedStrings *LocalizedStrings::LocStr()
{
	StartTrace(LocalizedStrings.LocStr);
	if (! fgLocStrings ) {
		LockUnlockEntry me(fgLocMutex);
		if (! fgLocStrings ) {
			Trace("creating new instance");
			fgLocStrings = new LocalizedStrings();
		}
	}
	return fgLocStrings;
}

bool LocalizedStrings::Lookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(LocalizedStrings.Lookup, "key [" << NotNull(key) << "]");
	return fLocalizedStrings.LookupPath(result, key, delim, indexdelim);
}

//---- LocalizationModule -----------------------------------------------------------
RegisterModule(LocalizationModule);

LocalizationModule::LocalizationModule(const char *name) : WDModule(name)
{
	StartTrace(LocalizationModule.LocalizationModule);
}

LocalizationModule::~LocalizationModule()
{
	StartTrace(LocalizationModule.~LocalizationModule);
}

bool LocalizationModule::Init(const ROAnything config)
{
	StartTrace(LocalizationModule.Init);
	// add localized strings
	if (config.IsDefined("StringFile")) {
		Trace("loading StringFile [" << NotNull(config["StringFile"].AsCharPtr()) << "]");
		if (!ReadFromFile(fLocalizedStrings, config["StringFile"].AsCharPtr())) {
			return false;
		}
	}
	LocalizedStrings *theHandle = LocalizedStrings::LocStr();
	Assert(theHandle);
	if (theHandle) {
		theHandle->fLocalizedStrings = fLocalizedStrings;
	}
	return 0 != theHandle;
}

bool LocalizationModule::Finis()
{
	StartTrace(LocalizationModule.Finis);
	{
		LockUnlockEntry me(LocalizedStrings::fgLocMutex);
		if ( LocalizedStrings::fgLocStrings ) {
			Trace("deleting global instance");
			delete LocalizedStrings::fgLocStrings;
			LocalizedStrings::fgLocStrings = 0;
		}
		fLocalizedStrings = Anything();
	}
	return true;
}

bool LocalizationModule::ReadFromFile(Anything &config, const char *filename)
{
	StartTrace(LocalizationModule.ReadFromFile);
	std::istream *ifp = Coast::System::OpenStream(filename, "any");
	if (ifp == 0) {
		String logMsg;
		SystemLog::Error(logMsg << "LocalizationModule::ReadFromFile: can't open file " << filename << ".any");
		return false;
	}
	config.Import(*ifp);
	delete ifp;
	return true;
}
