/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTPoolDAImpl_H
#define _SybCTPoolDAImpl_H

//---- SybCTGenericDAImpl include -------------------------------------------------
#include "config_sybasect.h"
#include "SybCTGenericDAImpl.h"

//---- forward declaration -----------------------------------------------

//---- SybCTPoolDAImpl ----------------------------------------------------------
//:single line description of the class
// further explanation of the purpose of the class
// this may contain <B>HTML-Tags</B>
// ...
class EXPORTDECL_SYBASECT SybCTPoolDAImpl : public SybCTGenericDAImpl
{
public:
	//--- constructors
	SybCTPoolDAImpl(const char *name);
	~SybCTPoolDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//: executes the transaction
	//!param: c - The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	//: Helper method to report errors
	//	void SetErrorMsg(const char* msg, SybDB *SybDB, Context &context, Mapper *out );

private:
	//constructor
	SybCTPoolDAImpl();
	SybCTPoolDAImpl(const SybCTPoolDAImpl &);
	//assignement
	SybCTPoolDAImpl &operator=(const SybCTPoolDAImpl &);

	friend class SybCTPoolDAImplTest;
};

#endif
