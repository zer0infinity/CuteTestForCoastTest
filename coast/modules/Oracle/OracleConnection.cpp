/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

OracleConnection::OracleConnection(OracleEnvironment &rEnv) :
	fConnected(false), fOracleEnv(rEnv), fErrhp(), fSrvhp(), fSvchp(), fUsrhp()
{
	// --- alloc error handle
	if ( OCIHandleAlloc( fEnvhp.getHandle(), fErrhp.getVoidAddr(), (ub4) OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0 )
		 != OCI_SUCCESS ) {
		SysLog::Error( "FAILED: OCIHandleAlloc(): alloc error handle failed" );
		return false;
	}

	String strErr;

	// --- alloc service context handle
	if ( checkError( OCIHandleAlloc( fEnvhp.getHandle(), fSvchp.getVoidAddr(), (ub4) OCI_HTYPE_SVCCTX, (size_t) 0,
									 (dvoid **) 0 ), strErr ) ) {
		SysLog::Error( String( "FAILED: OCIHandleAlloc(): alloc service context handle failed (" ) << strErr << ")" );
		return false;
	}
	// --- alloc server connection handle
	if ( checkError( OCIHandleAlloc( fEnvhp.getHandle(), fSrvhp.getVoidAddr(), (ub4) OCI_HTYPE_SERVER, (size_t) 0,
									 (dvoid **) 0 ), strErr ) ) {
		SysLog::Error( String( "FAILED: OCIHandleAlloc(): alloc server connection handle failed (" ) << strErr << ")" );
		return false;
	}
	// --- alloc user session handle
	if ( checkError( OCIHandleAlloc( fEnvhp.getHandle(), fUsrhp.getVoidAddr(), (ub4) OCI_HTYPE_SESSION, (size_t) 0,
									 (dvoid **) 0 ), strErr ) ) {
		SysLog::Error( String( "FAILED: OCIHandleAlloc(): alloc user session handle failed (" ) << strErr << ")" );
		return false;
	}
	if ( checkError( OCIHandleAlloc( fEnvhp.getHandle(), fDschp.getVoidAddr(), (ub4) OCI_HTYPE_DESCRIBE, (size_t) 0,
									 (dvoid **) 0 ), strErr ) ) {
		SysLog::Error( String( "FAILED: OCIHandleAlloc(): alloc describe handle failed (" ) << strErr << ")" );
		return false;
	}
}

bool OracleConnection::Open( String const &strServer, String const &strUsername, String const &strPassword )
{
	StartTrace(OracleConnection.Open);

	text const *server( reinterpret_cast<const text *> ( (const char *) strServer ) );
	text const *username( reinterpret_cast<const text *> ( (const char *) strUsername ) );
	text const *password( reinterpret_cast<const text *> ( (const char *) strPassword ) );

	if ( fConnected ) {
		SYSERROR("tried to open already opened connection to server [" << strServer << "] and user [" << strUsername << "]!");
		return false;
	}

	// --- attach server
	if ( checkError( OCIServerAttach( fSrvhp.getHandle(), fErrhp.getHandle(), server, strlen( (const char *) server ),
									  (ub4) OCI_DEFAULT ), strErr ) ) {
		SysLog::Error( String( "FAILED: OCIServerAttach() to server [" ) << strServer << "] failed (" << strErr << ")" );
		return false;
	}

	// --- set attribute server context in the service context
	if ( checkError( OCIAttrSet( fSvchp.getHandle(), (ub4) OCI_HTYPE_SVCCTX, fSrvhp.getHandle(), (ub4) 0,
								 (ub4) OCI_ATTR_SERVER, fErrhp.getHandle() ), strErr ) ) {
		SysLog::Error( String( "FAILED: OCIAttrSet(): setting attribute <server> into the service context failed (" )
					   << strErr << ")" );
		return false;
	}

	// --- set attributes in the authentication handle
	if ( checkError( OCIAttrSet( fUsrhp.getHandle(), (ub4) OCI_HTYPE_SESSION, (dvoid *) username, (ub4) strlen(
									 (const char *) username ), (ub4) OCI_ATTR_USERNAME, fErrhp.getHandle() ), strErr ) ) {
		SysLog::Error( String(
						   "FAILED: OCIAttrSet(): setting attribute <username> in the authentication handle failed (" ) << strErr
					   << ")" );
		return false;
	}

	if ( checkError( OCIAttrSet( fUsrhp.getHandle(), (ub4) OCI_HTYPE_SESSION, (dvoid *) password, (ub4) strlen(
									 (const char *) password ), (ub4) OCI_ATTR_PASSWORD, fErrhp.getHandle() ), strErr ) ) {
		SysLog::Error( String(
						   "FAILED: OCIAttrSet(): setting attribute <password> in the authentication handle failed (" ) << strErr
					   << ")" );
		return false;
	}

	if ( checkError( OCISessionBegin( fSvchp.getHandle(), fErrhp.getHandle(), fUsrhp.getHandle(), OCI_CRED_RDBMS,
									  (ub4) OCI_DEFAULT ), strErr ) ) {
		SysLog::Error( String( "FAILED: OCISessionBegin() with user [" ) << strUsername << "] failed (" << strErr
					   << ")" );
		return false;
	}
	Trace( "connected to oracle as " << strUsername );

	// --- Set the authentication handle in the Service handle
	if ( checkError( OCIAttrSet( fSvchp.getHandle(), (ub4) OCI_HTYPE_SVCCTX, fUsrhp.getHandle(), (ub4) 0,
								 OCI_ATTR_SESSION, fErrhp.getHandle() ), strErr ) ) {
		SysLog::Error( String( "FAILED: OCIAttrSet(): setting attribute <session> into the service context failed (" )
					   << strErr << ")" );
		return false;
	}
}

OracleConnection::~OracleConnection()
{
	Close();
}

void OracleConnection::Close( )
{
	if ( OCISessionEnd( fSvchp.getHandle(), fErrhp.getHandle(), fUsrhp.getHandle(), (ub4) 0 ) ) {
		SysLog::Error( "FAILED: OCISessionEnd() on svchp failed" );
	}
	if ( OCIServerDetach( fSrvhp.getHandle(), fErrhp.getHandle(), (ub4) OCI_DEFAULT ) ) {
		SysLog::Error( "FAILED: OCISessionEnd() on srvhp failed" );
	}

	fSrvhp.reset();
	fSvchp.reset();
	fErrhp.reset();
	fUsrhp.reset();
	fDschp.reset();
}
