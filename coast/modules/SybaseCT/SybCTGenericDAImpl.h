/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTGenericDAImpl_H
#define _SybCTGenericDAImpl_H

//--- superclass interface ---
#include "config_sybasect.h"
#include "DataAccessImpl.h"

//--- modules used in the interface ---

//: DataAccess for
class EXPORTDECL_SYBASECT SybCTGenericDAImpl: public DataAccessImpl
{
public:
	SybCTGenericDAImpl(const char *name);
	~SybCTGenericDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//: executes the transaction
	//!param: c - The context of the transaction
	virtual bool Exec(Context &c, InputMapper *, OutputMapper *);

protected:
	void FillParameters(Context &context, InputMapper *in, OutputMapper *out, Anything &queryParams);
	bool PutResults(Context &context, InputMapper *in, OutputMapper *out, Anything &queryParams, Anything &queryResults, Anything &queryTitles);
	void PutMessages(Context &context, OutputMapper *out, Anything &aMsgAny);

private:
	//constructor
	SybCTGenericDAImpl();
	SybCTGenericDAImpl(const SybCTGenericDAImpl &);
	//assignement
	SybCTGenericDAImpl &operator=(const SybCTGenericDAImpl &);
};

/* Don't add stuff after this #endif */
#endif		//not defined _SybCTGenericDAImpl_H
