/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLEEXCEPTION_H_
#define ORACLEEXCEPTION_H_

#include "config_coastoracle.h"
#include "OracleConnection.h"
#include <exception>

class EXPORTDECL_COASTORACLE OracleException : public std::exception
{
	OracleConnection &fConnection;
	sword fStatus;
	String fMessage;
public:
	OracleException(OracleConnection &rConn, String const &strMessage);
	OracleException(OracleConnection &rConn, sword status);
	~OracleException() throw() {};
	const String &getMessage() const;
	const char *what() const throw();
};

#endif /* ORACLEEXCEPTION_H_ */
