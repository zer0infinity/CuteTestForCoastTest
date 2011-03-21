/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConfigurableStoreResultMapper_H
#define _ConfigurableStoreResultMapper_H

#include "Mapper.h"

//---- ConfigurableStoreResultMapper ----------------------------------------------------------
//! Store \c Put requests into a store inside Context different from the default \c TmpStore.
/*! Sometimes it would be nice to store/map values directly into a store different from TmpStore, like the Session or Role store.
 * This is where this ResultMapper can be used as it just retrieves the target Anything within the store given in the mapper configuration.
 * @section csrm Mapper configuration
\code
{
	/Store 			String
}
\endcode
 * @par \c Store
 * Optional, defaults to TmpStore\n
 * Store the rendered key in a specific store in the context: \c Role -> RoleStore, \c Session -> SessionStore, \c Request
 */
class ConfigurableStoreResultMapper: public ResultMapper {
	ConfigurableStoreResultMapper();
	ConfigurableStoreResultMapper(const ConfigurableStoreResultMapper &);
	ConfigurableStoreResultMapper &operator=(const ConfigurableStoreResultMapper &);
public:
	/*! constructor
	 \param name the objects name */
	ConfigurableStoreResultMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) ConfigurableStoreResultMapper(fName);
	}

protected:
	//! Looks up the destination Anything at key in Context using StoreFinder
	/*! @copydoc ResultMapper::DoGetDestinationAny(const char *, Anything &, Context &) */
	virtual void DoGetDestinationAny(const char *key, Anything &targetAny, Context &ctx);
};

#endif
