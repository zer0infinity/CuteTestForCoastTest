/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLERESULTSET_H_
#define ORACLERESULTSET_H_

//--- modules used in the interface
#include "config_coastoracle.h"
#include "Anything.h"

#include <memory>

class OracleStatement;

//---- OracleResultset -----------------------------------------------------------
//
class EXPORTDECL_COASTORACLE OracleResultset
{
public:
	enum Status {
		NOT_READY, READY, END_OF_FETCH, DATA_AVAILABLE, STREAM_DATA_AVAILABLE
	};
private:
	OracleStatement &frStmt;
	Status fFetchStatus;

	bool DefineOutputArea();
public:
	OracleResultset( OracleStatement &rStmt ) :
		frStmt( rStmt ), fFetchStatus( NOT_READY ) {
	}
	~OracleResultset();
	ROAnything GetOutputDescription();
	Status next();
	String getString( long lColumnIndex );
};

typedef std::auto_ptr<OracleResultset> OracleResultsetPtr;

#endif /* ORACLERESULTSET_H_ */
