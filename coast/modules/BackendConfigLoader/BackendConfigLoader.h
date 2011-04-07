/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef BackendConfigLoader_H
#define BackendConfigLoader_H

#include "WDModule.h"

class BackendConfigLoaderModule: public WDModule {
	bool RegisterBackend(const String& backendName, ROAnything roaBackendConfig);
	Anything GetBackendList();
	static Anything backendConfigurations;
	friend class BackendConfigLoaderTest;

public:
	BackendConfigLoaderModule(const char *name) :
		WDModule(name) {
	}
	static ROAnything GetBackendConfig(const String &backendName);
	static ROAnything GetBackendConfig();

protected:
	virtual bool Init(const ROAnything config);
	virtual bool Finis();
};

#endif
