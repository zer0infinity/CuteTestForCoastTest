/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPTRX_H
#define _HTTPTRX_H

//--- superclass interface ---
#include "config_stddataaccess.h"
#include "DataAccessImpl.h"

class String;
class Anything;
class ROAnything;
class OStringStream;

//! DataAccess for performing HTTP Requests, uses meta data defined in HTTPMeta.any
class EXPORTDECL_STDDATAACCESS HTTPTRX: public DataAccessImpl
{
public:
	HTTPTRX(const char *name);
	~HTTPTRX();

	// returns a new TRX object
	IFAObject *Clone() const;

	//! executes the transaction
	//! \param c The context of the transaction
	//! \pre fData2Send contains the HTTP-Request
	//! \post fData2Get contains either an slot /MsgReceived containing the HTTP Response or the slot /Error storing information about the error occured.
	virtual bool Exec(Context &c, InputMapper *, OutputMapper *);

protected:
	void PutArgs(Context &context, InputMapper *mpIn);
	void GetArgs(Context &context, OutputMapper *mpOut);
	//! gets properties by name
	//! \param key The property name
	//! \return ROAnything that contains the property value
	//! \pre None
	//! \post The meta data file has been read in
	virtual ROAnything GetMetaData(const char *key) ;
	//! retrieves Authorisation information from meta data and writes it to the HTTP request header
	//! Builds a String with the Authorization Information from MetaData.
	//! The form of the String is according to the HTTP protokoll :
	//! \param data  Anything containing user data. (Not used because the AuthInfo is taken from Metadata, but subclasses might want to use user data instead)
	//! \return the Authorisation Header line
	//! Maybe the interface should be adapted to the one of MakeAuthorizationInfo
	virtual String MakeAuthorizationInfo(Anything &data);
	//! Except Authorisation and Content-lenght any more Header lines might be added here
	//! \param	os Stream where the lines are  written to
	//! \param data Anything containing user data providing the values for the parameters. (Not used, but subclasses might want to use it)
	virtual void AppendHeaderLines(OStringStream &os, Anything &data);
	//! Builds an URL conforming key-value-pair list
	//! Builds a String with the Parameters defined by SingleParams and ListParams
	//! from MetaData.The form of the String is according to the URL standard : name=value{&name=value}
	//! \param data Anything containing user data providing the values for the parameters.
	//! \return The parameter list
	virtual String BuildParameterList(Anything &data);
	//! converts special characters in a string to url-hex-encoding
	//!	differs slightly from the version in URLutils, converts ',' and does *not* convert '&'
	//!	FIXME: implement according to spec eventually!
	String urlEncode(const char *p);
	//! parses the input stream up to an empty line which separates header and body.
	//!  Does not parse anything in the body.
	//! \param in     -	stream to read from
	//! \param header(out) -	Information about the HTTP-status of the reply in slot header/Status, all the header-lines in lower case slots (e.g. /content_type "text/plain")
	//! \pre This procedure is called only when no error has occured and we may reasonably expect there is legal HTTP-Header to parse
	void ParseHeader(istream &in, Anything &header);

	Anything	fData2Send;
	Anything	fData2Get;

private:
	ROAnything fTRXMetaData;

	//constructor
	HTTPTRX();
	HTTPTRX(const HTTPTRX &);
	//assignement
	HTTPTRX &operator=(const HTTPTRX &);

	friend class HTTPTRXTest;
};

/* Don't add stuff after this #endif */
#endif		//not defined _HTTPTRX_H
