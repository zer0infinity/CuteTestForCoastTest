/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RequestLineRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "SecurityModule.h"
#include "Dbg.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//---- RequestLineRenderer ---------------------------------------------------------
RegisterRenderer(RequestLineRenderer);

RequestLineRenderer::RequestLineRenderer(const char *name) : Renderer(name)
{
	StartTrace(RequestLineRenderer.Ctor);
}

RequestLineRenderer::~RequestLineRenderer()
{
	StartTrace(RequestLineRenderer.Dtor);
}

void RequestLineRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(RequestLineRenderer.RenderAll);
	SubTraceAny(TraceConfig, config, "config");
	String buffer(1024L);
	{
		StringStream replyDebugBuffer(buffer);
		// logic is, if CurrentServer.Path is available, render that but if link nr is available try to render that first

		// config not so interesting - does this renderer even have one??
		SubTraceAny(TraceTmp, c.GetTmpStore(), "tmp store" );
		ROAnything roaResult;
		bool postIsMethod = false;

		if ( c.Lookup("CurrentServer.Method", roaResult) ) {
			Trace( "RequestMethod is->" << roaResult.AsString("") );
			String methodName = 	roaResult.AsString("");
			methodName.ToUpper();

			if ( methodName == "POST" ) {
				postIsMethod = true;
			}
			replyDebugBuffer << methodName << " ";
		} else {
			Trace( "Default RequestMethod is-> GET" );
			replyDebugBuffer << "GET ";
		}

		String path;
		if ( c.Lookup("CurrentServer.Path", roaResult) ) {
			RenderOnString(path, c, roaResult);
			long qmarkPos = path.StrChr('?');
			if (qmarkPos >= 0 && !postIsMethod && c.Lookup("CurrentServer.formContents", roaResult) ) {
				path = path.SubString(0, qmarkPos);
			}
		} else {
			path = "/";
		}

		Trace( "RequestURI is->" << path );
		replyDebugBuffer << path;

		String formContentString;
		if ( c.Lookup("CurrentServer.formContents", roaResult) ) {
			formContentString = roaResult.AsString("");
			// If 'Enctype' is multipart/form-data and a specific boundary was given too
			// then "resolve" the formContent to a boundary separated multipart content:
			String encTypeString = c.Lookup("CurrentServer.Enctype", "");
			if ( encTypeString.Contains("multipart/form-data; boundary") == 0 ) {
				// formContentString already resolved to boundaryseparated multiparts!
				// no encoding required here.
			} else {
				long posn = 0L;
				while ( (posn = formContentString.StrChr(' ', 0)) > 0 ) {
					Trace("b4 - formContentString [" << formContentString << "]");
					formContentString.ReplaceAt(posn, "+", 1 );
					Trace("after -formContentString [" << formContentString << "]");
				}
			}

			if ( !postIsMethod ) {
				// append form content in GET to URI
				replyDebugBuffer << "?";
				replyDebugBuffer << formContentString;
			}
		} else if ( c.Lookup("CurrentServer.MsgBody", roaResult) ) {
			for (long i = 0; i < roaResult.GetSize(); ++i ) {
				formContentString.Append( roaResult[i].AsString("") );
			}
		}

		Trace("END formContentString [" << formContentString << "]");

		if ( c.Lookup("CurrentServer.OverallProtocol", roaResult) ) {
			Trace( "RequestProtocol is->" << roaResult.AsString("") );
			replyDebugBuffer << roaResult.AsString("");
		} else {
			Anything renderedheader;
			renderedheader["User-Agent"] = c.Lookup("CurrentServer.UserAgent", "Mozilla/4.51 [en] (WinNT; I)");
			String hostHeader(c.Lookup("CurrentServer.ServerName", "localhost"));
			long lPort = -1L;
			if ( ( lPort = c.Lookup("CurrentServer.Port", -1L) ) != -1L ) {
				hostHeader << ':' << lPort;
			}
			renderedheader["Host"] = hostHeader;
			renderedheader["Connection"] = "Close";
			renderedheader["Accept"] = "image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*";
			renderedheader["Accept-Language"] = "en-GB,de";
			renderedheader["Accept-Charset"] = "iso-8859-1,*,utf-8";

			for (long i = 0; i < config["Headerfields"].GetSize(); i ++) {
				String rendered;
				Renderer::RenderOnString(rendered, c, config["Headerfields"][i]);
				renderedheader[config["Headerfields"].SlotName(i)] = rendered;
			}
			replyDebugBuffer << " HTTP/1.1";
			for (long j = 0; j < renderedheader.GetSize(); j ++) {
				replyDebugBuffer << "\r\n" << renderedheader.SlotName(j) << ": " << renderedheader[j].AsString();
			}
		}

		// check if we have to add an authorization
		ROAnything roaUser, roaPass;
		if ( c.Lookup("CurrentServer.User", roaUser) && c.Lookup("CurrentServer.Pass", roaPass) ) {
			// encode the string and add the authorization slot to the request
			String strUserPass;
			String authorizationHeaderFieldName;
			RenderOnString(strUserPass, c, roaUser);
			Trace("User:[" << strUserPass << "]");

			strUserPass << ":";
			RenderOnString(strUserPass, c, roaPass);
			Trace("User:Pass:[" << strUserPass << "]");
			ROAnything roaAuth;
			if ( c.Lookup("CurrentServer.BasicAuthorizationHeaderFieldName", roaAuth) ) {
				RenderOnString(authorizationHeaderFieldName, c, roaAuth);
				Trace("authorizationHeaderFieldName: " << authorizationHeaderFieldName << "Length: " << authorizationHeaderFieldName.Length());
			}
			if ( authorizationHeaderFieldName.Length() == 0L ) {
				authorizationHeaderFieldName = "Authorization";
			}
			Trace("Authorization header field name:[" << authorizationHeaderFieldName << "]");
			Encoder *base64 = Encoder::FindEncoder("Base64Regular");
			String encodeddata(strUserPass.Length() * 4 / 3);
			if (base64) {
				base64->DoEncode(encodeddata, strUserPass);
				replyDebugBuffer << "\r\n" << authorizationHeaderFieldName << ": Basic " << encodeddata;
				Trace("string [" << strUserPass << "] encoded [" << encodeddata << "]");
			} else {
				SystemLog::Warning("Base64 encoder not found! Authorization not added to request!");
			}
		}

		// render cookie
		Renderer *r = Renderer::FindRenderer("CookieToServerRenderer");
		r->RenderAll(replyDebugBuffer, c, config);

		if ( formContentString.Length() > 0 && postIsMethod ) {
			replyDebugBuffer << "\r\nContent-Length: " << formContentString.Length();
			replyDebugBuffer << "\r\nContent-Type: " << c.Lookup("CurrentServer.Enctype", "");
			replyDebugBuffer << "\r\n\r\n";
			replyDebugBuffer << formContentString;
		} else {
			replyDebugBuffer << "\r\n\r\n";
		}
		// this information is passed up to and used by the Watchdog as a kind of pseudo trace
		Anything myEnv = c.GetQuery();
		String ThreadNumber = "default";
		if ( myEnv.IsDefined("Id" ) ) {
			ThreadNumber = myEnv["Id"].AsString("unknown");
		}
		String infoMsg = "\r\nRequest to server from Thread [" ;
		infoMsg << ThreadNumber  << "] (";
		infoMsg << "Name:" << c.Lookup("CurrentServer.ServerName", "");
		infoMsg << " IP:" << c.Lookup("CurrentServer.Server", "");
		infoMsg << " Port:" << c.Lookup("CurrentServer.Port", "") << ")\r\n";
		infoMsg << replyDebugBuffer.str();

#ifdef COAST_TRACE
		if ( Tracer::CheckWDDebug("RequestLineRenderer.Render", Storage::Current()) ) {
			SystemLog::Info( infoMsg ); // perhaps enable this line with an entry in RequestLineRenderer.any.... future
		}
#endif

#ifdef WDOG_RELEASE
		// tracing of the request which in the Watchdog release version can be switched on/off from the GUI
		// Here the Anything in which all requests are held is built
		String noOfCurrentEmsg = myTmpStore["result"]["ConfigStep"].AsString("");
		long noOfMessagesSoFar = myTmpStore["result"]["InfoMessageCtr"][noOfCurrentEmsg].GetSize();
		Anything anyMessagesSoFar(noOfMessagesSoFar);
		myTmpStore["result"]["InfoMessageCtr"][noOfCurrentEmsg][anyMessagesSoFar.AsString("")] = infoMsg;
#endif
	}
	Trace("request [" << buffer << "]");
	reply << buffer << flush;
}
