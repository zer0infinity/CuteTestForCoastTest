/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef BackendConfigLoader_H
#define BackendConfigLoader_H

#include "config_BackendConfigLoader.h"
#include "WDModule.h"
#include "Threads.h"

class Context;

//---- BackendConfigLoaderModule -----------------------------------------------------------

class EXPORTDECL_BACKENDCONFIGLOADER BackendConfigLoaderModule : public WDModule
{
	friend class BackendConfigLoaderTest;
public:
	BackendConfigLoaderModule(const char *name);
	~BackendConfigLoaderModule();

	virtual bool Init(const Anything &config);
	virtual bool Finis();

protected:

	Anything fLogConnections;
	ROAnything fROLogConnections;

	static BackendConfigLoaderModule *fgBackendConfigLoaderModule;
};

#endif
