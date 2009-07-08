/*
 * OciAutoHandle.h
 *
 *  Created on: Jul 8, 2009
 *      Author: m1huber
 */

#ifndef OCIAUTOHANDLE_H_
#define OCIAUTOHANDLE_H_

#include "oci.h"

//---- oci_auto_handle -----------------------------------------------------------
//! <B>wrapper template class around OCI handles</B>
/*!
Simplifies memory handling on allocated handles, frees the internal handle if allocated. It works almost like a std::auto_ptr in behavior of assignment, construction etc.
*/
template<typename handle, ub4 handleType>
struct oci_auto_handle {
	typedef handle *handle_ptr_type;
	typedef handle * &handle_ptr_type_ref;
	typedef handle **handle_ptr_addr_type;

	handle_ptr_type fHandle;
	oci_auto_handle(): fHandle(0) {}
	~oci_auto_handle() {
		doDelete();
	}
	oci_auto_handle &operator=(oci_auto_handle &rhs) throw() {
		reset(rhs.release());
		return *this;
	}
	handle_ptr_type release() {
		handle_ptr_type p(fHandle);
		fHandle = 0;
		return p;
	}
	void reset(handle_ptr_type ptr = 0) throw() {
		if ( fHandle != ptr) {
			doDelete();
			fHandle = ptr;
		}
	}
	void doDelete() throw() {
		if ( fHandle ) {
			OCIHandleFree(reinterpret_cast<dvoid *>(release()), handleType);
		}
	}
	handle_ptr_type getHandle() const {
		return fHandle;
	}

	// the following operators are only used when creating a new handle
	handle_ptr_addr_type getHandleAddr() {
		doDelete();
		return &fHandle;
	}
	dvoid **getVoidAddr() {
		doDelete();
		return (dvoid **)&fHandle;
	}
	operator const bool() const {
		return fHandle != 0;
	}
private:
	oci_auto_handle(oci_auto_handle &rhs);
};

typedef oci_auto_handle<OCIDescribe, OCI_HTYPE_DESCRIBE> DscHandleType;
typedef oci_auto_handle<OCIStmt, OCI_HTYPE_STMT> StmtHandleType;
typedef oci_auto_handle<OCIEnv, OCI_HTYPE_ENV> EnvHandleType; // OCI environment handle
typedef oci_auto_handle<OCIError, OCI_HTYPE_ERROR> ErrHandleType; // OCI error handle
typedef oci_auto_handle<OCIServer, OCI_HTYPE_SERVER> SrvHandleType; // OCI server connection handle	(at most one
typedef oci_auto_handle<OCISvcCtx, OCI_HTYPE_SVCCTX> SvcHandleType; // OCI service context handle
typedef oci_auto_handle<OCISession, OCI_HTYPE_SESSION> UsrHandleType; // OCI user session handle

#endif /* OCIAUTOHANDLE_H_ */
