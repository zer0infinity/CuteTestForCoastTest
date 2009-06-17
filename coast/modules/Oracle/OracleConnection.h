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
		return fStmthp;
	}
	OCIError *ErrorHandle() {
		return fErrhp;
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
	OCIEnv *fEnvhp; // OCI environment handle
	OCIError *fErrhp; // OCI error handle
	OCIServer *fSrvhp; // OCI server connection handle	(at most one
	//     outstanding call at a time!)
	OCISvcCtx *fSvchp; // OCI service context handle
	OCISession *fUsrhp; // OCI user session handle
	OCIStmt *fStmthp;
};

#endif /* O8CONNECTION_H_ */
