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
#include "OraclePooledConnection.h"
#include "OracleStatement.h"
#include "OracleException.h"
#include "OracleResultset.h"
#include "OracleModule.h"
#include "SysLog.h"
#include "Timers.h"
#include "StringStream.h"
#include "TimeStamp.h"
#include "Dbg.h"
#include "AnyIterators.h"

#include <limits>

//--- c-library modules used ---------------------------------------------------
#include <cstring>

static const long glStringBufferSize( 4096L );
//---- OracleDAImpl ---------------------------------------------------------
RegisterDataAccessImpl( OracleDAImpl);

OracleDAImpl::OracleDAImpl( const char *name ) :
	DataAccessImpl( name )
{
	StatTrace(OracleDAImpl.OracleDAImpl, name, Storage::Current());
}

OracleDAImpl::~OracleDAImpl()
{
	StatTrace(OracleDAImpl.~OracleDAImpl, "nothing to do", Storage::Current());
}

IFAObject *OracleDAImpl::Clone() const
{
	StatTrace(OracleDAImpl.Clone, fName, Storage::Current());
	return new OracleDAImpl( fName );
}

String prefixResultSlot( String const &strPrefix, String const &strSlotName )
{
	String strReturn;
	if ( strPrefix.Length() ) {
		strReturn.Append( strPrefix ).Append( '.' );
	}
	return strReturn.Append( strSlotName );
}

void OracleDAImpl::ProcessResultSet( OracleResultset &aRSet, ParameterMapper *& in, Context &ctx,
									 ResultMapper *& out, String strResultPrefix )
{
	StartTrace(OracleDAImpl.ProcessResultSet);

	ROAnything desc( aRSet.GetOutputDescription() );
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
		out->Put( prefixResultSlot( strResultPrefix, "QueryTitles" ), temp, ctx );
	}
	OracleResultset::Status rsetStatus( aRSet.next() );
	Trace("ResultSet->next() status: " << (long)rsetStatus );
	long lRowCount( 0L );
	while ( rsetStatus == OracleResultset::DATA_AVAILABLE || rsetStatus == OracleResultset::STREAM_DATA_AVAILABLE ) {
		// do something
		AnyExtensions::Iterator<ROAnything> aAnyIter( desc );
		ROAnything roaCol;
		Anything anyResult;
		while ( aAnyIter.Next( roaCol ) ) {
			long lColType( roaCol["Type"].AsLong() );
			Trace("column data type: " << lColType);
			if ( lColType == SQLT_CUR || lColType == SQLT_RSET ) {
			} else {
				String strValueCol( aRSet.getString( roaCol["Idx"].AsLong() ) );
				Trace("value of column [" << roaCol["Name"].AsString() << "] has value [" << strValueCol << "]")
				if ( bTitlesOnce ) {
					anyResult[aAnyIter.Index()] = strValueCol;
				} else {
					anyResult[roaCol["Name"].AsString()] = strValueCol;
				}
			}
		}
		out->Put( prefixResultSlot( strResultPrefix, "QueryResult" ), anyResult, ctx );
		++lRowCount;
		rsetStatus = aRSet.next();
		Trace("ResultSet.next() status: " << (long)rsetStatus );
	}
	bool bShowRowCount( true );
	in->Get( "ShowQueryCount", bShowRowCount, ctx );
	if ( bShowRowCount ) {
		Trace("count according to statement " << (long)aRSet.getStatement()->getUpdateCount())
		out->Put( prefixResultSlot( strResultPrefix, "QueryCount" ), lRowCount, ctx );
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
		//!@FIXME a nicer solution would be appreciated where the following line could be omitted
		Coast::Oracle::ConnectionPool::ConnectionLock aConnLock( *pConnectionPool );
		// we need the server and user to see if there is an existing and Open OraclePooledConnection
		String user, server, passwd;
		in->Get( "DBUser", user, ctx );
		in->Get( "DBPW", passwd, ctx );
		in->Get( "DBConnectString", server, ctx );
		Trace("USER IS:" << user);
		Trace("Connect string is [" << server << "]");
		out->Put( "QuerySource", server, ctx );

		bool bDoRetry = true;
		long lTryCount( 1L );
		in->Get( "DBTries", lTryCount, ctx );
		if ( lTryCount < 1 ) {
			lTryCount = 1L;
		}
		long lPrefetchRows( 10L );
		in->Get( "PrefetchRows", lPrefetchRows, ctx );
		// we slipped through and are ready to get a connection and execute a query
		OraclePooledConnection *pPooledConnection = NULL;

		// find a free OraclePooledConnection, we should always get a valid OraclePooledConnection here!
		if ( !pConnectionPool->BorrowConnection( pPooledConnection, server, user ) ) {
			Error( ctx, out, "Exec: unable to get OracleConnection" );
		} else {
			while ( bDoRetry && --lTryCount >= 0 ) {
				// do not move away command, it is used to log when retrying
				String command;
				if ( pPooledConnection->isOpen() || pPooledConnection->Open( server, user, passwd ) ) {
					OracleConnection *pConnection( pPooledConnection->getConnection() );
					if ( DoPrepareSQL( command, ctx, in ) ) {
						OracleStatementPtr aStmt( pConnection->createStatement( command, lPrefetchRows ) );
						Trace("statement status:" << (long)aStmt->status());
						if ( aStmt.get() && aStmt->status() == OracleStatement::PREPARED ) {
							Trace("statement is prepared");
							out->Put( "Query", command, ctx );
							try {
								Trace("executing statement");
								OracleStatement::Status status = aStmt->execute( OracleStatement::EXEC_COMMIT );
								switch ( status ) {
									case OracleStatement::RESULT_SET_AVAILABLE: {
										Trace("RESULT_SET_AVAILABLE");
										OracleResultset::OracleResultsetPtr aRSet( aStmt->getResultset() );
										ProcessResultSet( *aRSet.get(), in, ctx, out, "" );
										break;
									}
									case OracleStatement::UPDATE_COUNT_AVAILABLE: {
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
							} catch ( OracleException &ex ) {
								String strError( "Exec: " );
								strError << ex.getMessage();
								Error( ctx, out, strError );
							}
						}
					} else if ( DoPrepareSP( command, ctx, in ) ) {
						Trace("STORED PROCEDURE IS [" << command << "]");
						try {
							String strReturnName;
							in->Get( "Return", strReturnName, ctx );
							OracleStatementPtr aStmt( pConnection->createStatement( command, lPrefetchRows, OracleConnection::TYPE_UNK, strReturnName ) );

							Trace("statement status:" << (long)aStmt->status());
							if ( aStmt.get() && aStmt->status() == OracleStatement::PREPARED ) {
								out->Put( "Query", aStmt->getStatement(), ctx );
								if ( BindSPVariables( aStmt->GetOutputDescription(), in, out, *aStmt.get(), ctx ) ) {
									Trace("executing statement");
									OracleStatement::Status status = aStmt->execute( OracleStatement::EXEC_DEFAULT );
									switch ( status ) {
										case OracleStatement::RESULT_SET_AVAILABLE:
											Trace("RESULT_SET_AVAILABLE")
											break;
										case OracleStatement::UPDATE_COUNT_AVAILABLE: {
											Trace("UPDATE_COUNT_AVAILABLE")
											AnyExtensions::Iterator<ROAnything> aAnyIter( aStmt->GetOutputDescription() );
											ROAnything roaCol;
											while ( aAnyIter.Next( roaCol ) ) {
												long lOraColIdx( roaCol["Idx"].AsLong() );
												long lColType( roaCol["Type"].AsLong() );
												Trace("got column of type " << lColType)
												if ( lColType == SQLT_CUR || lColType == SQLT_RSET ) {
													OracleResultset::OracleResultsetPtr aRSet(
														aStmt->getCursor( lOraColIdx ) );
													ProcessResultSet( *aRSet.get(), in, ctx, out, roaCol["Name"].AsString() );
												} else {
													String strValueCol( aStmt->getString( lOraColIdx ) );
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
								}
							}
						} catch ( OracleException &ex ) {
							String strError( "Exec: " );
							strError << ex.getMessage();
							Error( ctx, out, strError );
						}
					} else {
						out->Put( "Messages", "Rendered slot SQL resulted in an empty string", ctx );
						bDoRetry = false;
					}
				}
				if ( bDoRetry && lTryCount > 0 ) {
					pPooledConnection->Close();
					SYSWARNING("Internally retrying to execute command [" << command << "]");
				}
			}
			if ( pConnectionPool && pPooledConnection ) {
				pConnectionPool->ReleaseConnection( pPooledConnection, server, user );
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
	return in->Get( "Name", command, ctx );
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

bool OracleDAImpl::BindSPVariables( ROAnything desc, ParameterMapper *pmapIn, ResultMapper *pmapOut,
									OracleStatement &aStmt, Context &ctx )
{
	StartTrace(OracleDAImpl.BindSPVariables);
	// use 'desc' to allocate output area used by oracle library
	// to bind variables (binary Anything buffers are allocated and
	// stored within the 'desc' structure... for automatic storage
	// management)

	String strErr( "BindSPVariables: " );
	sword status;

	AnyExtensions::Iterator<ROAnything> descIter( desc );
	ROAnything col;
	while ( descIter.Next( col ) ) {
		// first bind variable position is 1
		long bindPos( descIter.Index() + 1 );
		String strParamname( col["Name"].AsString() );

		switch ( col["Type"].AsLong() ) {
			case SQLT_CUR:
			case SQLT_RSET:
				Trace("cursor or resultset binding")
				if ( col["IoMode"].AsLong() == (long) OCI_TYPEPARAM_OUT || col["IoMode"].AsLong()
					 == (long) OCI_TYPEPARAM_INOUT ) {
					Trace("binding value of type:" << col["Type"].AsLong())
					aStmt.registerOutParam( bindPos );
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
					aStmt.setString( bindPos, strValue );
				}
				if ( col["IoMode"].AsLong() == (long) OCI_TYPEPARAM_OUT || col["IoMode"].AsLong()
					 == (long) OCI_TYPEPARAM_INOUT ) {
					Trace("binding value of type: " << col["Type"].AsLong())
					long lBufferSize( glStringBufferSize );
					pmapIn->Get( "StringBufferSize", lBufferSize, ctx );
					aStmt.registerOutParam( bindPos, OracleStatement::INTERNAL, lBufferSize );
				}
		}
	}
	return true;
}
