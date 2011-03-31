/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSession.h"
#include "SessionListManagerTest.h"
#include "Dbg.h"

//---- TestSession ----------------------------------------------------------
//:test the notify protocol of session
TestSession::TestSession(const char *name, Context &ctx, SessionListManagerTest *slmt)
	: 	Session(name),
		fTest(slmt)
{
	StartTrace(TestSession.Ctor);
}

TestSession::~TestSession()
{
	StartTrace(TestSession.Dtor);
}

void TestSession::Notify(Session::ESessionEvt evt, Context &ctx)
{
	StartTrace(TestSession.Ctor);
	fTest->NotifyCalled(evt, ctx);
}

//---- SessionFactory -----------------------------------------------------------
RegisterSessionFactory(TestSessionFactory);
//:test the factory configuration of SessionListManager
TestSessionFactory::TestSessionFactory(const char *testSessionFactoryName, SessionListManagerTest *slmt)
	:	SessionFactory(testSessionFactoryName),
		fTest(slmt)
{
	StartTrace(TestSessionFactory.Ctor);
}

TestSessionFactory::~TestSessionFactory()
{
	StartTrace(TestSessionFactory.Ctor);
}
//:allocation protocol for session allocation
Session *TestSessionFactory::DoMakeSession(Context &ctx)
{
	StartTrace(TestSessionFactory.Ctor);
	return new (Coast::Storage::Global()) TestSession("Test", ctx, fTest);
}
//:custom steps to prepare a session
Session *TestSessionFactory::DoPrepareSession(Context &ctx, Session *session, bool &isBusy)
{
	StartTrace(TestSessionFactory.Ctor);
	fTest->DoPrepareSessionCalled(ctx, session);
	return session;
}

void TestSessionFactory::SetTest(SessionListManagerTest *test)
{
	fTest = test;
}

