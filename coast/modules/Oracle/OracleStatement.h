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
#include "Anything.h"

class EXPORTDECL_COASTORACLE OracleStatement
{
	OracleStatement();
	OracleConnection *fpConnection;
	String fStmt;
	StmtHandleType fStmthp;
	Anything fErrorMessages;
	bool AllocHandle();
	void Cleanup();

public:
	OracleStatement( OracleConnection *pConn, String const &strStmt );
	virtual ~OracleStatement();

	bool Prepare();
	sword GetReplyDescription();
	sword Execute( ub4 mode );
	sword Fetch( ub4 numRows = 1 );

	OCIStmt *getHandle() const {
		return fStmthp.getHandle();
	}
	OracleConnection *getConnection() const {
		return fpConnection;
	}
	bool GetOutputDescription( Anything &desc, ub2 &fncode );
	bool DefineOutputArea( Anything &desc );

	const Anything &GetErrorMessages() const {
		return fErrorMessages;
	}
	String GetLastErrorMessage() const {
		return ROAnything( fErrorMessages )[fErrorMessages.GetSize() - 1L].AsString( "" );
	}
};

#endif /* ORACLESTATEMENT_H_ */
