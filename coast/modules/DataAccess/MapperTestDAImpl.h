/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MapperTestDAImpl_H
#define _MapperTestDAImpl_H

//---- baseclass include -------------------------------------------------
#include "config_dataaccess.h"
#include "DataAccessImpl.h"

//---- MapperTestDAImpl ----------------------------------------------------------
// This DAImpl can be used to test Mappers, it uses executes a Get and a Put of a specified type.
// The structure of Configuration is basically adapted from LoopbackDAImpl
//	Structure of config:
//<PRE>	{
//		/transfer {					config entry to specify slots to transfer
//			/FromSlot1	ToSlot1		using InputMapper->Get to lookup FromSlot and
//									putting it using OutputMapper->Put to store it at ToSlot
//			/FromSlot2	ToSlot2		...
//			...
//		}
//		/DataType					optional, default String, specifies the Get/Put methods used
//									String: 	use Get(FromSlot, String, Context), Put(ToSlot, String, Context)
//									int: 		use Get(FromSlot, int, Context), Put(ToSlot, int, Context)
//									long: 		use Get(FromSlot, long, Context), Put(ToSlot, long, Context)
//									bool: 		use Get(FromSlot, bool, Context), Put(ToSlot, bool, Context)
//									float: 		use Get(FromSlot, float, Context), Put(ToSlot, float, Context)
//									double: 	use Get(FromSlot, double, Context), Put(ToSlot, double, Context)
//									Anything: 	use Get(FromSlot, Anything, Context), Put(ToSlot, Anything, Context)
//									Anything2: 	use Get(FromSlot, Anything, Context, ROAnything()), Put(ToSlot, Anything, Context)
//									StringStream: use Get(FromSlot, StringStream, Context), Put(ToSlot, StringStream, Context)
//									StringStream2: use Get(FromSlot, StringStream, Context, ROAnything()), Put(ToSlot, StringStream, Context)
//	}
class EXPORTDECL_DATAACCESS MapperTestDAImpl : public DataAccessImpl
{
public:
	//--- constructors
	MapperTestDAImpl(const char *name);
	~MapperTestDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, InputMapper *, OutputMapper *);

private:
	//constructor
	MapperTestDAImpl();
	MapperTestDAImpl(const MapperTestDAImpl &);
	//assignement
	MapperTestDAImpl &operator=(const MapperTestDAImpl &);
};

#endif
