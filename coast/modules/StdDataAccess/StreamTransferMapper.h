/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StreamTransferMapper_H
#define _StreamTransferMapper_H

//---- Mapper include -------------------------------------------------
#include "Mapper.h"

//---- StreamTransferMapper ----------------------------------------------------------
//! transfer stream of DataAccessImpl to the stream in the context
//! future versions might incorporate post processing which we ignore
//! in the moment
class StreamTransferMapper : public ResultMapper
{
public:
	//--- constructors
	StreamTransferMapper(const char *name) : ResultMapper(name) {};
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) StreamTransferMapper(fName);
	};

	/*! @copydoc ResultMapper::DoPutStream(const char *, std::istream &, Context &, ROAnything) */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config);
};

#endif
