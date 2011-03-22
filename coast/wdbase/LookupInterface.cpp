/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LookupInterface.h"
#include "Anything.h"
#include "Dbg.h"

ROAnything LookupInterface::Lookup(const char *key, char delim, char indexdelim) const
{
	StartTrace1(LookupInterface.LookupRO, "key:<" << NotNull(key) << ">");
	ROAnything a;
	DoLookup(key, a, delim, indexdelim);
	return a;
}

bool LookupInterface::Lookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(LookupInterface.LookupBasic, "key: <" << NotNull(key) << ">");

	return DoLookup( key, result, delim, indexdelim );
}

const char *LookupInterface::Lookup(const char *key, const char *dflt, char delim, char indexdelim) const
{
	StartTrace1(LookupInterface.LookupChar, "key: <" << NotNull(key) << ">" << " default: " << dflt << " delim: " << delim << " indexdelim: " << indexdelim);
	ROAnything a;
	if ( DoLookup(key, a, delim, indexdelim) ) {
		return a.AsCharPtr(dflt);
	}
	return dflt;
}

long LookupInterface::Lookup(const char *key, long dflt, char delim, char indexdelim) const
{
	StartTrace1(LookupInterface.LookupLong, "key: <" << NotNull(key) << ">" << " default: " << dflt << " delim: " << delim << " indexdelim: " << indexdelim);
	ROAnything a;
	if ( DoLookup(key, a, delim, indexdelim) ) {
		return a.AsLong(dflt);
	}
	return dflt;
}

double LookupInterface::Lookup(const char *key, double dflt, char delim, char indexdelim) const
{
	StartTrace1(LookupInterface.LookupDouble, "key: <" << NotNull(key) << ">" << " default: " << dflt << " delim: " << delim << " indexdelim: " << indexdelim);
	ROAnything a;
	if ( DoLookup(key, a, delim, indexdelim) ) {
		return a.AsDouble(dflt);
	}
	return dflt;
}
