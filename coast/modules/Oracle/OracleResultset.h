/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLERESULTSET_H_
#define ORACLERESULTSET_H_

//--- modules used in the interface
#include "config_coastoracle.h"
#include "Anything.h"

#include <memory>

class OracleStatement;

//---- OracleResultset -----------------------------------------------------------
//! <b>Abstraction for a set of results </b>
/*!
 * @par Description
 * This class - or a corresponding object of it - acts like a row iterator on the underlying statement. An instance
 * will be returned by calling OracleStatement::getResultset or OracleStatement::getCursor.
 * Processing of such a OracleResultset will currently be done using OracleDAImpl::ProcessResultSet because Mappers
 * are needed to store the columns of a row. It is possible that some common parts of result row processing will
 * move into this class.
 * @par Configuration
 * --
 */
class EXPORTDECL_COASTORACLE OracleResultset
{
public:
	/*! Status is used for internal state transition */
	enum Status {
		//! this is the initial state when next() was not called yet
		NOT_READY,
		//! ready signals that the output area is initialized
		READY,
		//! this state signals that no more rows can be read
		END_OF_FETCH,
		//! signals that data is available (fetched)
		DATA_AVAILABLE,
		//! same as DATA_AVAILABLE but for data streams (not implemented)
		STREAM_DATA_AVAILABLE
	};
private:
	OracleStatement &frStmt;
	Status fFetchStatus;

	bool DefineOutputArea();

	OracleResultset();
	OracleResultset( const OracleResultset & );
public:
	/*! Initializes this OracleResultset object using the given OracleStatement
	 * @param rStmt OracleStatement to use for result processing
	 */
	OracleResultset( OracleStatement &rStmt ) :
		frStmt( rStmt ), fFetchStatus( NOT_READY ) {
	}
	//! does nothing
	~OracleResultset();
	/*! Retrieve column layout of the current result set
	 * @return Read only copy of the column descriptions
	 */
	ROAnything GetOutputDescription();
	/*! Gain access to the underlying OracleStatement
	 * @return Pointer to the underlying OracleStatement
	 */
	OracleStatement *getStatement() const {
		return &frStmt;
	}
	/*! Move forward - fetch the next row if available
	 * @return OracleResultset::Status signaling what to do next
	 */
	Status next();
	/*! Get a column value as String
	 * @param lColumnIndex 1-based column index
	 * @return String representing the columns result
	 * @note lColumnIndex is 1-based, it is not like any normal C++ array index...!
	 */
	String getString( long lColumnIndex );
};

//! Convenience type for automated resource management of OracleResultset
typedef std::auto_ptr<OracleResultset> OracleResultsetPtr;

#endif /* ORACLERESULTSET_H_ */
