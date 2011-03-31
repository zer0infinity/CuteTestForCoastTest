/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConfiguredLookupAdapter_H
#define _ConfiguredLookupAdapter_H

#include "LookupInterface.h"
#include "Anything.h"

//---- newclass ----------------------------------------------------------
//! LookupInterface with configured default
/*!
Param1		ROAnything containing the configuration
Param2		ROAnything containing the defaults

If the configuration and the defaults have the same structure
this helper class is very handy to provide default values not found
in the configuration (Param1).
*/
class ConfiguredLookupAdapter: public LookupInterface
{
public:
	ConfiguredLookupAdapter(ROAnything config, ROAnything DefaultConfig);

	bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;
private:
	ROAnything fConfig;
	ROAnything fDefaultConfig;
};

#endif
