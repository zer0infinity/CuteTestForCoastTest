/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleMappers.h"
#include "Context.h"
#include "Tracer.h"

//---- OracleParameterMapper ------------------------------------------------------------------
RegisterParameterMapper(OracleParameterMapper);

OracleParameterMapper::OracleParameterMapper( const char *name ) :
	ParameterMapper( name )
{
	StatTrace(OracleParameterMapper.Ctor, name, Coast::Storage::Current());
}

IFAObject *OracleParameterMapper::Clone(Allocator *a) const
{
	StatTrace(OracleParameterMapper.Clone, fName, Coast::Storage::Current());
	return new (a) OracleParameterMapper( fName );
}

bool OracleParameterMapper::DoGetAny( const char *key, Anything &value, Context &ctx, ROAnything script )
{
	StartTrace1(OracleParameterMapper.DoGetAny, NotNull(key) );
	String strKey( key );
	String strParamPrefix( "Params" );
	strParamPrefix.Append( getDelim() );
	if ( strKey.StartsWith( strParamPrefix ) ) {
		strKey.Replace( strParamPrefix, "" );
	}
	return ParameterMapper::DoGetAny( strKey, value, ctx, script );
}

//---- OracleResultMapper ------------------------------------------------------------------
RegisterResultMapper(OracleResultMapper);

OracleResultMapper::OracleResultMapper( const char *name ) :
	ResultMapper( name )
{
	StatTrace(OracleResultMapper.Ctor, name, Coast::Storage::Current());
}

IFAObject *OracleResultMapper::Clone(Allocator *a) const
{
	StatTrace(OracleResultMapper.Clone, fName, Coast::Storage::Current());
	return new (a) OracleResultMapper( fName );
}

bool OracleResultMapper::DoPutAny( const char *key, Anything &value, Context &ctx, ROAnything script )
{
	StartTrace1(OracleResultMapper.DoPutAny, NotNull(key));
	String strKey( key );
	long lDelimIdx( strKey.StrRChr( getDelim() ) );
	if ( lDelimIdx != -1 ) {
		Trace("combined key")
		// check within script/LookupInterface if combined key exists
		ROAnything roaScript( SelectScript( key, script, ctx ) );
		if ( roaScript.IsNull() ) {
			strKey.TrimFront( lDelimIdx + 1 );
			Trace("key reduced to [" << strKey << "]")
			roaScript = SelectScript( strKey, script, ctx );
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
	if ( strIdxValue.Length() ) {
		Trace("index value is:" << strIdxValue);
		path.Append( getDelim() ).Append( strArrayValuesSlot );
		path.Append( getIndexDelim() ).Append( strIdxValue );
	}
	if ( path.Length() > 0 && kPrefix.Length() ) {
		path.Append( getDelim() );
	}
	path.Append( kPrefix );
	Trace("Path for slotfinder: [" << path << "]");
	SlotFinder::Operate( ctx.GetTmpStore(), targetAny, path, getDelim(), getIndexDelim() );
	TraceAny(targetAny, "current value at target");
}
