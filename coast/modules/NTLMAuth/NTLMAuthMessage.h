/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NTLMAuthMessage_H
#define _NTLMAuthMessage_H

#include "ITOString.h"

//! simple class wrapping NTLMAuthMessages
/*!
 further explanation of the purpose of the class
 this may contain <B>HTML-Tags</B>
 */
class NTLMAuthMessage {
public:
	NTLMAuthMessage(const String &binarybuffer) :
			fMsg(binarybuffer) {
	}
	virtual ~NTLMAuthMessage() {
	}
	virtual bool IsValid();
	long GetMessageType();
	virtual unsigned long GetFlags() {
		return 0;
	}
	virtual String GetDomain() {
		return "";
	}
	virtual String GetHost() {
		return "";
	}
	virtual String GetNonce() {
		return ""; // only for type 2
	}
	virtual String GetUser() {
		return ""; // only for type 3
	}
	virtual String GetNTHash() {
		return ""; // only for type 3
	}
	virtual String GetLMHash() {
		return ""; // only for type 3
	}
	String GetAsBase64();
	static NTLMAuthMessage *MakeMessage(const String &binarybuffer);
protected:
	String DecodeString(long pos, bool striputf = false);
	long DecodeLong(long pos);
	long DecodeShort(long pos);
	String fMsg;
};
class NTLMAuthClientMsgType1: public NTLMAuthMessage {
public:
	NTLMAuthClientMsgType1(const String &binarybuffer) :
			NTLMAuthMessage(binarybuffer) {
	}
	virtual bool IsValid();
	virtual unsigned long GetFlags();
	virtual String GetDomain();
	virtual String GetHost();
};
class NTLMAuthServerMsgType2: public NTLMAuthMessage {
public:
	NTLMAuthServerMsgType2(const String &binarybuffer) :
			NTLMAuthMessage(binarybuffer) {
	}
	virtual bool IsValid();

	virtual unsigned long GetFlags();
	virtual String GetNonce();
};
class NTLMAuthClientMsgType3: public NTLMAuthMessage {
public:
	NTLMAuthClientMsgType3(const String &binarybuffer) :
			NTLMAuthMessage(binarybuffer) {
	}
	virtual bool IsValid();

	virtual unsigned long GetFlags();
	virtual String GetDomain();
	virtual String GetHost();
	virtual String GetUser();
	virtual String GetNTHash();
	virtual String GetLMHash();

};
#endif
