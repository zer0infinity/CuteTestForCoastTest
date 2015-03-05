/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DecimalFormatRenderer.h"
#include "AnythingUtils.h"
#include "SystemLog.h"
#include "Tracer.h"

//---- DecimalFormatRenderer ---------------------------------------------------------------
RegisterRenderer(DecimalFormatRenderer);

DecimalFormatRenderer::DecimalFormatRenderer(const char *name) : Renderer(name) { }

DecimalFormatRenderer::~DecimalFormatRenderer() { }

void DecimalFormatRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(DecimalFormatRenderer.RenderAll);

	TraceAny(config, "config");
	String sString, sScale, sDecSeparator, sMaxLength;

	if (!ReadConfig( ctx, config, sString, sScale, sDecSeparator )) {
		return;
	}

	if (sString.Length()) {
		if (!FormatNumber( sString, sScale, sDecSeparator )) {
			return;
		}
	}

	Trace("string to return [" << sString << "]");
	reply << sString;
}

bool DecimalFormatRenderer::ReadConfig( Context &ctx, const ROAnything &config, String &sString, String &sScale, String &sDecSeparator )
{
	StartTrace(DecimalFormatRenderer.ReadConfig);

	ROAnything roaSlotConfig;
	if (config.LookupPath(roaSlotConfig, "String")) {
		RenderOnString(sString, ctx, roaSlotConfig);
	} else {
		SystemLog::Error("DecimalFormatRenderer::RenderAll: String not defined");
		return false;
	}
	Trace("String: [" << sString << "]");

	if (config.LookupPath(roaSlotConfig, "Scale")) {
		RenderOnString(sScale, ctx, roaSlotConfig);
	} else {
		SystemLog::Error("DecimalFormatRenderer::RenderAll: Scale not defined");
		return false;
	}
	Trace("Scale: [" << sScale << "]");

	if (config.LookupPath(roaSlotConfig, "DecimalSeparator")) {
		RenderOnString(sDecSeparator, ctx, roaSlotConfig);
	} else {
		sDecSeparator = ",";
	}
	Trace("DecimalSeparator: [" << sDecSeparator << "]");

	return true;

}

bool DecimalFormatRenderer::FormatNumber( String &sString, String &sScale, String &sDecSeparator )
{
	StartTrace(DecimalFormatRenderer.FormatNumber);

	String strTmp(sString);
	String strToken, strNumber, strDecPlaces;
	StringTokenizer tok(strTmp, sDecSeparator[0L]);

	if ( tok(strToken) ) {

		Trace("current token [" << strToken << "]");
		strNumber = strToken;
	} else {
		strNumber = "0";
	}

	if ( tok(strToken) ) {
		Trace("current token [" << strToken << "]");
		strDecPlaces = strToken;
	} else {
		strDecPlaces = "";
	}

	if (sScale.AsLong(0L) > 0L) {
		InsertFiller( sScale, strDecPlaces );
		Trace("decimal [" << strDecPlaces << "] scale [" << sScale.AsLong(0L) << "]");
		sString = strNumber << sDecSeparator << strDecPlaces.SubString(0L, sScale.AsLong(0L));
	} else {
		sString = strNumber;
		Trace("result scale = 0 [" << sString << "]");
	}

	return true;

}

void DecimalFormatRenderer::InsertFiller(String  &sScale, String &strDecPlaces )
{
	StartTrace(DecimalFormatRenderer.InsertFiller);

	for ( long lIdx = strDecPlaces.Length(); lIdx < sScale.AsLong(0L) ; lIdx++) {
		strDecPlaces.Append("0");
		Trace("scale [ " << strDecPlaces << "] index [ " << lIdx << " ]");
	}
}

