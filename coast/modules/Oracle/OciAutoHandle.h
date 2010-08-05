/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef OCIAUTOHANDLE_H_
#define OCIAUTOHANDLE_H_

#include <memory>
#include "oci.h"

//---- oci_auto_handle -----------------------------------------------------------
//! Template wrapper around OCI handles
/*!
 * Simplifies memory handling on allocated handles, frees the internal handle if allocated. It works almost like a
 * std::auto_ptr in behavior of assignment, construction etc.
 */
template<typename handle, ub4 handleType>
struct oci_auto_handle {
	/*! convenience pointer type */
	typedef handle *handle_ptr_type;
	/*! convenience reference to pointer type */
	typedef handle * &handle_ptr_type_ref;
	/*! convenience pointer to pointer type (used to re-allocate internal handle) */
	typedef handle **handle_ptr_addr_type;

	/*! data field of type handle_ptr_addr_type to store the tracked handle */
	handle_ptr_type fHandle;

	/*! default ctor to initialize empty object
	 */
	oci_auto_handle() :
		fHandle( 0 ) {
	}
	/*! ctor using newly created handle to store internally
	 * @param pHandle newly allocated handle of type handle
	 */
	oci_auto_handle( handle_ptr_type pHandle ) :
		fHandle( pHandle ) {
	}
	//! frees allocated handle if stored internally
	~oci_auto_handle() {
		doDelete();
	}
	/*! assignment operator to pass ownership from right hand side to left hand side object
	 * @param rhs non const reference to right hand side object
	 * @note rhs will not contain the internal handle anymore after the call
	 */
	oci_auto_handle &operator=( oci_auto_handle &rhs ) throw () {
		reset( rhs.release() );
		return *this;
	}
	/*! this function is used to release ownership of the currently held handle
	 * @return internal handle
	 */
	handle_ptr_type release() {
		handle_ptr_type p( fHandle );
		fHandle = 0;
		return p;
	}
	/*! assign a new handle to the current object and free the current one if valid
	 * @param ptr new handle or 0 if you want to force deletion of the current handle
	 */
	void reset( handle_ptr_type ptr = 0 ) throw () {
		if ( fHandle != ptr ) {
			doDelete();
			fHandle = ptr;
		}
	}
	/*! call the handle specific free function to deallocate the previously allocated resource */
	void doDelete() throw () {
		if ( fHandle ) {
			OCIHandleFree( reinterpret_cast<dvoid *> ( release() ), handleType );
		}
	}
	/*! access the stored handle
	 * @return internal handle
	 * */
	handle_ptr_type getHandle() const {
		return fHandle;
	}
	ub4 getHandleType() const {
		return handleType;
	}
	/*! Accessor to use when creating a new handle.
	 * This method will give access to the handle data field which is needed to allocate a new handle of a given type in this object.
	 * @return address of the internal handle data area
	 * @note Calling this method repeatedly will delete the previously allocated handle object! Usually you just need to use this function once when initially allocating the resource handle.
	 */
	handle_ptr_addr_type getHandleAddr() {
		doDelete();
		return &fHandle;
	}
	/*! Accessor to use when creating a new handle.
	 * This method will give access to the handle data field which is needed to allocate a new handle of a given type in this object.
	 * @return address of the internal handle data area as void**
	 * @note Calling this method repeatedly will delete the previously allocated handle object! Usually you just need to use this function once when initially allocating the resource handle.
	 */
	dvoid **getVoidAddr() {
		doDelete();
		return (dvoid **) &fHandle;
	}
	/*! Convenience operator to check for validity of handle.
	 * @return true in case the handle data field is not 0
	 */
	operator const bool() const {
		return fHandle != 0;
	}
private:
	/*! forbid copy ctor
	 */
	oci_auto_handle( oci_auto_handle &rhs );
};

//! OCI describe handle
typedef oci_auto_handle<OCIDescribe, OCI_HTYPE_DESCRIBE> DscHandleType;
//! OCI statement handle
typedef oci_auto_handle<OCIStmt, OCI_HTYPE_STMT> StmtHandleType;
//! OCI environment handle
typedef oci_auto_handle<OCIEnv, OCI_HTYPE_ENV> EnvHandleType;
//! OCI error handle
typedef oci_auto_handle<OCIError, OCI_HTYPE_ERROR> ErrHandleType;
//! OCI server connection handle
typedef oci_auto_handle<OCIServer, OCI_HTYPE_SERVER> SrvHandleType;
//! OCI service context handle
typedef oci_auto_handle<OCISvcCtx, OCI_HTYPE_SVCCTX> SvcHandleType;
//! OCI user session handle
typedef oci_auto_handle<OCISession, OCI_HTYPE_SESSION> UsrHandleType;

class OracleResultset;
class OracleStatement;
class OracleConnection;
class OracleEnvironment;

//! type definition of auto cleanup'd OracleStatement
typedef std::auto_ptr<OracleStatement> OracleStatementPtr;
typedef std::auto_ptr<OracleResultset> OracleResultsetPtr;
typedef std::auto_ptr<OracleConnection> OracleConnectionPtr;
typedef std::auto_ptr<OracleEnvironment> OracleEnvironmentPtr;

#endif /* OCIAUTOHANDLE_H_ */
