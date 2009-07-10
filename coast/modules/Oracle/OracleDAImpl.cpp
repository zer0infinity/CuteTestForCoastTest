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
#include "OracleConnection.h"
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

using namespace oracle::occi;

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
void OracleDAImpl::ProcessResultSet( oracle::occi::ResultSet &aRSet, ParameterMapper *& in, Context &ctx,
									 ResultMapper *& out, String strResultPrefix )
{
	StartTrace(OracleDAImpl.ProcessResultSet);
	typedef std::vector<MetaData> MetaDataVector;
	MetaDataVector rsetMetaData( aRSet.getColumnListMetaData() );
	MetaThing desc;
	MetaDataVector::iterator aIter( rsetMetaData.begin() );
	int iNumCols( 0 );
	while ( aIter != rsetMetaData.end() ) {
		++iNumCols;
		String strName( aIter->getString( MetaData::ATTR_NAME ).c_str() );
		Anything anyColDesc;
		anyColDesc["Name"] = strName;
		anyColDesc["Idx"] = iNumCols;
		anyColDesc["Type"] = (long) ( aIter->getInt( MetaData::ATTR_DATA_TYPE ) );
		desc.Append( anyColDesc );
		++aIter;
	}
	TraceAny(desc, "column descriptions (" << iNumCols << ")")
	String resultformat, resultsize;
	in->Get( "DBResultFormat", resultformat, ctx );
	bool bTitlesOnce = resultformat.IsEqual( "TitlesOnce" );
	if ( bTitlesOnce ) {
		// put column name information
		AnyExtensions::Iterator<ROAnything> aAnyIter( desc );
		ROAnything roaCol;
		Anything temp;
		while ( aAnyIter.Next( roaCol ) ) {
			String strColName( roaCol["Name"].AsString() );
			Trace("colname@" << aAnyIter.Index() << " [" << strColName << "]");
			temp[strColName] = aAnyIter.Index();
		}
		out->Put( prefixResultSlot(strResultPrefix, "QueryTitles"), temp, ctx );
	}
	ResultSet::Status rsetStatus( aRSet.next() );
	Trace("ResultSet->next() status: " << (long)rsetStatus );
	long lRowCount( 0L );
	while ( rsetStatus == ResultSet::DATA_AVAILABLE || rsetStatus == ResultSet::STREAM_DATA_AVAILABLE ) {
		// do something
		AnyExtensions::Iterator<ROAnything> aAnyIter( desc );
		ROAnything roaCol;
		Anything anyResult;
		while ( aAnyIter.Next( roaCol ) ) {
			long lColType( roaCol["Type"].AsLong() );
			if ( lColType == OCCI_SQLT_CUR || lColType == OCCI_SQLT_RSET ) {
			} else {
				String strValueCol( aRSet.getString( roaCol["Idx"].AsLong() ).c_str() );
				Trace("value of column [" << roaCol["Name"].AsString() << "] has value [" << strValueCol << "]")
				if ( bTitlesOnce ) {
					anyResult[aAnyIter.Index()] = strValueCol;
				} else {
					anyResult[roaCol["Name"].AsString()] = strValueCol;
				}
			}
		}
		out->Put( prefixResultSlot(strResultPrefix, "QueryResult"), anyResult, ctx );
		++lRowCount;
		rsetStatus = aRSet.next();
		Trace("ResultSet.next() status: " << (long)rsetStatus );
	}
	bool bShowRowCount( true );
	in->Get( "ShowQueryCount", bShowRowCount, ctx );
	if ( bShowRowCount ) {
		out->Put( prefixResultSlot(strResultPrefix, "QueryCount"), lRowCount, ctx );
	}
}

bool OracleDAImpl::Exec( Context &ctx, ParameterMapper *in, ResultMapper *out )
{
	StartTrace(OracleDAImpl.Exec);
	bool bRet( false );
	DAAccessTimer(OracleDAImpl.Exec, fName, ctx);
	OracleModule *pModule = SafeCast(WDModule::FindWDModule("OracleModule"), OracleModule);
	Coast::Oracle::ConnectionPool *pConnectionPool( 0 );
	if ( pModule && ( pConnectionPool = pModule->GetConnectionPool() ) ) {
		// we need the server and user to see if there is an existing and Open OracleConnection
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
		// find a free OracleConnection, we should always get a valid OracleConnection here!
		while ( bDoRetry && --lTryCount >= 0 ) {
			OracleConnection *pConnection = NULL;
			String command;

			// --- establish db connection
			if ( !pConnectionPool->BorrowConnection( pConnection, bIsOpen, server, user ) ) {
				Error( ctx, out, "Exec: unable to get OracleConnection" );
				bDoRetry = false;
			} else {
				if ( bIsOpen || pConnection->Open( server, user, passwd ) ) {
					bIsOpen = true;
					if ( DoPrepareSQL( command, ctx, in ) ) {
						std::string ssStmt( (const char *) command );
						StatementPtrType aStmt( pConnection->getConnection()->createStatement( ssStmt ),
												pConnection->getConnection(), &Connection::terminateStatement );

						Trace("statement status:" << (long)aStmt->status());
						if ( aStmt->status() == Statement::PREPARED ) {
							Trace("statement is prepared");
							out->Put( "Query", command, ctx );
							try {
								Trace("executing statement");
								Statement::Status status = aStmt->execute();
								switch ( status ) {
									case Statement::RESULT_SET_AVAILABLE: {
										Trace("RESULT_SET_AVAILABLE");
										ResultSetPtrType aRSet( aStmt->getResultSet(), aStmt.get(),
																&Statement::closeResultSet );
										ProcessResultSet( *aRSet.get(), in, ctx, out, "" );
										break;
									}
									case Statement::UPDATE_COUNT_AVAILABLE: {
										Trace("UPDATE_COUNT_AVAILABLE")
										bool bShowUpdateCount( false );
										in->Get( "ShowUpdateCount", bShowUpdateCount, ctx );
										if ( bShowUpdateCount ) {
											out->Put( "UpdateCount", (long) aStmt->getUpdateCount(), ctx );
										}
										break;
									}
									default:
										Trace("got status:" << (long)status)
										break;
								}
								bRet = true;
								bDoRetry = false;
							} catch ( SQLException &ex ) {
								String strError( "Exec: " );
								strError << ex.getMessage().c_str();
								Error( ctx, out, strError );
							}
						}
					} else if ( DoPrepareSP( command, ctx, in ) ) {
						Trace("STORED PROCEDURE IS [" << command << "]");
						MetaThing desc;
						if ( GetSPDescription( command, desc, in, out, pConnection, ctx ) ) {
							Trace(String("prepare stored procedure/function: ") << command);
							std::string ssStmt( (const char *) command );
							StatementPtrType aStmt( pConnection->getConnection()->createStatement( ssStmt ),
													pConnection->getConnection(), &Connection::terminateStatement );

							Trace("statement status:" << (long)aStmt->status());
							if ( aStmt->status() == Statement::PREPARED ) {
								Trace("statement is prepared");
								out->Put( "Query", command, ctx );
								if ( BindSPVariables( desc, in, out, *aStmt.get(), ctx ) ) {
									try {
										Trace("executing statement");
										Statement::Status status = aStmt->execute();
										switch ( status ) {
											case Statement::RESULT_SET_AVAILABLE:
												Trace("RESULT_SET_AVAILABLE")
												break;
											case Statement::UPDATE_COUNT_AVAILABLE: {
												Trace("UPDATE_COUNT_AVAILABLE")
												AnyExtensions::Iterator<ROAnything> aAnyIter( desc );
												ROAnything roaCol;
												while ( aAnyIter.Next( roaCol ) ) {
													long lOraColIdx( roaCol["Idx"].AsLong() );
													long lColType( roaCol["Type"].AsLong() );
													Trace("got column of type " << lColType)
													if ( lColType == OCCI_SQLT_CUR || lColType == OCCI_SQLT_RSET ) {
														ResultSetPtrType aRSet( aStmt->getCursor(lOraColIdx), aStmt.get(),
																				&Statement::closeResultSet );
														ProcessResultSet( *aRSet.get(), in, ctx, out, roaCol["Name"].AsString() );
													} else {
														String strValueCol(
															aStmt->getString( lOraColIdx ).c_str() );
														Trace("value of column [" << roaCol["Name"].AsString() << "] has value [" << strValueCol << "]")
														out->Put( roaCol["Name"].AsString(), strValueCol, ctx );
													}
												}
												bool bShowUpdateCount( false );
												in->Get( "ShowUpdateCount", bShowUpdateCount, ctx );
												if ( bShowUpdateCount ) {
													out->Put( "UpdateCount", (long) aStmt->getUpdateCount(), ctx );
												}
												break;
											}
											default:
												Trace("got status:" << (long)status)
												break;
										}
										bRet = true;
										bDoRetry = false;
									} catch ( SQLException &ex ) {
										String strError( "Exec: " );
										strError << ex.getMessage().c_str();
										Error( ctx, out, strError );
									}
								}
							}
						}
					} else {
						out->Put( "Messages", "Rendered slot SQL resulted in an empty string", ctx );
						bDoRetry = false;
					}
				}
			}
			if ( pConnectionPool && pConnection ) {
				pConnectionPool->ReleaseConnection( pConnection, bIsOpen, server, user );
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

bool OracleDAImpl::GetSPDescription( String &command, Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut,
									 OracleConnection *pConnection, Context &ctx )
{
	StartTrace1(OracleDAImpl.GetSPDescription, "procedure/function name [" << command << "]");

	std::string ssCmd( (const char *) command );
	String strError( "GetSPDescription: " );
	bool bIsFunction( true );
	try {
		MetaData aMetaData( pConnection->getConnection()->getMetaData( ssCmd ) );
		int iType( aMetaData.getInt( MetaData::ATTR_PTYPE ) );
		Trace(command << " is of type " << iType)
		// oracle starts numbering at 1 ...
		int iColIdx( 1 );
		switch ( iType ) {
			case MetaData::PTYPE_PROC:
				bIsFunction = false;
			case MetaData::PTYPE_FUNC: {
				typedef std::vector<MetaData> MetaDataVector;
				MetaDataVector argMetaData( aMetaData.getVector( MetaData::ATTR_LIST_ARGUMENTS ) );
				MetaDataVector::iterator aIter( argMetaData.begin() );
				while ( aIter != argMetaData.end() ) {
					String strName( aIter->getString( MetaData::ATTR_NAME ).c_str() );
					Anything anyColDesc;
					// column index 0 is for functions only
					//  it is used to define the return value 'pseudo' column
					if ( bIsFunction && iColIdx == 1 ) {
						// we always use the function name as name to put the value as except...
						strName = command;
						// ... someone overrides it using the special slotname Return
						pmapIn->Get( "Return", strName, ctx );
					}
					anyColDesc["Name"] = strName;
					anyColDesc["Idx"] = iColIdx;
					anyColDesc["Type"] = (long) aIter->getInt( MetaData::ATTR_DATA_TYPE );
					// 0 = IN (OCI_TYPEPARAM_IN), 1 = OUT (OCI_TYPEPARAM_OUT), 2 = IN/OUT (OCI_TYPEPARAM_INOUT)
					anyColDesc["IoMode"] = (long) aIter->getInt( MetaData::ATTR_IOMODE );
					desc.Append( anyColDesc );
					++aIter;
					++iColIdx;
				}
				TraceAny(desc, "column descriptions (" << iColIdx << ")")
				break;
			}
			default:
				strError << command << " is neither a stored procedure nor a function";
				Error( ctx, pmapOut, strError );
				return false;
		}
	} catch ( SQLException &ex ) {
		strError << ex.getMessage().c_str();
		Error( ctx, pmapOut, strError );
		return false;
	}
	command = ConstructSPStr( command, bIsFunction, desc );
	return true;
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
		fStr.Append( ':' ).Append( anyParam["Name"].AsString() );
	}
};

String OracleDAImpl::ConstructSPStr( String const &command, bool pIsFunction, Anything const &desc )
{
	String plsql, strParams;
	plsql << "BEGIN ";
	Anything_const_iterator begin( desc.begin() );
	if ( pIsFunction ) {
		plsql << ":" << ( *begin ).At( "Name" ).AsString() << " := ";
		++begin;
	}
	std::for_each( begin, desc.end(), ConstructPlSql( strParams ) );
	plsql << command << "(" << strParams << "); END;";
	StatTrace(OracleDAImpl.ConstructSPStr, "SP string [" << plsql << "]", Storage::Current());
	return plsql;
}

bool OracleDAImpl::BindSPVariables( Anything const &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut,
									Statement &aStmt, Context &ctx )
{
	StartTrace(OracleDAImpl.BindSPVariables);
	// use 'desc' to allocate output area used by oracle library
	// to bind variables (binary Anything buffers are allocated and
	// stored within the 'desc' structure... for automatic storage
	// management)

	String strErr;
	AnyExtensions::Iterator<ROAnything> descIter( desc );
	ROAnything col;
	while ( descIter.Next( col ) ) {
		// first bind variable position is 1
		long bindPos( descIter.Index() + 1 );
		String strParamname( col["Name"].AsString() );

		switch ( col["Type"].AsLong() ) {
			case OCCI_SQLT_CUR:
			case OCCI_SQLT_RSET:
				Trace("cursor or resultset binding")
				if ( col["IoMode"].AsLong() == (long) OCI_TYPEPARAM_OUT || col["IoMode"].AsLong()
					 == (long) OCI_TYPEPARAM_INOUT ) {
					Trace("binding value of type:" << col["Type"].AsLong() << " as OCCICURSOR")
					aStmt.registerOutParam( bindPos, OCCICURSOR );
				}
				break;
			default:
				if ( col["IoMode"].AsLong() == (long) OCI_TYPEPARAM_IN || col["IoMode"].AsLong()
					 == (long) OCI_TYPEPARAM_INOUT ) {
					String strValue;
					if ( !pmapIn->Get( String( "Params." ).Append( strParamname ), strValue, ctx ) ) {
						Error( ctx, pmapOut, String( "BindSPVariables: In(out) parameter [" ) << strParamname
							   << "] not found in config, is it defined in upper case letters?" );
						return false;
					}
					std::string ssValue( (const char *) strValue );
					aStmt.setString( bindPos, ssValue );
				}
				if ( col["IoMode"].AsLong() == (long) OCI_TYPEPARAM_OUT || col["IoMode"].AsLong()
					 == (long) OCI_TYPEPARAM_INOUT ) {
					Trace("binding value of type:" << col["Type"].AsLong() << " as OCCISTRING")
					long lBufferSize( glStringBufferSize );
					pmapIn->Get( "StringBufferSize", lBufferSize, ctx );
					aStmt.registerOutParam( bindPos, OCCISTRING, lBufferSize );
				}
		}
	}
	return true;
}
