/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLESTATEMENT_H_
#define ORACLESTATEMENT_H_

#include "config_coastoracle.h"
#include "OracleConnection.h"
#include "Anything.h"
#include <memory>

class OracleResultset;
class OracleStatement;

typedef std::auto_ptr<OracleStatement> OracleStatementPtr;

class EXPORTDECL_COASTORACLE OracleStatement : public IFAObject
{
public:
	enum Status {
		UNPREPARED, PREPARED, RESULT_SET_AVAILABLE, UPDATE_COUNT_AVAILABLE, NEEDS_STREAM_DATA, STREAM_DATA_AVAILABLE
	};

	enum StmtType {
		Coast_OCI_STMT_UNKNOWN = OCI_STMT_UNKNOWN, /* Unknown statement */
		Coast_OCI_STMT_SELECT = OCI_STMT_SELECT, /* select statement */
		Coast_OCI_STMT_UPDATE = OCI_STMT_UPDATE, /* update statement */
		Coast_OCI_STMT_DELETE = OCI_STMT_DELETE, /* delete statement */
		Coast_OCI_STMT_INSERT = OCI_STMT_INSERT, /* Insert Statement */
		Coast_OCI_STMT_CREATE = OCI_STMT_CREATE, /* create statement */
		Coast_OCI_STMT_DROP = OCI_STMT_DROP, /* drop statement */
		Coast_OCI_STMT_ALTER = OCI_STMT_ALTER, /* alter statement */
		Coast_OCI_STMT_BEGIN = OCI_STMT_BEGIN, /* begin ... (pl/sql statement)*/
		Coast_OCI_STMT_DECLARE = OCI_STMT_DECLARE, /* declare .. (pl/sql statement ) */
		Coast_OCI_STMT_CALL = OCI_STMT_CALL, /* corresponds to kpu call */
	};
private:
	OracleStatement();
	OracleConnection *fpConnection;
	String fStmt;
	StmtHandleType fStmthp;
	Anything fErrorMessages;
	Status fStatus;
	StmtType fStmtType;
	Anything fanyDescription;
	Anything fBuffer;
	Anything fSubStatements;
	bool AllocHandle();
	void Cleanup();

	OracleStatement( OracleConnection *pConn, OCIStmt *phStmt );

public:
	OracleStatement( OracleConnection *pConn, String const &strStmt );
	virtual ~OracleStatement();

	bool Prepare();
	Status execute( ub4 mode, ub4 iters = 0 );
	sword Fetch( ub4 numRows = 1 );

	Status status() const {
		return fStatus;
	}
	StmtType getStatementType() const {
		return fStmtType;
	}
	unsigned long getUpdateCount() const;

	OracleResultset *getResultset();
	OracleResultset *getCursor( long lColumnIndex );
	String getString( long lColumnIndex );

	OCIStmt *getHandle() const {
		return fStmthp.getHandle();
	}
	OracleConnection *getConnection() const {
		return fpConnection;
	}

	ROAnything GetOutputDescription();
	bool DefineOutputArea();

	const Anything &GetErrorMessages() const {
		return fErrorMessages;
	}
	String GetLastErrorMessage() const {
		return ROAnything( fErrorMessages )[fErrorMessages.GetSize() - 1L].AsString( "" );
	}

	virtual IFAObject *Clone() const {
		return NULL;
	}
};

#endif /* ORACLESTATEMENT_H_ */
