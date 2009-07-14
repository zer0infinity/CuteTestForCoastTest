/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "OracleDAImpl.h"

//--- standard modules used ----------------------------------------------------
//#include "OracleConnectionManager.h"
#include "OraclePooledConnection.h"
#include "OracleEnvironment.h"
#include "OracleConnection.h"
#include "OracleStatement.h"
#include "OracleColumn.h"
#include "OracleModule.h"
#include "SysLog.h"
#include "Timers.h"
#include "StringStream.h"
#include "TimeStamp.h"
#include "Dbg.h"
#include "AnyIterators.h"

#include <limits>
#include <algorithm>

//--- c-library modules used ---------------------------------------------------
#include <cstring>

static const long glStringBufferSize( 4096L );
//---- OracleDAImpl ---------------------------------------------------------
RegisterDataAccessImpl( OracleDAImpl);

OracleDAImpl::OracleDAImpl( const char *name ) :
	DataAccessImpl( name )
{
	StartTrace(OracleDAImpl.OracleDAImpl);
}

OracleDAImpl::~OracleDAImpl()
{
	StartTrace(OracleDAImpl.~OracleDAImpl);
}

IFAObject *OracleDAImpl::Clone() const
{
	StartTrace(OracleDAImpl.Clone);
	return new OracleDAImpl( fName );
}

String prefixResultSlot(String const &strPrefix, String const &strSlotName)
{
	String strReturn;
	if ( strPrefix.Length() ) {
		strReturn.Append(strPrefix).Append('.');
	}
	return strReturn.Append(strSlotName);
}

void OracleDAImpl::ProcessResultSet( OracleResultset &aRSet, ParameterMapper *& in, Context &ctx,
									 ResultMapper *& out, String strResultPrefix )
{
	StartTrace(OracleDAImpl.ProcessResultSet);
//	typedef std::vector<MetaData> MetaDataVector;
//	MetaDataVector rsetMetaData( aRSet.getColumnListMetaData() );
//	MetaThing desc;
//	MetaDataVector::iterator aIter( rsetMetaData.begin() );
//	int iNumCols( 0 );
//	while ( aIter != rsetMetaData.end() ) {
//		++iNumCols;
//		String strName( aIter->getString( MetaData::ATTR_NAME ).c_str() );
//		Anything anyColDesc;
//		anyColDesc["Name"] = strName;
//		anyColDesc["Idx"] = iNumCols;
//		anyColDesc["Type"] = (long) ( aIter->getInt( MetaData::ATTR_DATA_TYPE ) );
//		desc.Append( anyColDesc );
//		++aIter;
//	}TraceAny(desc, "column descriptions (" << iNumCols << ")")
//	String resultformat, resultsize;
//	in->Get( "DBResultFormat", resultformat, ctx );
//	bool bTitlesOnce = resultformat.IsEqual( "TitlesOnce" );
//	if ( bTitlesOnce ) {
//		// put column name information
//		AnyExtensions::Iterator<ROAnything> aAnyIter( desc );
//		ROAnything roaCol;
//		Anything temp;
//		while ( aAnyIter.Next( roaCol ) ) {
//			String strColName( roaCol["Name"].AsString() );
//			Trace("colname@" << aAnyIter.Index() << " [" << strColName << "]");
//			temp[strColName] = aAnyIter.Index();
//		}
//		out->Put( prefixResultSlot(strResultPrefix, "QueryTitles"), temp, ctx );
//	}
//	ResultSet::Status rsetStatus( aRSet.next() );
//	Trace("ResultSet->next() status: " << (long)rsetStatus );
//	long lRowCount( 0L );
//	while ( rsetStatus == ResultSet::DATA_AVAILABLE || rsetStatus == ResultSet::STREAM_DATA_AVAILABLE ) {
//		// do something
//		AnyExtensions::Iterator<ROAnything> aAnyIter( desc );
//		ROAnything roaCol;
//		Anything anyResult;
//		while ( aAnyIter.Next( roaCol ) ) {
//			long lColType( roaCol["Type"].AsLong() );
//			if ( lColType == OCCI_SQLT_CUR || lColType == OCCI_SQLT_RSET ) {
//			} else {
//				String strValueCol( aRSet.getString( roaCol["Idx"].AsLong() ).c_str() );
//				Trace("value of column [" << roaCol["Name"].AsString() << "] has value [" << strValueCol << "]")
//				if ( bTitlesOnce )
//					anyResult[aAnyIter.Index()] = strValueCol;
//				else
//					anyResult[roaCol["Name"].AsString()] = strValueCol;
//			}
//		}
//		out->Put( prefixResultSlot(strResultPrefix, "QueryResult"), anyResult, ctx );
//		++lRowCount;
//		rsetStatus = aRSet.next();
//		Trace("ResultSet.next() status: " << (long)rsetStatus );
//	}
//	bool bShowRowCount( true );
//	in->Get( "ShowQueryCount", bShowRowCount, ctx );
//	if ( bShowRowCount ) {
//		out->Put( prefixResultSlot(strResultPrefix, "QueryCount"), lRowCount, ctx );
//	}
}

bool OracleDAImpl::Exec( Context &ctx, ParameterMapper *in, ResultMapper *out )
{
	StartTrace(OracleDAImpl.Exec);
	bool bRet( false );
	DAAccessTimer(OracleDAImpl.Exec, fName, ctx);
	OracleModule *pModule = SafeCast(WDModule::FindWDModule("OracleModule"), OracleModule);
	ConnectionPool *pConnectionPool( 0 );
	if ( pModule && ( pConnectionPool = pModule->GetConnectionPool() ) ) {
		// we need the server and user to see if there is an existing and Open OraclePooledConnection
		String user, server, passwd;
		in->Get( "DBUser", user, ctx );
		in->Get( "DBPW", passwd, ctx );
		in->Get( "DBConnectString", server, ctx );
		Trace("USER IS:" << user);
		Trace("Connect string is [" << server << "]");
		out->Put( "QuerySource", server, ctx );

		bool bIsOpen = false, bDoRetry = true;
		long lTryCount( 1L );
		in->Get( "DBTries", lTryCount, ctx );
		if ( lTryCount < 1 ) {
			lTryCount = 1L;
		}
		// we slipped through and are ready to get a connection and execute a query
		// find a free OraclePooledConnection, we should always get a valid OraclePooledConnection here!
		while ( bDoRetry && --lTryCount >= 0 ) {
			OraclePooledConnection *pPooledConnection = NULL;
			String command;

			// --- establish db connection
			if ( !pConnectionPool->BorrowConnection( pPooledConnection, bIsOpen, server, user ) ) {
				Error( ctx, out, "Exec: unable to get OracleConnection" );
				bDoRetry = false;
			} else {
				if ( bIsOpen || pPooledConnection->Open( server, user, passwd ) ) {
					OracleConnection *pConnection( pPooledConnection->getConnection() );
					bIsOpen = true;
					OCIError *eh = pConnection->ErrorHandle();
					if ( DoPrepareSQL( command, ctx, in ) ) {
						OracleStatement aStmt( pConnection, command );
						if ( !aStmt.Prepare() ) {
							Error( ctx, out, aStmt.GetLastErrorMessage() );
						}
						String strErr("Exec: ");
						out->Put( "Query", command, ctx );
						ub2 fncode = 0;
						MetaThing desc;
						// --- determine format of retrieved data
						if ( aStmt.GetOutputDescription( desc, fncode ) ) {
							TraceAny(desc, "row description");
							if ( fncode == OCI_STMT_SELECT ) {
								if ( aStmt.DefineOutputArea( desc ) ) {
									// --- map header information
									String resultformat, resultsize;
									in->Get( "DBResultFormat", resultformat, ctx );

									bool bTitlesOnce = resultformat.IsEqual( "TitlesOnce" );
									//FIXME: TitlesAlways should be possible too
									if ( bTitlesOnce ) {
										// put column name information
										Anything temp;
										for ( sword col = 0; col < desc.GetSize(); ++col ) {
											String strColName( desc[col][OracleColumn::eColumnName].AsString() );
											Trace("colname@" << col << " [" << strColName << "]");
											temp[strColName] = (long) col;
										}
										out->Put( "QueryTitles", temp, ctx );
									}

									//FIXME: implement result size limit by kB
									// in->Get("DBMaxResultSize", resultsize, ctx);
									// --- get data rows
									FetchRowData( desc, in, out, aStmt, ctx );
									bRet = true;
									bDoRetry = false;
								} else {
									Error( ctx, out, aStmt.GetLastErrorMessage() );
								}
							} else {
								if ( pConnection->checkError( aStmt.Execute( OCI_COMMIT_ON_SUCCESS ), strErr ) ) {
									Error( ctx, out, strErr );
								} else {
									// cancel cursor
									if ( pConnection->checkError( aStmt.Fetch( 0 ), strErr ) ) {
										Error( ctx, out, strErr );
									}
								}
							}
						} else {
							Error( ctx, out, aStmt.GetLastErrorMessage() );
						}
					} else if ( DoPrepareSP( command, ctx, in ) ) {
						Trace("STORED PROCEDURE IS [" << command << "]");
						bool error( false );
						String strErr;

						MetaThing desc;
						bool bIsFunction( false );
						error = GetSPDescription( command, bIsFunction, desc, in, out, pConnection, ctx );

						String strSP( ConstructSPStr( command, bIsFunction, desc ) );

						Trace(String("prepare stored procedure: ") << strSP);
						OracleStatement aStmt( pConnection, strSP );
						if ( !aStmt.Prepare() ) {
							Error( ctx, out, aStmt.GetLastErrorMessage() );
						} else {
							out->Put( "Query", strSP, ctx );
							if ( BindSPVariables( desc, in, out, aStmt, ctx ) ) {
								if ( pConnection->checkError( aStmt.Execute( OCI_DEFAULT ), strErr ) ) {
									Error( ctx, out, strErr );
								} else {
									Anything record;
									GetRecordData( desc, record, false ); //TitlesOnce
									TraceAny(record, "fetched result");
									AnyExtensions::Iterator<Anything> recordIter( record );
									Anything anyEntry;
									String slotname;
									while ( recordIter.Next( anyEntry ) ) {
										recordIter.SlotName( slotname );
										out->Put( slotname, anyEntry, ctx );
									}
									//									bool bShowRowCount(true);
									//									in->Get("ShowQueryCount", bShowRowCount, ctx);
									//									if (bShowRowCount) {
									//										// append summary if extract of available data is returned
									//										out->Put("QueryCount", 1L, ctx);
									//									}
									bRet = true;
									bDoRetry = false;
								}
							}
						}
					} else {
						out->Put( "Messages", "Rendered slot SQL resulted in an empty string", ctx );
						bDoRetry = false;
					}
				}
			}
			if ( pConnectionPool && pPooledConnection ) {
				pConnectionPool->ReleaseConnection( pPooledConnection, bIsOpen, server, user );
			}
			if ( bDoRetry && lTryCount > 0 ) {
				SYSWARNING("Internally retrying to execute command [" << command << "]");
			}
		}
	} else {
		if ( !pModule ) {
			SYSERROR("unable to get OracleModule, aborting!");
		} else {
			SYSERROR("unable to get ConnectionPool, aborting!");
		}
	}
	return bRet;
}

bool OracleDAImpl::DoPrepareSQL( String &command, Context &ctx, ParameterMapper *in )
{
	StartTrace(OracleDAImpl.DoPrepareSQL);
	DAAccessTimer(OracleDAImpl.DoPrepareSQL, fName, ctx);
	OStringStream os( command );
	in->Get( "SQL", os, ctx );
	os.flush();
	SubTrace(Query, "QUERY IS [" << command << "]");
	return ( command.Length() > 0L );
}

bool OracleDAImpl::DoPrepareSP( String &command, Context &ctx, ParameterMapper *in )
{
	StartTrace(OracleDAImpl.DoPrepareSP);
	DAAccessTimer(OracleDAImpl.DoPrepareSP, fName, ctx);
	return in->Get( "SP", command, ctx );
}

void OracleDAImpl::Error( Context &ctx, ResultMapper *pResultMapper, String str )
{
	StartTrace(OracleDAImpl.Error);
	String strErr( "OracleDAImpl::" );
	strErr.Append( str );
	SysLog::Error( TimeStamp::Now().AsStringWithZ().Append( ' ' ).Append( strErr ) );
	if ( pResultMapper ) {
		pResultMapper->Put( "Messages", strErr, ctx );
	}
}

void OracleDAImpl::GetRecordData( Anything &descs, Anything &record, bool bTitlesOnce )
{
	StartTrace(OracleDAImpl.GetRecordData);
	// extract the values from a fetched row of data

	for ( sword col = 0; col < descs.GetSize(); col++ ) {
		Anything &desc = descs[col], value;
		SubTraceAny(TraceDesc, desc, "desc at col:" << (long) col);
		SubTrace(TraceColType, "column type is: " << desc[OracleColumn::eColumnType].AsLong() << " indicator: " << desc[OracleColumn::eIndicator].AsLong());
		SubTrace(TraceBuf, "buf ptr " << (long) (desc[OracleColumn::eRawBuf].AsCharPtr()) << " length: " << (long) * ((ub2 *) desc[OracleColumn::eEffectiveLength].AsCharPtr()));
		switch ( desc[OracleColumn::eColumnType].AsLong() ) {
			case SQLT_INT:
				Trace("SQLT_INT");
				if ( desc[OracleColumn::eIndicator].AsLong() == OCI_IND_NULL ) {
					value = 0L;
				} else {
					value = ( * ( (sword *) desc[OracleColumn::eRawBuf].AsCharPtr() ) );
				}
				break;
			case SQLT_FLT:
				Trace("SQLT_FLT");
				if ( desc[OracleColumn::eIndicator].AsLong() == OCI_IND_NULL ) {
					value = 0.0f;
				} else {
					value = ( * ( (float *) desc[OracleColumn::eRawBuf].AsCharPtr() ) );
				}
				break;
			case SQLT_STR:
				Trace("SQLT_STR");
				if ( desc[OracleColumn::eIndicator].AsLong() == OCI_IND_NULL ) {
					value = "";
				} else {
					value = String( desc[OracleColumn::eRawBuf].AsCharPtr() );
				}
				break;
			default:
				SubTraceBuf(TraceBuf, desc[OracleColumn::eRawBuf].AsCharPtr(), *((ub2 *) desc[OracleColumn::eEffectiveLength].AsCharPtr()));
				if ( desc[OracleColumn::eIndicator].AsLong() == OCI_IND_NULL ) {
					value = "";
				} else {
					value
					= String(
						  static_cast<void *> ( const_cast<char *> ( desc[OracleColumn::eRawBuf].AsCharPtr() ) ),
						  (long) * ( reinterpret_cast<ub2 *> ( const_cast<char *> ( desc[OracleColumn::eEffectiveLength].AsCharPtr() ) ) ) );
				}
				break;
		}
		if ( bTitlesOnce ) {
			record.Append( value );
		} else {
			record[desc[OracleColumn::eColumnName].AsCharPtr()] = value;
		}
	}
	TraceAny(record, "row record");
}

void OracleDAImpl::FetchRowData( Anything &descs, ParameterMapper *pmapIn, ResultMapper *pmapOut,
								 OracleStatement &aStmt, Context &ctx )
{
	StartTrace(OracleDAImpl.FetchRowData);

	// return subset of the fetched data rows (if 'start' or 'count'
	// is <0, then all the data rows are returned)

	TraceAny(descs, "descriptions");

	bool bRet( true );
	OCIError *eh = aStmt.getConnection()->ErrorHandle();

	// --- successful execute returns the 1st row
	sword rc = aStmt.Execute( OCI_COMMIT_ON_SUCCESS );
	OCIStmt *pStmthp( aStmt.getHandle() ); // OCI statement handle

	String strMaxRows;
	pmapIn->Get( "DBMaxRows", strMaxRows, ctx );

	String resultformat;
	pmapIn->Get( "DBResultFormat", resultformat, ctx );

	bool bTitlesOnce( resultformat.IsEqual( "TitlesOnce" ) );

	long rowCount( 0L ), lMaxRows( strMaxRows.AsLong( std::numeric_limits<long>::max() ) );

	while ( bRet && ( rc == OCI_SUCCESS || rc == OCI_SUCCESS_WITH_INFO ) && rowCount < lMaxRows ) {
		// fetch data into preallocated areas within 'descs'
		// (all the data is fetched - and possibly discarded - to
		// determine the total number of results)

		Anything record;
		GetRecordData( descs, record, bTitlesOnce );
		SubTraceAny(TraceRow, record, "putting into QueryResult for row " << (long) (rowCount));
		bRet = pmapOut->Put( "QueryResult", record, ctx );

		rc = aStmt.Fetch( 1 );
		++rowCount;
	};

	String strErr("FetchRowData: ");
	if ( aStmt.getConnection()->checkError( rc, strErr ) ) {
		Error( ctx, pmapOut, strErr );
	}

	bool bShowRowCount( true );
	pmapIn->Get( "ShowQueryCount", bShowRowCount, ctx );
	if ( bShowRowCount ) {
		// append summary if extract of available data is returned
		pmapOut->Put( "QueryCount", rowCount, ctx );
	}
}

bool OracleDAImpl::GetSPDescription( String const &spname, bool &pIsFunction, Anything &desc, ParameterMapper *pmapIn,
									 ResultMapper *pmapOut, OracleConnection *pConnection, Context &ctx )
{
	StartTrace(OracleDAImpl.GetSPDescription);
	// returns array of element descriptions: each description is
	// an Anything array with 4 entries:
	// name of the column, type of the data, length of the data in bytes, scale

	text const *objptr = reinterpret_cast<const text *> ( (const char *) spname );
	bool error( false );
	String strErr("GetSPDescription: ");

	pIsFunction = false;
	Trace("get the describe handle for the procedure");
	DscHandleType aDscHandle;
	OCIHandleAlloc( pConnection->getEnvironment().EnvHandle(), aDscHandle.getVoidAddr(), (ub4) OCI_HTYPE_DESCRIBE, 0, 0 );
	error = pConnection->checkError( OCIDescribeAny( pConnection->SvcHandle(), pConnection->ErrorHandle(),
									 (dvoid *) objptr, (ub4) strlen( (char *) objptr ), OCI_OTYPE_NAME, 0, OCI_PTYPE_PROC,
									 aDscHandle.getHandle() ), strErr );
	Trace("dscaddr after describe proc: &" << (long)&aDscHandle.fHandle << " content: " << (long)aDscHandle.getHandle());
	if ( error ) {
		Trace("try to describe function");
		pIsFunction = true;
		error = pConnection->checkError( OCIDescribeAny( pConnection->SvcHandle(), pConnection->ErrorHandle(),
										 (dvoid *) objptr, (ub4) strlen( (char *) objptr ), OCI_OTYPE_NAME, 0, OCI_PTYPE_FUNC,
										 aDscHandle.getHandle() ), strErr );
		Trace("dscaddr after describe func: &" << (long)&aDscHandle.fHandle << " content: " << (long)aDscHandle.getHandle());
		if ( error ) {
			Error( ctx, pmapOut, String( "GetSPDescription: DB-Object is neither procedure nor function (" ) << strErr << ")" );
			return error;
		}
	}

	Trace("get the parameter handle");
	OCIParam *parmh( 0 );
	error = pConnection->checkError( OCIAttrGet( aDscHandle.getHandle(), OCI_HTYPE_DESCRIBE, (dvoid *) &parmh,
									 (ub4 *) 0, OCI_ATTR_PARAM, pConnection->ErrorHandle() ), strErr );
	if ( error ) {
		Error( ctx, pmapOut, strErr );
		return error;
	}
	Trace("addr of parmh: &" << (long)parmh);
	Trace("get the number of arguments and the arg list");
	OCIParam *arglst( 0 );
	ub2 numargs = 0;
	error = pConnection->checkError( OCIAttrGet( (dvoid *) parmh, OCI_DTYPE_PARAM, (dvoid *) &arglst, (ub4 *) 0,
									 OCI_ATTR_LIST_ARGUMENTS, pConnection->ErrorHandle() ), strErr );
	if ( error ) {
		Error( ctx, pmapOut, strErr );
		return error;
	}

	error = pConnection->checkError( OCIAttrGet( (dvoid *) arglst, OCI_DTYPE_PARAM, (dvoid *) &numargs, (ub4 *) 0,
									 OCI_ATTR_NUM_PARAMS, pConnection->ErrorHandle() ), strErr );
	if ( error ) {
		Error( ctx, pmapOut, strErr );
		return error;
	}

	Trace(String("number of arguments: ") << numargs);

	OCIParam *arg( 0 );
	text *name;
	ub4 namelen;
	ub2 dtype;
	OCITypeParamMode iomode;
	ub4 data_len;

	// For a procedure, we begin with i = 1; for a function, we begin with i = 0.
	int start = 0;
	int end = numargs;
	if ( !pIsFunction ) {
		++start;
		++end;
	}

	for ( int i = start; i < end; ++i ) {
		error = pConnection->checkError( OCIParamGet( (dvoid *) arglst, OCI_DTYPE_PARAM, pConnection->ErrorHandle(),
										 (dvoid **) &arg, (ub4) i ), strErr );
		if ( error ) {
			Error( ctx, pmapOut, strErr );
			return error;
		}
		namelen = 0;
		name = 0;
		data_len = 0;

		error = pConnection->checkError( OCIAttrGet( (dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &dtype, (ub4 *) 0,
										 OCI_ATTR_DATA_TYPE, pConnection->ErrorHandle() ), strErr );
		if ( error ) {
			Error( ctx, pmapOut, strErr );
			return error;
		}
		Trace("Data type: " << dtype);

		error = pConnection->checkError( OCIAttrGet( (dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &name, (ub4 *) &namelen,
										 OCI_ATTR_NAME, pConnection->ErrorHandle() ), strErr );
		if ( error ) {
			Error( ctx, pmapOut, strErr );
			return error;
		}
		String strName( (char *) name, namelen );
		// the first param of a function is the return param
		if ( pIsFunction && i == start && strName.Length() == 0 ) {
			Trace("Overriding return param name");
			strName = spname;
			pmapIn->Get( "Return", strName, ctx );
		}
		Trace("Name: " << strName);

		// 0 = IN (OCI_TYPEPARAM_IN), 1 = OUT (OCI_TYPEPARAM_OUT), 2 = IN/OUT (OCI_TYPEPARAM_INOUT)
		error = pConnection->checkError( OCIAttrGet( (dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &iomode, (ub4 *) 0,
										 OCI_ATTR_IOMODE, pConnection->ErrorHandle() ), strErr );
		if ( error ) {
			Error( ctx, pmapOut, strErr );
			return error;
		}
		Trace("IO type: " << iomode);

		error = pConnection->checkError( OCIAttrGet( (dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &data_len, (ub4 *) 0,
										 OCI_ATTR_DATA_SIZE, pConnection->ErrorHandle() ), strErr );
		if ( error ) {
			Error( ctx, pmapOut, strErr );
			return error;
		}
		Trace("Size: " << (int)data_len);

		Anything param;
		param[0L] = Anything(); // dummy
		param[OracleColumn::eColumnName] = strName;
		param[OracleColumn::eColumnType] = dtype;
		param[OracleColumn::eDataLength] = (int) data_len;
		param[OracleColumn::eInOutType] = iomode;
		desc.Append( param );
	}
	TraceAny(desc, "stored procedure description");
	return error;
}

struct ConstructPlSql {
	String &fStr;
	ConstructPlSql( String &str ) :
		fStr( str ) {
	}
	void operator()( const Anything &anyParam ) {
		if ( fStr.Length() ) {
			fStr.Append( ',' );
		}
		fStr.Append( ':' ).Append( anyParam[OracleColumn::eColumnName].AsString() );
	}
};

String OracleDAImpl::ConstructSPStr( String &command, bool pIsFunction, Anything const &desc )
{
	String plsql, strParams;
	plsql << "BEGIN ";
	Anything_const_iterator begin( desc.begin() );
	if ( pIsFunction ) {
		plsql << ":" << ( *begin ).At( OracleColumn::eColumnName ).AsString() << " := ";
		++begin;
	}
	std::for_each( begin, desc.end(), ConstructPlSql( strParams ) );
	plsql << command << "(" << strParams << "); END;";
	StatTrace(OracleDAImpl.ConstructSPStr, "SP string [" << plsql << "]", Storage::Current());
	return plsql;
}

bool OracleDAImpl::BindSPVariables( Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut,
									OracleStatement &aStmt, Context &ctx )
{
	StartTrace(OracleDAImpl.BindSPVariables);
	// use 'desc' to allocate output area used by oracle library
	// to bind variables (binary Anything buffers are allocated and
	// stored within the 'desc' structure... for automatic storage
	// management)

	String strErr("BindSPVariables: ");
	sword status;

	AnyExtensions::Iterator<Anything> descIter( desc );
	Anything col;
	while ( descIter.Next( col ) ) {
		long bindPos( descIter.Index() + 1 ); // first bind variable position is 1

		long len( 0L );
		Anything buf;
		String strValue;
		String strParamname( col[OracleColumn::eColumnName].AsString() );

		if ( col[OracleColumn::eInOutType] == OCI_TYPEPARAM_IN || col[OracleColumn::eInOutType] == OCI_TYPEPARAM_INOUT ) {
			if ( !pmapIn->Get( String( "Params." ).Append( strParamname ), strValue, ctx ) ) {
				Error( ctx, pmapOut, String( "BindSPVariables: In(out) parameter [" ) << strParamname
					   << "] not found in config, is it defined in upper case letters?" );
				return true;
			}
			col[OracleColumn::eDataLength] = strValue.Length();
			col[OracleColumn::eColumnType] = SQLT_STR;
			len = col[OracleColumn::eDataLength].AsLong() + 1;
			buf = Anything( (void *) (const char *) strValue, len );
		} else {
			switch ( col[OracleColumn::eColumnType].AsLong() ) {
				case SQLT_DAT:
					col[OracleColumn::eDataLength] = 9;
					col[OracleColumn::eColumnType] = SQLT_STR;
					len = col[OracleColumn::eDataLength].AsLong() + 1;
					break;
				case SQLT_CHR:
				case SQLT_STR: {
					long lBufSize( glStringBufferSize );
					pmapIn->Get( "StringBufferSize", lBufSize, ctx );
					col[OracleColumn::eDataLength] = lBufSize;
					col[OracleColumn::eColumnType] = SQLT_STR;
					len = col[OracleColumn::eDataLength].AsLong() + 1;
					break;
				}
				case SQLT_NUM:
					col[OracleColumn::eDataLength] = 38;
					col[OracleColumn::eColumnType] = SQLT_STR;
					len = col[OracleColumn::eDataLength].AsLong() + 1;
					break;
				case SQLT_CUR:
				case SQLT_RSET: {
					//TODO
					col[OracleColumn::eColumnType] = SQLT_RSET;

//				result = OCIHandleAlloc (
//					to->conn->environment_handle,
//					reinterpret_cast <void **> (&rs_handle),
//					OCI_HTYPE_STMT,
//					0,		// extra memory to allocate
//					NULL);	// pointer to user-memory
//
//				// bind statement handle as result set
//				if (result == OCI_SUCCESS)
//					result = OCIBindByName (
//						to->stmt_handle,
//						&bind_handle,
//						to->conn->error_handle,
//						(text *) param_name.data (),
//						param_name.length (),
//						&rs_handle,
//						size,
//						oci_type,
//						NULL,	// pointer to array of indicator variables
//						NULL,	// pointer to array of actual length of array elements
//						NULL,	// pointer to array of column-level return codes
//						0,		// maximum possible number of elements of type m_nType
//						NULL,	// a pointer to the actual number of elements (PL/SQL binds)
//						OCI_DEFAULT);
					len = sizeof(OCIStmt *);
					//				OCIStmt* pHndl;
					//				OCIHandleAlloc();
					//				buf = Anything();
					break;
				}
				default:
					len = col[OracleColumn::eDataLength].AsLong() + 1;
					break;
			}
			if ( buf.IsNull() ) {
				buf = Anything( (void *) 0, len );
			}
		}

		col[OracleColumn::eRawBuf] = buf;
		Trace("binding variable: " << strParamname << ", length: " << len);

		// allocate space for NULL indicator
		Anything indicator = Anything( (void *) 0, sizeof(OCIInd) );
		col[OracleColumn::eIndicator] = indicator;

		// allocate space to store effective result size
		ub2 ub2Len( (ub2) col[OracleColumn::eDataLength].AsLong( 0L ) );
		Anything effectiveSize = Anything( (void *) (ub2 *) &ub2Len, sizeof(ub2) );
		col[OracleColumn::eEffectiveLength] = effectiveSize;

		OCIBind *bndp = 0;
		status = OCIBindByPos( aStmt.getHandle(), &bndp, aStmt.getConnection()->ErrorHandle(), (ub4) bindPos,
							   (dvoid *) col[OracleColumn::eRawBuf].AsCharPtr(), (sword) len, col[OracleColumn::eColumnType].AsLong(),
							   (dvoid *) indicator.AsCharPtr(), (ub2 *) 0, (ub2) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT );
		if ( aStmt.getConnection()->checkError( status, strErr ) ) {
			Error( ctx, pmapOut, strErr );
			return false;
		}
	}

	TraceAny(desc, "bind description");
	return true;
}
