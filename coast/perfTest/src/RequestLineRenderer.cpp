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
#include "SysLog.h"
#include "SecurityModule.h"
#include "Dbg.h"

//---- RequestLineRenderer ---------------------------------------------------------
RegisterRenderer(RequestLineRenderer);

RequestLineRenderer::RequestLineRenderer(const char *name) : Renderer(name)
{
}

RequestLineRenderer::~RequestLineRenderer()
{
}

void RequestLineRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(RequestLineRenderer.Render);
	TraceAny(config, "config");
	String buffer;
	{
		StringStream replyDebugBuffer(&buffer);
		// logic is, if CurrentServer.Path is available, render that but if link nr is available try to render that first

		// config not so interesting - does this renderer even have one??
		Anything myTmpStore = c.GetTmpStore();
		TraceAny( myTmpStore, "tmp store" );

		Anything lookUpResult;
		bool postIsMethod = false;

		if (myTmpStore.LookupPath(lookUpResult, "CurrentServer.Method") ) {
			Trace( "RequestMethod is->" << lookUpResult.AsString("") );

			String methodName = 	lookUpResult.AsString("");
			methodName.ToUpper();

			if ( methodName == "POST" ) {
				postIsMethod = true;
			}
			replyDebugBuffer << lookUpResult.AsString("") << " ";
		} else {
			Trace( "Default RequestMethod is-> GET" );
			replyDebugBuffer << "GET ";
		}

		String path;
		if (myTmpStore.LookupPath(lookUpResult, "CurrentServer.Path") ) {
			RenderOnString(path, c, lookUpResult);
			long qmarkPos = path.StrChr('?');
			if (qmarkPos >= 0 && !postIsMethod && myTmpStore["CurrentServer"].IsDefined("formContents")) {
				path = path.SubString(0, qmarkPos);
			}
		} else {
			path = "/";
		}

		Trace( "RequestURI is->" << path );
		replyDebugBuffer << path;

		String formContentString = "";

		if (myTmpStore.LookupPath(lookUpResult, "CurrentServer.formContents") ) {
			formContentString = myTmpStore["CurrentServer"]["formContents"].AsString("");
			// If 'Enctype' is multipart/form-data and a specific boundary was given too
			// then "resolve" the formContent to a boundary separated multipart content:
			String encTypeString = myTmpStore["CurrentServer"]["Enctype"].AsString();
			if ( encTypeString.Contains("multipart/form-data; boundary") == 0 ) {
				// formContentString already resolved to boundaryseparated multiparts!
				// no encoding required here.
			} else {
				long posn = 0L;
				while ( (posn = formContentString.StrChr(' ', 0)) > 0 ) {
					Trace("b4 - formContentString" << formContentString);
					formContentString.ReplaceAt(posn, "+", 1 );
					Trace("after -formContentString" << formContentString);
				}
			}

			if ( !postIsMethod ) { // append form content in GET to URI
				replyDebugBuffer << "?";
				replyDebugBuffer << formContentString;
			}

		} else if (myTmpStore.LookupPath(lookUpResult, "CurrentServer.MsgBody")) {
			for (long i = 0; i < myTmpStore["CurrentServer"]["MsgBody"].GetSize(); i++ ) {
				formContentString.Append( myTmpStore["CurrentServer"]["MsgBody"].At(i).AsString("") );
			}
		}

		Trace("END formContentString" << formContentString);

		if (myTmpStore.LookupPath(lookUpResult, "CurrentServer.OverallProtocol") ) {
			Trace( "RequestProtocol is->" << lookUpResult.AsString("") );
			replyDebugBuffer << lookUpResult.AsString("");
		} else {
			Anything renderedheader;
			renderedheader["User-Agent"] = myTmpStore["CurrentServer"]["UserAgent"].AsString("Mozilla/4.51 [en] (WinNT; I)");
			String hostHeader(myTmpStore["CurrentServer"]["ServerName"].AsString("localhost"));

			renderedheader["Host"] = myTmpStore["CurrentServer"]["ServerName"].AsString("localhost");
			if (myTmpStore["CurrentServer"].IsDefined("Port")) {
				hostHeader << ":" << myTmpStore["CurrentServer"]["Port"].AsString("");
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
		if (myTmpStore["CurrentServer"].IsDefined("User") && myTmpStore["CurrentServer"].IsDefined("Pass")) {
			// encode the string and add the authorization slot to the request
			Anything someData;
			String strUserPass;
			if (myTmpStore.LookupPath(someData, "CurrentServer.User")) {
				String strData;
				RenderOnString(strData, c, someData);
				strUserPass << strData;
				Trace("User:[" << strData << "]");
			}
			strUserPass << ":";
			if (myTmpStore.LookupPath(someData, "CurrentServer.Pass")) {
				String strData;
				RenderOnString(strData, c, someData);
				strUserPass << strData;
				Trace("Pass:[" << strData << "]");
			}
			Encoder *base64 = Encoder::FindEncoder("Base64Regular");
			String encodeddata(strUserPass.Length() * 4 / 3);
			if (base64) {
				base64->DoEncode(encodeddata, strUserPass);
				replyDebugBuffer << "\r\nAuthorization: Basic " << encodeddata;
				Trace("string [" << strUserPass << "] encoded [" << encodeddata << "]");
			} else {
				SysLog::Warning("Base64 encoder not found! Authorization not added to request!");
			}
		}

		// render cookie
		Renderer *r = Renderer::FindRenderer("CookieToServerRenderer");
		r->RenderAll(replyDebugBuffer, c, config);

		if ( formContentString.Length() > 0 && postIsMethod ) {
			replyDebugBuffer << "\r\nContent-Length: " << formContentString.Length();
			replyDebugBuffer << "\r\nContent-Type: " << myTmpStore["CurrentServer"]["Enctype"].AsString("");
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
		infoMsg << "Name:" << myTmpStore["CurrentServer"]["ServerName"].AsString("");
		infoMsg << " IP:" << myTmpStore["CurrentServer"]["Server"].AsString("");
		infoMsg << " Port:" << myTmpStore["CurrentServer"]["Port"].AsString("") << ")\r\n";
		infoMsg << replyDebugBuffer.str();

#ifdef DEBUG
		if ( Tracer::CheckWDDebug("RequestLineRenderer.Render") ) {
			SysLog::Info( infoMsg ); // perhaps enable this line with an entry in RequestLineRenderer.any.... future
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
	reply << buffer;
}
