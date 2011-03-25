/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Resolver_H
#define _Resolver_H

#include "ITOString.h"//lint !e537

namespace {
	static const String defaultIP("127.0.0.1");
	static const String defaultName("localhost");
}
//---- Resolver ----------------------------------------------------------
//!dns to ip adress and vice versa resolver
//!api wrapper to resolver library;
//!converts DNS names to ip addresses
class Resolver
{
public:
	//!resolve the dns name to an ip address using the resolver lib
	static String DNS2IPAddress( const String &dnsName, const String &dflt=defaultIP );
	//!find the dns name of the ip address
	static String IPAddress2DNS( const String &ipAddress, const String &dflt=defaultName );

private:
	Resolver();
	Resolver(const Resolver &);
	Resolver &operator=(const Resolver &);
};

#endif
