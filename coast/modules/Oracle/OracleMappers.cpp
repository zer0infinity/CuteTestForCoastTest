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
	StatTrace(OracleParameterMapper.Ctor, name, Storage::Current());
}

IFAObject *OracleParameterMapper::Clone() const
{
	StatTrace(OracleParameterMapper.Clone, fName, Storage::Current());
	return new OracleParameterMapper( fName );
}

bool OracleParameterMapper::DoGetAny( const char *key, Anything &value, Context &ctx, ROAnything script )
{
	StartTrace1(OracleParameterMapper.DoGetAny, NotNull(key) );

	String strKey( key );
	//!@FIXME Lookup("Delim,...) should be extracted into ResultMapper function, -> ResultMapper::getDelim()
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
	StatTrace(OracleResultMapper.Ctor, name, Storage::Current());
}

IFAObject *OracleResultMapper::Clone() const
{
	StatTrace(OracleResultMapper.Clone, fName, Storage::Current());
	return new OracleResultMapper( fName );
}

bool OracleResultMapper::DoPutAny( const char *key, Anything value, Context &ctx, ROAnything script )
{
	StartTrace1(OracleResultMapper.DoPutAny, NotNull(key));
	String strKey( key );
	//!@FIXME Lookup("Delim,...) should be extracted into ResultMapper function, -> ResultMapper::getDelim()
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

void OracleResultMapper::DoGetDestinationAny( const char *key, Anything &targetAny, Context &ctx )
{
	StartTrace1(OracleResultMapper.DoGetDestinationAny, NotNull(key));
	String path = GetDestinationSlot( ctx ), kPrefix( key ), strIdxValue = ctx.Lookup( "_OracleArrayResultIndex_", "" );
	String strArrayValuesSlot = Lookup("ArrayValuesSlotName", "ArrayResults");
	char cDelim = Lookup( "Delim", "." )[0L], cIndexDelim = Lookup( "IndexDelim", ":" )[0L];

	if ( strIdxValue.Length() ) {
		Trace("index value is:" << strIdxValue);
		path.Append( cDelim ).Append( strArrayValuesSlot );
		path.Append( cIndexDelim ).Append( strIdxValue );
	}
	if ( path.Length() > 0 && kPrefix.Length() ) {
		path.Append( cDelim );
	}
	path.Append( kPrefix );
	Trace("Path for slotfinder: [" << path << "]");
	SlotFinder::Operate( ctx.GetTmpStore(), targetAny, path, cDelim, cIndexDelim );
	TraceAny(targetAny, "current value at target");
}
