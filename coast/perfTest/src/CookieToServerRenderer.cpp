/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "CookieToServerRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- CookieToServerRenderer ---------------------------------------------------------
RegisterRenderer(CookieToServerRenderer);

CookieToServerRenderer::CookieToServerRenderer(const char *name) : Renderer(name)
{
}

CookieToServerRenderer::~CookieToServerRenderer()
{
}

void CookieToServerRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(CookieToServerRenderer.Render);
	//TraceAny(config, "config");
	// config not so interesting - does this renderer even have one??
	Anything myTmpStore = c.GetTmpStore();
	TraceAny( myTmpStore, "<<- Overall TMP Store" );

	String explicitDomainName = myTmpStore["CurrentServer"]["ServerName"].AsString("");
	// without Port part
	OutputCookies( explicitDomainName, reply, c );

	explicitDomainName << ":" << myTmpStore["CurrentServer"]["Port"].AsString("");

	// with Port part
	OutputCookies( explicitDomainName, reply, c );
}

void CookieToServerRenderer::OutputCookies(String &explicitDomainName, ostream &reply, Context &c )
{
	StartTrace(CookieToServerRenderer.OutputCookies);

	Anything myTmpStore = c.GetTmpStore();
	Trace( "try to find" << explicitDomainName  );
	TraceAny( myTmpStore["Cookies"],  "<<- in here" );
	String newString = String( explicitDomainName );
	newString.ToUpper();

	// Old search relied on exact match
	//long mySlotNo= myTmpStore["Cookies"].FindIndex(newString); //
	long mySlotNo = -1;

	// could be .y.z match cookie to be sent out
	for (long i = 0; i < myTmpStore["Cookies"].GetSize(); i++ ) {
		String potPartDomain = myTmpStore["Cookies"].SlotName(i);
		Trace("PotPartDomain " << potPartDomain << " NewString "  << newString);
		long posn = newString.Contains( potPartDomain );
		Trace("Posn " << posn <<  " potPartDomain.Length " << potPartDomain.Length() << " newString.Length "  << newString.Length());
		if ( (posn + potPartDomain.Length()) == newString.Length() ) {
			// match of domains at end of string...only
			Anything cookieTin = myTmpStore["Cookies"].At(i);
			Trace( "(part) domain found at slot no" << i );
			i = myTmpStore["Cookies"].GetSize(); // finish For loop condition set...

			Trace( "result " << i );
			reply << "\r\nCookie: ";
			// output all cookies in the current cookie container...
			long sz = cookieTin.GetSize();
			for ( mySlotNo = 0; mySlotNo < sz;  mySlotNo++ ) {
				Trace("Cookie passed to server: " << cookieTin.At(mySlotNo).AsString(""));
				reply << cookieTin.At(mySlotNo).AsString("");
				if (mySlotNo < sz - 1) {
					reply << "; ";
				}
			}
		}
	}
}
