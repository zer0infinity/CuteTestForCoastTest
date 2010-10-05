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

#include <algorithm>
#include <cstring>

OracleStatement::OracleStatement( OracleConnection *pConn, String const &strStmt ) :
	fpConnection( pConn ), fStmt( strStmt ), fStatus( UNPREPARED ), fStmtType( STMT_UNKNOWN )
{
}

OracleStatement::OracleStatement( OracleConnection *pConn, OCIStmt *phStmt ) :
	fpConnection( pConn ), fStmt(), fStmthp( phStmt ), fStatus( PREPARED ), fStmtType( STMT_SELECT )
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

OracleStatement::Status OracleStatement::execute( ExecMode mode, long lIterations )
{
	StartTrace1(OracleStatement.execute, "statement type " << (long)fStmtType);
	if ( fStatus == PREPARED ) {
		// depending on the query type, we could use 'scrollable cursor mode' ( OCI_STMT_SCROLLABLE_READONLY )
		// executes a SQL statement (first row is also fetched only if iters > 0)
		ub4 iters = ( fStmtType == STMT_SELECT ? 0 : lIterations );
		Trace("iterations:" << (long)iters);
		sword execStatus = OCIStmtExecute( fpConnection->SvcHandle(), getHandle(), fpConnection->ErrorHandle(), iters, 0,
										   NULL, NULL, mode );
		Trace("execution status:" << (long)execStatus);
		if ( execStatus == OCI_SUCCESS || execStatus == OCI_SUCCESS_WITH_INFO || execStatus == OCI_NO_DATA ) {
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
			throw OracleException( *fpConnection, execStatus );
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
	StatTrace(OracleStatement.getUpdateCount, "update count " << (long)count, Storage::Current());
	return count;
}

unsigned long OracleStatement::getErrorCount() const
{
	ub4 count( 0 );
	ErrHandleType aHandlePtr;
	if ( fpConnection->AllocateHandle( aHandlePtr ) ) {
		OCIAttrGet( getHandle(), OCI_HTYPE_STMT, (dvoid *) &count, 0, OCI_ATTR_NUM_DML_ERRORS, aHandlePtr.getHandle() );
		StatTrace(OracleStatement.getErrorCount, "error count " << (long)count, Storage::Current());
	}
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

OracleResultsetPtr OracleStatement::getCursor( long lColumnIndex, long lRowIdx )
{
	StartTrace1(OracleStatement.getCursor, "column index: " << lColumnIndex << " rowidx:" << lRowIdx);
	OracleResultsetPtr pResult;
	--lColumnIndex;
	if ( lColumnIndex >= 0 && lColumnIndex < fDescriptions.GetSize() ) {
		OracleStatement::Description::Element aDescEl = fDescriptions[lColumnIndex];
		Anything value;
		SubTrace(TraceColType, "column type is: " << aDescEl.AsLong("Type") << " indicator: " << (long)aDescEl.getIndicatorValue(lRowIdx));
		SubTrace(TraceBuf, "buf ptr " << (long) (aDescEl.getRawBufferPtr(lRowIdx)) );
		switch ( aDescEl.AsLong( "Type" ) ) {
			case SQLT_RSET:
			case SQLT_CUR: {
				Trace("SQLT_RSET/SQLT_CUR");
				OCIStmt *phStmt = * ( (OCIStmt **) aDescEl.getRawBufferPtr( lRowIdx ) );
				Trace("retrieved statement handle pointer &" << (long)phStmt);
				if ( phStmt ) {
					OracleStatement *pStmt = new ( Storage::Current() ) OracleStatement( fpConnection, phStmt );
					fSubStatements.Append( pStmt );
					pResult = OracleResultsetPtr( new ( Storage::Current() ) OracleResultset( *pStmt ) );
				}
				break;
			}
			default:
				Trace("wrong type accessed");
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
	Trace( "column descriptions (" << fDescriptions.GetSize() << ")" );
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
		Trace("colname@" << aDescIter.Index() << " [" << strColName << "] has type " << aDescEl.AsLong("Type"));
		long lColIndex = aDescEl.AsLong( "Idx" );
		long len = 0L;
		switch ( aDescEl.AsLong( "Type" ) ) {
			case SQLT_DAT:
				Trace("SQLT_DAT");
				aDescEl["Length"] = 9;
				aDescEl["Type"] = SQLT_STR;
				++len;
				break;
			case SQLT_NUM:
				Trace("SQLT_NUM");
				aDescEl["Length"] = 38;
				aDescEl["Type"] = SQLT_STR;
				++len;
				break;
			case SQLT_CHR:
			case SQLT_STR:
				++len;
				break;
			default:
				Trace("SQLT_DEFAULT");
				break;
		}
		len += aDescEl.AsLong( "Length" );

		// allocate space for the returned data
		Anything buf = Anything( static_cast<void *>(0), len );
		aDescEl["RawBuf"] = buf;

		// accocate space for NULL indicator
		Anything indicator = Anything( static_cast<void *>(0), sizeof(OCIInd) );
		aDescEl["Indicator"] = indicator;

		// allocate space to store effective result size
		Anything effectiveSize = Anything( static_cast<void *>(0), sizeof(ub2) );
		aDescEl["EffectiveLength"] = effectiveSize;

		OCIDefine *defHandle = 0;
		sword execStatus = OCIDefineByPos( pStmthp, &defHandle, eh, lColIndex, (void *) buf.AsCharPtr(), len,
										   aDescEl.AsLong( "Type" ), (dvoid *) indicator.AsCharPtr(), (ub2 *) effectiveSize.AsCharPtr(), 0,
										   OCI_DEFAULT );
		if ( execStatus != OCI_SUCCESS ) {
			throw OracleException( *getConnection(), execStatus );
		}
	}
	return true;
}

Anything OracleStatement::getValue( long lColumnIndex, long lRowIdx )
{
	StartTrace1(OracleStatement.getValue, "col index: " << lColumnIndex << " rowidx:" << lRowIdx);
	Anything anyColValue;
	// oracle always uses 1-based indexes...except for functions...
	--lColumnIndex;
	if ( lColumnIndex >= 0 && lColumnIndex < fDescriptions.GetSize() ) {
		OracleStatement::Description::Element aDescEl = fDescriptions[lColumnIndex];
		if ( aDescEl.getIndicatorValue(lRowIdx) == OCI_IND_NULL ) {
			Trace("result value is NULL, returning");
			return anyColValue;
		}
		SubTrace(TraceColType, "column type is: " << aDescEl.AsLong("Type") << " indicator: " << (long)aDescEl.getIndicatorValue(lRowIdx));
		SubTrace(TraceBuf, "buf ptr " << (long) (aDescEl.getRawBufferPtr(lRowIdx)) << " length: " << (long)aDescEl.getEffectiveLengthValue());
		switch ( aDescEl.AsLong( "Type" ) ) {
			case SQLT_INT:
				Trace("SQLT_INT");
				anyColValue = *reinterpret_cast<sword *>(aDescEl.getRawBufferPtr( lRowIdx ) );
				break;
			case SQLT_FLT:
				Trace("SQLT_FLT");
				anyColValue = *reinterpret_cast<float *>(aDescEl.getRawBufferPtr( lRowIdx ) );
				break;
			case SQLT_STR:
				Trace("SQLT_STR");
				anyColValue = aDescEl.getRawBufferPtr( lRowIdx );
				break;
			default:
				Trace("default type, using String");
				SubTraceBuf(TraceBuf, aDescEl.getRawBufferPtr( lRowIdx ), aDescEl.getEffectiveLengthValue());
				anyColValue = Anything(String(static_cast<void *> ( aDescEl.getRawBufferPtr( lRowIdx ) ), (long)aDescEl.getEffectiveLengthValue() ));
				break;
		}
	}
	TraceAny(anyColValue, "returned value");
	return anyColValue;
}

void OracleStatement::adjustColumnType( OracleStatement::Description::Element &aDescEl )
{
	StartTrace1(OracleStatement.adjustColumnType, "column index:" << aDescEl.AsLong("Idx", -1L) << " type:" << aDescEl.AsLong( "Type" ));
	switch ( aDescEl.AsLong( "Type" ) ) {
		case SQLT_DAT:
			Trace("SQLT_DAT");
			aDescEl["Length"] = 9;
			aDescEl["Type"] = SQLT_STR;
			break;
		case SQLT_CHR:
		case SQLT_STR: {
			Trace("SQLT_CHR/STR");
			aDescEl["Length"] = aDescEl.AsLong( "MaxStringBufferSize", 0L );
			aDescEl["Type"] = SQLT_STR;
			break;
		}
		case SQLT_NUM:
			Trace("SQLT_NUM");
			aDescEl["Length"] = 38;
			aDescEl["Type"] = SQLT_STR;
			break;
		case SQLT_CUR:
		case SQLT_RSET: {
			Trace("SQLT_CUR/RSET");
			aDescEl["Type"] = SQLT_RSET;
			aDescEl["Length"] = (long)sizeof(OCIStmt *);
			break;
		}
		default:
			Trace("SQLT_DEFAULT");
			break;
	}
}

void OracleStatement::prepareAndBindBuffer( OracleStatement::Description::Element &aDescEl, long lBindPos, long lRows)
{
	StartTrace1(OracleStatement.prepareAndBindBuffer, "bindPos:" << lBindPos << "  numRows:" << lRows);

	long len( aDescEl["Length"].AsLong() );
	Anything buf;
	if ( aDescEl["Type"] != SQLT_RSET ) {
		// add one byte for 'terminating' character
		++len;
	}
	Trace("allocating buffer of size len*rows=" << len << "*" << lRows << "=" << len * lRows);
	buf = Anything( (void *)0, len * lRows );
	aDescEl["RawBuf"] = buf;

	// allocate space for NULL indicator
	// a OCIInd value of 0 equals using a non-NULL value
	aDescEl["Indicator"] = Anything( static_cast<void *>(0), sizeof(OCIInd) * lRows );

	// fake effective result size
	// only used when not using bind
	aDescEl["EffectiveLength"] = Anything( static_cast<void *>(0), sizeof(ub2) * lRows );

	sword execStatus;
	execStatus = bindColumn( lBindPos, aDescEl, len );
	if ( execStatus != OCI_SUCCESS ) {
		throw OracleException( *getConnection(), execStatus );
	}
}

sword OracleStatement::bindColumn( long lBindPos, OracleStatement::Description::Element &aDescEl, long len )
{
	StartTrace1(OracleStatement.bindColumn, "column index " << lBindPos);
	OCIBind *bndp = 0;
	return OCIBindByPos( getHandle(), &bndp, getConnection()->ErrorHandle(), (ub4) lBindPos,
						 (dvoid *) aDescEl.getRawBufferPtr( 0L ), (sword) len, aDescEl.AsLong( "Type" ),
						 (dvoid *) aDescEl.getIndicatorBufferPtr( 0L ), 0, 0, 0, 0, OCI_DEFAULT );
}

void OracleStatement::adjustPrepareAndBindCols( long lRows)
{
	StartTrace1(OracleStatement.adjustPrepareAndBindCols, "numRows:" << lRows);
	AnyExtensions::Iterator<OracleStatement::Description, OracleStatement::Description::Element> aDescIter( GetOutputDescription() );
	OracleStatement::Description::Element aDescEl;
	while ( aDescIter.Next( aDescEl ) ) {
		// first bind variable position is 1
		long lBindPos = aDescEl.AsLong( "Idx" );
		adjustColumnType( aDescEl );
		prepareAndBindBuffer( aDescEl, lBindPos, lRows);
	}
}

void OracleStatement::bindAndFillInputValues( ROAnything const roaArrayValues )
{
	StartTrace1(OracleStatement.bindAndFillInputValues, "numRows:" << roaArrayValues.GetSize());

	adjustPrepareAndBindCols( roaArrayValues.GetSize() );

	AnyExtensions::Iterator<ROAnything> aRowIterator( roaArrayValues);
	ROAnything roaRowValues;
	while ( aRowIterator.Next(roaRowValues) ) {
		TraceAny(roaRowValues, "values of row:" << aRowIterator.Index());
		AnyExtensions::Iterator<OracleStatement::Description, OracleStatement::Description::Element> aDescIter( GetOutputDescription() );
		OracleStatement::Description::Element aDescEl;
		while ( aDescIter.Next( aDescEl ) ) {
			String strParamname( aDescEl.AsString( "Name" ) );
			ROAnything roaColValue = roaRowValues[strParamname];
			TraceAny(roaColValue, "value for column [" << strParamname << "]");
			fillRowColValue( aDescEl, aRowIterator.Index(), roaColValue );
		}
	}
}

namespace {
	void fillCBuffer( OracleStatement::Description::Element &aDescEl, char const *pcWhere, long len, long lRowIndex,
					  Anything &buf )
	{
		char *pC = const_cast<char *> ( aDescEl[pcWhere].AsCharPtr() );
		StatTrace(OracleStatement.fillRowColValue, "adjust for row in buffer, before:" << (long)pC << " after:" << (long)(pC + (len * lRowIndex)), Storage::Current());
		pC += ( len * lRowIndex );
		String strBuf( buf.AsString() );
		void *pDest = static_cast<void *> ( pC );
		void *pSrc = static_cast<void *> ( const_cast<char *>((const char *)strBuf) );
		memcpy( pDest, pSrc, strBuf.Length() );
	}
}

void OracleStatement::fillRowColValue( OracleStatement::Description::Element &aDescEl, long lRowIndex, ROAnything const roaValue )
{
	StartTrace1(OracleStatement.fillRowColValue, "Row:" << lRowIndex << " [" << roaValue.AsString() << "]");
	Anything buf;
	long len = aDescEl.AsLong("Length", 0L);
	if ( aDescEl["Type"].AsLong() == SQLT_RSET ) {
		OCIStmt *phCursor( 0 );
		sword execStatus;
		execStatus = OCIHandleAlloc( getConnection()->getEnvironment().EnvHandle(), (void **) &phCursor,
									 OCI_HTYPE_STMT, 0, // extra memory to allocate
									 NULL ); // pointer to user-memory
		buf = Anything( (void *) &phCursor, len );
		fillCBuffer(aDescEl, "RawBuf", len, lRowIndex, buf);
	} else {
		if ( roaValue.IsNull() ) {
			OCIInd nullInd( -1 );
			long lenNULL = sizeof(OCIInd);
			Anything indBuf = Anything( static_cast<void *>(&nullInd), lenNULL );
			fillCBuffer(aDescEl, "Indicator", lenNULL, lRowIndex, indBuf);
		} else {
			String strValue(roaValue.AsString());
			buf = Anything(static_cast<void *>(const_cast<char *>((const char *)strValue)), std::min(len, strValue.Length()));
		}
		fillCBuffer(aDescEl, "RawBuf", len + 1, lRowIndex, buf);
	}
	ub2 ub2len( len );
	buf = Anything( (void *) &ub2len, sizeof(ub2) );
	fillCBuffer(aDescEl, "EffectiveLength", sizeof(ub2), lRowIndex, buf);
}
