/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LOCALIZEDSTRINGS_H
#define _LOCALIZEDSTRINGS_H

#include "config_wdbase.h"
#include "Threads.h"
#include "WDModule.h"

//---- LocalizationModule -----------------------------------------------------------
class EXPORTDECL_WDBASE LocalizationModule : public WDModule
{
	Anything fLocalizedStrings;
	bool ReadFromFile(Anything &config, const char *filename);
public:
	LocalizationModule(const char *name);
	~LocalizationModule();

	virtual bool Init(const Anything &config);
	virtual bool Finis();
};

//---- LocalizedStrings -----------------------------------------------------------
class EXPORTDECL_WDBASE LocalizedStrings
{
public:
	LocalizedStrings();
	~LocalizedStrings();

	static LocalizedStrings *LocStr();

	bool Lookup(const char *key, ROAnything &result, char delim = '.', char indexdelim = ':') const;

protected:
	ROAnything fLocalizedStrings; // initialized by LocalizationModule

	static LocalizedStrings *fgLocStrings;	// the singleton
	static Mutex fgLocMutex;				// mutex used for initialization

	friend class LocalizationModule;

private:
	// don't use these
	LocalizedStrings(const LocalizedStrings &);
	LocalizedStrings &operator=(const LocalizedStrings &);
};

#endif		//ifndef _LOCALIZEDSTRINGS_H
