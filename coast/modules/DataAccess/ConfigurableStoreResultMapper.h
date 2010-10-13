/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConfigurableStoreResultMapper_H
#define _ConfigurableStoreResultMapper_H

#include "config_dataaccess.h"

//---- Mapper include -------------------------------------------------
#include "Mapper.h"

//---- ConfigurableStoreResultMapper ----------------------------------------------------------

class EXPORTDECL_DATAACCESS ConfigurableStoreResultMapper : public ResultMapper
{
public:
	/*! constructor
		\param name the objects name */
	ConfigurableStoreResultMapper(const char *name);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

protected:
	//! Looks up the Anything at key in Context using Slotfinder
	/*!	\param key the key usually defines the associated kind of output-value
		\param targetAny Anything reference into TmpStore to finally put values at. It uses DestinationSlot and key to get the correct location in Context.
		\param ctx the context of the invocation */
	virtual void DoGetDestinationAny(const char *key, Anything &targetAny, Context &ctx);

private:
	ConfigurableStoreResultMapper();
	ConfigurableStoreResultMapper(const ConfigurableStoreResultMapper &);
	ConfigurableStoreResultMapper &operator=(const ConfigurableStoreResultMapper &);
};

#endif
