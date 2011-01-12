/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleEnvironment.h"
#include "OracleConnection.h"
#include "SystemLog.h"
#include "MT_Storage.h"
#include "Dbg.h"

// set to 1 to track OCI's memory usage using our MemTracker infrastructure
#if (1)
dvoid *malloc_func( void *ctxp, size_t size )
{
	Allocator *pAlloc( Storage::Global() );
	if ( ctxp != 0 ) {
		pAlloc = ( reinterpret_cast<OracleEnvironment *> ( ctxp ) )->getAllocator();
	}
	dvoid *ptr( pAlloc->Malloc( size ) );
	StatTrace(OracleEnvironment.mem_alloc, "size: " << (long)size << " ptr: &" << (long)ptr << " allocator:" << (pAlloc == Storage::Global() ? "global" : "local") << "&" << (long)pAlloc, Storage::Current());
	return ptr;
}
dvoid *realloc_func( void *ctxp, void *ptr, size_t size )
{
	Allocator *pAlloc( Storage::Global() );
	if ( ctxp != 0 ) {
		pAlloc = ( reinterpret_cast<OracleEnvironment *> ( ctxp ) )->getAllocator();
	}
	dvoid *nptr( realloc( ptr, size ) );
	StatTrace(OracleEnvironment.mem_realloc, "size: " << (long)size << " oldptr: &" << (long)ptr << " new ptr: &" << (long)nptr << " allocator:" << (pAlloc == Storage::Global() ? "global" : "local") << "&" << (long)pAlloc, Storage::Current());
	return ( nptr );
}
void free_func( void *ctxp, dvoid *ptr )
{
	Allocator *pAlloc( Storage::Global() );
	if ( ctxp != 0 ) {
		pAlloc = ( reinterpret_cast<OracleEnvironment *> ( ctxp ) )->getAllocator();
	}
	StatTrace(OracleEnvironment.mem_free, "ptr: &" << (long)ptr << " allocator:" << (pAlloc == Storage::Global() ? "global" : "local") << "&" << (long)pAlloc, Storage::Current());
	pAlloc->Free( ptr );
}
#define ctx_ptr this
#else
#	define	malloc_func		NULL
#	define	realloc_func	NULL
#	define	free_func		NULL
#	define	ctx_ptr			NULL
#endif

OracleEnvironment::OraTerminator OracleEnvironment::fgOraTerminator;

OracleEnvironment::OracleEnvironment( Mode eMode, u_long ulPoolId, u_long ulPoolSize,
									  unsigned long ulBuckets ) :
	fEnvhp(), fMemPool( new PoolAllocator( (0xac1e << 16) | ulPoolId, ulPoolSize, ulBuckets ) )
{
	StartTrace(OracleEnvironment.OracleEnvironment);
	// caution: the following memory handles supplied must allocate on Storage::Global()
	// because memory gets allocated through them in Open and freed in Close. Throughout the
	// lifetime of the connection, multiple threads could share the same connection and so we
	// must take care not to allocate on the first Thread opening the connection
	if ( OCIEnvCreate( fEnvhp.getHandleAddr(), eMode, ctx_ptr, // context
					   malloc_func, // malloc function to allocate handles and env specific memory
					   realloc_func, // realloc function to allocate handles and env specific memory
					   free_func, // free function to allocate handles and env specific memory
					   0, // extra memory to allocate
					   NULL // pointer to user-memory
					 ) != OCI_SUCCESS ) {
		SystemLog::Error( "FAILED: OCIEnvCreate(): could not create OCI environment" );
		fEnvhp.reset();
	}
}

OracleEnvironment::~OracleEnvironment()
{
	StartTrace(OracleEnvironment.~OracleEnvironment);
	fEnvhp.reset();
}

OracleConnectionPtr OracleEnvironment::createConnection( String const &strSrv, String const &strUsr, String const &strPwd )
{
	StartTrace(OracleEnvironment.createConnection);
	OracleConnectionPtr pConnection( new OracleConnection( *this ) );
	if ( pConnection.get() ) {
		if ( !pConnection->Open( strSrv, strUsr, strPwd ) ) {
			pConnection.reset();
		}
	}
	return pConnection;
}
