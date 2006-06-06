/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WDMODULE_H
#define _WDMODULE_H

#include "IFAConfObject.h"

class Registry;
class Server;

//---- WDModule -----------------------------------------------------------
//!component for initialization, termination and reset
//!extension api that supports installation, termination and resets of components<br>
//!static api supports installation, termination and resets of configured extensions
//!WDModule components are singletons since they are not cloned
class EXPORTDECL_WDBASE WDModule : public NotCloned
{
public:
	//!standard named object constructor
	WDModule(const char *name);
	//!does nothing
	~WDModule();

	//!implementers should initialize module using config
	virtual bool Init(const ROAnything roaConfig) = 0;
	//!implementers should terminate module expecting destruction
	virtual bool Finis() = 0;
	//!initializes module after termination for reinitialization; default uses Init
	virtual bool ResetInit(const ROAnything roaConfig);
	//!terminates module for reinitialization; default uses Finis
	virtual bool ResetFinis(const ROAnything roaConfig);

	//!installs a list of "Modules" defined in config or all modules registered so far
	static int Install(const ROAnything roaConfig);
	//!terminates a list of "Modules" defined in config or all modules registered
	static int Terminate(const ROAnything roaConfig);
	//!resets all modules terminating with oldconfig and reinitialising with config
	static int Reset(const ROAnything roaOldconfig, const ROAnything roaConfig);

	RegCacheDef(WDModule);	// FindWDModule()

protected:
	//!terminating a category of RegisterableObjects using title as name on cerr
	virtual bool StdFinis(const char *category, const char *title);
	//!installing modules after terminating for reset
	static int ResetInstall(const ROAnything roaConfig);
	//!terminating modules for reset
	static int ResetTerminate(const ROAnything roaConfig);

private:
	//!should not be used
	WDModule();
	//!should not be used
	WDModule(const WDModule &);
	//!should not be used
	WDModule &operator=(const WDModule &);
};

#define RegisterModule(name) RegisterObject(name, WDModule)

#endif		//ifndef _WDMODULE_H
