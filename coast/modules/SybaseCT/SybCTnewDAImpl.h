/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTnewDAImpl_H
#define _SybCTnewDAImpl_H

#include "DataAccessImpl.h"
#include "Threads.h"
#include <ctpublic.h>

class SybCTnewDA;
class PeriodicAction;

//---- SybCTnewDAImpl ----------------------------------------------------------
//:single line description of the class
// further explanation of the purpose of the class
// this may contain <B>HTML-Tags</B>
// ...
class SybCTnewDAImpl : public DataAccessImpl
{
public:
	//--- constructors
	SybCTnewDAImpl(const char *name);
	~SybCTnewDAImpl();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//: executes the transaction
	//!param: c - The context of the transaction
	virtual bool Exec(Context &ctx, ParameterMapper *in, ResultMapper *out);

	static bool Init(ROAnything config);
	//:implementers should terminate module expecting destruction
	static bool Finis();

	static bool CheckCloseOpenedConnections(long lTimeout);

protected:
	static bool IntGetOpen(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user);
	static bool DoGetConnection(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user);
	static void DoPutbackConnection(SybCTnewDA *&pSyb, bool bIsOpen, const String &server, const String &user);

	bool DoPrepareSQL(String &command, Context &ctx, ParameterMapper *in);

	//: Helper method to report errors
	static SimpleMutex fgStructureMutex;
	static Anything fgListOfSybCT;
	static Anything fgContextMessages;
	static CS_CONTEXT *fg_cs_context;
	static bool		fgInitialized;
	static bool		fbUseDelayedCommit;
	static PeriodicAction *fgpPeriodicAction;
	static Semaphore *fgpResourcesSema;

private:
	static bool IntDoGetConnection(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user);
	static void IntDoPutbackConnection(SybCTnewDA *&pSyb, bool bIsOpen, const String &server, const String &user);

	//constructor
	SybCTnewDAImpl();
	SybCTnewDAImpl(const SybCTnewDAImpl &);
	//assignement
	SybCTnewDAImpl &operator=(const SybCTnewDAImpl &);

	friend class SybaseModule;
	friend class SybCTnewDAImplTest;
	friend class SybCTThreadTest;
};

#endif
