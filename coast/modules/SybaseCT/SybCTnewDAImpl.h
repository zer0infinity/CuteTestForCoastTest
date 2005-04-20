/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTnewDAImpl_H
#define _SybCTnewDAImpl_H

//---- SybCTGenericDAImpl include -------------------------------------------------
#include "config_sybasect.h"
#include "DataAccessImpl.h"
#include "Threads.h"
#include <ctpublic.h>

//---- forward declaration -----------------------------------------------
class SybCTnewDA;
class PeriodicAction;

//---- SybCTnewDAImpl ----------------------------------------------------------
//:single line description of the class
// further explanation of the purpose of the class
// this may contain <B>HTML-Tags</B>
// ...
class EXPORTDECL_SYBASECT SybCTnewDAImpl : public DataAccessImpl
{
public:
	//--- constructors
	SybCTnewDAImpl(const char *name);
	~SybCTnewDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//: executes the transaction
	//!param: c - The context of the transaction
	virtual bool Exec(Context &ctx, InputMapper *in, OutputMapper *out);

	static bool Init(ROAnything config);
	//:implementers should terminate module expecting destruction
	static bool Finis();

	static bool CheckCloseOpenedConnections(long lTimeout);

protected:
	static bool IntGetOpen(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user);
	static bool DoGetConnection(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user);
	static void DoPutbackConnection(SybCTnewDA *&pSyb, bool bIsOpen, const String &server, const String &user);

	bool DoPrepareSQL(String &command, Context &ctx, InputMapper *in);

	//: Helper method to report errors
	static Mutex    fgStructureMutex;
	static Anything fgListOfSybCT;
	static Anything fgContextMessages;
	static CS_CONTEXT *fg_cs_context;
	static bool		fgInitialized;
	static PeriodicAction *fgpPeriodicAction;
	static Semaphore *fgpResourcesSema;

private:
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
