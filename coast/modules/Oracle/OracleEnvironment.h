/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLEENVIRONMENT_H_
#define ORACLEENVIRONMENT_H_

//--- modules used in the interface
#include "AllocatorNewDelete.h"
#include "OciAutoHandle.h"
#include "PoolAllocator.h"
#include <memory>

class OracleConnection;
class String;

typedef std::auto_ptr<Allocator> AllocatorPtr;

//! Abstraction for an Oracle environment
/*!
 * This class serves as abstraction for an OCI environment. Such an environment is needed to create an OracleConnection.
 */
class OracleEnvironment : public Coast::AllocatorNewDelete
{
private:
	//! OCI environment handle
	EnvHandleType fEnvhp;
	AllocatorPtr fMemPool;

	OracleEnvironment();
	OracleEnvironment( const OracleEnvironment & );
	OracleEnvironment &operator=( const OracleEnvironment & );

public:
	/*! construction mode of environment */
	enum Mode {
		//! unknown default mode
		DEFAULT = OCI_DEFAULT,
		//! appl. in threaded environment
		THREADED_MUTEXED = OCI_THREADED,
		//! appl. in threaded environment but the environment handle will not be protected by a mutex internally
		THREADED_UNMUTEXED = OCI_THREADED | OCI_NO_MUTEX,
	};
	/*! specify mode of environment when creating it
	 * @param eMode chose one of the possible OracleEnvironment::Mode flags
	 * @note As we use an environment per OraclePooledConnection, we could safely use OracleEnvironment::THREADED_UNMUTEXED
	 */
	OracleEnvironment( Mode eMode, unsigned long ulPoolId, u_long ulPoolSize, u_long ulBuckets );
	/*! destruction of environment handle
	 */
	~OracleEnvironment();

	/*! access internal OCIEnv handle pointer
	 * @return OCIEnv handle
	 */
	OCIEnv *EnvHandle() {
		return fEnvhp.getHandle();
	}

	/*! check if the handle was allocated (!=0)
	 * @return true if a handle is available
	 */
	bool valid() const {
		return fEnvhp;
	}

	/*! Create a new connection based on this objects environment handle
	 * @param strSrv oracle database connection string
	 * @param strUsr database user to connect with
	 * @param strPwd password for the above user
	 * @return pointer to newly created OracleConnection object
	 * @note The returned OracleConnection object must be freed by the caller!
	 */
	OracleConnectionPtr createConnection( String const &strSrv, String const &strUsr, String const &strPwd );

	Allocator *getAllocator() {
		return fMemPool.get();
	}
	static OracleEnvironment &getGlobalEnv();
};

#endif /* ORACLEENVIRONMENT_H_ */
