/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "OracleMappers.h"
//--- project modules used -----------------------------------------------------
//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
//--- c-modules used -----------------------------------------------------------

//---- OracleParameterMapper ------------------------------------------------------------------
RegisterParameterMapper(OracleParameterMapper);

OracleParameterMapper::OracleParameterMapper( const char *name ) :
	ParameterMapper( name )
{
	StartTrace(OracleParameterMapper.Ctor);
}

IFAObject *OracleParameterMapper::Clone() const
{
	return new OracleParameterMapper( fName );
}

bool OracleParameterMapper::DoGetAny( const char *key, Anything &value, Context &ctx, ROAnything script )
{
	StartTrace1(OracleParameterMapper.DoGetAny, NotNull(key) );

	String strKey( key );
	//FIXME: should be extracted into ResultMapper function
	char cDelim = Lookup( "Delim", "." )[0L];
	String strParamPrefix( "Params" );
	strParamPrefix.Append( cDelim );
	if ( strKey.StartsWith( strParamPrefix ) ) {
		strKey.Replace( strParamPrefix, "" );
		return ParameterMapper::DoGetAny( strKey, value, ctx, script );
	}
	return ParameterMapper::DoGetAny( key, value, ctx, script );
}
//---- OracleResultMapper ------------------------------------------------------------------
RegisterResultMapper(OracleResultMapper);

OracleResultMapper::OracleResultMapper( const char *name ) :
	ResultMapper( name )
{
	StartTrace(OracleResultMapper.Ctor);
}

IFAObject *OracleResultMapper::Clone() const
{
	return new OracleResultMapper( fName );
}

bool OracleResultMapper::DoPutAny( const char *key, Anything value, Context &ctx, ROAnything script )
{
	StartTrace1(OracleResultMapper.DoPutAny, NotNull(key));
	String strKey( key );
	//FIXME: should be extracted into ResultMapper function
	char cDelim = Lookup( "Delim", "." )[0L];
	long lDelimIdx( strKey.StrRChr( cDelim ) );
	if ( lDelimIdx != -1 ) {
		Trace("combined key")
		// check within script/LookupInterface if combined key exists
		ROAnything roaScript( DoSelectScript( key, script, ctx ) );
		if ( roaScript.IsNull() ) {
			strKey.TrimFront( lDelimIdx + 1 );
			Trace("key reduced to [" << strKey << "]")
			roaScript = DoSelectScript( strKey, script, ctx );
		}
		return ResultMapper::DoPutAny( key, value, ctx, roaScript );
	}
	return ResultMapper::DoPutAny( key, value, ctx, script );
}
