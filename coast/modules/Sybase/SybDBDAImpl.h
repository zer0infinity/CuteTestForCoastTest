/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybDBDAImpl_H
#define _SybDBDAImpl_H

//---- DataAccessImpl include -------------------------------------------------
#include "config_sybase.h"
#include "DataAccessImpl.h"

//---- forward declaration -----------------------------------------------
class Context;

//---- SybDBDAImpl ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
class EXPORTDECL_SYBASE SybDBDAImpl : public DataAccessImpl
{
public:
	//--- constructors
	SybDBDAImpl(const char *name);
	~SybDBDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, InputMapper *, OutputMapper *);

private:
	//constructor
	SybDBDAImpl();
	SybDBDAImpl(const SybDBDAImpl &);
	//assignement
	SybDBDAImpl &operator=(const SybDBDAImpl &);

	friend class SybDBDAImplTest;
};

#endif
