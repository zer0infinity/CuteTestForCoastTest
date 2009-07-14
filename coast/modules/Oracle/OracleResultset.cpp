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
#include "AnyIterators.h"
#include "Anything.h"
#include "Dbg.h"

OracleResultset::~OracleResultset()
{
}

ROAnything OracleResultset::GetOutputDescription()
{
	StartTrace( OracleResultset.GetOutputDescription );

	if ( fanyDescription.IsNull() ) {
		if ( frStmt.getStatementType() == OCI_STMT_SELECT ) {
			String strErr;
			OCIError *eh( frStmt.getConnection()->ErrorHandle() );

			OCIParam *mypard;
			ub2 dtype;
			ub4 data_len;
			text *col_name;
			ub4 col_name_len;

			// Request a parameter descriptor for position 1 in the select-list
			ub4 counter = 1;
			sword parm_status = OCIParamGet( frStmt.getHandle(), OCI_HTYPE_STMT, eh, (void **) &mypard, (ub4) counter );

			if ( parm_status != OCI_SUCCESS ) {
				throw OracleException( *frStmt.getConnection(), parm_status );
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
					throw OracleException( *frStmt.getConnection(), parm_status );
				}

				parm_status = OCIAttrGet( (dvoid *) mypard, OCI_DTYPE_PARAM, (dvoid *) &data_len, 0, OCI_ATTR_DISP_SIZE,
										  eh );
				if ( parm_status != OCI_SUCCESS ) {
					throw OracleException( *frStmt.getConnection(), parm_status );
				}

				// Retrieve the column name attribute
				parm_status = OCIAttrGet( (dvoid *) mypard, OCI_DTYPE_PARAM, (dvoid **) &col_name, (ub4 *) &col_name_len,
										  OCI_ATTR_NAME, eh );
				if ( parm_status != OCI_SUCCESS ) {
					throw OracleException( *frStmt.getConnection(), parm_status );
				}

				Anything param;
				param["Name"] = String( (char *) col_name, col_name_len );
				param["Type"] = dtype;
				param["Length"] = (int) data_len;
				param["Idx"] = (long) counter;
				fanyDescription.Append( param );

				// increment counter and get next descriptor, if there is one
				++counter;
				parm_status = OCIParamGet( frStmt.getHandle(), OCI_HTYPE_STMT, eh, (void **) &mypard, (ub4) counter );
			}
		}
	}
	TraceAny( fanyDescription, "column descriptions (" << fanyDescription.GetSize() << ")" );
	return ROAnything( fanyDescription );
}

bool OracleResultset::DefineOutputArea()
{
	StartTrace(OracleResultset.DefineOutputArea);
	// use fBuffer to allocate output area used by oracle library
	// to store fetched data (binary Anything buffers are allocated and
	// stored within the fBuffer structure... for automatic storage
	// management)

	OCIError *eh = frStmt.getConnection()->ErrorHandle();
	OCIStmt *pStmthp( frStmt.getHandle() );
	String strErr;

	fBuffer = fanyDescription.DeepClone();
	AnyExtensions::Iterator<Anything> aAnyIter( fBuffer );
	Anything anyCol;
	while ( aAnyIter.Next( anyCol ) ) {
		String strColName( anyCol["Name"].AsString() );
		Trace("colname@" << aAnyIter.Index() << " [" << strColName << "]");
		long lColIndex = anyCol["Idx"].AsLong();

		long len;
		if ( anyCol["Type"].AsLong() == SQLT_DAT ) {
			// --- date field
			anyCol["Length"] = 9;
			anyCol["Type"] = SQLT_STR;
			len = anyCol["Length"].AsLong() + 1;
		} else if ( anyCol["Type"].AsLong() == SQLT_NUM ) {
			anyCol["Length"] = (sword) sizeof(sword);
			anyCol["Type"] = SQLT_INT;
			len = anyCol["Length"].AsLong();
		} else {
			anyCol["Type"] = anyCol["Type"].AsLong();
			len = anyCol["Length"].AsLong() + 1;
		}

		// allocate space for the returned data
		Anything buf = Anything( (void *) 0, len );
		anyCol["RawBuf"] = buf;

		// accocate space for NULL indicator
		Anything indicator = Anything( (void *) 0, sizeof(OCIInd) );
		anyCol["Indicator"] = indicator;

		// allocate space to store effective result size
		Anything effectiveSize = Anything( (void *) 0, sizeof(ub2) );
		anyCol["EffectiveLength"] = effectiveSize;

		OCIDefine *defHandle = 0;
		sword status = OCIDefineByPos( pStmthp, &defHandle, eh, lColIndex, (void *) buf.AsCharPtr(), len,
									   anyCol["Type"].AsLong(), (dvoid *) indicator.AsCharPtr(), (ub2 *) effectiveSize.AsCharPtr(), 0,
									   OCI_DEFAULT );
		if ( status != OCI_SUCCESS ) {
			throw OracleException( *frStmt.getConnection(), status );
		}
	}
	return true;
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
				// if ( status == SQL_NO_DATA ) ??
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
	String strColValue( 32L );
	// oracle always uses 1-based indexes...
	--lColumnIndex;
	if ( lColumnIndex >= 0 && lColumnIndex < fBuffer.GetSize() ) {
		Anything &desc = fBuffer[lColumnIndex], value;
		SubTraceAny(TraceDesc, desc, "desc at col:" << lColumnIndex)
		SubTrace(TraceColType, "column type is: " << desc["Type"].AsLong() << " indicator: " << desc["Indicator"].AsLong())
		SubTrace(TraceBuf, "buf ptr " << (long) (desc["RawBuf"].AsCharPtr()) << " length: " << (long) * ((ub2 *) desc["EffectiveLength"].AsCharPtr()));
		switch ( desc["Type"].AsLong() ) {
			case SQLT_INT:
				Trace("SQLT_INT")
				if ( desc["Indicator"].AsLong() == OCI_IND_NULL ) {
					value = 0L;
				} else {
					value = ( * ( (sword *) desc["RawBuf"].AsCharPtr() ) );
				}
				break;
			case SQLT_FLT:
				Trace("SQLT_FLT")
				if ( desc["Indicator"].AsLong() == OCI_IND_NULL ) {
					value = 0.0f;
				} else {
					value = ( * ( (float *) desc["RawBuf"].AsCharPtr() ) );
				}
				break;
			case SQLT_STR:
				Trace("SQLT_STR")
				if ( desc["Indicator"].AsLong() == OCI_IND_NULL ) {
					value = "";
				} else {
					value = String( desc["RawBuf"].AsCharPtr() );
				}
				break;
			default:
				SubTraceBuf(TraceBuf, desc["RawBuf"].AsCharPtr(), *((ub2 *) desc["EffectiveLength"].AsCharPtr()));
				if ( desc["Indicator"].AsLong() == OCI_IND_NULL ) {
					value = "";
				} else {
					value
					= String(
						  static_cast<void *> ( const_cast<char *> ( desc["RawBuf"].AsCharPtr() ) ),
						  (long) * ( reinterpret_cast<ub2 *> ( const_cast<char *> ( desc["EffectiveLength"].AsCharPtr() ) ) ) );
				}
				break;
		}
		strColValue = value.AsString();
	}
	Trace("returning value [" << strColValue << "]")
	return strColValue;
}
