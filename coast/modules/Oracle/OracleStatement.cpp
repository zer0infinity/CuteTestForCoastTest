/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleStatement.h"

#include "OracleEnvironment.h"
#include "OracleException.h"
#include "OracleResultset.h"
#include "Dbg.h"

OracleStatement::OracleStatement( OracleConnection *pConn, String const &strStmt ) :
	fpConnection( pConn ), fStmt( strStmt ), fStatus( UNPREPARED ), fStmtType( Coast_OCI_STMT_UNKNOWN )
{
}

OracleStatement::~OracleStatement()
{
	Cleanup();
}

bool OracleStatement::AllocHandle()
{
	StartTrace(OracleStatement.AllocHandle);
	// allocates and returns new statement handle
	String strError( 32L );
	bool bSuccess( !fpConnection->checkError( OCIHandleAlloc( fpConnection->getEnvironment().EnvHandle(),
				   fStmthp.getVoidAddr(), (ub4) OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0 ), strError ) );
	if ( !bSuccess ) {
		fErrorMessages.Append( strError );
	}
	return bSuccess;
}

void OracleStatement::Cleanup()
{
	StartTrace(OracleStatement.Cleanup);
	// according to documentation, def handles will get cleaned too
	fStmthp.reset();
	fStatus = UNPREPARED;
}

OracleStatement::Status OracleStatement::execute( ub4 mode, ub4 iters )
{
	StartTrace(OracleStatement.execute);
	// executes a SQL statement (first row is also fetched only if iters > 0)
	// depending on the query type, we could use 'scrollable cursor mode' ( OCI_STMT_SCROLLABLE_READONLY )
	sword status = OCIStmtExecute( fpConnection->SvcHandle(), getHandle(), fpConnection->ErrorHandle(), iters, 0, NULL,
								   NULL, mode );
	if ( status != OCI_SUCCESS ) {
		throw OracleException( *fpConnection, status );
	} else {
		// how can we find out about the result type? -> see Prepare
		switch ( fStmtType ) {
			case Coast_OCI_STMT_SELECT:
				fStatus = RESULT_SET_AVAILABLE;
				break;
			case Coast_OCI_STMT_CREATE:
			case Coast_OCI_STMT_DROP:
			case Coast_OCI_STMT_ALTER:
				//FIXME: is this correct?
				fStatus = UPDATE_COUNT_AVAILABLE;
				break;
			case Coast_OCI_STMT_DELETE:
			case Coast_OCI_STMT_INSERT:
			case Coast_OCI_STMT_UPDATE:
				fStatus = UPDATE_COUNT_AVAILABLE;
				break;
			default:
				//FIXME: check if this makes sense
				fStatus = PREPARED;
		}
	}
	return fStatus;
}

sword OracleStatement::Fetch( ub4 numRows )
{
	StartTrace(OracleStatement.Fetch);
	// fetch another row
	return OCIStmtFetch2( getHandle(), fpConnection->ErrorHandle(), numRows, OCI_FETCH_NEXT, 0, OCI_DEFAULT );
}

unsigned long OracleStatement::getUpdateCount() const
{

	return 0UL;
}

bool OracleStatement::Prepare()
{
	StartTrace(OracleStatement.Prepare);
	// prepare SQL statement for execution
	String strErr( 32L );
	bool bSuccess( AllocHandle() );
	if ( bSuccess ) {
		if ( ! ( bSuccess = !fpConnection->checkError( OCIStmtPrepare( getHandle(), fpConnection->ErrorHandle(),
							(const text *) (const char *) fStmt, (ub4) fStmt.Length(), OCI_NTV_SYNTAX, OCI_DEFAULT ), strErr ) ) ) {
			fErrorMessages.Append( strErr );
		} else {
			ub2 fncode;
			if ( fpConnection->checkError( OCIAttrGet( getHandle(), OCI_HTYPE_STMT, (dvoid *) &fncode, 0,
										   OCI_ATTR_STMT_TYPE, fpConnection->ErrorHandle() ), strErr ) ) {
				fErrorMessages.Append( strErr );
				bSuccess = false;
			} else {
				fStmtType = (StmtType) fncode;
				Trace("statement type is " << (long)fncode);
				fStatus = PREPARED;
			}
		}
	}
	return bSuccess;
}

OracleResultset *OracleStatement::getResultset()
{
	OracleResultset *pResult( 0 );
	if ( fStatus == RESULT_SET_AVAILABLE ) {
		pResult = new OracleResultset( *this );
	} else {
		String strMessage("Error - getResultset failed, no resultset available, current status is ");
		strMessage << (long)fStatus;
		throw OracleException(*fpConnection, strMessage);
	}
	return pResult;
}
