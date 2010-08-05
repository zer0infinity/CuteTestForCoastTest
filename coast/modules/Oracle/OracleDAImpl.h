/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ORACLEDAIMPL_H
#define _ORACLEDAIMPL_H

//--- modules used in the interface
#include "config_coastoracle.h"
#include "DataAccessImpl.h"
#include "OracleStatement.h"

class OracleResultset;
class OraclePooledConnection;

//---- OracleDAImpl -----------------------------------------------------------
//! Oracle DataAccessImpl using OCI
/*!
 * Oracle specific DataAccessImpl to execute database access based on settings in the Context and configuration
 * for OracleDAImpl, ParameterMapper and ResultMapper.
 * @section oracleparammapper ParameterMapper keys
 *
 * @subsection oracleparameterbasicconnsettings Basic connection settings
 *
 * @par \c DBConnectString
 * \b mandatory \n
 * String value specifying oracle database connection string according to <a href="http://www.oracle.com/technology/tech/oci/instantclient/ic-faq.html#A4428">Oracle-InstantClient FAQ</a>
 * @par \c DBUser
 * \b mandatory \n
 * String value specifying the database user to connect with
 * @par \c DBPW
 * \b mandatory \n
 * String value containing the password for the given user
 * @par \c DBTries
 * optional, default 3\n
 * How many times to retry if a database request fails due to recoverable errors
 *
 * @subsection oracleparametersimplequeries Simple sql queries
 *
 * @par \c SQL
 * \b mandatory if it is one of [select|insert|update|delete|create|drop|alter|...], but not a stored procedure/function or another form of a PL/SQL block\n
 * String value representing the SQL query to execute in valid oracle syntax.
 *
 * @subsection oracleparameterprocedurefunction Stored procedure/function
 *
 * Any OracleDAImpl used to execute server side stored procedures should be used in conjunction with OracleParameterMapper.
 * If not used together, parameters might not get looked up correctly.
 * @par \c Name
 * \b mandatory if it is a stored procedure/function\n
 * String value specifying the name of the stored procedure or function\n
 * <em>Other forms of a PL/SQL blocks are not supported!</em>
 *
 * @par \c Return
 * optional, default is the name of the stored function\n
 * String value specifying the name of the output slot to use when putting a stored function return value
 *
 * @par \c Params \c . \c PARAMNAME
 * \b mandatory if IN or INOUT parameters, except SYS_REFCURSOR INOUT values\n
 * Values to use as input parameters for stored procedures/functions\n
 * <em>Note that parameter names \b must be specified in upper case letters, oracle is not case preserving/sensitive for parameter names!</em>
 *
 * @subsection oracleparameteroutput Customizing output
 *
 * @par \c ShowQueryCount
 * optional, default 1\n
 * Long value (as boolean) to enable/disable \c QueryCount slot in result output
 * @par \c ShowUpdateCount
 * optional, default 0\n
 * Long value (as boolean) to enable/disable \c UpdateCount slot in result output
 * @par \c DBResultFormat
 * optional, [TitlesAlways|TitlesOnce]\n
 * Specifies how columns will show up in output.
 *  - TitlesAlways, default\n
 *  	Will create a named slot with its value for every column of a result row. This might be a performance killer
 *  	because every slotname inside the result Anything has to be allocated in memory and is therefore time consuming
 *  	and will fill your memory unnecessarily. For high performance applications it is best disabled.
 *  - TitlesOnce\n
 *		Will create a \c QueryTitles output slot containing a dictionary of column names and their index to access the
 *		column value in a \c QueryResult row using this index
 *
 * @subsection oracleparameterperformance Optimizing performance
 *
 * @par \c StringBufferSize
 * optional, default 4096\n
 * Long value used to determine String buffer size to reserve for CHAR/VARCHAR/STRING OUT/INOUT columns
 * @par \c PrefetchRows
 * optional, default 10\n
 * Long value, how many rows to fetch in a OCI server round trip
 *
 * @section oracleresultmapper ResultMapper keys
 *
 * @subsection oracleresultcommonoutput Common output
 *
 * @par \c QuerySource
 * This slot will contain the connect string of the back end where the query was executed
 * @par \c Query
 * The executed query string, regardless whether it is a simple query or a stored procedure or function call will be non pretty printed as String
 * @par \c Messages
 * Errors occurring during execution will be appended as textual messages
 *
 * @subsection oracleresultsimplequeries Simple SQL queries
 *
 * @par \c QueryCount
 * enabled by default, set \c ShowQueryCount to 0 in ParameterMapper configuration to disable output\n
 * Count of the actual number of rows retrieved by the query.\n
 * @par \c QueryResult
 * Result slot containing all retrieved rows and their columns
 * @par \c QueryTitles
 * Only present if \c DBResultFormat was set to \c TitlesOnce
 *  - ColName\n
 *		Column name as used by oracle, in upper case letters
 *  - colidx\n
 *		Column index of the column with name ColName to access its value in the result row like \c QueryResult:rowidx:colidx
 *
 * @par \c UpdateCount
 * disabled by default, set \c ShowUpdateCount to 1 in ParameterMapper configuration to enable output\n
 * Long value, if a non select query was issued, it represents what the database tells us about updates that were made.\n
 *
 * @subsection oracleresultprocedurefuntcion Stored procedure/function
 *
 * @par \c PARAMNAME
 * Every stored procedure/function parameter will be put using its PARAMNAME. If it is a simple value, the value appears as
 * value of the PARAMNAME. Otherwise, the put structure of \c QueryCount, \c QueryResult and \c QueryTitles will be put using
 * PARAMNAME as prefix to be able to distinguish between different result sets of different parameters.
 *  - value\n
 *  	If the parameter is a simple value parameter
 *  - {...}\n
 *  	If the parameter is a resultset [CURSOR|SYS_REFCURSOR]
 */
class EXPORTDECL_COASTORACLE OracleDAImpl: public DataAccessImpl
{
public:
	/*! Initialize base class using the name given to register this DataAccessImpl in the Registry
	 *
	 * @param name Name to register this DataAccessImpl
	 */
	OracleDAImpl( const char *name );
	~OracleDAImpl();

	/*! Implement Clone interface and return a copy of this OracleDAImpl
	 *
	 * @return A copy of this OracleDAImpl
	 */
	IFAObject *Clone() const;

	/*! protocol provided to implement data access
	 * \param ctx The context in which the transaction takes place
	 * \param in A ParameterMapper object that is mapping data from the client space (Context) to the data access object on request
	 * \param out A ResultMapper object that maps the result into client space (Context)
	 */
	virtual bool Exec( Context &ctx, ParameterMapper *in, ResultMapper *out );

private:
	bool DoPrepareSQL( String &command, Context &ctx, ParameterMapper *in );
	bool DoPrepareSP( String &command, Context &ctx, ParameterMapper *in );

	void ProcessResultSet( OracleResultset &aRSet, ParameterMapper *& in, Context &ctx, ResultMapper *& out,
						   String strResultPrefix );

	void Error( Context &ctx, ResultMapper *pResultMapper, String str );

	Anything getMappedInputValues( ParameterMapper *pmapIn, OracleStatement &aStmt, Context &ctx, bool &bIsArrayExecute );

	bool TryExecuteQuery( ParameterMapper *in, Context &ctx, OraclePooledConnection *& pPooledConnection,
						  String &server, String &user, String &passwd, ResultMapper *out, bool bRet );

	// compiler supplied functions forbidden due to Clone() interface
	OracleDAImpl();
	OracleDAImpl( const OracleDAImpl & );
	OracleDAImpl &operator =( const OracleDAImpl & );
};

#endif
