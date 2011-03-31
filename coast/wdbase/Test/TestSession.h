/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TestSession_H
#define _TestSession_H

#include "Session.h"
#include "SessionListManager.h"

//---- forward declaration -----------------------------------------------
class Context;
class SessionListManagerTest;

//---- TestSession ----------------------------------------------------------
//!test the notify protocol of session
class TestSession : public Session
{
public:
	//--- constructors
	TestSession(const char *name, Context &ctx, SessionListManagerTest *slmt);
	~TestSession();

	void Notify(Session::ESessionEvt, Context &ctx);

protected:
	//!callback into test
	SessionListManagerTest *fTest;
private:

};

//---- SessionFactory -----------------------------------------------------------
//!test the factory configuration of SessionListManager
class TestSessionFactory : public SessionFactory
{
public:
	TestSessionFactory(const char *testSessionFactoryName, SessionListManagerTest *slmt = 0);
	~TestSessionFactory();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) TestSessionFactory(fName, fTest);
	}
	//!allocation protocol for session allocation
	virtual Session *DoMakeSession(Context &ctx);
	//!custom steps to prepare a session
	virtual Session *DoPrepareSession(Context &ctx, Session *session, bool &isBusy);

	void SetTest(SessionListManagerTest *test);

protected:
	//!callback into test
	SessionListManagerTest *fTest;

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	TestSessionFactory();
	TestSessionFactory(const TestSessionFactory &);
	TestSessionFactory &operator=(const TestSessionFactory &);

};

#endif
