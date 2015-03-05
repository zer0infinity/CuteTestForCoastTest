/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MapperTestDAImpl_H
#define _MapperTestDAImpl_H

#include "DataAccessImpl.h"

//---- MapperTestDAImpl ----------------------------------------------------------
//! This DataAccessImpl can be used to test ParameterMapper or ResultMapper implementations
/*! This data access implementation uses ParameterMapper.Get() and ResultMapper.Put() of a specified type to put through the requested values.
 * The structure of Configuration is basically adapted from LoopbackDAImpl
 * @todo Merge with LoopBackDAImpl
@code
{
	/transfer {					config entry to specify slots to transfer
		/FromSlot1	ToSlot1		using ParameterMapper->Get to lookup FromSlot and
								putting it using ResultMapper->Put to store it at ToSlot
		/FromSlot2	ToSlot2		...
		{ /FromSlot ToSlot1 }	alternate form when wanting to use the same get key multiple times
		{ /FromSlot ToSlot2 }
	}
	/DataType					optional, default String, specifies the Get/Put methods used
								String: 	use Get(FromSlot, String, Context), Put(ToSlot, String, Context)
								int: 		use Get(FromSlot, int, Context), Put(ToSlot, int, Context)
								long: 		use Get(FromSlot, long, Context), Put(ToSlot, long, Context)
								bool: 		use Get(FromSlot, bool, Context), Put(ToSlot, bool, Context)
								float: 		use Get(FromSlot, float, Context), Put(ToSlot, float, Context)
								double: 	use Get(FromSlot, double, Context), Put(ToSlot, double, Context)
								Anything: 	use Get(FromSlot, Anything, Context), Put(ToSlot, Anything, Context)
								StringStream: use Get(FromSlot, StringStream, Context), Put(ToSlot, StringStream, Context)
}
@endcode
 */
class MapperTestDAImpl: public DataAccessImpl {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	MapperTestDAImpl(const char *name) :
		DataAccessImpl(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MapperTestDAImpl(fName);
	}

protected:
	//! Loopback like implementation of the data access
	/*! @copydetails DataAccessImpl::Exec() */
	virtual bool Exec(Context &ctx, ParameterMapper *input, ResultMapper *output);
};

#endif
