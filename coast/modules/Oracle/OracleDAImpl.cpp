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
			//			if ( lColType == OCCI_SQLT_CUR || lColType == OCCI_SQLT_RSET ) {
			if ( lColType == OCI_DTYPE_RSET ) {
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
		out->Put( prefixResultSlot( strResultPrefix, "QueryCount" ), lRowCount, ctx );
	}
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
					if ( DoPrepareSQL( command, ctx, in ) ) {
						OracleStatementPtr aStmt( pConnection->createStatement( command ) );
						Trace("statement status:" << (long)aStmt->status());
						if ( aStmt->status() == OracleStatement::PREPARED ) {
							Trace("statement is prepared");
							out->Put( "Query", command, ctx );
							try {
								Trace("executing statement");
								OracleStatement::Status status = aStmt->execute( OCI_COMMIT_ON_SUCCESS );
								switch ( status ) {
									case OracleStatement::RESULT_SET_AVAILABLE: {
										Trace("RESULT_SET_AVAILABLE");
										OracleResultsetPtr aRSet( aStmt->getResultset() );
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
						MetaThing desc;
						if ( GetSPDescription( command, desc, in, out, pConnection, ctx ) ) {
							Trace(String("prepare stored procedure/function: ") << command);
							OracleStatementPtr aStmt( pConnection->createStatement( command ) );
							Trace("statement status:" << (long)aStmt->status());
							if ( aStmt->status() == OracleStatement::PREPARED ) {
								out->Put( "Query", command, ctx );
								if ( BindSPVariables( desc, in, out, *aStmt.get(), ctx ) ) {
									try {
										Trace("executing statement");
										OracleStatement::Status status = aStmt->execute(OCI_DEFAULT);
										switch ( status ) {
											case OracleStatement::RESULT_SET_AVAILABLE:
												Trace("RESULT_SET_AVAILABLE")
												break;
											case OracleStatement::UPDATE_COUNT_AVAILABLE: {
												Trace("UPDATE_COUNT_AVAILABLE")
												AnyExtensions::Iterator<ROAnything> aAnyIter( desc );
												ROAnything roaCol;
												while ( aAnyIter.Next( roaCol ) ) {
													long lOraColIdx( roaCol["Idx"].AsLong() );
													long lColType( roaCol["Type"].AsLong() );
													Trace("got column of type " << lColType)
//												if ( lColType == OCCI_SQLT_CUR || lColType == OCCI_SQLT_RSET ) {
													if ( lColType == OCI_DTYPE_RSET ) {
//													ResultSetPtrType aRSet( aStmt->getCursor( lOraColIdx ),
//															aStmt.get(), &Statement::closeResultSet );
//													ProcessResultSet( *aRSet.get(), in, ctx, out,
//															roaCol["Name"].AsString() );
													} else {
//													String strValueCol( aStmt->getString( lOraColIdx ) );
//													Trace("value of column [" << roaCol["Name"].AsString() << "] has value [" << strValueCol << "]")
//													out->Put( roaCol["Name"].AsString(), strValueCol, ctx );
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
									} catch ( OracleException &ex ) {
										String strError( "Exec: " );
										strError << ex.getMessage();
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

bool OracleDAImpl::GetSPDescription( String &command, Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut,
									 OracleConnection *pConnection, Context &ctx )
{
	StartTrace(OracleDAImpl.GetSPDescription);
	// returns array of element descriptions: each description is
	// an Anything array with 4 entries:
	// name of the column, type of the data, length of the data in bytes, scale

	text const *objptr = reinterpret_cast<const text *> ( (const char *) command );
	bool error( false );
	String strErr( "GetSPDescription: " );

	Trace("get the describe handle for the procedure");
	DscHandleType aDscHandle;
	sword attrStat;
	OCIHandleAlloc( pConnection->getEnvironment().EnvHandle(), aDscHandle.getVoidAddr(), (ub4) OCI_HTYPE_DESCRIBE, 0, 0 );
	attrStat = OCIDescribeAny( pConnection->SvcHandle(), pConnection->ErrorHandle(), (dvoid *) objptr, (ub4) strlen(
								   (char *) objptr ), OCI_OTYPE_NAME, 0, OCI_PTYPE_UNK, aDscHandle.getHandle() );
	Trace("status after DESCRIBEANY::OCI_PTYPE_UNK: " << (long)attrStat)
	if ( attrStat != OCI_SUCCESS ) {
		strErr << command << " is neither a stored procedure nor a function";
		Error( ctx, pmapOut, strErr );
		return false;
	}
	Trace("dscaddr after describe proc: &" << (long)&aDscHandle.fHandle << " content: " << (long)aDscHandle.getHandle())
	OCIParam *parmh( 0 );
	ub1 ubFuncType( 0 );
	Trace("get the parameter handle")
	attrStat = OCIAttrGet( aDscHandle.getHandle(), OCI_HTYPE_DESCRIBE, (dvoid *) &parmh, 0, OCI_ATTR_PARAM,
						   pConnection->ErrorHandle() );
	Trace("status after OCI_HTYPE_DESCRIBE::OCI_ATTR_PARAM: " << (long)attrStat);
	attrStat
	= OCIAttrGet( parmh, OCI_DTYPE_PARAM, (dvoid *) &ubFuncType, 0, OCI_ATTR_PTYPE, pConnection->ErrorHandle() );
	Trace("status after OCI_DTYPE_PARAM::OCI_ATTR_PTYPE: " << (long)attrStat << " funcType:" << (long)ubFuncType);
	bool bIsFunction = ( ubFuncType == OCI_PTYPE_FUNC );

	Trace("get the number of arguments and the arg list")
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
	if ( !bIsFunction ) {
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
		if ( bIsFunction && i == start && strName.Length() == 0 ) {
			Trace("Overriding return param name");
			strName = command;
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
		param["Name"] = strName;
		param["Type"] = dtype;
		param["Length"] = (int) data_len;
		param["IoMode"] = iomode;
		desc.Append( param );
	}
	TraceAny(desc, "stored procedure description");

	//	int iType( ubFuncType );
	//	Trace(command << " is of type " << iType)
	//	// oracle starts numbering at 1 ...
	//	int iColIdx( 1 );
	//	switch ( iType ) {
	//	case OCI_PTYPE_PROC:
	//		bIsFunction = false;
	//	case OCI_PTYPE_FUNC: {
	////		typedef std::vector<MetaData> MetaDataVector;
	////		MetaDataVector argMetaData( aMetaData.getVector( MetaData::ATTR_LIST_ARGUMENTS ) );
	////		MetaDataVector::iterator aIter( argMetaData.begin() );
	////		while ( aIter != argMetaData.end() ) {
	////			String strName( aIter->getString( MetaData::ATTR_NAME ).c_str() );
	////			Anything anyColDesc;
	////			// column index 0 is for functions only
	////			//  it is used to define the return value 'pseudo' column
	////			if ( bIsFunction && iColIdx == 1 ) {
	////				// we always use the function name as name to put the value as except...
	////				strName = command;
	////				// ... someone overrides it using the special slotname Return
	////				pmapIn->Get( "Return", strName, ctx );
	////			}
	////			anyColDesc["Name"] = strName;
	////			anyColDesc["Idx"] = iColIdx;
	////			anyColDesc["Type"] = (long) aIter->getInt( MetaData::ATTR_DATA_TYPE );
	////			// 0 = IN (OCI_TYPEPARAM_IN), 1 = OUT (OCI_TYPEPARAM_OUT), 2 = IN/OUT (OCI_TYPEPARAM_INOUT)
	////			anyColDesc["IoMode"] = (long) aIter->getInt( MetaData::ATTR_IOMODE );
	////			desc.Append( anyColDesc );
	////			++aIter;
	////			++iColIdx;
	////		}
	//		TraceAny(desc, "column descriptions (" << iColIdx << ")")
	//		break;
	//	}
	//	default:
	//		strError << command << " is neither a stored procedure nor a function";
	//		Error( ctx, pmapOut, strError );
	//		return false;
	//	}
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

bool OracleDAImpl::BindSPVariables( Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut,
									OracleStatement &aStmt, Context &ctx )
{
	StartTrace(OracleDAImpl.BindSPVariables);
	// use 'desc' to allocate output area used by oracle library
	// to bind variables (binary Anything buffers are allocated and
	// stored within the 'desc' structure... for automatic storage
	// management)

	String strErr( "BindSPVariables: " );
	sword status;

	AnyExtensions::Iterator<Anything> descIter( desc );
	Anything col;
	while ( descIter.Next( col ) ) {
		long bindPos( descIter.Index() + 1 ); // first bind variable position is 1

		long len( 0L );
		Anything buf;
		String strValue;
		String strParamname( col["Name"].AsString() );

		if ( col["IoMode"] == OCI_TYPEPARAM_IN || col["IoMode"] == OCI_TYPEPARAM_INOUT ) {
			if ( !pmapIn->Get( String( "Params." ).Append( strParamname ), strValue, ctx ) ) {
				Error( ctx, pmapOut, String( "BindSPVariables: In(out) parameter [" ) << strParamname
					   << "] not found in config, is it defined in upper case letters?" );
				return true;
			}
			col["Length"] = strValue.Length();
			col["Type"] = SQLT_STR;
			len = col["Length"].AsLong() + 1;
			buf = Anything( (void *) (const char *) strValue, len );
		} else {
			switch ( col["Type"].AsLong() ) {
				case SQLT_DAT:
					col["Length"] = 9;
					col["Type"] = SQLT_STR;
					len = col["Length"].AsLong() + 1;
					break;
				case SQLT_CHR:
				case SQLT_STR: {
					long lBufSize( glStringBufferSize );
					pmapIn->Get( "StringBufferSize", lBufSize, ctx );
					col["Length"] = lBufSize;
					col["Type"] = SQLT_STR;
					len = col["Length"].AsLong() + 1;
					break;
				}
				case SQLT_NUM:
					col["Length"] = 38;
					col["Type"] = SQLT_STR;
					len = col["Length"].AsLong() + 1;
					break;
				case SQLT_CUR:
				case SQLT_RSET: {
					//TODO
					col["Type"] = SQLT_RSET;

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
					len = col["Length"].AsLong() + 1;
					break;
			}
			if ( buf.IsNull() ) {
				buf = Anything( (void *) 0, len );
			}
		}

		col["RawBuf"] = buf;
		Trace("binding variable: " << strParamname << ", length: " << len);

		// allocate space for NULL indicator
		Anything indicator = Anything( (void *) 0, sizeof(OCIInd) );
		col["Indicator"] = indicator;

		// allocate space to store effective result size
		ub2 ub2Len( (ub2) col["Length"].AsLong( 0L ) );
		Anything effectiveSize = Anything( (void *) (ub2 *) &ub2Len, sizeof(ub2) );
		col["EffectiveLength"] = effectiveSize;

		OCIBind *bndp = 0;
		status = OCIBindByPos( aStmt.getHandle(), &bndp, aStmt.getConnection()->ErrorHandle(), (ub4) bindPos,
							   (dvoid *) col["RawBuf"].AsCharPtr(), (sword) len, col["Type"].AsLong(),
							   (dvoid *) indicator.AsCharPtr(), (ub2 *) 0, (ub2) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT );
		if ( aStmt.getConnection()->checkError( status, strErr ) ) {
			Error( ctx, pmapOut, strErr );
			return false;
		}
	}

	TraceAny(desc, "bind description");
	return true;
}
