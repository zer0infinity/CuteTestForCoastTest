/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "BasicAuthenticationData.h"
#include "Dbg.h"
#include "Base64.h"
#include "StringStream.h"

//---- BasicAuthenticationData ----------------------------------------------------------------
BasicAuthenticationData::BasicAuthenticationData(const String &authorizationHeaderValue)
{
	StartTrace(BasicAuthenticationData.BasicAuthenticationData);

	Trace("Authorization header: " << authorizationHeaderValue);

	String authScheme, base64Part;
	IStringStream iss(authorizationHeaderValue);
	iss >> authScheme;
	iss >> base64Part;
	if (authScheme != "Basic") {
		fSyntaxValid = false;
		return;
	}
	String namePW;
	Base64("").DoDecode(namePW, base64Part);

	StringTokenizer tok(namePW, ':');
	fSyntaxValid = tok(fUserName) && tok(fPassword);

	Trace( "name : pw = [" << fUserName << " : " << fPassword << "]" );
}
