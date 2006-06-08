/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SimpleTestApp_h_
#define _SimpleTestApp_h_

#include "Server.h"

class AppBooterTest;

//---- SimpleTestApp -----------------------------------------------------------
//! test application to test sequencing of init and terminate calls by AppBooter
class SimpleTestApp : public Application
{
public:
	SimpleTestApp();
	SimpleTestApp(const char *AppName);
	~SimpleTestApp();

	IFAObject *Clone() const {
		return new SimpleTestApp(fName);
	}

	//! GlobalInit: installs ressources shared among all instances
	virtual int GlobalInit(int argc, char *argv[], const ROAnything config);
	//!starts up the application; an InterruptHandler is set up to catch signals for shutdown, reset etc.
	virtual int GlobalRun();
	//! frees ressources shared among all instances
	virtual int GlobalTerminate(int val);

	//! loads configuration based on instance's name
	//! \return 0 if everything worked fine and application is ready to run
	//! \pre None
	//! \post application is ready to be runned
	//! subclasses overwriting Init should call parent's Init !  -- refine this
	virtual int Init();

	//! hook method to be overriden by subclasses; this implementation does nothing but writes a greeting on cerr
	//! \return return code to pass up to calling process
	//! \pre application is ready for running
	virtual int Run();

	//! termination
	virtual int Terminate(int val);

	//!implementation of LookupInterface
	//! Looks up key in the instance's store then config and finally in the global config

	//!set the test that drives the booter
	void SetTest(AppBooterTest *test);

protected:
	AppBooterTest *fTest;
};

class SimpleTestServer : public Server
{
public:
	SimpleTestServer();
	SimpleTestServer(const char *AppName);
	~SimpleTestServer();

	IFAObject *Clone() const {
		return new SimpleTestServer(fName);
	}

	//! GlobalInit: installs ressources shared among all instances
	virtual int GlobalInit(int argc, char *argv[], const ROAnything config);
	//!starts up the application; an InterruptHandler is set up to catch signals for shutdown, reset etc.
	virtual int GlobalRun();
	//! frees ressources shared among all instances
	virtual int GlobalTerminate(int val);

	//! loads configuration based on instance's name
	//! \return 0 if everything worked fine and application is ready to run
	//! \pre None
	//! \post application is ready to be runned
	//! subclasses overwriting Init should call parent's Init !  -- refine this
	virtual int Init();

	//! hook method to be overriden by subclasses; this implementation does nothing but writes a greeting on cerr
	//! \return return code to pass up to calling process
	//! \pre application is ready for running
	virtual int Run();

	//! termination
	virtual int Terminate(int val);

	//!implementation of LookupInterface
	//! Looks up key in the instance's store then config and finally in the global config

	//!set the test that drives the booter
	void SetTest(AppBooterTest *test);

protected:
	AppBooterTest *fTest;
};

#endif
