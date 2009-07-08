#ifndef O8CONNECTION_H_
#define O8CONNECTION_H_

//--- modules used in the interface
#include "config_coastoracle.h"
#include "ITOString.h"
#include "IFAObject.h"

#include "OciAutoHandle.h"

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

	OCIError *ErrorHandle() {
		return fErrhp.getHandle();
	}
	OCIEnv *EnvHandle() {
		return fEnvhp.getHandle();
	}

	OCISvcCtx *SvcHandle() {
		return fSvchp.getHandle();
	}

	OCIDescribe *DscHandle() {
		return fDschp.getHandle();
	}

	String errorMessage(sword status);
	bool checkError(sword status, String &message);
	bool checkError(sword status);

protected:
	// returns nothing, object not cloneable
	IFAObject *Clone() const {
		return NULL;
	};

	bool fConnected;

	// --- oracle API
	EnvHandleType fEnvhp; // OCI environment handle
	ErrHandleType fErrhp; // OCI error handle
	SrvHandleType fSrvhp; // OCI server connection handle (at most one outstanding call at a time!)
	SvcHandleType fSvchp; // OCI service context handle
	UsrHandleType fUsrhp; // OCI user session handle
	DscHandleType fDschp;
};

#endif /* O8CONNECTION_H_ */
