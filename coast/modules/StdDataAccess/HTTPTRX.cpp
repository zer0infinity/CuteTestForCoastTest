/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Mapper.h"
#include "CacheHandler.h"
#include "StringStream.h"
#include "Socket.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "HTTPTRX.h"

//--- HTTPTRX -----------------------------------------------------
RegisterDataAccessImpl(HTTPTRX);

HTTPTRX::HTTPTRX(const char *name) : DataAccessImpl(name)
{
	GetMetaData(fName); // initializes fTRXMetaData
}

HTTPTRX::~HTTPTRX()
{

}

IFAObject *HTTPTRX::Clone() const
{
	return new HTTPTRX(fName);
}

//--- provide transaction input
void HTTPTRX::PutArgs(Context &context, InputMapper *mpIn)
/*	Builds the HTTP-Request and puts it in fData2Send

*/
{
	StartTrace(HTTPTRX.PutArgs);

	String input;
	OStringStream os(&input);
	// get the data information from client
	MetaThing data;
	mpIn->Get("Data", data, context);

	String method(data["env"]["REQUEST_METHOD"].AsCharPtr(GetMetaData("Method").AsCharPtr("GET")));
	SubTraceAny(Data, data, "Data: ");
	String params = BuildParameterList(data);

	String uri(GetMetaData("URI").AsCharPtr(""));	// URI is URL without protokoll and host/port
	// Generate the header-information of the HTTP-request from the config-info
	os << method << " " << uri;								// METHOD and uri
	if ( !GetMetaData("Suppress_URI").AsBool(0) ) {
		os << data["URI_PARAM"].AsCharPtr("");
	}
	if (method == "GET" && params.Length() > 0 ) {
		// append the parameterlist to the uri
		os << "?" << params;
	}

	os << " " << GetMetaData("HTTPVersion").AsCharPtr("") << ENDL;	// HTTP-version
	// append authorization line
	String auth_line(MakeAuthorizationInfo(data));
	if (auth_line.Length() != 0 ) {
		os << "Authorization: " << auth_line << ENDL;
	}
	// append additional Header lines
	AppendHeaderLines(os, data);

	if (method == "GET") {
		// append the parameterlist to the uri
		os << "Content-length: " << 0 << ENDL << ENDL << flush;
	} else if (data["query"].IsDefined("postparams")) {
		String pp = data["query"]["postparams"].AsCharPtr("");
		os << "Content-length: " << pp.Length() << ENDL << ENDL
		   << pp << flush;
	} else {
		os << "Content-length: " << params.Length() << ENDL
		   << ENDL 								// separator
		   << params << flush;					// and parameter
	}
	// store it for later use by DoExec
	fData2Send = input;

	SubTrace(fData2Send, "Buf: <" << input << ">");
}

bool HTTPTRX::Exec(Context &context, InputMapper *mpIn, OutputMapper *mpOut)
{
	StartTrace(HTTPTRX.DoExec);
	PutArgs(context, mpIn);
	String msgIn(fData2Send.AsString());
	String msgOut;

	//--- server connection
	SubTrace(MsgSent, "MsgSent: " << NotNullStr(msgIn));

	const char *tcpip = GetMetaData("Server").AsCharPtr("127.0.0.1");
	long port = GetMetaData("Port").AsLong(80);		// get ip-address and port

	Connector csc(tcpip, port);//DoMakeConnector(GetMetaData("UseSSL").AsBool(0), tcpip, port);
	Socket *socket = csc.Use();
	iostream *Ios = csc.GetStream();

	if ( !Ios ) {
		fData2Get["Error"]["Code"] = "10.01";
		String message("Could not open socket");
		fData2Get["Error"]["Message"] = "Could not open socket";
	} else {
		(*Ios) << msgIn;

		// read reply and dump it also to a file

		char c;

		while ((Ios->get(c)).good()) {
			msgOut.Append(c);				// We must do something else for cin!
		}
		socket->ShutDown();
	}

	SubTrace(MsgReceived, "<" << NotNullStr(msgOut) << ">");

	fData2Get["MsgReceived"] = msgOut;

	GetArgs(context, mpOut);
	return true;
}

void HTTPTRX::GetArgs(Context &context, OutputMapper *mpOut)
/* convert the output (An HTTP-reply) to a Anything.
   Some error handling is performed.
   ( A better handling of content type related parsing could be applied here)
 */
{
	StartTrace(HTTPTRX.GetArgs);

	Anything header;				// info about the HTTP-header
	Anything error;					// to return an error message

	if (!fData2Get.LookupPath(error, "Error")) {	// no error defined, TRX was executed
		String msgIn(fData2Get["MsgReceived"].AsString());
		IStringStream is(&msgIn);

		ParseHeader(is, header);	// parses all header lines, leaves body
		SubTraceAny(Header, header, "Header parsed")
		mpOut->Put("Header", header, context);
		if ((header["ResponseCode"].AsLong(0) == 200) || (header["ResponseCode"].AsLong(0) == 201)) {
			if (header["content-type"] == "application/x-ifa-anything") {
				Anything data;		// gathers the data of the TRX

				data.Import(is);
				SubTraceAny(Data, data, "Data:" );
				mpOut->Put("Reply", data, context);
			} else if (header["content-type"] == "image/gif") {
				// FIXME: do something special for gifs (binary any)
				Anything graphic;
				// Actually this does too much copying
				String graphData;
				graphData.Append(is, msgIn.Length());		// reasonable maximum

				graphic = Anything((void *)((const char *)graphData),
								   graphData.Length());
				SubTraceAny(Graphic, graphic, "Graphic:" );
				mpOut->Put("Graphic", graphic, context);
			} else {
				// Assume some plain text contents
//				Anything data(is.str());	// retrieve rest of buffer
				String data;
				data.Append(is, msgIn.Length());
				mpOut->Put("Reply", data, context);
			} // if
		} else {
			error["Error"]["Code"] = "10.03";
			String message("HTTP status ");		// construct message
			OStringStream ms(&message);
			ms << header["Status"].AsLong(-1) << " returned";

			error["Error"]["ErrorMsg"] = message;
			error["Error"]["Msg"] = is.str();	// retrieve rest of buffer
			mpOut->Put("Error", error, context);
		} // if
	} else {						// there was an error, we just return it in /Error
		mpOut->Put("Error", error, context);
	} // if
}

ROAnything HTTPTRX::GetMetaData(const char *name)
{
	StartTrace1(HTTPTRX.GetMetaData, "TRX: <" << name << ">");
	SubTraceAny(Repository, fTRXMetaData, "MtaInfo");

	if (fTRXMetaData.GetType() == Anything::eNull) {
		// Config not loaded yet
		SimpleAnyLoader sal;
		ROAnything meta = CacheHandler::Get()->Load("TRX", "HTTPTRXMeta", &sal);
		if ( meta.IsDefined(fName) ) {
			// Take out only slot that matches the name of the TRX
			fTRXMetaData = meta[fName];
		} else {
			fTRXMetaData = meta;
		}
	}
	if ( fTRXMetaData.IsDefined(name) ) {
		SubTraceAny(MetaData, fTRXMetaData[name], "fTRXMetaData");
		return fTRXMetaData[name];
	}
	return Anything();
}

void HTTPTRX::ParseHeader(istream &in, Anything &header)
/* parses the input stream up to an empty line which separates header and body.
   Does not parse anything in the body.
   in: in     -	stream to read from
  out: header -	Information about the HTTP-status of the reply in slot header/Status,
				all the header-lines in lower case slots
				(e.g. /content_type "text/plain")
  assumption: This procedure is called only when no error has occured and we may
	   reasonably expect there is legal HTTP-Header to parse
 */
{
	StartTrace(HTTPTRX.ParseHeader);
	String line ;
	String s;
	long status;

	in >> s;							// regular HTTP, just a version
	header["HTTP-version"] = s;

	in >> status;
	header["ResponseCode"] = status;
	getline(in, s, '\n');			// status message
	s.Trim(s.Length() - 1);			// remove last char (cr, HTTP uses crlf to separat lines)
	header["ResponseMsg"] = s;
	Trace("Status message: " << s);

	while ((getline(in, line, '\n'), line) != "" && line != "\r")
		// depends on how \r\n is treated
	{
		// still header
		Trace("line read: " << line);
		line.Trim(line.Length() - 1);	// remove trailing cr from getline

		String fieldname;
		String item;
		long pos = 0;

		pos = line.StrChr(':');
		if (pos > 0) {								// Assumed regular, a line with :
			fieldname = line.SubString(0, pos);
			fieldname.ToLower();	// all header fields stored in lower case
			item = line.SubString(pos + 2);

			Trace("field name: " << fieldname);
			Trace("value: " << item);

			if ( !header.IsDefined(fieldname) ) {
				header[fieldname] = item;
			} else {
				header[fieldname].Append(item);
			}
		} else {
			header["otherLines"].Append(line);
		}
	} // while

//	char c;
//	while( !in.get(c).eof() && c != '<' )
//		;
//	if (!!in)
//		in.putback(c);
	// now we are just at the beginning of the body
} // ParseHeader

String HTTPTRX::MakeAuthorizationInfo(Anything &data)
/* Builds a String with the Authorization Information from MetaData.
   The form of the String is according to the HTTP protokoll :
	In:	data -  Anything containing user data. (Not used because the AuthInfo is taken from Metadata,
				but subclasses might want to use user data instead)
*/
{
	String path("env.header.Authorization");
	Anything authInfo;

	if ( data.LookupPath(authInfo, path) ) {
		String auth("Authorization: ");
		auth << authInfo.AsCharPtr("");
		return auth;
	}
	return GetMetaData("Authorization").AsString(""); // PS: optimize it, also for readability
}

void HTTPTRX::AppendHeaderLines(OStringStream &os, Anything &data)
/* Except Authorisation and Content-lenght any more Header lines might be added here
	In:	os 	 -	Stream where the lines have to written on
		data -  Anything containing user data providing the values for the parameters. (Not used,
				but subclasses might want to use it)
*/
{
	StartTrace(HTTPTRX.AppendHeaderLines);
	long i;

	ROAnything headerlines = GetMetaData("Header");
	TraceAny(headerlines, "Config Header: ");
	for (i = 0; i < headerlines.GetSize(); i++) {
		// simply write each slot to os
		os << headerlines[i].AsCharPtr("") << ENDL;
	} // for

	TraceAny(data, "Data: ");
	Anything header(data["env"]["header"]);
	Anything headerElement;
	long sz = header.GetSize();
	const char *slot;
	for (i = 0; i < sz; i++) {
		slot = header.SlotName(i);
		if ( slot ) {
			os << slot << ": ";
		}
		headerElement = header[i];
		long elSz = headerElement.GetSize();
		for (long j = 0; j < elSz; j++) {
			os << headerElement[j].AsCharPtr("");
			if (j < (elSz - 1)) {
				os << ",";
			}
		}
		os  << ENDL;
	} // for

}

String HTTPTRX::BuildParameterList(Anything &data)
/* Builds a String with the Parameters defined by SingleParams and ListParams
   from MetaData.The form of the String is according to the URL standard : name=value{&name=value}
	In:	data -  Anything containing user data providing the values for the parameters.
*/
{
	StartTrace(HTTPTRX.BuildParameterList);
	String bs;
	OStringStream bodyStream(&bs);
	SubTraceAny(data, data, "InputData");
	ROAnything singleParams = GetMetaData("SingleParams");
	int i;
	for (i = 0; i < singleParams.GetSize(); i++) {		// process the single params
		String singleVar(singleParams[i].AsString(""));
		const char *slotname = singleParams.SlotName(i);
		if ( slotname ) {
			// Inline Parameter
			bodyStream << slotname << '='
					   << singleVar << '&';

		} else if (data.IsDefined(singleVar)) {
			// if the slot appears in data we write it into the query
			bodyStream << singleVar << '='
					   << data[singleVar].AsCharPtr("") << '&';
		} // if
	} // for

	ROAnything listParams(GetMetaData("ListParams"));
	for (i = 0; i < listParams.GetSize(); i++) {		// process the list params
		String listVar(listParams[i].AsString(""));
		if (data.IsDefined(listVar)) {
			Anything listVals(data[listVar]);		// list as f1=v1&f2=v2 ...

			for (int j = 0; j < listVals.GetSize(); j++) {
				bodyStream << listVar << '='
						   << listVals[j].AsCharPtr("") << '&';
			} // for
		} // if
	} // for

	bodyStream << flush;
	if (bs.Length() > 0) {		// some parameter string was constructed
		bs.Trim(bs.Length() - 1);		// remove last &, nicer than if all the time
	} // if

	String body (urlEncode(bs));
	// construct actual body-string using our urlEncode
	SubTrace(params, "Parameterlist constructed: <" << bs << ">");
	return body;
}

String HTTPTRX::urlEncode(const char *p)
/* converts special characters in a string p to hex-encoding
	differs slightly from the version in URLutils, converts ',' and does
	*not* convert '&'
	FIXME: implement according to spec eventually!
 */
{
	String result;
	if ( p ) {
		char c;
		while ( (c = *p++) ) {
			switch (c) {
				case '\n':
					break;
				case ' ':
				case ',':
				case '"':
				case '%':
				case '?':
				case '/':
				case '#':
					result.Append('%');
					result.AppendAsHex(c);
					break;
				default:
					result.Append(c);
					break;
			}
		}
	}
	return result;
}

