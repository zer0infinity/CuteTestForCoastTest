/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "BuildAnyFromSeparatedListAction.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "AnythingUtils.h"
#include "Renderer.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- BuildAnyFromSeparatedListAction ---------------------------------------------------------------
RegisterAction(BuildAnyFromSeparatedListAction);

BuildAnyFromSeparatedListAction::BuildAnyFromSeparatedListAction(const char *name) : Action(name) { }

BuildAnyFromSeparatedListAction::~BuildAnyFromSeparatedListAction() { }

bool BuildAnyFromSeparatedListAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(BuildAnyFromSeparatedListAction.DoExecAction);
	Trace("Start of BuildAnyFromSeparatedListAction");

	String text, recordSeparator, fieldSeparator, useSlotName, destination, sHeaderSeparator;
	bool bFieldSep = false;
	bool bUseSlot = false;
	bool bRet = false;
	long lHeaderRows = 0L;

	ROAnything destConfig;
	if ( !config.LookupPath(destConfig, "Destination") ) {
		return bRet;
	}
	TraceAny(destConfig, "Destination");
	// get name and value of defined params
	if ( ReadConfig( text, recordSeparator, fieldSeparator, bFieldSep, useSlotName, bUseSlot, lHeaderRows, sHeaderSeparator, ctx, config)) {
		Anything tmp;
		// build anything from text field
		if ( BuildAnything (text, recordSeparator, fieldSeparator, bFieldSep, bUseSlot, lHeaderRows, sHeaderSeparator, tmp )) {
			// Place result at destination slot
			StorePutter::Operate(tmp, ctx, destConfig);
			bRet = true;
		}
	}

	return bRet;
}

bool BuildAnyFromSeparatedListAction::ReadConfig(String &text, String &recordSeparator, String &fieldSeparator, bool &bFieldSep,
		String &useSlotName, bool &bUseSlot, long &lHeaderRows, String &sHeaderSeparator, Context &ctx, const ROAnything &config)
{
	// Read used Pathes and values from the config
	StartTrace(BuildAnyFromSeparatedListAction.ReadConfig);
	Trace("Start of ReadConfig");

	if (! ReadValue(text, config, "Text", ctx)) {
		return false;
	}
	text = TrimEndingSpaces(text);
	Trace("Text: [" << text << "]");

	if (! ReadValue(recordSeparator, config, "RecordSeparator", ctx))
		// set default if not defined
	{
		recordSeparator = "\r\n";
	}
	recordSeparator = TrimEndingSpaces(recordSeparator);
	Trace("RecordSeparator: [" << recordSeparator << "]");

	if ( ReadValue(fieldSeparator, config, "FieldSeparator", ctx)) {
		bFieldSep = true;
		fieldSeparator = TrimEndingSpaces(fieldSeparator);
		Trace("FieldSeparator: [" << fieldSeparator << "]");
	}

	if ( ReadValue(useSlotName, config, "useSlotName", ctx)) {
		Trace("useSlotName: [" << useSlotName << "]");
		bUseSlot = (config["useSlotName"].AsLong(0L) == 1L);
	}

	String sNHR;
	if ( ReadValue(sNHR, config, "NumberOfHeaderRows", ctx)) {
		Trace("NumberOfHeaderRows: [" << sNHR << "]");
		lHeaderRows = config["NumberOfHeaderRows"].AsLong(0L);
	}

	if ( ReadValue(sHeaderSeparator, config, "HeaderSeparator", ctx)) {
		Trace("HeaderSeparator: [" << sHeaderSeparator << "]");
		sHeaderSeparator = TrimEndingSpaces(sHeaderSeparator);
	}

	return true;
}

bool BuildAnyFromSeparatedListAction::ReadValue(String &retValue, const ROAnything &store, const String &location, Context &ctx)
{
	// reads data from store
	StartTrace(BuildAnyFromSeparatedListAction.ReadValue);
	Trace("Start of ReadValue");

	ROAnything roaSlotConfig;
	retValue = "";
	if (store.LookupPath(roaSlotConfig, location)) {
		Renderer::RenderOnString(retValue, ctx, roaSlotConfig);
	} else {
		Trace("Error in BuildAnyFromSeparatedListAction::ReadValue" << location << " not found");
		return false;
	}

	return true;
}

String BuildAnyFromSeparatedListAction::TrimEndingSpaces(String &value)
{
	long i, iMax =  value.Length();
	for ( i = iMax - 1; i >= 0; i-- ) {
		if ( value[i] != ' ' ) {
			break;
		}
	}
	value.Trim(i + 1);
	return value;
}

bool BuildAnyFromSeparatedListAction::DoBuildHeader(Anything &anyHeaderTok, Anything &anyHeader, String &sHeaderSeparator, bool &bBuildHeader)
{
	StartTrace(BuildAnyFromSeparatedListAction.DoBuildHeader);
	Trace("header elements [" <<  anyHeaderTok.GetSize() << "]");

	for (long lIdx2 = 0L; lIdx2 < anyHeaderTok[0L].GetSize() ; lIdx2++) {
		String sElementname;
		for (long lIdx = 0L; lIdx < anyHeaderTok.GetSize() ; lIdx++) {
			if (lIdx > 0 && anyHeaderTok[lIdx][lIdx2].AsString() != "") {
				sElementname = sElementname << sHeaderSeparator << anyHeaderTok[lIdx][lIdx2].AsString();
			} else {
				sElementname = sElementname <<  anyHeaderTok[lIdx][lIdx2].AsString();
			}
		}
		anyHeader[String() << lIdx2] = sElementname;
		TraceAny(anyHeader , "Elementname ");
	}
	bBuildHeader = false;
	return true;

}

bool BuildAnyFromSeparatedListAction::DoTokenize(Anything &anyTokens, String &text, String &strSep)
{
	StartTrace(BuildAnyFromSeparatedListAction.DoTokenize);
	Trace("separating token [" << strSep << "]");
	String strTmp(text);
	if ( strSep.Length() > 1L ) {
		long lIdx = -1L;
		while ( strTmp.Length() ) {
			lIdx = strTmp.Contains(strSep);
			Trace("found pattern at index:" << lIdx);
			String strToken = strTmp.SubString(0, lIdx);
			Trace("current token [" << strToken << "]");
			anyTokens.Append(strToken);
			if ( lIdx < 0 ) {
				strTmp.Trim(0);
			} else {
				strTmp = strTmp.SubString(strToken.Length() + strSep.Length());
			}
			Trace("remaining string " << (strTmp.Length() ? "> 0" : "empty"));
		}
	} else {
		StringTokenizer tok(strTmp, strSep[0L]);
		String strToken;
		while ( tok(strToken) ) {
			Trace("current token [" << strToken << "]");
			anyTokens.Append(strToken);
		}
	}
	TraceAny(anyTokens, "tokens so far");
	return ( anyTokens.GetSize() > 0L );
}

bool   BuildAnyFromSeparatedListAction::BuildAnything (String &text, String &recordSeparator, String &fieldSeparator, bool &bFieldSep,
		bool &bUseSlot, long &lHeaderRows, String &sHeaderSeparator, Anything &tmp )
{
	StartTrace(BuildAnyFromSeparatedListAction.BuildAnything);

	bool bBuildHeader = false;
	long lHeaderIndex = lHeaderRows;
	Anything anyRecords, anyHeaderTok, anyHeader;
	if ( DoTokenize(anyRecords, text, recordSeparator) ) {
		for (long lIdx = 0L; lIdx < anyRecords.GetSize(); lIdx++) {
			String strLine(anyRecords[lIdx].AsString());
			Trace("Current length of input [ " << strLine.Length() << "]");
			if (strLine.Length() > 0 ) {
				Anything anyTokens;
				Trace("current line [" << strLine << "]");
				Trace("doFieldSep:" << (bFieldSep ? "yes" : "no"));
				if ( bFieldSep ) {
					if ( lHeaderIndex > 0L ) {
						Anything anyHeaderLine;
						if ( DoTokenize(anyHeaderLine, strLine, fieldSeparator)) {
							for (long lhdx = 0L; lhdx < anyHeaderLine.GetSize(); lhdx++) {
								anyHeaderTok[String() << lHeaderIndex][String() << lhdx] = anyHeaderLine[lhdx].AsString();
							}
						}
						bBuildHeader = true;
						Trace("current headerindex [" << lHeaderIndex << "]");
						TraceAny(anyHeaderTok, "headertokens ");
					} else {
						if ( bBuildHeader ) {
							if (!DoBuildHeader( anyHeaderTok, anyHeader, sHeaderSeparator, bBuildHeader)) {
								return false;
							}
						}
						Anything anyLineTok;
						if ( DoTokenize(anyLineTok, strLine, fieldSeparator) ) {
							for (long lJdx = 0L; lJdx < anyLineTok.GetSize(); lJdx++) {
								if ( bUseSlot ) {
									if (lHeaderRows > 0L) {
										anyTokens[anyHeader[String() << lJdx].AsString()] = anyLineTok[lJdx].AsString();
									} else {
										anyTokens[String() << lJdx] = anyLineTok[lJdx].AsString();
									}
								} else {
									anyTokens.Append(anyLineTok[lJdx].AsString());
								}
							}
						}
					}
				} else {
					anyTokens = strLine;
				}
				TraceAny(anyTokens, "field token(s)");

				if ( lHeaderIndex == 0L ) {
					if ( bUseSlot ) {
						tmp[String() << (lIdx-lHeaderRows)] = anyTokens;
						Trace("current Index [" << (lIdx - lHeaderRows) << "]");
					} else {
						tmp.Append(anyTokens);
					}
				}
				if ( lHeaderIndex > 0L ) {
					lHeaderIndex--;
				}
			} else {
				Trace("!!! empty inputrecord [" << strLine << "] will be removed");
			}
		}
	}
	TraceAny(tmp, "result");
	return (tmp.GetSize() > 0L);
}
