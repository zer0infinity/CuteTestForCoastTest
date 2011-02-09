/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "NullParameterMapper.h"
//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "AnyIterators.h"

//---- NullParameterMapper ------------------------------------------------------------------
RegisterParameterMapper(NullParameterMapper);

NullParameterMapper::NullParameterMapper( const char *name ) :
	ParameterMapper( name )
{
	StatTrace(NullParameterMapper.Ctor, name, Coast::Storage::Current());
}

IFAObject *NullParameterMapper::Clone(Allocator *a) const
{
	StatTrace(NullParameterMapper.Clone, fName, Coast::Storage::Current());
	return new (a) NullParameterMapper( fName );
}

bool NullParameterMapper::DoGetAny( const char *key, Anything &value, Context &ctx, ROAnything script )
{
	StartTrace1(NullParameterMapper.DoGetAny, NotNull(key) );
	if ( ParameterMapper::DoGetAny( key, value, ctx, script) ) {
		Trace("value before replacement [" << (value.IsNull()?String("*"):value.AsString()) << "]");
		ROAnything roaTreatAsNullValues=Lookup("TreatAsNull");
		if ( !value.IsNull() && !roaTreatAsNullValues.IsNull() ) {
			ROAnything roaTestValue;
			AnyExtensions::Iterator<ROAnything> nullValIterator(roaTreatAsNullValues);
			while ( nullValIterator.Next(roaTestValue) ) {
				Trace("testing against listvalue [" << roaTestValue.AsString() << "]");
				if ( roaTestValue.IsEqual(value) ) {
					Trace("value matched in list [" << value.AsString() << "]");
					value = Anything(value.GetAllocator());
					break;
				}
			}
		}
		Trace("returning true");
		return true;
	}
	Trace("returning false");
	return false;
}

