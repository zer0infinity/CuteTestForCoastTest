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

class OracleConnection;
class OracleResultset;
class OracleStatement;

//---- OracleDAImpl -----------------------------------------------------------
//! <b>Oracle DataAccessImpl using OCI</b>
/*!
 * @par Description
 * Oracle specific DataAccessImpl to execute database access based on settings in the Context and configuration
 * for OracleDAImpl, ParameterMapper and ResultMapper.
 * @par Configuration
<b>ParameterMapper Slots used:</b>
\code
{
# basic connection settings

	/DBConnectString	MapperSpec		mandatory, String value specifying oracle database connection string
											according to hints in instant client ic-faq.html#A4428
	/DBUser				MapperSpec		mandatory, String value specifying the database user to connect with
	/DBPW				MapperSpec		mandatory, String value containing the password for the above user
	/DBTries			MapperSpec		optional, default 3, how many times to retry if a database request fails
											due to recoverable errors

# simple sql queries section

	/SQL				MapperSpec		mandatory if it is one of [select|insert|update|delete|create|drop|alter|...],
											but not a stored procedure/function or another form of a PL/SQL block

# stored procedure/function section, best used with OracleParameterMapper

	/Name				MapperSpec		mandatory if it is a stored procedure/function, resulting in a simple String
											specifying the name of the stored procedure or function
										-> other forms of a PL/SQL blocks are not supported!
	/Return				MapperSpec		optional, String value specifying the name of the output slot to use when putting
											a stored function return value, default is the name of the stored function
	/Params {
		/PARAMNAME		MapperSpec		mandatory if IN or INOUT parameters, values to use as input parameters for stored
											procedures/functions, REFCURSOR INOUT values do not need to be specified
										-> note that parameter names *must* be specified in upper case letters, oracle is
										not case preserving/sensitive for parameter names
		...
	}

# customizing output

	/ShowQueryCount		MapperSpec		optional, long value (as boolean) to enable/disable QueryCount slot in result output
											default is enabled
	/ShowUpdateCount	MapperSpec		optional, long value (as boolean) to enable/disable UpdateCount slot in result output
											default is disabled
	/DBResultFormat		MapperSpec		optional, [*TitlesAlways*|TitlesOnce], specifies how columns will show up in output
										TitlesAlways:	will create a named slot with its value for every column of a result row
										TitlesOnce:		will create a QueryTitles output slot containing a dictionary of
														column names and their index to access the column value in a
														QueryResult row using this index

# optimizing performance

	/StringBufferSize	MapperSpec		optional, long value, default 4096, used to determine String buffer size to reserve
											for CHAR/VARCHAR/STRING OUT/INOUT columns
	/PrefetchRows		MapperSpec		optional, long value, default 10, how many rows to fetch in a OCI server round trip
}
\endcode
<b>ResultMapper Slots used:</b>
\code
{
# common output section

	/QuerySource						This slot will contain the connect string of the back end where the query was executed
	/Query								The executed query string, regardless whether it is a simple query or a stored procedure
											or function call will be non pretty printed as String
	/Messages {							errors occurring during execution will be appended as textual messages
		"some message"
		...
	}

# simple sql queries section

	/QueryCount							Count of the actual number of rows retrieved by the query
	/QueryResult {						Result slot containing all retrieved rows and their columns
		{ results of first row }
		{ ... }
	}
	/QueryTitles {						Optional, only present if DBResultFormat was set to TitlesOnce
		/ColName	colidx				The result value of column with name ColName is stored at QueryResult:rowidx:colidx
		...
	}

	/UpdateCount						Optional, only present if a non select query was issued, it represents what the
											database tells us about the updates that were made

# stored procedure/function section

	/PARAMNAME		value				Every stored procedure/function parameter will get put using its PARAMNAME. If
											it is a simple value, it will show up as shown on the left
	/PARAMNAME {						If the parameter is a resultset (CURSOR, SYS_REFCURSOR), the output is almost the
											same as if we issued a select query. Its results will be put below the
											corresponding PARAMNAME to be able to distinguish between different parameter
											resultsets.
		/QueryCount						see above for description
		/QueryResult					see above for description
		/QueryTitles					see above for description
	}
}
\endcode
 */
class EXPORTDECL_COASTORACLE OracleDAImpl: public DataAccessImpl
{
public:
	/*! Initialize base class using the name given to register this DataAccessImpl in the Registry
	 *
	 * @param name Name of this DataAccessImpl
	 */
	OracleDAImpl( const char *name );
	//! Do nothing dtor
	~OracleDAImpl();

	/*! Implement Clone interface and return a copy of this OracleDAImpl
	 *
	 * @return A copy of this OracleDAImpl
	 */
	IFAObject *Clone() const;

	/*! protocol provided to implement data access
	 * \param ctx The context in with the transaction takes place
	 * \param in A mapper object that is mapping data from the client space (Context) to the data access object on request
	 * \param out a mapper object that maps the result of the access back into client space (Context)
	*/
	virtual bool Exec( Context &ctx, ParameterMapper *in, ResultMapper *out );

private:
	bool ConnectOracleUser( const char *name, const char *pw );

	bool GetSPDescription( String &spname, Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut,
						   OracleConnection *pConnection, Context &ctx );
	bool BindSPVariables( Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut, OracleStatement &aStmt,
						  Context &ctx );

	bool DoPrepareSQL( String &command, Context &ctx, ParameterMapper *in );
	bool DoPrepareSP( String &command, Context &ctx, ParameterMapper *in );

	String ConstructSPStr( String const &command, bool pIsFunction, Anything const &desc );
	void ProcessResultSet( OracleResultset &aRSet, ParameterMapper *& in, Context &ctx, ResultMapper *& out,
						   String strResultPrefix );

	void Error( Context &ctx, ResultMapper *pResultMapper, String str );

	//! default ctor
	OracleDAImpl();
	//! copy ctor
	OracleDAImpl( const OracleDAImpl & );

	//! assignment operator
	OracleDAImpl &operator=( const OracleDAImpl & );
};

#endif
