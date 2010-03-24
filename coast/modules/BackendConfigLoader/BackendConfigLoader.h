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

class Context;

//---- BackendConfigLoaderModule -----------------------------------------------------------

class EXPORTDECL_BACKENDCONFIGLOADER BackendConfigLoaderModule : public WDModule
{
	friend class BackendConfigLoaderTest;
public:
	BackendConfigLoaderModule(const char *name);
	~BackendConfigLoaderModule();

	virtual bool Init(const ROAnything config);
	virtual bool Finis();
	static ROAnything GetBackendConfig(const String &backendName);
	static ROAnything GetBackendConfig();
	static Anything GetBackendList();

protected:
	static bool RegisterBackend(String backendName);
	static bool RegisterBackends();

private:
	static Anything backendConfigurations;
	static BackendConfigLoaderModule *fgBackendConfigLoaderModule;
};

#endif
