/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Resolver_H
#define _Resolver_H

//---- baseclass include -------------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION

//---- forward declaration -----------------------------------------------
class EXPORTDECL_FOUNDATION String;
//---- Resolver ----------------------------------------------------------
//!dns to ip adress and vice versa resolver
//!api wrapper to resolver library;
//!converts DNS names to ip addresses
class EXPORTDECL_FOUNDATION Resolver
{
public:
	//!resolve the dns name to an ip address using the resolver lib
	static String DNS2IPAddress( const String &dnsName );
	//!find the dns name of the ip address
	static String IPAddress2DNS( const String &ipAddress );

protected:
	//!subclass protocol for platform specific implementation of dns to ip mapping
	virtual bool DNS2IP(String &ipAddress, const String &dnsName ) = 0;
	//!subclass protocol for plattform specific implementation of ip to dns mapping
	virtual bool IP2DNS(String &dnsName, const String &ipAddress, unsigned long addr ) = 0;

	Resolver();
	virtual ~Resolver();

private:
	// use careful, you inhibit subclass use
	//--- private class api
	Resolver(const Resolver &);
	Resolver &operator=(const Resolver &);
	//--- private member variables
};

#endif
