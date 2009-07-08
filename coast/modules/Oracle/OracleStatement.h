/*
 * OracleStatement.h
 *
 *  Created on: Jul 8, 2009
 *      Author: m1huber
 */

#ifndef ORACLESTATEMENT_H_
#define ORACLESTATEMENT_H_

#include "config_coastoracle.h"
#include "OracleConnection.h"

class EXPORTDECL_COASTORACLE OracleStatement
{
	OracleStatement();
	OracleConnection fpConnection;
	StmtHandleType fStmthp;
	Anything fErrorMessages;
	bool AllocHandle();

public:
	OracleStatement(OracleConnection *pConn, String const &strStmt);
	virtual ~OracleStatement();

	bool Prepare();
	sword GetReplyDescription();
	sword Execute();
	void Cleanup();

	const Anything &GetErrorMessages() const {
		return fErrorMessages;
	}
};

#endif /* ORACLESTATEMENT_H_ */
