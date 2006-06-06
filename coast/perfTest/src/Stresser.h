/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Stresser_h_
#define _Stresser_h_

#include "config_perftest.h"
#include "WDModule.h"

//---- StressersModule -----------------------------------------------------------
class EXPORTDECL_PERFTEST StressersModule : public WDModule
{
public:
	StressersModule(const char *);
	~StressersModule();

	virtual bool Init(const ROAnything config);
	virtual bool Finis();
};

//---- Stresser -----------------------------------------------------------
class EXPORTDECL_PERFTEST Stresser : public ConfNamedObject
{
public:
	Stresser(const char *StresserName);
	~Stresser();

	//! the stressers main method. It performs the actual stress test
	//! \return an Anything containing collected data
	virtual Anything Run(long id) = 0;

	static Anything RunStresser(const String &StresserName, long id = 0);

//---- registry interface
	RegCacheDef(Stresser);	// FindStresser()

protected:
	// generate the config file name (without extension, which is assumed to be any)
	// out of category and objName
	// this implementation just returns "StresserMeta"
	virtual bool DoGetConfigName(const char *category, const char *objName, String &configFileName);

	//! loads an anything and make it available by storing a reference in fConfig.
	//! Using the cache handler, the file that name is defined with DoGetConfigName is loaded.
	//! Out of this the slot that matches the Stresser's name is taken as its configuration
	virtual bool DoLoadConfig(const char *category);

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	Stresser();
	Stresser(const Stresser &);
	Stresser &operator=(const Stresser &);

};

#define RegisterStresser(name) RegisterObject(name, Stresser)

#endif
