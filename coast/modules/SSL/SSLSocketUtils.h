/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SSLSocketUtils_H
#define _SSLSocketUtils_H

//---- baseclass include -------------------------------------------------

//---- framework classes -------------------------------------------------
#include "SSLAPI.h"
#include "Dbg.h"
#include "Anything.h"

//---- SSLSocketUtils ----------------------------------------------------------
//! Helper methods for X509 certificates
class SSLSocketUtils
{
public:
	//--- constructors
	SSLSocketUtils();
	~SSLSocketUtils();
	// Print a human readable form of a X509 Name
	static String GetNameFromX509Name(X509_NAME *x509_name, unsigned long flags = XN_FLAG_ONELINE);
	static String GetPeerAsString(X509 *cert);
	static String GetPeerIssuerAsString(X509 *cert);
	// Create a Anything consisting of key/value pairs making up the passed in distinguished name.
	static Anything ParseDN(String dn);
	// The string contains filter specs of the form name=value, name1=value1 ....
	// True indicates all specified name/value pairs were found in the passed in parsed distinguished
	// name.
	static bool VerifyDN(String filter, ROAnything dnParts);
};

#endif
