/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ROAnyLookupAdapter_H
#define _ROAnyLookupAdapter_H

#include "LookupInterface.h"

//---- ROAnyLookupAdapter ----------------------------------------------------------
//! <B>Wraps ROAnything in a LookupInterface (as needed eg. by Context::Push())</B>
class ROAnyLookupAdapter : public virtual LookupInterface
{
public:
	ROAnyLookupAdapter(ROAnything ro): fRoAny(ro) {}
	~ROAnyLookupAdapter() {}
protected:
	ROAnything fRoAny;

	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
		return fRoAny.LookupPath(result, key, delim, indexdelim);
	}
};

#endif
