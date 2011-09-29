/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleException.h"
#include "OracleConnection.h"

OracleException::OracleException(OracleConnection &rConn, String const &strMessage) :
		fConnection(rConn), fStatus(0), fMessage(strMessage) {
}

OracleException::OracleException(OracleConnection &rConn, sword status) :
		fConnection(rConn), fStatus(status), fMessage(fConnection.errorMessage(fStatus)) {
}

const String &OracleException::getMessage() const {
	return fMessage;
}

const char *OracleException::what() const throw () {
	return fMessage;
}
