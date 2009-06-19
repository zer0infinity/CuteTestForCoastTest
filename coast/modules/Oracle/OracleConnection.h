#ifndef O8CONNECTION_H_
#define O8CONNECTION_H_

//--- modules used in the interface
#include "config_coastoracle.h"
#include "ITOString.h"
#include "IFAObject.h"

#include "oci.h"

//---- OracleConnection -----------------------------------------------------------
// connection to oracle db ... not thread safe... may not be used concurrently

class EXPORTDECL_COASTORACLE OracleConnection: public IFAObject
{

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

public:
	OracleConnection();
	~OracleConnection();

	bool Open(String const &strServer, String const &strUsername, String const &strPassword);
	bool Close(bool bForce = false);

	bool SuccessfullyConnected() {
		return fConnected;
	}

	// request execution
	sword StmtPrepare(text *stmt);
	sword GetReplyDescription();
	sword ExecuteStmt();
	void StmtCleanup();

	OCIStmt *StmtHandle() {
		return fStmthp.getHandle();
	}
	OCIError *ErrorHandle() {
		return fErrhp.getHandle();
	}

	String checkerr(OCIError *errhp, sword status, bool &estatus);

protected:
	sword AllocStmtHandle();

	// returns nothing, object not cloneable
	IFAObject *Clone() const {
		return NULL;
	};

	bool fConnected;

	// --- oracle API
	oci_auto_handle<OCIEnv, OCI_HTYPE_ENV> fEnvhp; // OCI environment handle
	oci_auto_handle<OCIError, OCI_HTYPE_ERROR> fErrhp; // OCI error handle
	oci_auto_handle<OCIServer, OCI_HTYPE_SERVER> fSrvhp; // OCI server connection handle	(at most one
	//     outstanding call at a time!)
	oci_auto_handle<OCISvcCtx, OCI_HTYPE_SVCCTX> fSvchp; // OCI service context handle
	// OCI user session handle
	oci_auto_handle<OCISession, OCI_HTYPE_SESSION> fUsrhp;
	oci_auto_handle<OCIStmt, OCI_HTYPE_STMT> fStmthp;
};

#endif /* O8CONNECTION_H_ */
