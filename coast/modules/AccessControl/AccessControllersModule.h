/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AccessControllersModule_H
#define _AccessControllersModule_H

#include "config_AccessControl.h"
#include "WDModule.h"

//---- AccessControllersModule -----------------------------------------------------------
/*!
<B>Manages access controllers (three different types).</B><PRE>
{
	# UserDataAccessControllers
	/User {
		/UDACImpl1	{ ... names ... }
		/UDACImpl2	{ ... names ... }
	}

	# TokenDataAccessControllers
	/Token {
		/TDACImpl	{ ... names ... }
	}

	# EntityDataAccessControllers
	/Entity {
		/EDACImpl	{ ... names ... }
	}

e.g.

	/User {
		/MockUDAC	{ LdapUserData LocalUserData }
	}

	/Token {
		/FileTDAC 	{ ActerTokenData }
	}

	/Entity {
		/MockEDAC	{ RightSet1 RightSet2 }
		/FileEDAC	{ PersistentRights }
	}

}</PRE>*/
class EXPORTDECL_ACCESSCONTROL AccessControllersModule : public WDModule
{
public:
	AccessControllersModule(const char *);
	~AccessControllersModule();

	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything config);
	virtual bool Finis();
};

#endif
