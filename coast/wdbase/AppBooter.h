/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AppBooter_H
#define _AppBooter_H

//---- baseclass include -------------------------------------------------
#include "config_wdbase.h"

//---- forward declaration -----------------------------------------------
#include "Anything.h"
class Application;

//---- AppBooter ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class EXPORTDECL_WDBASE AppBooter
{
	// load the global config file with the information
	// set in the environment variable WD_ROOT
	// or with a minimal relative path
public:
	AppBooter();
	virtual ~AppBooter();
	virtual int Run(int argc, char *argv[], bool doHalt = true);

	virtual bool Boot(Anything &config); // access the intial config file
	virtual Application *FindApplication(const Anything &config, String &applicationName);

	virtual bool OpenLibs(const Anything &config);
	virtual bool CloseLibs();
	virtual bool ReadFromFile(Anything &config, const char *filename); // reading of configuration files
	virtual void HandleArgs(int argc, char *argv[], Anything &config);
	virtual void SetSignalMask();

	//!sets WD_ROOT and WD_PATH if set in args; returns bootfilename
	virtual String PrepareBootFileLoading(const ROAnything &roconfig);

	//!merges command line arguments into global configuration anything
	virtual void MergeConfigWithArgs(Anything &config, const Anything &args);

	virtual void Halt(const Anything &config);

protected:
	Anything fLibArray;
	friend class AppBooterTest;
};

#endif
