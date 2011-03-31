/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BasicAuthenticationData_H
#define _BasicAuthenticationData_H

#include "ITOString.h"

//---- BasicAuthenticationData ----------------------------------------------------------
//! Decoder for HTTP Basic authentication scheme
class BasicAuthenticationData
{
public:
	BasicAuthenticationData(const String &authorizationHeaderValue);
	String GetUserName() const	{
		return fUserName;
	}
	String GetPassword() const	{
		return fPassword;
	}
	bool IsSyntaxValid() const	{
		return fSyntaxValid;
	}
private:
	String fUserName;
	String fPassword;
	bool fSyntaxValid;
};
#endif
