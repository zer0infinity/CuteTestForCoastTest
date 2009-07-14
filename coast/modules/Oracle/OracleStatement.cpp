/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleStatement.h"

#include "OracleEnvironment.h"
#include "OracleColumn.h"
#include "Dbg.h"

OracleStatement::OracleStatement( OracleConnection *pConn, String const &strStmt ) :
	fpConnection( pConn ), fStmt( strStmt )
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
	bool bSuccess( !fpConnection->checkError( OCIHandleAlloc( fpConnection->getEnvironment().EnvHandle(), fStmthp.getVoidAddr(),
				   (ub4) OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0 ), strError ) );
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
}

sword OracleStatement::GetReplyDescription()
{
	StartTrace(OracleStatement.GetReplyDescription);
	// retrieves descriptions of return values for a given SQL statement
	return Execute( OCI_DESCRIBE_ONLY );
}

sword OracleStatement::Execute( ub4 mode )
{
	StartTrace(OracleStatement.Execute);
	// executes a SQL statement (first row is also fetched)
	return OCIStmtExecute( fpConnection->SvcHandle(), getHandle(), fpConnection->ErrorHandle(), (ub4) 1,
						   (ub4) 0, (const OCISnapshot *) NULL, (OCISnapshot *) NULL, mode );
}

sword OracleStatement::Fetch( ub4 numRows )
{
	StartTrace(OracleStatement.Fetch);
	// fetch another row
	return OCIStmtFetch( getHandle(), fpConnection->ErrorHandle(), numRows, OCI_FETCH_NEXT, OCI_DEFAULT );
}

bool OracleStatement::Prepare()
{
	StartTrace(OracleStatement.Prepare);
	// prepare SQL statement for execution
	String strErr( 32L );
	bool bSuccess( AllocHandle() );
	if ( bSuccess ) {
		if ( ! ( bSuccess = !fpConnection->checkError( OCIStmtPrepare( getHandle(),
							fpConnection->ErrorHandle(), (const text *) (const char *) fStmt, (ub4) fStmt.Length(),
							(ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT ), strErr ) ) ) {
			fErrorMessages.Append( strErr );
		}
	}
	return bSuccess;
}

bool OracleStatement::GetOutputDescription( Anything &desc, ub2 &fncode )
{
	StartTrace(OracleStatement.GetOutputDescription);
	// returns array of element descriptions: each description is
	// an Anything array with 4 entries:
	// name of the collumn, type of the data, length of the data in bytes, scale

	OCIError *eh( fpConnection->ErrorHandle() );
	String strErr;

	OCIStmt *pStmthp( getHandle() ); // OCI statement handle

	if ( fpConnection->checkError( OCIAttrGet( (dvoid *) pStmthp, OCI_HTYPE_STMT, (dvoid *) &fncode, (ub4 *) 0,
								   OCI_ATTR_STMT_TYPE, eh ), strErr ) ) {
		fErrorMessages.Append( strErr );
		return false;
	}

	if ( fncode == OCI_STMT_SELECT ) {

		if ( fpConnection->checkError( GetReplyDescription(), strErr ) ) {
			fErrorMessages.Append( strErr );
			return false;
		}

		OCIParam *mypard;
		ub2 dtype;
		ub4 data_len;
		text *col_name;
		ub4 col_name_len;

		// Request a parameter descriptor for position 1 in the select-list
		ub4 counter = 1;
		sb4 parm_status = OCIParamGet( pStmthp, OCI_HTYPE_STMT, eh, (void **) &mypard, (ub4) counter );

		if ( fpConnection->checkError( parm_status, strErr ) ) {
			fErrorMessages.Append( strErr );
			return false;
		}

		// Loop only if a descriptor was successfully retrieved for
		// current  position, starting at 1

		while ( parm_status == OCI_SUCCESS ) {
			data_len = 0;
			col_name = 0;
			col_name_len = 0;

			// Retrieve the data type attribute
			if ( fpConnection->checkError( OCIAttrGet( (dvoid *) mypard, (ub4) OCI_DTYPE_PARAM, (dvoid *) &dtype,
										   (ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE, (OCIError *) eh ), strErr ) ) {
				fErrorMessages.Append( strErr );
				return false;
			}

			if ( fpConnection->checkError( OCIAttrGet( (dvoid *) mypard, (ub4) OCI_DTYPE_PARAM, (dvoid *) &data_len,
										   (ub4 *) 0, (ub4) OCI_ATTR_DISP_SIZE, (OCIError *) eh ), strErr ) ) {
				fErrorMessages.Append( strErr );
				return false;
			}

			// Retrieve the column name attribute
			if ( fpConnection->checkError( OCIAttrGet( (dvoid *) mypard, (ub4) OCI_DTYPE_PARAM, (dvoid **) &col_name,
										   (ub4 *) &col_name_len, (ub4) OCI_ATTR_NAME, (OCIError *) eh ), strErr ) ) {
				fErrorMessages.Append( strErr );
				return false;
			}

			Anything param;
			param[0L] = Anything(); // dummy
			param[OracleColumn::eColumnName] = String( (char *) col_name, col_name_len );
			param[OracleColumn::eColumnType] = dtype;
			param[OracleColumn::eDataLength] = (int) data_len;
			desc.Append( param );

			// increment counter and get next descriptor, if there is one
			++counter;
			parm_status = OCIParamGet( pStmthp, OCI_HTYPE_STMT, eh, (void **) &mypard, (ub4) counter );
		}
	}
	TraceAny(desc, "descriptions");
	return true;
}

bool OracleStatement::DefineOutputArea( Anything &desc )
{
	StartTrace(OracleStatement.DefineOutputArea);
	// use 'desc' to allocate output area used by oracle library
	// to store fetched data (binary Anything buffers are allocated and
	// stored within the 'desc' structure... for automatic storage
	// management)

	OCIError *eh = fpConnection->ErrorHandle();
	OCIStmt *pStmthp( getHandle() ); // OCI statement handle
	String strErr;

	for ( ub4 i = 0; i < desc.GetSize(); i++ ) {
		Anything &col = desc[static_cast<long> ( i )];

		sb4 dummy;
		OCIDefine *defHandle = 0;
		long len;
		if ( col[OracleColumn::eColumnType].AsLong() == SQLT_DAT ) {
			// --- date field

			col[OracleColumn::eDataLength] = 9;
			col[OracleColumn::eColumnType] = SQLT_STR;

			len = col[OracleColumn::eDataLength].AsLong() + 1;

		} else if ( col[OracleColumn::eColumnType].AsLong() == SQLT_NUM ) {
			// --- number field
			//			if (col[OracleColumn::eScale].AsLong() != 0) {					// dont need floats now
			//				col[OracleColumn::eDataLength]= (sword) sizeof(float);
			//				col[OracleColumn::eColumnType]= SQLT_FLT;
			//            } else {
			col[OracleColumn::eDataLength] = (sword) sizeof(sword);
			col[OracleColumn::eColumnType] = SQLT_INT;
			//            }
			len = col[OracleColumn::eDataLength].AsLong();

		} else {
			len = col[OracleColumn::eDataLength].AsLong() + 1;
		}

		// allocate space for the returned data
		Anything buf = Anything( (void *) 0, len );
		col[OracleColumn::eRawBuf] = buf;

		// accocate space for NULL indicator
		Anything indicator = Anything( (void *) 0, sizeof(OCIInd) );
		col[OracleColumn::eIndicator] = indicator;

		// allocate space to store effective result size
		Anything effectiveSize = Anything( (void *) 0, sizeof(ub2) );
		col[OracleColumn::eEffectiveLength] = effectiveSize;

		if ( fpConnection->checkError( OCIDefineByPos( pStmthp, &defHandle, eh, i + 1, (void *) buf.AsCharPtr(), len,
									   col[OracleColumn::eColumnType].AsLong(), (dvoid *) indicator.AsCharPtr(),
									   (ub2 *) effectiveSize.AsCharPtr(), 0, OCI_DEFAULT ), strErr ) ) {
			fErrorMessages.Append( strErr );
			return false;
		}
	}
	return true;
}
