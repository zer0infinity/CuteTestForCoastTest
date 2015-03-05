/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FilterFieldsMapper_H
#define _FilterFieldsMapper_H

#include "Mapper.h"

//---- FilterFieldsMapper ----------------------------------------------------------
//! Mapper to filter out unwanted named slots from within the value Anything to put
/*!
\par Configuration
\code
{
	/FieldList	{				optional, default all slots, define subset of slots to put with mapper
		...			String
	}
	/Filters {

	}
}
\endcode
*/
class FilterFieldsMapper: public ResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	FilterFieldsMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) FilterFieldsMapper(fName);
	}

protected:
	/*! @copydoc ResultMapper::DoPutAny(const char *, Anything &, Context &, ROAnything) */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);
};

#endif
