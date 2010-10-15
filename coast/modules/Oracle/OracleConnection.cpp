/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleConnection.h"
#include "OracleEnvironment.h"
#include "OracleStatement.h"
#include "OracleException.h"
#include "AnyIterators.h"
#include "SystemLog.h"
#include "Dbg.h"

#include <string.h>	// for strlen
Anything OracleConnection::fgDescriptionCache( Anything::ArrayMarker(), Storage::Global() );
ROAnything OracleConnection::fgDescriptionCacheRO( fgDescriptionCache );
RWLock OracleConnection::fgDescriptionLock( "OracleDescriptorLock", Storage::Global() );

OracleConnection::OracleConnection( OracleEnvironment &rEnv ) :
	fStatus( eUnitialized ), fOracleEnv( rEnv ), fErrhp(), fSrvhp(), fSvchp(), fUsrhp()
{
	StartTrace(OracleConnection.OracleConnection);

	// ---
	if ( AllocateHandle( fErrhp ) /* alloc error handle */
		 && AllocateHandle( fSvchp ) /* alloc service context handle */
		 && AllocateHandle( fSrvhp ) /* alloc server connection handle */
		 && AllocateHandle( fUsrhp ) /* alloc user session handle */
	   ) {
		fStatus = eHandlesAllocated;
	}
}

template< class handlePtrType >
bool OracleConnection::AllocateHandle( handlePtrType &aHandlePtr )
{
	StartTrace(OracleConnection.AllocateHandle);
	if ( OCIHandleAlloc( fOracleEnv.EnvHandle(), aHandlePtr.getVoidAddr(), aHandlePtr.getHandleType(), (size_t) 0,
						 (dvoid **) 0 ) != OCI_SUCCESS ) {
		SystemLog::Error( String( "FAILED: OCIHandleAlloc(): alloc handle of type " ) << (long) aHandlePtr.getHandleType()
						  << " failed" );
		return false;
	}
	return true;
}

bool OracleConnection::Open( String const &strServer, String const &strUsername, String const &strPassword )
{
	StartTrace(OracleConnection.Open);

	if ( fStatus < eHandlesAllocated ) {
		SYSERROR("Allocation of OCI handles failed, can not connect to server [" << strServer << "] with user [" << strUsername << "]!");
		return false;
	}
	if ( fStatus > eHandlesAllocated ) {
		SYSERROR("tried to open already opened connection to server [" << strServer << "] and user [" << strUsername << "]!");
		return false;
	}
	text const *server( reinterpret_cast<const text *> ( (const char *) strServer ) );
	text const *username( reinterpret_cast<const text *> ( (const char *) strUsername ) );
	text const *password( reinterpret_cast<const text *> ( (const char *) strPassword ) );

	String strErr( 128L );

	// --- attach server
	if ( checkError( OCIServerAttach( fSrvhp.getHandle(), fErrhp.getHandle(), server, strlen( (const char *) server ),
									  (ub4) OCI_DEFAULT ), strErr ) ) {
		SystemLog::Error( String( "FAILED: OCIServerAttach() to server [" ) << strServer << "] failed (" << strErr << ")" );
		return false;
	}
	fStatus = eServerAttached;

	// --- set attribute server context in the service context
	if ( checkError( OCIAttrSet( fSvchp.getHandle(), (ub4) OCI_HTYPE_SVCCTX, fSrvhp.getHandle(), (ub4) 0,
								 (ub4) OCI_ATTR_SERVER, fErrhp.getHandle() ), strErr ) ) {
		SystemLog::Error( String( "FAILED: OCIAttrSet(): setting attribute <server> into the service context failed (" )
						  << strErr << ")" );
		return false;
	}

	// --- set attributes in the authentication handle
	if ( checkError( OCIAttrSet( fUsrhp.getHandle(), (ub4) OCI_HTYPE_SESSION, (dvoid *) username, (ub4) strlen(
									 (const char *) username ), (ub4) OCI_ATTR_USERNAME, fErrhp.getHandle() ), strErr ) ) {
		SystemLog::Error( String(
							  "FAILED: OCIAttrSet(): setting attribute <username> in the authentication handle failed (" ) << strErr
						  << ")" );
		return false;
	}

	if ( checkError( OCIAttrSet( fUsrhp.getHandle(), (ub4) OCI_HTYPE_SESSION, (dvoid *) password, (ub4) strlen(
									 (const char *) password ), (ub4) OCI_ATTR_PASSWORD, fErrhp.getHandle() ), strErr ) ) {
		SystemLog::Error( String(
							  "FAILED: OCIAttrSet(): setting attribute <password> in the authentication handle failed (" ) << strErr
						  << ")" );
		return false;
	}

	if ( checkError( OCISessionBegin( fSvchp.getHandle(), fErrhp.getHandle(), fUsrhp.getHandle(), OCI_CRED_RDBMS,
									  (ub4) OCI_DEFAULT ), strErr ) ) {
		SystemLog::Error( String( "FAILED: OCISessionBegin() with user [" ) << strUsername << "] failed (" << strErr
						  << ")" );
		return false;
	}
	Trace( "connected to oracle as " << strUsername )

	// --- Set the authentication handle into the Service handle
	if ( checkError( OCIAttrSet( fSvchp.getHandle(), (ub4) OCI_HTYPE_SVCCTX, fUsrhp.getHandle(), (ub4) 0,
								 OCI_ATTR_SESSION, fErrhp.getHandle() ), strErr ) ) {
		SystemLog::Error( String( "FAILED: OCIAttrSet(): setting attribute <session> into the service context failed (" )
						  << strErr << ")" );
		return false;
	}
	fStatus = eSessionValid;
	return true;
}

OracleConnection::~OracleConnection()
{
	StartTrace(OracleConnection.~OracleConnection);
	Close();
	fSrvhp.reset();
	fSvchp.reset();
	fErrhp.reset();
	fUsrhp.reset();
	fStatus = eUnitialized;
}

void OracleConnection::Close()
{
	StartTrace(OracleConnection.Close);
	if ( fStatus == eSessionValid ) {
		if ( OCISessionEnd( fSvchp.getHandle(), fErrhp.getHandle(), fUsrhp.getHandle(), 0 ) ) {
			SystemLog::Error( "FAILED: OCISessionEnd() on svchp failed" );
		}
	}
	if ( fStatus >= eServerAttached ) {
		if ( OCIServerDetach( fSrvhp.getHandle(), fErrhp.getHandle(), OCI_DEFAULT ) ) {
			SystemLog::Error( "FAILED: OCIServerDetach() on srvhp failed" );
		}
	}
	fStatus = eHandlesAllocated;
}

bool OracleConnection::checkError( sword status )
{
	bool bRet( true );
	if ( status == OCI_SUCCESS || status == OCI_NO_DATA ) {
		bRet = false;
	}
	StatTrace(OracleConnection.checkError, "status: " << (long) status << " retcode: " << (bRet ? "true" : "false"), Storage::Current());
	return bRet;
}

bool OracleConnection::checkError( sword status, String &message )
{
	bool bError( checkError( status ) );
	if ( bError ) {
		message = errorMessage( status );
		StatTrace(OracleConnection.checkError, "status: " << (long) status << " message [" << message << "]", Storage::Current());
	}
	return bError;
}

String OracleConnection::errorMessage( sword status )
{
	// error handling: checks 'status' for errors
	// in case of an error an error message is generated

	text errbuf[512];
	sb4 errcode;
	String error( 128L );

	switch ( status ) {
		case OCI_NO_DATA:
		case OCI_SUCCESS:
			// no error
			return error;
			break;
		case OCI_SUCCESS_WITH_INFO:
			error << "Error - OCI_SUCCESS_WITH_INFO";
			break;
		case OCI_NEED_DATA:
			error << "Error - OCI_NEED_DATA";
			break;
		case OCI_ERROR:
			if ( fErrhp.getHandle() )
				OCIErrorGet( (dvoid *) fErrhp.getHandle(), 1, NULL, &errcode, errbuf, (ub4) sizeof ( errbuf ),
							 OCI_HTYPE_ERROR );
			error << "Error - " << (char *) errbuf;
			break;
		case OCI_INVALID_HANDLE:
			error << "Error - OCI_INVALID_HANDLE";
			break;
		case OCI_STILL_EXECUTING:
			error << "Error - OCI_STILL_EXECUTE";
			break;
		case OCI_CONTINUE:
			error << "Error - OCI_CONTINUE";
			break;
		default:
			break;
	}

	return error;
}

OracleStatementPtr OracleConnection::createStatement( String strStatement, long lPrefetchRows,
		OracleConnection::ObjectType aObjType, String strReturnName )
{
	StartTrace1(OracleConnection.createStatement, "sp name [" << strStatement << "]");
	ROAnything desc;
	if ( aObjType == OracleConnection::TYPE_UNK ) {
		try {
			aObjType = GetSPDescription( strStatement, desc );
			strStatement = ConstructSPStr( strStatement, aObjType == OracleConnection::TYPE_FUNC, desc, strReturnName );
			Trace(String("prepared stored ") << ( (aObjType == OracleConnection::TYPE_FUNC) ? "function: " : "procedure: ") << strStatement);
		} catch ( OracleException &ex ) {
			Trace("caught exception but throwing again...");
			throw ex;
		}
	}
	OracleStatementPtr pStmt( new (Storage::Current()) OracleStatement( this, strStatement ) );
	if ( pStmt.get() ) {
		pStmt->setPrefetchRows( lPrefetchRows );
		if ( pStmt->Prepare() && pStmt->getStatementType() == OracleStatement::STMT_BEGIN ) {
			pStmt->setSPDescription( desc, strReturnName );
		}
	}
	return pStmt;
}

bool checkGetCacheEntry(ROAnything &roaCache, OracleConnection::ObjectType &aObjType, const String &command, ROAnything &desc )
{
	StartTrace(OracleConnection.checkGetCacheEntry);
	ROAnything roaCacheEntry;
	if ( roaCache.LookupPath( roaCacheEntry, command ) ) {
		desc = roaCacheEntry["description"];
		aObjType = (OracleConnection::ObjectType) roaCacheEntry["type"].AsLong( (long) OracleConnection::TYPE_UNK );
		TraceAny(roaCacheEntry, "entry [" << command << "] found in cache");
		return true;
	}
	return false;
}

OracleConnection::ObjectType OracleConnection::GetSPDescription( const String &command, ROAnything &desc )
{
	StartTrace(OracleConnection.GetSPDescription);

	ObjectType aObjType( TYPE_UNK );
	{
		LockUnlockEntry aLockEntry( fgDescriptionLock, RWLock::eReading );
		if ( checkGetCacheEntry(fgDescriptionCacheRO, aObjType, command, desc) ) {
			return aObjType;
		}
	}
	{
		LockUnlockEntry aLockEntry( fgDescriptionLock, RWLock::eWriting );
		// check if someone was faster and already stored the description
		if ( checkGetCacheEntry(fgDescriptionCacheRO, aObjType, command, desc) ) {
			return aObjType;
		}
		Anything anyDesc( fgDescriptionCache.GetAllocator() );
		Anything anyType( fgDescriptionCache.GetAllocator() );
		aObjType = ReadSPDescriptionFromDB( command, anyDesc );
		anyType = (long) aObjType;

		SlotPutter::Operate(anyDesc, fgDescriptionCache, String(command).Append('.').Append("description"));
		SlotPutter::Operate(anyType, fgDescriptionCache, String(command).Append('.').Append("type"));

		TraceAny(anyDesc, "new entry [" << command << "] stored in cache");
		if ( checkGetCacheEntry(fgDescriptionCacheRO, aObjType, command, desc) ) {
			return aObjType;
		}
	}
	return aObjType;
}

OracleConnection::ObjectType OracleConnection::DescribeObjectByName(const String &command, DscHandleType &aDschp, OCIParam *&parmh)
{
	StartTrace(OracleConnection.DescribeObjectByName);
	text const *objptr = reinterpret_cast<const text *> ( (const char *) command );
	String strErr( "DescribeObjectByName: " );
	sword attrStat = OCIDescribeAny( SvcHandle(), ErrorHandle(), (dvoid *) objptr, (ub4) strlen( (char *) objptr ),
									 OCI_OTYPE_NAME, 0, OCI_PTYPE_UNK, aDschp.getHandle() );
	Trace("status after DESCRIBEANY::OCI_PTYPE_UNK: " << (long)attrStat)
	if ( checkError( attrStat, strErr ) ) {
		strErr << "{" << command << " is neither a stored procedure nor a function}";
		throw OracleException( *this, strErr );
	}

	Trace("get the parameter handle")
	attrStat = OCIAttrGet( aDschp.getHandle(), OCI_HTYPE_DESCRIBE, (dvoid *) &parmh, 0, OCI_ATTR_PARAM, ErrorHandle() );
	Trace("status after OCI_HTYPE_DESCRIBE::OCI_ATTR_PARAM: " << (long)attrStat);
	if ( checkError( attrStat ) ) {
		throw OracleException( *this, attrStat );
	}

	ObjectType aStmtType( TYPE_UNK );
	ub1 ubFuncType( 0 );
	attrStat = OCIAttrGet( parmh, OCI_DTYPE_PARAM, (dvoid *) &ubFuncType, 0, OCI_ATTR_PTYPE, ErrorHandle() );
	Trace("status after OCI_DTYPE_PARAM::OCI_ATTR_PTYPE: " << (long)attrStat << " funcType:" << (long)ubFuncType);
	if ( checkError( attrStat ) ) {
		throw OracleException( *this, attrStat );
	}
	aStmtType = (ObjectType)ubFuncType;
	return aStmtType;
}

OracleConnection::ObjectType OracleConnection::ReadSPDescriptionFromDB( const String &command, Anything &desc )
{
	StartTrace(OracleConnection.ReadSPDescriptionFromDB);

	String strErr( "ReadSPDescriptionFromDB: " );

	MemChecker aCheckerLocal( "OracleConnection.ReadSPDescriptionFromDB", getEnvironment().getAllocator() );
	MemChecker aCheckerGlobal( "OracleConnection.ReadSPDescriptionFromDB",
							   OracleEnvironment::getGlobalEnv().getAllocator() );

	sword attrStat;
	DscHandleType aDschp;
	if ( checkError( ( attrStat = OCIHandleAlloc( getEnvironment().EnvHandle(), aDschp.getVoidAddr(),
								  OCI_HTYPE_DESCRIBE, 0, 0 ) ) ) ) {
		throw OracleException( *this, attrStat );
	}
	Trace("after HandleAlloc, local allocator:" << (long)getEnvironment().getAllocator());
	Trace("after HandleAlloc, global allocator:" << (long)OracleEnvironment::getGlobalEnv().getAllocator());

	OCIParam *parmh( 0 );
	ObjectType aStmtType = DescribeObjectByName(command, aDschp, parmh);
	if ( aStmtType == TYPE_SYN ) {
		Trace("as we identified a synonym, we need to collect the scheme name and the referenced object name to ask for description");
		text *name(0);
		ub4 namelen(0);
		String strSchemaName;
		attrStat = OCIAttrGet( (dvoid *) parmh, OCI_DTYPE_PARAM, (dvoid *) &name, (ub4 *) &namelen, OCI_ATTR_SCHEMA_NAME, ErrorHandle() );
		if ( checkError( ( attrStat ) ) ) {
			throw OracleException( *this, attrStat );
		}
		strSchemaName.Append(String( (char *) name, namelen ));
		Trace("SchemaName: " << strSchemaName);

		attrStat = OCIAttrGet( (dvoid *) parmh, OCI_DTYPE_PARAM, (dvoid *) &name, (ub4 *) &namelen, OCI_ATTR_NAME, ErrorHandle() );
		if ( checkError( ( attrStat ) ) ) {
			throw OracleException( *this, attrStat );
		}
		if ( strSchemaName.Length() ) {
			strSchemaName.Append('.');
		}
		strSchemaName.Append(String( (char *) name, namelen ));

		Trace("trying to get descriptions for " << strSchemaName);
		aStmtType = DescribeObjectByName(strSchemaName, aDschp, parmh);
	}
	bool bIsFunction = ( aStmtType == TYPE_FUNC );

	Trace("get the number of arguments and the arg list for stored " << (bIsFunction ? "function" : "procedure"))
	OCIParam *arglst( 0 );
	ub2 numargs = 0;
	if ( checkError( ( attrStat = OCIAttrGet( (dvoid *) parmh, OCI_DTYPE_PARAM, (dvoid *) &arglst, (ub4 *) 0,
								  OCI_ATTR_LIST_ARGUMENTS, ErrorHandle() ) ) ) ) {
		throw OracleException( *this, attrStat );
	}
	if ( checkError( ( attrStat = OCIAttrGet( (dvoid *) arglst, OCI_DTYPE_PARAM, (dvoid *) &numargs, (ub4 *) 0,
								  OCI_ATTR_NUM_PARAMS, ErrorHandle() ) ) ) ) {
		throw OracleException( *this, attrStat );
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
		if ( checkError( ( attrStat = OCIParamGet( (dvoid *) arglst, OCI_DTYPE_PARAM, ErrorHandle(), (dvoid **) &arg,
									  (ub4) i ) ) ) ) {
			throw OracleException( *this, attrStat );
		}
		namelen = 0;
		name = 0;
		data_len = 0;

		if ( checkError( ( attrStat = OCIAttrGet( (dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &dtype, (ub4 *) 0,
									  OCI_ATTR_DATA_TYPE, ErrorHandle() ) ) ) ) {
			throw OracleException( *this, attrStat );
		}
		Trace("Data type: " << dtype)

		if ( checkError( ( attrStat = OCIAttrGet( (dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &name, (ub4 *) &namelen,
									  OCI_ATTR_NAME, ErrorHandle() ) ) ) ) {
			throw OracleException( *this, attrStat );
		}
		String strName( (char *) name, namelen );
		// the first param of a function is the return param
		if ( bIsFunction && i == start ) {
			strName = command;
			Trace("Name: " << strName)
		}

		// 0 = IN (OCI_TYPEPARAM_IN), 1 = OUT (OCI_TYPEPARAM_OUT), 2 = IN/OUT (OCI_TYPEPARAM_INOUT)
		if ( checkError( ( attrStat = OCIAttrGet( (dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &iomode, (ub4 *) 0,
									  OCI_ATTR_IOMODE, ErrorHandle() ) ) ) ) {
			throw OracleException( *this, attrStat );
		}
		Trace("IO type: " << iomode)

		if ( checkError( ( attrStat = OCIAttrGet( (dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &data_len, (ub4 *) 0,
									  OCI_ATTR_DATA_SIZE, ErrorHandle() ) ) ) ) {
			throw OracleException( *this, attrStat );
		}
		Trace("Size: " << (int)data_len)

		Anything param( desc.GetAllocator() );
		param["Name"] = strName;
		param["Type"] = dtype;
		param["Length"] = (int) data_len;
		param["IoMode"] = iomode;
		param["Idx"] = (long) ( bIsFunction ? i + 1 : i );
		desc.Append( param );
		if ( checkError( ( attrStat = OCIDescriptorFree( arg, OCI_DTYPE_PARAM ) ) ) ) {
			throw OracleException( *this, attrStat );
		}
	}

	TraceAny(desc, "parameter description");
	return aStmtType;
}

String OracleConnection::ConstructSPStr( String const &command, bool pIsFunction, ROAnything desc,
		const String &strReturnName )
{
	String plsql, strParams;
	plsql << "BEGIN ";
	AnyExtensions::Iterator<ROAnything> aIter( desc );
	ROAnything roaEntry;
	if ( pIsFunction ) {
		aIter.Next( roaEntry );
		plsql << ":" << ( strReturnName.Length() ? strReturnName : roaEntry["Name"].AsString() ) << " := ";
	}
	while ( aIter.Next( roaEntry ) ) {
		if ( strParams.Length() ) {
			strParams.Append( ',' );
		}
		strParams.Append( ':' ).Append( roaEntry["Name"].AsString() );
	}
	plsql << command << "(" << strParams << "); END;";
	StatTrace(OracleConnection.ConstructSPStr, "SP string [" << plsql << "]", Storage::Current());
	return plsql;
}

