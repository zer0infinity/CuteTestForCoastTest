/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybDB_H
#define _SybDB_H

//---- baseclass include -------------------------------------------------
#include "config_sybase.h"
#include <sybfront.h>
#include <sybdb.h>

//---- forward declaration -----------------------------------------------
class String;
class Anything;
class SimpleMutex;

//---- SybDB ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
class EXPORTDECL_SYBASE SybDB
{
public:
	//--- constructors
	SybDB();
	virtual ~SybDB();

	//--- public api
	//! \param aParam explanation of aParam (important : paramname - explanation )
	//! \return explanation of return value
	//! \pre explanation of precondition for the method call
	//! \post explanation of postcondition for the method call
	virtual bool Init(void);
	virtual bool Open(String user, String password, String server, String appl);
	virtual bool SqlExec(String cmd);
	virtual bool GetResult( Anything &anyResult );
	virtual bool Close(void);
	const Anything &GetMessages() const {
		return fMessages;
	}

	static Anything fgMessages;
	static SimpleMutex fgMessagesMutex;

protected:
	//--- subclass api
	void	Warning(String str);
	void	Error(String str);
	virtual bool DoGetResult( Anything &anyResult );

	virtual RETCODE GetRetCode() {
		return fret_code;
	};

	//--- member variables declaration
	DBPROCESS	*fdbproc;		// The connection with SQL Server
	LOGINREC	*flogin;		// The login information
	RETCODE		fret_code;
	Anything	fMessages;		// Store error and warnings-messages coming from DataBase

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

	friend class SybDBTest;
};

#endif
