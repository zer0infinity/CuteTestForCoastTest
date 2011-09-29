/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLEEXCEPTION_H_
#define ORACLEEXCEPTION_H_

#include "ITOString.h"
#include "oratypes.h"

class OracleConnection;
//! Provide useful message when an exception gets thrown
/*!
 * This class is used to abort the program flow with either a self written message or a message provided using the
 * return code of the last OCI API call.
*/
class OracleException : public std::exception
{
	OracleConnection &fConnection;
	sword fStatus;
	String fMessage;
public:
	/*! Create an exception object using an individual reason message
	 * @param rConn reference to OracleConnection object
	 * @param strMessage Individual mesage string returned to the catcher
	 */
	OracleException(OracleConnection &rConn, String const &strMessage);
	/*! Create an exception object using OracleConnection and the status value to create a useful message string
	 * @param rConn reference to OracleConnection object used to collect OCI specific error code and message
	 * @param status value of last OCI API call
	 */
	OracleException(OracleConnection &rConn, sword status);
	//! do nothing dtor
	~OracleException() throw() {};
	/*! Access error message
	 * @return internally stored error message which lead to the exception
	 */
	const String &getMessage() const;
	/*! Access error message
	 * @return internally stored error message which lead to the exception
	 */
	const char *what() const throw();
};

#endif /* ORACLEEXCEPTION_H_ */
