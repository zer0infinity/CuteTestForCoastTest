/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleEnvironment.h"
#include "OracleConnection.h"
#include "SysLog.h"
#include "Dbg.h"

// set to 1 to track OCI's memory usage
#if (1)
dvoid *malloc_func(dvoid * /* ctxp */, size_t size)
{
//	dvoid * ptr(malloc(size));
	dvoid *ptr(Storage::Global()->Malloc(size));
	StatTrace(OracleEnvironment.malloc_func, "size: " << (long)size << " ptr: &" << (long)ptr, Storage::Current());
	return ptr;
}
dvoid *realloc_func(dvoid * /* ctxp */, dvoid *ptr, size_t size)
{
	dvoid *nptr(realloc(ptr, size));
	StatTrace(OracleEnvironment.realloc_func, "size: " << (long)size << " oldptr: &" << (long)ptr << " new ptr: &" << (long)nptr, Storage::Current());
	return (nptr);
}
void free_func(dvoid * /* ctxp */, dvoid *ptr)
{
	StatTrace(OracleEnvironment.free_func, "ptr: &" << (long)ptr, Storage::Current());
//	free(ptr);
	Storage::Global()->Free(ptr);
}
#else
#	define	malloc_func		NULL
#	define	realloc_func	NULL
#	define	free_func		NULL
#endif

OracleEnvironment::OracleEnvironment(Mode eMode) : fEnvhp()
{
	StartTrace(OracleEnvironment.OracleEnvironment);
	// caution: the following memory handles supplied must allocate on Storage::Global()
	// because memory gets allocated through them in Open and freed in Close. Throughout the
	// lifetime of the connection, mutliple threads could share the same connection and so we
	// must take care not to allocate on the first Thread opening the connection
	if ( OCIEnvCreate( fEnvhp.getHandleAddr(), eMode, NULL, // context
					   malloc_func, // malloc function to allocate handles and env specific memory
					   realloc_func, // realloc function to allocate handles and env specific memory
					   free_func, // free function to allocate handles and env specific memory
					   0, // extra memory to allocate
					   NULL // pointer to user-memory
					 ) != OCI_SUCCESS ) {
		SysLog::Error( "FAILED: OCIEnvCreate(): could not create OCI environment" );
		fEnvhp.reset();
	}
}

OracleEnvironment::~OracleEnvironment()
{
	StartTrace(OracleEnvironment.~OracleEnvironment);
	fEnvhp.reset();
}

OracleConnection *OracleEnvironment::createConnection( String const &strSrv, String const &strUsr, String const &strPwd )
{
	StartTrace(OracleEnvironment.createConnection);
	OracleConnection *pConnection( new OracleConnection(*this) );
	if ( pConnection ) {
		if ( !pConnection->Open(strSrv, strUsr, strPwd) ) {
			delete pConnection;
			pConnection = 0;
		}
	}
	return pConnection;
}
