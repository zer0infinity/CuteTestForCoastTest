/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URI2FileNameMapper_H
#define _URI2FileNameMapper_H

//---- Mapper include -------------------------------------------------
#include "Mapper.h"

//---- URI2FileNameMapper ----------------------------------------------------------
//! <B>really brief class description</B><BR>Configuration:
//! <PRE>
//! {
//!		/Slot1		[mandatory|optional], ...
//!		...
//! }
//! </PRE>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class URI2FileNameMapper : public ParameterMapper
{
public:
	//--- constructors
	URI2FileNameMapper(const char *name) : ParameterMapper(name) {};
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) URI2FileNameMapper(fName);
	};

	bool DoFinalGetAny(const char *key, Anything &value, Context &ctx);

protected:
	//! cut off trailing arguments after ? and remove the service name prefix
	//! as known from the context, if any
	void TrimUriArguments(String &filename, Context &ctx);
	virtual bool ResolveInvalidFile(String &path, String &uri, String &value, Context &ctx);

private:
	URI2FileNameMapper();
	URI2FileNameMapper(const URI2FileNameMapper &);
	URI2FileNameMapper &operator=(const URI2FileNameMapper &);
};

#endif
