/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//#define XML_MAPPER_TRACING // does a lookup with the generated lookup-path

//--- interface include --------------------------------------------------------
#include "XMLMapper.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- XMLMapper ------------------------------------------------------------------
RegisterOutputMapper(XMLMapper);

XMLMapper::XMLMapper(const char *name) : ResultMapper(name)
{
	StartTrace(XMLMapper.Ctor);
}

IFAObject *XMLMapper::Clone() const
{
	StartTrace(XMLMapper.Clone);
	return new XMLMapper(fName);
}

// ignores its config altogether, operates on the fConfig
bool XMLMapper::DoPutAny(const char *key, Anything value, Context &ctx, ROAnything)
{
	StartTrace1(XMLMapper.DoPutAny, NotNull(key));
	String pathSoFar;
	String slotName;
	Anything result;
	bool bFound = false;
	fDelim =      		fConfig["Delim"].AsCharPtr(".")[0L];
	fIndexDelim = 		fConfig["IndexDelim"].AsCharPtr(":")[0L];
	fIndexedPathOnly =	fConfig["IndexedPathOnly"].AsBool(0);
	if (! fConfig.IsDefined("Elements")  || key == "") {
		return false;
	}
	long slotIndex = 0;
	Iterate(value, pathSoFar, slotIndex, slotName, bFound, result);
	TraceAny(result, "Result");
#ifdef XML_MAPPER_TRACING
	for (long l = 0; l < result.GetSize(); l++) {
		Anything verify;
		value.LookupPath(verify, result[l].AsString(), fDelim, fIndexDelim);
		TraceAny(verify, "verify");
	}
#endif
	ctx.GetTmpStore()[fName]["LookupPathes"] = result;
	ctx.GetTmpStore()[fName]["ParsedXMLAsAny"] = value;
	return true;
}

bool XMLMapper::Iterate(Anything currentAny, String pathSoFar, long slotIndex, String slotName, bool bFound, Anything &result)
{
	StartTrace1(XMLMapper.Iterate, pathSoFar);
	if (fIndexedPathOnly || slotName.Length() == 0) {
		pathSoFar << fIndexDelim << slotIndex;
	} else {
		if (!fIndexedPathOnly) {
			pathSoFar << fDelim << slotName;
		}
	}
	for ( long l = 0; l < currentAny.GetSize(); l++ ) {
		Anything newAny = currentAny;
		if (newAny.GetType() == Anything::eArray) {
			slotName = newAny.SlotName(l);
			if (fConfig["Elements"].Contains(slotName)) {
				bFound = true;
			}
			Iterate(newAny[l], pathSoFar, l, slotName, bFound, result);
		} else {
			String outPath(pathSoFar);
			outPath = outPath.SubString(2);
			outPath  << fIndexDelim  << l;
			if ( bFound ) {
				result.Append(outPath);
			}
		}
	}
	return true;
}
