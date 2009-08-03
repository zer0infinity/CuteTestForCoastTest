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
#include "AnyIterators.h"
#include "Dbg.h"

OracleStatement::OracleStatement( OracleConnection *pConn, String const &strStmt ) :
	fpConnection( pConn ), fStmt( strStmt ), fStatus( UNPREPARED ), fStmtType( STMT_UNKNOWN )
{
}

OracleStatement::OracleStatement( OracleConnection *pConn, OCIStmt *phStmt ) :
	fpConnection( pConn ), fStmthp( phStmt ), fStmt(), fStatus( PREPARED ), fStmtType( STMT_SELECT )
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
	String strError( 128L );
	bool bSuccess( !fpConnection->checkError( OCIHandleAlloc( fpConnection->getEnvironment().EnvHandle(),
				   fStmthp.getVoidAddr(), OCI_HTYPE_STMT, 0, 0 ), strError ) );
	if ( !bSuccess ) {
		fErrorMessages.Append( strError );
	}
	return bSuccess;
}

void OracleStatement::Cleanup()
{
	StartTrace(OracleStatement.Cleanup);
	AnyExtensions::Iterator<ROAnything> aAnyIter( fSubStatements );
	ROAnything roaObject;
	while ( aAnyIter.Next( roaObject ) ) {
		OracleStatement *pStmt = SafeCast(roaObject.AsIFAObject(0), OracleStatement);
		if ( pStmt ) {
			delete pStmt;
		}
	}
	fSubStatements = Anything();
	// we need to cleanup the sub-statement handle
	fStmthp.reset();
	fStatus = UNPREPARED;
}

OracleStatement::Status OracleStatement::execute( ExecMode mode )
{
	StartTrace1(OracleStatement.execute, "statement type " << (long)fStmtType);
	if ( fStatus == PREPARED ) {
		// depending on the query type, we could use 'scrollable cursor mode' ( OCI_STMT_SCROLLABLE_READONLY )
		// executes a SQL statement (first row is also fetched only if iters > 0)
		ub4 iters = ( fStmtType == STMT_SELECT ? 0 : 1 );
		sword status = OCIStmtExecute( fpConnection->SvcHandle(), getHandle(), fpConnection->ErrorHandle(), iters, 0,
									   NULL, NULL, mode );
		if ( status == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO || status == OCI_NO_DATA ) {
			// how can we find out about the result type? -> see Prepare
			switch ( fStmtType ) {
				case STMT_SELECT:
					fStatus = RESULT_SET_AVAILABLE;
					break;
				case STMT_CREATE:
				case STMT_DROP:
				case STMT_ALTER:
					fStatus = UPDATE_COUNT_AVAILABLE;
					break;
				case STMT_DELETE:
				case STMT_INSERT:
				case STMT_UPDATE:
					fStatus = UPDATE_COUNT_AVAILABLE;
					break;
				case STMT_BEGIN:
					fStatus = UPDATE_COUNT_AVAILABLE;
					break;
				default:
					fStatus = PREPARED;
			}
		} else {
			throw OracleException( *fpConnection, status );
		}
	}
	return fStatus;
}

sword OracleStatement::Fetch( ub4 numRows )
{
	StatTrace(OracleStatement.Fetch, "fetching " << (long)numRows << " rows", Storage::Current());
	// fetch another row
	return OCIStmtFetch2( getHandle(), fpConnection->ErrorHandle(), numRows, OCI_FETCH_NEXT, 0, OCI_DEFAULT );
}

unsigned long OracleStatement::getUpdateCount() const
{
	ub4 count( 0 );
	OCIAttrGet( getHandle(), OCI_HTYPE_STMT, (dvoid *) &count, 0, OCI_ATTR_ROW_COUNT, fpConnection->ErrorHandle() );
	StatTrace(OracleStatement.getUpdateCount, "update count " << (long)count, Storage::Current())
	return count;
}

bool OracleStatement::Prepare()
{
	StartTrace(OracleStatement.Prepare);
	// prepare SQL statement for execution
	String strErr( 128L );
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

void OracleStatement::setPrefetchRows( long lPrefetchRows )
{
	String strErr( 128L );
	ub4 prefetch( lPrefetchRows );
	if ( fpConnection->checkError( OCIAttrSet( getHandle(), OCI_HTYPE_STMT, &prefetch, sizeof ( prefetch ),
								   OCI_ATTR_PREFETCH_ROWS, fpConnection->ErrorHandle() ) ) ) {
		fErrorMessages.Append( strErr );
	}
}

OracleResultsetPtr OracleStatement::getResultset()
{
	StartTrace(OracleStatement.getResultset);
	OracleResultsetPtr pResult;
	if ( fStatus == RESULT_SET_AVAILABLE ) {
		pResult = OracleResultsetPtr( new ( Storage::Current() ) OracleResultset( *this ) );
	} else {
		String strMessage( "Error - getResultset failed, no resultset available, current status is " );
		strMessage << (long) fStatus;
		throw OracleException( *fpConnection, strMessage );
	}
	return pResult;
}

OracleResultsetPtr OracleStatement::getCursor( long lColumnIndex )
{
	StartTrace1(OracleStatement.getCursor, "column index: " << lColumnIndex);
	OracleResultsetPtr pResult;
	--lColumnIndex;
	if ( lColumnIndex >= 0 && lColumnIndex < fDescriptions.GetSize() ) {
		OracleStatement::Description::Element aDescEl = fDescriptions[lColumnIndex];
		Anything value;
		SubTrace(TraceColType, "column type is: " << aDescEl.AsLong("Type") << " indicator: " << aDescEl.AsLong("Indicator"))
		SubTrace(TraceBuf, "buf ptr " << (long) (aDescEl.AsCharPtr("RawBuf")) );
		switch ( aDescEl.AsLong( "Type" ) ) {
			case SQLT_RSET:
			case SQLT_CUR: {
				Trace("SQLT_RSET/SQLT_CUR")
				OCIStmt *phStmt = * ( (OCIStmt **) aDescEl.AsCharPtr( "RawBuf" ) );
				Trace("retrieved statement handle pointer &" << (long)phStmt)
				if ( phStmt ) {
					OracleStatement *pStmt = new ( Storage::Current() ) OracleStatement( fpConnection, phStmt );
					fSubStatements.Append( pStmt );
					pResult = OracleResultsetPtr( new ( Storage::Current() ) OracleResultset( *pStmt ) );
				}
				break;
			}
			default:
				Trace("wrong type accessed")
				break;
		}
	}
	return pResult;
}

void OracleStatement::setSPDescription( ROAnything roaSPDescription, const String &strReturnName )
{
	fDescriptions = roaSPDescription;
	// strReturnName is only set when we are constructing a stored function statement
	if ( strReturnName.Length() > 0 ) {
		fDescriptions[0]["Name"] = strReturnName;
	}
}

OracleStatement::Description &OracleStatement::GetOutputDescription()
{
	StartTrace1( OracleStatement.GetOutputDescription, "statement type " << (long)fStmtType );

	if ( fDescriptions.GetSize() == 0L ) {
		if ( getStatementType() == STMT_SELECT ) {
			OCIError *eh( getConnection()->ErrorHandle() );

			OCIParam *mypard;
			ub2 dtype;
			ub4 data_len;
			text *col_name;
			ub4 col_name_len;

			// Request a parameter descriptor for position 1 in the select-list
			ub4 counter = 1;
			sword parm_status = OCIParamGet( getHandle(), OCI_HTYPE_STMT, eh, (void **) &mypard, counter );
			if ( parm_status != OCI_SUCCESS ) {
				throw OracleException( *getConnection(), parm_status );
			}

			// Loop only if a descriptor was successfully retrieved for
			// current  position, starting at 1

			while ( parm_status == OCI_SUCCESS ) {
				data_len = 0;
				col_name = 0;
				col_name_len = 0;

				// Retrieve the data type attribute
				parm_status = OCIAttrGet( (dvoid *) mypard, OCI_DTYPE_PARAM, (dvoid *) &dtype, 0, OCI_ATTR_DATA_TYPE, eh );
				if ( parm_status != OCI_SUCCESS ) {
					throw OracleException( *getConnection(), parm_status );
				}

				parm_status = OCIAttrGet( (dvoid *) mypard, OCI_DTYPE_PARAM, (dvoid *) &data_len, 0, OCI_ATTR_DISP_SIZE,
										  eh );
				if ( parm_status != OCI_SUCCESS ) {
					throw OracleException( *getConnection(), parm_status );
				}

				// Retrieve the column name attribute
				parm_status = OCIAttrGet( (dvoid *) mypard, OCI_DTYPE_PARAM, (dvoid **) &col_name, (ub4 *) &col_name_len,
										  OCI_ATTR_NAME, eh );
				if ( parm_status != OCI_SUCCESS ) {
					throw OracleException( *getConnection(), parm_status );
				}

				Anything param;
				param["Name"] = String( (char *) col_name, col_name_len );
				param["Type"] = dtype;
				param["Length"] = (int) data_len;
				param["Idx"] = (long) counter;
				fDescriptions.Append( param );

				// increment counter and get next descriptor, if there is one
				++counter;
				parm_status = OCIParamGet( getHandle(), OCI_HTYPE_STMT, eh, (void **) &mypard, counter );
			}
		}
	}
	Trace( "column descriptions (" << fDescriptions.GetSize() << ")" )
	return fDescriptions;
}

bool OracleStatement::DefineOutputArea()
{
	StartTrace(OracleStatement.DefineOutputArea);
	// use fBuffer to allocate output area used by oracle library
	// to store fetched data (binary Anything buffers are allocated and
	// stored within the fBuffer structure... for automatic storage
	// management)

	OCIError *eh = getConnection()->ErrorHandle();
	OCIStmt *pStmthp( getHandle() );

	AnyExtensions::Iterator<OracleStatement::Description, OracleStatement::Description::Element> aDescIter(
		fDescriptions );
	OracleStatement::Description::Element aDescEl;
	while ( aDescIter.Next( aDescEl ) ) {
		String strColName( aDescEl.AsString( "Name" ) );
		Trace("colname@" << aDescIter.Index() << " [" << strColName << "]");
		long lColIndex = aDescEl.AsLong( "Idx" );

		long len;
		if ( aDescEl.AsLong( "Type" ) == SQLT_DAT ) {
			// --- date field
			aDescEl["Length"] = 9;
			aDescEl["Type"] = SQLT_STR;
			len = aDescEl.AsLong( "Length" ) + 1;
		} else if ( aDescEl.AsLong( "Type" ) == SQLT_NUM ) {
			aDescEl["Length"] = (sword) sizeof(sword);
			aDescEl["Type"] = SQLT_INT;
			len = aDescEl.AsLong( "Length" );
		} else {
			aDescEl["Type"] = aDescEl.AsLong( "Type" );
			len = aDescEl.AsLong( "Length" ) + 1;
		}

		// allocate space for the returned data
		Anything buf = Anything( (void *) 0, len );
		aDescEl["RawBuf"] = buf;

		// accocate space for NULL indicator
		Anything indicator = Anything( (void *) 0, sizeof(OCIInd) );
		aDescEl["Indicator"] = indicator;

		// allocate space to store effective result size
		Anything effectiveSize = Anything( (void *) 0, sizeof(ub2) );
		aDescEl["EffectiveLength"] = effectiveSize;

		OCIDefine *defHandle = 0;
		sword status = OCIDefineByPos( pStmthp, &defHandle, eh, lColIndex, (void *) buf.AsCharPtr(), len,
									   aDescEl.AsLong( "Type" ), (dvoid *) indicator.AsCharPtr(), (ub2 *) effectiveSize.AsCharPtr(), 0,
									   OCI_DEFAULT );
		if ( status != OCI_SUCCESS ) {
			throw OracleException( *getConnection(), status );
		}
	}
	return true;
}

String OracleStatement::getString( long lColumnIndex )
{
	StartTrace1(OracleStatement.getString, "col index: " << lColumnIndex);
	String strColValue( 128L );
	// oracle always uses 1-based indexes...except for functions...
	--lColumnIndex;
	if ( lColumnIndex >= 0 && lColumnIndex < fDescriptions.GetSize() ) {
		OracleStatement::Description::Element aDescEl = fDescriptions[lColumnIndex];
		Anything value;
		SubTrace(TraceColType, "column type is: " << aDescEl.AsLong("Type") << " indicator: " << aDescEl.AsLong("Indicator"))
		SubTrace(TraceBuf, "buf ptr " << (long) (aDescEl.AsCharPtr("RawBuf")) << " length: " << (long) * ((ub2 *) aDescEl.AsCharPtr("EffectiveLength")));
		switch ( aDescEl.AsLong( "Type" ) ) {
			case SQLT_INT:
				Trace("SQLT_INT")
				if ( aDescEl.AsLong( "Indicator" ) == OCI_IND_NULL ) {
					value = 0L;
				} else {
					value = ( * ( (sword *) aDescEl.AsCharPtr( "RawBuf" ) ) );
				}
				break;
			case SQLT_FLT:
				Trace("SQLT_FLT")
				if ( aDescEl.AsLong( "Indicator" ) == OCI_IND_NULL ) {
					value = 0.0f;
				} else {
					value = ( * ( (float *) aDescEl.AsCharPtr( "RawBuf" ) ) );
				}
				break;
			case SQLT_STR:
				Trace("SQLT_STR")
				if ( aDescEl.AsLong( "Indicator" ) == OCI_IND_NULL ) {
					value = "";
				} else {
					value = String( aDescEl.AsCharPtr( "RawBuf" ) );
				}
				break;
			default:
				SubTraceBuf(TraceBuf, aDescEl.AsCharPtr("RawBuf"), *((ub2 *) aDescEl.AsCharPtr("EffectiveLength")));
				if ( aDescEl.AsLong( "Indicator" ) == OCI_IND_NULL ) {
					value = "";
				} else {
					value
					= String( static_cast<void *> ( const_cast<char *> ( aDescEl.AsCharPtr( "RawBuf" ) ) ),
							  (long) * ( reinterpret_cast<ub2 *> ( const_cast<char *> ( aDescEl.AsCharPtr(
											 "EffectiveLength" ) ) ) ) );
				}
				break;
		}
		strColValue = value.AsString();
	}
	Trace("returning value [" << strColValue << "]")
	return strColValue;
}

void OracleStatement::registerOutParam( long lBindPos, BindType bindType, long lBufferSize )
{
	StartTrace1(OracleStatement.registerOutParam, "column index " << lBindPos << " bindType " << (long)bindType);
	long len( 0L );
	Anything buf;
	sword status;
	--lBindPos;
	if ( lBindPos >= 0 && lBindPos < fDescriptions.GetSize() ) {
		OracleStatement::Description::Element aDescEl = fDescriptions[lBindPos];
		switch ( bindType ) {
			case CURSOR:
				aDescEl["Type"] = SQLT_RSET;
				break;
			case STRING:
				aDescEl["Type"] = SQLT_STR;
				break;
			default:
				break;
		}

		switch ( aDescEl.AsLong( "Type" ) ) {
			case SQLT_DAT:
				aDescEl["Length"] = 9;
				aDescEl["Type"] = SQLT_STR;
				len = aDescEl.AsLong( "Length" ) + 1;
				break;
			case SQLT_CHR:
			case SQLT_STR: {
				if ( lBufferSize >= 0 ) {
					aDescEl["Length"] = lBufferSize;
				}
				aDescEl["Type"] = SQLT_STR;
				len = aDescEl.AsLong( "Length" ) + 1;
				break;
			}
			case SQLT_NUM:
				aDescEl["Length"] = 38;
				aDescEl["Type"] = SQLT_STR;
				len = aDescEl.AsLong( "Length" ) + 1;
				break;
			case SQLT_CUR:
			case SQLT_RSET: {
				aDescEl["Type"] = SQLT_RSET;
				OCIStmt *phCursor( 0 );
				status = OCIHandleAlloc( getConnection()->getEnvironment().EnvHandle(), (void **) &phCursor,
										 OCI_HTYPE_STMT, 0, // extra memory to allocate
										 NULL ); // pointer to user-memory
				len = sizeof(OCIStmt *);
				aDescEl["Length"] = len;
				buf = Anything( (void *) &phCursor, len );
				Trace("statement handle &" << (long)phCursor)
				break;
			}
			default:
				len = aDescEl.AsLong( "Length" ) + 1;
				break;
		}

		if ( buf.IsNull() ) {
			buf = Anything( (void *) 0, len );
		}

		aDescEl["RawBuf"] = buf;

		status = bindColumn( lBindPos + 1, aDescEl, len );
		if ( status != OCI_SUCCESS ) {
			throw OracleException( *getConnection(), status );
		}
	}
}

void OracleStatement::setString( long lBindPos, String const &strValue )
{
	StartTrace1(OracleStatement.setString, "column index " << lBindPos << " value [" << strValue << "]");
	long len( 0L );
	Anything buf;
	--lBindPos;
	if ( lBindPos >= 0 && lBindPos < fDescriptions.GetSize() ) {
		sword status;
		OracleStatement::Description::Element aDescEl = fDescriptions[lBindPos];

		aDescEl["Length"] = strValue.Length();
		aDescEl["Type"] = SQLT_STR;
		len = aDescEl.AsLong( "Length" ) + 1;
		buf = Anything( (void *) (const char *) strValue, len );

		aDescEl["RawBuf"] = buf;

		status = bindColumn( lBindPos + 1, aDescEl, len, strValue.IsEqual( "NULL" ) );
		if ( status != OCI_SUCCESS ) {
			throw OracleException( *getConnection(), status );
		}
	}
}

sword OracleStatement::bindColumn( long lBindPos, OracleStatement::Description::Element &aDescEl, long len,
								   bool bIsNull )
{
	StartTrace1(OracleStatement.bindColumn, "column index " << lBindPos << " is " << (bIsNull ? "" : "not ") << "null");

	// allocate space for NULL indicator
	OCIInd nullInd( 0 );
	if ( bIsNull ) {
		nullInd = -1;
	}
	aDescEl["Indicator"] = Anything( (void *) &nullInd, sizeof(OCIInd) );

	// allocate space to store effective result size
	ub2 ub2Len( (ub2) aDescEl.AsLong( "Length", 0L ) );
	aDescEl["EffectiveLength"] = Anything( (void *) (ub2 *) &ub2Len, sizeof(ub2) );

	OCIBind *bndp = 0;
	return OCIBindByPos( getHandle(), &bndp, getConnection()->ErrorHandle(), (ub4) lBindPos,
						 (dvoid *) aDescEl.AsCharPtr( "RawBuf" ), (sword) len, aDescEl.AsLong( "Type" ),
						 (dvoid *) aDescEl.AsCharPtr( "Indicator" ), 0, 0, 0, 0, OCI_DEFAULT );
}
