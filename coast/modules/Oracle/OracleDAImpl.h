/*
 * OracleDAImpl : DataAccessImpl using the OCI version Oracle8
 */

#ifndef _ORACLEDAIMPL_H
#define _ORACLEDAIMPL_H

//--- modules used in the interface
#include "config_coastoracle.h"
#include "DataAccessImpl.h"

#include "oci.h"

class OracleConnection;
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
	void FetchRowData( Anything &descs, ParameterMapper *pmapIn, ResultMapper *pmapOut, OracleStatement &aStmt, Context &ctx );
	void GetRecordData( Anything &descs, Anything &record, bool bTitlesOnce );

	// original sample methods
	//	bool GetOutputDescription(Anything &desc, ResultMapper *pmapOut, ub2 &fncode, OracleConnection *pConnection, Context &ctx);
	//	bool DefineOutputArea(Anything &desc, ResultMapper *pmapOut, OracleConnection *pConnection, Context &ctx);

	bool GetSPDescription( String const &spname, bool &pIsFunction, Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut,
						   OracleConnection *pConnection, Context &ctx );
	bool BindSPVariables( Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut, OracleStatement &aStmt, Context &ctx );

private:

	bool DoPrepareSQL( String &command, Context &ctx, ParameterMapper *in );
	bool DoPrepareSP( String &command, Context &ctx, ParameterMapper *in );

	String ConstructSPStr( String &command, bool pIsFunction, Anything const &desc );

	void Warning( Context &ctx, ResultMapper *pResultMapper, String str );
	void Error( Context &ctx, ResultMapper *pResultMapper, String str );

	OracleDAImpl();
	OracleDAImpl( const OracleDAImpl & );

	//assignement
	OracleDAImpl &operator=( const OracleDAImpl & );
};

#endif
