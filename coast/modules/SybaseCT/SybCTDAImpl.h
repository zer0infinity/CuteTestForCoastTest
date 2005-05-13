/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTDAImpl_H
#define _SybCTDAImpl_H

//---- SybCTGenericDAImpl include -------------------------------------------------
#include "config_sybasect.h"
#include "SybCTGenericDAImpl.h"
#include "Threads.h"
#include <ctpublic.h>

//---- forward declaration -----------------------------------------------

//---- SybCTDAImpl ----------------------------------------------------------
//:single line description of the class
// further explanation of the purpose of the class
// this may contain <B>HTML-Tags</B>
// ...
class EXPORTDECL_SYBASECT SybCTDAImpl : public SybCTGenericDAImpl
{
public:
	//--- constructors
	SybCTDAImpl(const char *name);
	~SybCTDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//: executes the transaction
	//!param: c - The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

	static bool Init(const Anything &config);
	//:implementers should terminate module expecting destruction
	static bool Finis();

protected:
	//: Helper method to report errors
	//	void SetErrorMsg(const char* msg, SybDB *SybDB, Context &context, Mapper *out );
	static Mutex    fgStructureMutex;
	static Anything fgListOfSybCT;
	static Anything fgContextMessages;
	static CS_CONTEXT *fg_cs_context;
	static bool		fgInitialized;
	static Semaphore *fgCountingSema;

private:
	//constructor
	SybCTDAImpl();
	SybCTDAImpl(const SybCTDAImpl &);
	//assignement
	SybCTDAImpl &operator=(const SybCTDAImpl &);

	friend class SybaseModule;
	friend class SybCTDAImplTest;
};

#endif
