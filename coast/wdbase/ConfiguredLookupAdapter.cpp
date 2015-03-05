/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConfiguredLookupAdapter.h"
#include "Tracer.h"

ConfiguredLookupAdapter::ConfiguredLookupAdapter(ROAnything config, ROAnything defaultConfig)
	: fConfig(config), fDefaultConfig(defaultConfig)
{
	StartTrace(ConfiguredLookupAdapter.Ctor);
}

bool ConfiguredLookupAdapter::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace(ConfiguredLookupAdapter.Ctor);
	return (fConfig.LookupPath(result, key, delim, indexdelim)
			|| fDefaultConfig.LookupPath(result, key, delim, indexdelim));
}
