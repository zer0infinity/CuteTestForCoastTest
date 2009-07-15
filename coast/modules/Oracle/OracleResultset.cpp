/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleResultset.h"
#include "OracleStatement.h"
#include "OracleException.h"
#include "Anything.h"
#include "Dbg.h"

OracleResultset::~OracleResultset()
{
}

ROAnything OracleResultset::GetOutputDescription()
{
	StartTrace( OracleResultset.GetOutputDescription );
	return frStmt.GetOutputDescription();
}

bool OracleResultset::DefineOutputArea()
{
	StartTrace(OracleResultset.DefineOutputArea);
	return frStmt.DefineOutputArea();
}

OracleResultset::Status OracleResultset::next()
{
	StartTrace(OracleResultset.next);
	if ( fFetchStatus == NOT_READY ) {
		DefineOutputArea();
		fFetchStatus = READY;
	}
	switch ( fFetchStatus ) {
		case READY:
		case DATA_AVAILABLE: {
			sword status = frStmt.Fetch( 1 );
			Trace("fetch status: " << (long)status)
			if ( status == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO ) {
				fFetchStatus = DATA_AVAILABLE;
			} else
				// SQL_NO_DATA and other error/warn conditions
			{
				fFetchStatus = END_OF_FETCH;
			}
			break;
		}
		default:
			break;
	}
	return fFetchStatus;
}

String OracleResultset::getString( long lColumnIndex )
{
	StartTrace1(OracleResultset.getString, "col index: " << lColumnIndex);
	return frStmt.getString( lColumnIndex );
}
