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
#include "AnyIterators.h"
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
	TraceAny( c.GetTmpStore(), "<<- Overall TMP Store" );

	String explicitDomainName = c.Lookup("CurrentServer.ServerName", "");
	explicitDomainName.ToUpper();
	// without Port part
	OutputCookies( explicitDomainName, reply, c );

	explicitDomainName << ":" << c.Lookup("CurrentServer.Port", "");

	// with Port part
	OutputCookies( explicitDomainName, reply, c );
}

void CookieToServerRenderer::OutputCookies(const String &explicitDomainName, ostream &reply, Context &c )
{
	StartTrace(CookieToServerRenderer.OutputCookies);

	String newString = explicitDomainName;
	newString.ToUpper();

	ROAnything roaAllCookieDomains;
	if ( c.Lookup("Cookies", roaAllCookieDomains) ) {
		Trace( "try to find [" << explicitDomainName << "]");
		TraceAny( roaAllCookieDomains,  "try to find [" << explicitDomainName << "] in here" );
		// could be .y.z match cookie to be sent out
		ROAnything roaCookies;
		AnyExtensions::Iterator<ROAnything, ROAnything, String> aCookieIter(roaAllCookieDomains);
		while ( aCookieIter.Next(roaCookies) ) {
			String potPartDomain;
			aCookieIter.SlotName(potPartDomain);
			Trace("PotPartDomain " << potPartDomain << " NewString "  << newString);
			long posn = newString.Contains( potPartDomain );
			Trace("Posn " << posn <<  " potPartDomain.Length " << potPartDomain.Length() << " newString.Length "  << newString.Length());
			if ( (posn + potPartDomain.Length()) == newString.Length() ) {
				// match of domains at end of string...only
				Trace( "(part) domain found at slot no" << aCookieIter.Index() );

				reply << "\r\nCookie: ";
				// output all cookies in the current cookie container...
				ROAnything roaEntry;
				AnyExtensions::Iterator<ROAnything, ROAnything, String> aDCookieIter(roaCookies);
				if ( aDCookieIter.Next(roaEntry) ) {
					Trace("Cookie passed to server [" << roaEntry.AsString() << "]");
					reply << roaEntry.AsString();
					while ( aDCookieIter.Next(roaEntry) ) {
						Trace("Cookie passed to server [" << roaEntry.AsString() << "]");
						reply << "; " << roaEntry.AsString();
					}
				}
				// finish loop now
				break;
			}
		}
	}
}
