/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Application_H
#define _Application_H

#include "IFAConfObject.h"
#include "Threads.h"

//! life-cycle of an application: Init - Run - Terminate
class Application : public HierarchConfNamed
{
	friend class ServerThread;
public:
	//!standard constructor with name
	Application(const char *applicationName);
	virtual ~Application();

	//! GlobalInit: installs ressources shared among all instances
	int GlobalInit(int argc, const char *argv[], const ROAnything config);

	//!starts up the application; an InterruptHandler is set up to catch signals for shutdown, reset etc.
	int GlobalRun();

	//! frees ressources shared among all instances
	int GlobalTerminate(int val);

	//!access to the global configuration
	static ROAnything GetConfig();
	static void InitializeGlobalConfig(Anything theConfiguration);

	RegCacheDef(Application);	// FindApplication()

	// Gets the first registered Application, either the first one listed as /Application or
	// the first registered one
	static Application *GetGlobalApplication(String &applicationName);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) Application(fName);
	}

protected:
	//! GlobalInit: installs ressources shared among all instances
	virtual int DoGlobalInit(int argc, const char *argv[], const ROAnything config);

	//!starts up the application; an InterruptHandler is set up to catch signals for shutdown, reset etc.
	virtual int DoGlobalRun();

	//! frees ressources shared among all instances
	virtual int DoGlobalTerminate(int val);

	//! loads configuration based on instance's name
	//! \return 0 if everything worked fine and application is ready to run
	//! \pre None
	//! \post application is ready to be runned
	//! subclasses overwriting Init should call parent's Init !  -- refine this
	virtual int DoInit();

	//! hook method to be overriden by subclasses; this implementation does nothing but writes a greeting on cerr
	//! \return return code to pass up to calling process
	//! \pre application is ready for running
	virtual int DoRun();

	//! termination
	virtual int DoTerminate(int val);

	//!implementation of LookupInterface
	//! Looks up key in the instance's store then config and finally in the global config
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

private:
	//! loads configuration based on instance's name
	//! \return 0 if everything worked fine and application is ready to run
	//! \pre None
	//! \post application is ready to be runned
	//! subclasses overwriting Init should call parent's Init !  -- refine this
	int Init();

	//! main entry to run an application or server, will call DoRun() method hook internally
	/*! \return code to pass up to calling process
		\pre application is ready for running */
	int Run();

	//! termination
	int Terminate(int val);

	// block the following default elements of this class
	// because they're not allowed to be used
	Application();
	Application(const Application &);
	Application &operator=(const Application &);

//protected:
	//! the process main configuration
	//! the main configuration initialized in GlobalInit, considered read-only after initialization
	//! only accessible by LookupInterface methods or as ROAnything.
	static Anything fgConfig;

	//! guard for initializing fgConfig
	static Mutex fgConfigMutex;
};

#define RegisterApplication(name) RegisterObject(name, Application)

#endif
