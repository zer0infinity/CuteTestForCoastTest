/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "CompareValuesMapper.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Renderer.h"
#include "SystemLog.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- CompareValuesMapper ------------------------------------------------------------------
RegisterResultMapper(CompareValuesMapper);

CompareValuesMapper::CompareValuesMapper(const char *name)
	: ResultMapper(name)
{
	StartTrace(CompareValuesMapper.Ctor);
}

IFAObject *CompareValuesMapper::Clone() const
{
	return new CompareValuesMapper(fName);
}

bool CompareValuesMapper::DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script)
{
	StartTrace1(CompareValuesMapper.DoPutAny, NotNull(key));
	ROAnything roaRightValue, roaLeftValue, roaOperator;
	if ( Lookup("RightValue", roaRightValue) && Lookup("LeftValue", roaLeftValue) && Lookup("Operator", roaOperator) ) {
		String strRightValue, strOperator, strLeftValue;
		TraceAny(roaLeftValue, "spec how to obtain left value to compare");
		TraceAny(roaRightValue, "spec how to obtain right value to compare");
		{
			Context::PushPopEntry<Anything> aEntry(ctx, "PutterValue", value);
			Renderer::RenderOnString(strLeftValue, ctx, roaLeftValue);
			Renderer::RenderOnString(strRightValue, ctx, roaRightValue);
		}
		Renderer::RenderOnString(strOperator, ctx, roaOperator);

		Trace("now testing for [" << strLeftValue << " " << strOperator << " " << strRightValue << "]");
		bool bPutResult = false;
		if ( strOperator.IsEqual("==") ) {
			bPutResult = (strLeftValue.AsLong(0L) == strRightValue.AsLong(0L));
		} else if ( strOperator.IsEqual("!=") ) {
			bPutResult = (strLeftValue.AsLong(0L) != strRightValue.AsLong(0L));
		} else if ( strOperator.IsEqual(">=") ) {	// must be before < because of possible partial match
			bPutResult = (strLeftValue.AsLong(0L) >= strRightValue.AsLong(0L));
		} else if ( strOperator.IsEqual(">") ) {
			bPutResult = (strLeftValue.AsLong(0L) > strRightValue.AsLong(0L));
		} else if ( strOperator.IsEqual("<=") ) {	// must be before < because of possible partial match
			bPutResult = (strLeftValue.AsLong(0L) <= strRightValue.AsLong(0L));
		} else if ( strOperator.IsEqual("<") ) {
			bPutResult = (strLeftValue.AsLong(0L) < strRightValue.AsLong(0L));
		}
		if ( bPutResult ) {
			Trace("comparison yields true, putting result");
			return ResultMapper::DoPutAny(key, value, ctx, script);
		}
		// result skipped
		return true;
	}
	SYSWARNING("not all required slots for mapper given");
	return false;
}
