/*
 * OracleStatement.cpp
 *
 *  Created on: Jul 8, 2009
 *      Author: m1huber
 */

#include "OracleStatement.h"

OracleStatement::OracleStatement()
{
	// TODO Auto-generated constructor stub
}

OracleStatement::~OracleStatement()
{
	Cleanup();
}

bool OracleStatement::AllocHandle()
{
	StartTrace(OracleStatement.AllocHandle);
	// allocates and returns new statement handle
	String strError(32L);
	bool bSuccess(!fpConnection->checkError(OCIHandleAlloc(fpConnection->EnvHandle(), fStmthp.getVoidAddr(), (ub4) OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0),
											strError));
	if (!bSuccess) {
		fErrorMessages.Append(strError);
	}
	return bSuccess;
}

void OracleStatement::Cleanup()
{
	StartTrace(OracleStatement.Cleanup);
	fStmthp.reset();
}

sword OracleStatement::GetReplyDescription()
{
	StartTrace(OracleStatement.GetReplyDescription);
	// retrieves descriptions of return values for a given SQL statement
	return OCIStmtExecute(fpConnection->SvcHandle(), fStmthp.getHandle(), fpConnection->ErrorHandle(), (ub4) 1, (ub4) 0, (const OCISnapshot *) NULL,
						  (OCISnapshot *) NULL, OCI_DESCRIBE_ONLY);
}

sword OracleStatement::Execute()
{
	StartTrace(OracleStatement.Execute);
	// executes a SQL statement (first row is also fetched)
	return OCIStmtExecute(fpConnection->SvcHandle(), fStmthp.getHandle(), fpConnection->ErrorHandle(), (ub4) 1, (ub4) 0, (const OCISnapshot *) NULL,
						  (OCISnapshot *) NULL, OCI_COMMIT_ON_SUCCESS);
}

bool OracleStatement::Prepare()
{
	StartTrace(OracleStatement.Prepare);
	bool bRet(false);
	// prepare SQL statement for execution
	if (AllocHandle()) {
		return OCIStmtPrepare(fStmthp.getHandle(), fpConnection->ErrorHandle(), stmt, (ub4) strlen((const char *) stmt), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	}
	return bRet;
}
