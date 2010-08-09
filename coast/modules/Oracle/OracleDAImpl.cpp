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
#include "SystemLog.h"
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

	OracleStatement::Description &desc( aRSet.GetOutputDescription() );
	String resultformat, resultsize;
	in->Get( "DBResultFormat", resultformat, ctx );
	bool bTitlesOnce = resultformat.IsEqual( "TitlesOnce" );
	if ( bTitlesOnce ) {
		// put column name information
		AnyExtensions::Iterator<OracleStatement::Description, OracleStatement::Description::Element> aDescIter( desc );
		OracleStatement::Description::Element aDescEl;
		Anything temp;
		while ( aDescIter.Next( aDescEl ) ) {
			String strColName( aDescEl.AsString( "Name" ) );
			Trace("colname@" << aDescIter.Index() << " [" << strColName << "]");
			temp[strColName] = aDescIter.Index();
		}
		out->Put( prefixResultSlot( strResultPrefix, "QueryTitles" ), temp, ctx );
	}
	OracleResultset::Status rsetStatus( aRSet.next() );
	Trace("ResultSet->next() status: " << (long)rsetStatus );
	long lRowCount( 0L );
	while ( rsetStatus == OracleResultset::DATA_AVAILABLE || rsetStatus == OracleResultset::STREAM_DATA_AVAILABLE ) {
		// do something
		AnyExtensions::Iterator<OracleStatement::Description, OracleStatement::Description::Element> aDescIter( desc );
		OracleStatement::Description::Element aDescEl;
		Anything anyResult;
		while ( aDescIter.Next( aDescEl ) ) {
			long lColType( aDescEl.AsLong( "Type" ) );
			Trace("column data type: " << lColType);
			if ( lColType == SQLT_CUR || lColType == SQLT_RSET ) {
			} else {
				Anything anyValueCol( aRSet.getValue( aDescEl.AsLong( "Idx" ) ) );
				Trace("value of column [" << aDescEl.AsString("Name") << "] has value [" << anyValueCol.AsString("NULL") << "]");
				if ( bTitlesOnce ) {
					anyResult[aDescIter.Index()] = anyValueCol;
				} else {
					anyResult[aDescEl.AsString( "Name" )] = anyValueCol;
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
		Trace("count according to statement " << (long)aRSet.getStatement()->getUpdateCount());
		out->Put( prefixResultSlot( strResultPrefix, "QueryCount" ), lRowCount, ctx );
	}
}

bool OracleDAImpl::TryExecuteQuery( ParameterMapper *in, Context &ctx, OraclePooledConnection *& pPooledConnection,
									String &server, String &user, String &passwd, ResultMapper *out, bool bRet )
{
	StartTrace(OracleDAImpl.TryExecuteQuery);
	bool bDoRetry = true;
	long lTryCount( 1L );
	in->Get( "DBTries", lTryCount, ctx );
	if ( lTryCount < 1 ) {
		lTryCount = 1L;
	}
	out->Put( "QuerySource", server, ctx );
	while ( bDoRetry && --lTryCount >= 0 ) {
		// do not move away command, it is used to log when retrying
		String command;
		if ( pPooledConnection->isOpen() || pPooledConnection->Open( server, user, passwd ) ) {
			OracleConnection *pConnection( pPooledConnection->getConnection() );
			long lPrefetchRows( 10L );
			in->Get( "PrefetchRows", lPrefetchRows, ctx );
			if ( DoPrepareSQL( command, ctx, in ) ) {
				Trace("SIMPLE STATEMENT IS [" << command << "]");
				out->Put( "Query", command, ctx );
				String strReturnName;
				OracleConnection::ObjectType aObjType = OracleConnection::TYPE_SIMPLE;
				try {
					OracleStatementPtr aStmt( pConnection->createStatement( command, lPrefetchRows, aObjType,
											  strReturnName ) );
					Trace("statement status:" << (long)aStmt->status());
					if ( aStmt.get() && aStmt->status() == OracleStatement::PREPARED ) {
						Trace("statement is prepared");
						Trace("executing statement [" << aStmt->getStatement() << "]");
						long lIterations(1L);
						OracleStatement::Status status = aStmt->execute( OracleStatement::EXEC_COMMIT, lIterations );
						switch ( status ) {
							case OracleStatement::RESULT_SET_AVAILABLE: {
								Trace("RESULT_SET_AVAILABLE");
								OracleResultsetPtr aRSet( aStmt->getResultset() );
								ProcessResultSet( *aRSet.get(), in, ctx, out, "" );
								break;
							}
							case OracleStatement::UPDATE_COUNT_AVAILABLE: {
								Trace("UPDATE_COUNT_AVAILABLE");
								bool bShowUpdateCount( false );
								in->Get( "ShowUpdateCount", bShowUpdateCount, ctx );
								if ( bShowUpdateCount ) {
									out->Put( "UpdateCount", (long) aStmt->getUpdateCount(), ctx );
								}
								break;
							}
							default:
								Trace("got status:" << (long)status);
								break;
						}
						bRet = true;
						bDoRetry = false;
					}
				} catch ( OracleException &ex ) {
					String strError( "TryExecuteQuery: " );
					strError << ex.getMessage();
					Error( ctx, out, strError );
				}
			} else if ( DoPrepareSP( command, ctx, in ) ) {
				Trace("STORED PROC/FUNC IS [" << command << "]");
				out->Put( "ProcedureName", command, ctx );
				String strReturnName;
				in->Get( "Return", strReturnName, ctx );
				OracleConnection::ObjectType aObjType = OracleConnection::TYPE_UNK;
				try {
					OracleStatementPtr aStmt( pConnection->createStatement( command, lPrefetchRows, aObjType,
											  strReturnName ) );
					Trace("statement status:" << (long)aStmt->status());
					if ( aStmt.get() && aStmt->status() == OracleStatement::PREPARED ) {
						Trace("executing statement [" << aStmt->getStatement() << "]");
						out->Put( "Query", aStmt->getStatement(), ctx );
						long lIterations(1L);
						bool bIsArrayExecute(false);
						Anything anyRowInputValues = getMappedInputValues( in, *aStmt.get(), ctx, bIsArrayExecute);
						lIterations = anyRowInputValues.GetSize();
						if ( lIterations > 0L ) {
							TraceAny(anyRowInputValues, "collected values");
							aStmt->bindAndFillInputValues(anyRowInputValues);
							OracleStatement::Status status = aStmt->execute( OracleStatement::EXEC_COMMIT, lIterations );
							switch ( status ) {
								case OracleStatement::RESULT_SET_AVAILABLE:
									Trace("RESULT_SET_AVAILABLE");
									break;
								case OracleStatement::UPDATE_COUNT_AVAILABLE: {
									Trace("UPDATE_COUNT_AVAILABLE");
									OracleStatement::Description &aOutDescription(aStmt->GetOutputDescription());
									for ( long lRowIdx = 0; lRowIdx < lIterations; ++lRowIdx) {
										Anything anyRowIndex(lRowIdx);
										Context::PushPopEntry<Anything> aRowIndexEntry(ctx, "ResultRowIndex", anyRowIndex, (bIsArrayExecute ? "_OracleArrayResultIndex_" : "Guguseli"));
										AnyExtensions::Iterator < OracleStatement::Description,
													  OracleStatement::Description::Element >
													  aDescIter( aOutDescription );
										Trace("processing " << (long)aOutDescription.GetSize() << " description elements");
										OracleStatement::Description::Element aDescEl;
										while ( aDescIter.Next( aDescEl ) ) {
											long lOraColIdx( aDescEl.AsLong( "Idx" ) );
											long lColType( aDescEl.AsLong( "Type" ) );
											Trace("got named column [" << aDescEl.AsString("Name") << "] of type " << lColType);
											if ( lColType == SQLT_CUR || lColType == SQLT_RSET ) {
												OracleResultsetPtr aRSet( aStmt->getCursor( lOraColIdx, lRowIdx ) );
												ProcessResultSet( *aRSet.get(), in, ctx, out, aDescEl.AsString("Name") );
											} else {
												Anything anyValueCol( aStmt->getValue( lOraColIdx, lRowIdx ) );
												Trace("value of column [" << aDescEl.AsString("Name") << "] has value [" << anyValueCol.AsString("NULL") << "]");
												out->Put( aDescEl.AsString("Name"), anyValueCol, ctx );
											}
										}
									}
									break;
								}
								default:
									Trace("got status:" << (long)status);
									break;
							}
							bRet = true;
							bDoRetry = false;
						}
					}
				} catch ( OracleException &ex ) {
					String strError( "TryExecuteQuery: " );
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
			if ( lTryCount > 0 ) {
				SYSWARNING("Internally retrying to execute command [" << command << "]");
			}
		}
	}
	return bRet;
}

bool OracleDAImpl::Exec( Context &ctx, ParameterMapper *in, ResultMapper *out )
{
	StartTrace(OracleDAImpl.Exec);
	bool bRet( false );
	DAAccessTimer( OracleDAImpl.Exec, fName, ctx );
	OracleModule *pModule = SafeCast(WDModule::FindWDModule("OracleModule"), OracleModule);
	Coast::Oracle::ConnectionPool *pConnectionPool( 0 );
	if ( pModule && ( pConnectionPool = pModule->GetConnectionPool() ) ) {
		// we need the server and user to see if there is an existing and Open OraclePooledConnection
		String user, server, passwd;
		bool bUseTLS(false);
		in->Get( "DBUser", user, ctx );
		in->Get( "DBPW", passwd, ctx );
		in->Get( "DBConnectString", server, ctx );
		in->Get( "UseTLS", bUseTLS, ctx);
		Trace("USER IS:" << user);
		Trace("Connect string is [" << server << "]");

		// we slipped through and are ready to get a connection and execute a query
		OraclePooledConnection *pPooledConnection = NULL;

		// find a free OraclePooledConnection, we should always get a valid OraclePooledConnection here!
		if ( !pConnectionPool->BorrowConnection( pPooledConnection, server, user, bUseTLS ) ) {
			Error( ctx, out, "Exec: unable to get OracleConnection" );
		} else {
			bRet = TryExecuteQuery( in, ctx, pPooledConnection, server, user, passwd, out, bRet );
			if ( pConnectionPool && pPooledConnection ) {
				pConnectionPool->ReleaseConnection( pPooledConnection, bUseTLS );
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
	DAAccessTimer( OracleDAImpl.DoPrepareSQL, fName, ctx );
	OStringStream os( command );
	in->Get( "SQL", os, ctx );
	os.flush();
	SubTrace(Query, "QUERY IS [" << command << "]");
	return ( command.Length() > 0L );
}

bool OracleDAImpl::DoPrepareSP( String &command, Context &ctx, ParameterMapper *in )
{
	StartTrace(OracleDAImpl.DoPrepareSP);
	DAAccessTimer( OracleDAImpl.DoPrepareSP, fName, ctx );
	return in->Get( "Name", command, ctx );
}

void OracleDAImpl::Error( Context &ctx, ResultMapper *pResultMapper, String str )
{
	String strErr( "OracleDAImpl::" );
	strErr.Append( str );
	StartTrace1(OracleDAImpl.Error, strErr);
	SystemLog::Warning( TimeStamp::Now().AsStringWithZ().Append( ' ' ).Append( strErr ) );
	if ( pResultMapper ) {
		pResultMapper->Put( "Messages", strErr, ctx );
	}
}

Anything OracleDAImpl::getMappedInputValues( ParameterMapper *pmapIn, OracleStatement &aStmt, Context &ctx, bool &bIsArrayExecute )
{
	StartTrace(OracleDAImpl.getMappedInputValues);

	long lIterations = 1L;
	String strArraySlot;
	pmapIn->Get( "ArrayValuesSlotName", strArraySlot, ctx);
	Anything anyArrayValues;
	bIsArrayExecute = false;
	if ( strArraySlot.Length() && pmapIn->Get(strArraySlot, anyArrayValues, ctx) ) {
		lIterations = anyArrayValues.GetSize();
		bIsArrayExecute = true;
	}

	long lBufferSize( glStringBufferSize );
	pmapIn->Get( "StringBufferSize", lBufferSize, ctx );
	MetaThing anyMappedInputValues;
	for (long lIdx = 0; lIdx < lIterations; ++lIdx) {
		Context::PushPopEntry<Anything> aEntry(ctx, "ArrayValues", anyArrayValues[lIdx]);
		Anything anyRow;
		AnyExtensions::Iterator<OracleStatement::Description, OracleStatement::Description::Element> aDescIter( aStmt.GetOutputDescription() );
		OracleStatement::Description::Element aDescEl;
		while ( aDescIter.Next( aDescEl ) ) {
			long lRowLength = aDescEl.AsLong( "MaxStringBufferSize", 0L );
			String strParamname( aDescEl.AsString( "Name" ) );
			Anything anyValue;
			if ( aDescEl.AsLong( "IoMode" ) == (long) OCI_TYPEPARAM_IN || aDescEl.AsLong( "IoMode" )
				 == (long) OCI_TYPEPARAM_INOUT ) {
				bool bIsRSET = ( aDescEl.AsLong( "Type" ) == SQLT_CUR ) || ( aDescEl.AsLong( "Type" ) == SQLT_RSET );
				if ( !pmapIn->Get( String( "Params." ).Append( strParamname ), anyValue, ctx ) && !bIsRSET ) {
					throw OracleException(*(aStmt.getConnection()), String( "getMappedInputValues: In(out) parameter [" ) << strParamname
										  << "] not found in config, is it defined in upper case letters?" );
				}
				anyRow[strParamname] = anyValue;
				lRowLength = std::max(lRowLength, anyValue.AsString().Length());
			} else {
				lRowLength = lBufferSize;
			}
			aDescEl["MaxStringBufferSize"] = lRowLength;
		}
		anyMappedInputValues[lIdx] = anyRow;
	}
	TraceAny(anyMappedInputValues, "collected input values");
	return anyMappedInputValues;
}
