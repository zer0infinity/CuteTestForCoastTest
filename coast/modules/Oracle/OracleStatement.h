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
#include "IFAObject.h"
#include <memory>

class OracleResultset;
class OracleStatement;

//! type definition of auto cleanup'd OracleStatement
typedef std::auto_ptr<OracleStatement> OracleStatementPtr;

//! @brief <b>Abstraction of an Oracle statement</b>
/*!
 * A statement is either a simple SQL query or a stored procedure call. The OracleStatement class is the main
 * interface when wanting to execute queries/SP or when to retrieve result data. Several access methods exist to
 * obtain either simple column/parameter values or a complete OracleResultset object to process row results. Internally
 * the state of the statement is tracked using an enumerator OracleStatement::Status. The primary state just after
 * creating a new OracleStatement is OracleStatement::UNPREPARED. After executing Prepare(), the state changes to
 * OracleStatement::PREPARED to signal that the underlying OCI call was successful and we know the type of statement
 * which is going to be executed.
 * If the statement type is OracleStatement::STMT_BEGIN, we have to bind the input parameters first before continuing
 * to execute() the statement.
 * If execution of the statement was successful, the state either changes to OracleStatement::RESULT_SET_AVAILABLE or
 * to OracleStatement::UPDATE_COUNT_AVAILABLE also depending on the type of the statement.
 *
 * @section DescribeSimpleSQL Simple Statements
 * OracleStatment::RESULT_SET_AVAILABLE tells us that we can call getResultset() to get a valid OracleResultset object
 * and work on it to fetch all rows of the query.
 *
 * @section DescribePLSQL PL/SQL stored procedure/function
 * Blabla
 *
 * @par Configuration
 * --
 */
class EXPORTDECL_COASTORACLE OracleStatement : public IFAObject
{
public:
	enum Status {
		UNPREPARED,
		PREPARED,
		RESULT_SET_AVAILABLE,
		UPDATE_COUNT_AVAILABLE,
		NEEDS_STREAM_DATA,
		STREAM_DATA_AVAILABLE
	};

	enum StmtType {
		//! Unknown statement
		STMT_UNKNOWN = OCI_STMT_UNKNOWN,
		//! select statement
		STMT_SELECT = OCI_STMT_SELECT,
		//! update statement
		STMT_UPDATE = OCI_STMT_UPDATE,
		//! delete statement
		STMT_DELETE = OCI_STMT_DELETE,
		//! Insert Statement
		STMT_INSERT = OCI_STMT_INSERT,
		//! create statement
		STMT_CREATE = OCI_STMT_CREATE,
		//! drop statement
		STMT_DROP = OCI_STMT_DROP,
		//! alter statement
		STMT_ALTER = OCI_STMT_ALTER,
		//! begin ... (pl/sql statement)
		STMT_BEGIN = OCI_STMT_BEGIN,
		//! declare .. (pl/sql statement )
		STMT_DECLARE = OCI_STMT_DECLARE,
		//! corresponds to kpu call
		STMT_CALL = OCI_STMT_CALL,
	};

	enum BindType {
		INTERNAL,
		CURSOR,
		STRING
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

	Anything &GetOutputArea();
	sword bindColumn( long lBindPos, Anything &col, long len, bool bIsNull = false );
public:
	OracleStatement( OracleConnection *pConn, String const &strStmt );
	virtual ~OracleStatement();

	bool Prepare();
	Status execute( ub4 mode );
	sword Fetch( ub4 numRows = 1 );

	Status status() const {
		return fStatus;
	}
	StmtType getStatementType() const {
		return fStmtType;
	}
	void setPrefetchRows( long lPrefetchRows );
	unsigned long getUpdateCount() const;

	OracleResultset *getResultset();
	OracleResultset *getCursor( long lColumnIndex );
	String getString( long lColumnIndex );

	void registerOutParam( long lBindPos, BindType bindType = INTERNAL, long lBufferSize = -1);
	void setString( long lBindPos, String const &strValue );

	OCIStmt *getHandle() const {
		return fStmthp.getHandle();
	}
	OracleConnection *getConnection() const {
		return fpConnection;
	}

	ROAnything GetOutputDescription();
	bool DefineOutputArea();
	void setSPDescription(ROAnything roaSPDescription);

	const Anything &GetErrorMessages() const {
		return fErrorMessages;
	}
	String GetLastErrorMessage() const {
		return ROAnything( fErrorMessages )[fErrorMessages.GetSize() - 1L].AsString( "" );
	}

protected:
	virtual IFAObject *Clone() const {
		return NULL;
	}
};

#endif /* ORACLESTATEMENT_H_ */
