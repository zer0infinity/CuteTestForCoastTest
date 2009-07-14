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

#include "oci.h"

class OracleConnection;
class OracleResultset;
class OracleStatement;

//---- OracleDAImpl -----------------------------------------------------------
// arbitrary input fields may be used to dynamically generate an SQL statment
// the following fields have a predefined semantic:
// {
// }

class EXPORTDECL_COASTORACLE OracleDAImpl: public DataAccessImpl
{
public:
	//--- constructors
	OracleDAImpl( const char *name );
	~OracleDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//: executes the transaction
	//!param: c - The context of the transaction
	virtual bool Exec( Context &ctx, ParameterMapper *in, ResultMapper *out );

protected:
	bool ConnectOracleUser( const char *name, const char *pw );

	bool GetSPDescription( String &spname, Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut,
						   OracleConnection *pConnection, Context &ctx );
	bool BindSPVariables( Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut, OracleStatement &aStmt, Context &ctx );

private:

	bool DoPrepareSQL( String &command, Context &ctx, ParameterMapper *in );
	bool DoPrepareSP( String &command, Context &ctx, ParameterMapper *in );

	String ConstructSPStr( String const &command, bool pIsFunction, Anything const &desc );
	void ProcessResultSet( OracleResultset &aRSet, ParameterMapper *& in, Context &ctx, ResultMapper *& out, String strResultPrefix );

	void Error( Context &ctx, ResultMapper *pResultMapper, String str );

	OracleDAImpl();
	OracleDAImpl( const OracleDAImpl & );

	//assignement
	OracleDAImpl &operator=( const OracleDAImpl & );
};

#endif
