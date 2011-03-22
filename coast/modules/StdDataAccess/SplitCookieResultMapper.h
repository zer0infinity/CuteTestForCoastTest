/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef SPLITCOOKIERESULTMAPPER_H_
#define SPLITCOOKIERESULTMAPPER_H_

#include "Mapper.h"

class SplitCookieResultMapper : public ResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	SplitCookieResultMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) SplitCookieResultMapper(fName);
	}
protected:
	//! Split cookie(s) and Put
	/*! @copydoc ResultMapper::DoPutAny() */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);
};

#endif /* SPLITCOOKIERESULTMAPPER_H_ */
