/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MailDAImpl.h"
#include "Socket.h"
#include "Registry.h"
#include "SecurityModule.h"
#include "Context.h"

//--- SMTPState -----------------
class SMTPState: public NotCloned {
	// abstract base class for communication states
public:
	// apply the smtp protocol
	static bool SendMail(std::iostream &Ios, Context &ctx, ParameterMapper *in, ResultMapper *out);

	// register a mail protocol state (state pattern)
	static void Register(const char *name, SMTPState *r);

	// terminate the module
	static void Terminate();
protected:
	// constructors are protected since objects of this type
	// are only constructed through register macro
	SMTPState(const char *name) :
		NotCloned(name) {
	}
	virtual ~SMTPState() {
	}

	// is this the final state
	virtual bool IsEndState() {
		return false;
	}

	// handle this protocol state with the given data
	virtual SMTPState *HandleState(Anything fMsgData, std::iostream &Ios);

	// produce the input for smtp server and flush the stream
	virtual bool DoProduceMsg(Anything &context, std::ostream &os);

	// produce the input for smtp server
	virtual bool ProduceMsg(Anything &context, std::ostream &os) = 0;

	// consume the reply
	virtual bool ConsumeReply(Anything &context, std::istream &is);

	// return the expected status number for this state
	virtual long DoGetStatus() {
		return 250;
	}

	// next state if the step produced an error
	virtual const char *NextErrorState() {
		return "MailERRORState";
	}

	// next state if the step succeeded
	virtual const char *NextOkState(Anything &result) = 0;

	// consume the rest of the reply line
	void ConsumeTillEol(std::istream &is, String &msg);

	static const String CRLF;

	RegCacheDef(SMTPState);
};

class MailFROMState: public SMTPState {
public:
	MailFROMState(const char *name) :
		SMTPState(name) {
	}

	virtual bool ProduceMsg(Anything &context, std::ostream &os);

	// next state if the step succeeded
	virtual const char *NextOkState(Anything &result) {
		return "MailRCPTState";
	}
};

class MailSTARTState: public SMTPState {
public:
	MailSTARTState(const char *name) :
		SMTPState(name) {
	}

	virtual SMTPState *HandleState(Anything fMsgData, std::iostream &Ios);
	virtual bool ProduceMsg(Anything &context, std::ostream &os);
	virtual bool ConsumeReply(Anything &context, std::istream &is);

	// next state if the step succeeded
	virtual const char *NextOkState(Anything &result) {
		return "MailFROMState";
	}
};

class MailRCPTState: public SMTPState {
public:
	MailRCPTState(const char *name) :
		SMTPState(name) {
	}

	virtual SMTPState *HandleState(Anything fMsgData, std::iostream &Ios);
	virtual bool ProduceMsg(Anything &context, std::ostream &os);

	// next state if the step succeeded
	virtual const char *NextOkState(Anything &result);

};

class MailDATAState: public SMTPState {
public:
	MailDATAState(const char *name) :
		SMTPState(name) {
	}

	virtual bool ProduceMsg(Anything &context, std::ostream &os);

	// return the expected status number
	virtual long DoGetStatus() {
		return 354;
	}

	// next state if the step succeeded
	virtual const char *NextOkState(Anything &result) {
		return "MailSENDState";
	}
};

class MailSENDState: public SMTPState {
public:
	MailSENDState(const char *name) :
		SMTPState(name) {
	}

	virtual bool ProduceMsg(Anything &context, std::ostream &os);

	// next state if the step succeeded
	virtual const char *NextOkState(Anything &result) {
		result["Ok"] = 1;
		return "MailQUITState";
	}
protected:
	void ProduceMultipartMsg(Anything &context, std::ostream &os);
protected:
	static const String BOUNDARY;
};

class MailERRORState: public SMTPState {
public:
	MailERRORState(const char *name) :
		SMTPState(name) {
	}

	virtual SMTPState *HandleState(Anything context, std::iostream &is);

	virtual bool ProduceMsg(Anything &context, std::ostream &os);
	virtual const char *NextOkState(Anything &) {
		return "MailENDState";
	}
};

class MailQUITState: public SMTPState {
public:
	MailQUITState(const char *name) :
		SMTPState(name) {
	}

	virtual SMTPState *HandleState(Anything context, std::iostream &is);

	virtual bool ProduceMsg(Anything &context, std::ostream &os);
	virtual const char *NextOkState(Anything &) {
		return "MailENDState";
	}
	virtual long DoGetStatus() {
		return 221;
	}
};

class MailENDState: public SMTPState {
public:
	MailENDState(const char *name) :
		SMTPState(name) {
	}

	virtual bool ProduceMsg(Anything &context, std::ostream &os) {
		return true;
	}
	virtual SMTPState *HandleState(Anything context, std::iostream &reply) {
		return this;
	}
	virtual bool IsEndState() {
		return true;
	}

	// next state if the step succeeded
	virtual const char *NextOkState(Anything &) {
		return "MailENDState";
	}
};

class SMTPStateInstaller {
public:
	SMTPStateInstaller(const char *name, SMTPState *s);
};
#define RegisterSMTPState(name) RegisterObject(name, SMTPState)
//----------------------- State classes ---------------------------------------------------
RegisterSMTPState(MailSTARTState);
RegisterSMTPState(MailFROMState);
RegisterSMTPState(MailRCPTState);
RegisterSMTPState(MailDATAState);
RegisterSMTPState(MailERRORState);
RegisterSMTPState(MailSENDState);
RegisterSMTPState(MailQUITState);
RegisterSMTPState(MailENDState);

const String SMTPState::CRLF("\x0D\x0A");

bool SMTPState::SendMail(std::iostream &Ios, Context &ctx, ParameterMapper *in, ResultMapper *out) {
	StartTrace(MailDAImpl.SendMail);
	SMTPState *st = SMTPState::FindSMTPState("MailSTARTState");

	Anything fMsgData;
	in->Get("Helo", fMsgData["Helo"], ctx);
	in->Get("From", fMsgData["From"], ctx);
	in->Get("To", fMsgData["To"], ctx);
	in->Get("Subject", fMsgData["Subject"], ctx);
	in->Get("Message", fMsgData["Message"], ctx);
	in->Get("Attachments", fMsgData["Attachments"], ctx);
	TraceAny(fMsgData, "Mail params");

	while (!st->IsEndState() && Ios.good()) {
		st = st->HandleState(fMsgData, Ios);
	} // end while  ! st.IsEndState()

	out->Put("MailResult", fMsgData["Result"], ctx);
	TraceAny(fMsgData, "Mail params");
	return fMsgData["Result"].IsDefined("Ok");
}

// returns next valid state
SMTPState *SMTPState::HandleState(Anything fMsgData, std::iostream &Ios) {
	String msg;

	if (DoProduceMsg(fMsgData, Ios)) {
		if (ConsumeReply(fMsgData, Ios)) { // read right to left
			return FindSMTPState(NextOkState(fMsgData["Result"]));
		}
	}
	return FindSMTPState(NextErrorState());;
}

bool SMTPState::DoProduceMsg(Anything &fMsgData, std::ostream &os) {
	StartTrace(SMTPState.DoProduceMsg);
	bool retVal = ProduceMsg(fMsgData, os);
	if (retVal) {
		os.flush();
	}

	return retVal && os.good();
}

bool SMTPState::ConsumeReply(Anything &context, std::istream &is) {
	//Check status of reply
	String msg;
	long status(-1);
	is >> status;
	StartTrace1(SMTPState.ConsumeReply, "Msg status " << status);

	if (!is) {
		Trace("Communication Error");
		context["Result"]["Error"] = "Communication Error";
		return false;
	} else {
		ConsumeTillEol(is, msg);
		Trace("Msg line " << msg);
		if (status != DoGetStatus()) {
			context["Result"]["Error"] = msg;
			return false;
		}
	}
	return true;
}

void SMTPState::ConsumeTillEol(std::istream &is, String &msg) {
	StartTrace(SMTPState.ConsumeTillEol);

	char c, peek;
	bool eol(false);
	while (!eol && is.good()) {
		is.get(c);
		if (c == '\x0D') {
			is.get(peek);
			if (peek == '\x0A') {
				eol = true;
			}
		}
		msg.Append(c);
	}
}

// returns next valid state
SMTPState *MailSTARTState::HandleState(Anything fMsgData, std::iostream &Ios) {
	// smtp server sends reply after connection
	String msg;
	ConsumeTillEol(Ios, msg);

	return SMTPState::HandleState(fMsgData, Ios);
}

bool MailSTARTState::ProduceMsg(Anything &context, std::ostream &os) {
	os << "HELO ";
	os << context["Helo"].AsCharPtr(DEF_UNKNOWN);
	os << CRLF; //!@FIXME should test whether this always works

	StatTrace(MailDAImpl.ProduceMsg, "Produce MailSTARTState " << context["Helo"].AsCharPtr(DEF_UNKNOWN), Coast::Storage::Current());
	return (os.good() != 0);
}

bool MailSTARTState::ConsumeReply(Anything &context, std::istream &is) {
	return SMTPState::ConsumeReply(context, is);
}

bool MailFROMState::ProduceMsg(Anything &context, std::ostream &os) {
	os << "MAIL FROM: ";
	if (context.IsDefined("From")) {
		os << context["From"].AsCharPtr("?");
	} else {
		os << "Unknown"; // Should define better default sender
	}

	os << CRLF; //!@FIXME should test whether this always works
	StatTrace(MailDAImpl.ProduceMsg, "Produce MailFROMState " << context["From"].AsCharPtr("?"), Coast::Storage::Current());
	return (os.good() != 0);
}

// returns next valid state
SMTPState *MailRCPTState::HandleState(Anything fMsgData, std::iostream &Ios) {
	Anything rcpts(fMsgData["To"]);

	if (rcpts.IsNull()) {
		return FindSMTPState("MailERRORState");
	}

	if (rcpts.GetType() == AnyCharPtrType) {
		return SMTPState::HandleState(fMsgData, Ios);
	}
	if (rcpts.GetType() == AnyArrayType) {
		SMTPState *st = this;
		SMTPState *error = FindSMTPState("MailERRORState");
		long i = 0;
		while (i < rcpts.GetSize() && st != error) {
			Anything a;
			a["To"] = rcpts[i];
			st = SMTPState::HandleState(a, Ios);
			++i;
		}
		return st;
	}
	return this;
}

bool MailRCPTState::ProduceMsg(Anything &context, std::ostream &os) {
	os << "RCPT TO: ";
	if (context.IsDefined("To")) {
		os << context["To"].AsCharPtr("?");
	} else {
		os << "Unknown"; // Should define better default sender
	}

	os << CRLF; //!@FIXME check whether this work always

	StatTrace(MailDAImpl.ProduceMsg, "Produce MailRCPTState " << context["To"].AsCharPtr("?"), Coast::Storage::Current());
	return (!!os);
}

// next state if the step succeeded
const char *MailRCPTState::NextOkState(Anything &result) {
	return "MailDATAState";
}

bool MailDATAState::ProduceMsg(Anything &context, std::ostream &os) {
	os << "DATA";
	os << CRLF;
	StatTrace(MailDAImpl.ProduceMsg, "Produce MailDATAState", Coast::Storage::Current());
	return (!!os);
}

const String MailSENDState::BOUNDARY("------MULTI-PART-DELIMITER---");
bool MailSENDState::ProduceMsg(Anything &context, std::ostream &os) {
	os << "From: ";
	os << context["From"].AsCharPtr("");
	os << "\nTo: ";
	if (context["To"].GetType() == AnyArrayType) {
		for (long i = 0, sz = context["To"].GetSize(); i < sz; ++i) {
			os << context["To"][i].AsCharPtr("");
			if (i < context["To"].GetSize() - 1) {
				os << ", ";
			}
		}
	} else {
		os << context["To"].AsCharPtr("");
	}
	os << "\nSubject: " << context["Subject"].AsCharPtr("");

	if (!context["Attachments"].IsNull()) {
		ProduceMultipartMsg(context, os);
	} else {
		os << "\n\n" << context["Message"].AsCharPtr("");
		// Append end of message sequence <CR><LF>.<CR><LF>
		os << CRLF;
		os << '.';
		os << CRLF;
		StatTrace(MailDAImpl.ProduceMsg, "Produce MailSENDState " << context["Message"].AsCharPtr(""), Coast::Storage::Current());
	}
	return (!!os);
}

void MailSENDState::ProduceMultipartMsg(Anything &context, std::ostream &os) {
	// create multipart message (i.e. header & body)

	// create header
	os << "\nMIME-version: 1.0";
	os << "\nContent-Type: multipart/mixed; boundary=" << BOUNDARY;

	os << "\n\nThis is a multipart message with attachments";

	os << "\n--" << BOUNDARY;
	os << "\nContent-Type: text/plain; charset=ISO-8859-1";
	os << "\nContent-transfer-encoding: 8bit";

	os << "\n\n" << context["Message"].AsCharPtr("");

	Anything attach = context["Attachments"];

	Encoder *base64 = Encoder::FindEncoder("Base64Regular");

	for (long i = 0, sz = attach.GetSize(); i < sz; ++i) {
		String name(attach[i]["FileName"].AsString("noname"));

		// the following might prove inefficient for large files...
		String rawdata(attach[i]["Data"].AsString(""));

		String encodeddata(rawdata.Length() * 4 / 3);
		base64->DoEncode(encodeddata, rawdata);

		os << "\n\n\n--" << BOUNDARY;
		os << "\nContent-Type: application/x-msdownload; name=\"" << name << "\"";
		os << "\nContent-transfer-encoding: base64";
		os << "\nContent-Disposition: attachment; filename=\"" << name << "\"";
		os << "\n";

		if (encodeddata.Length()) {
			os << "\n" << encodeddata;
		}
	}
	os << "\n\n\n--" << BOUNDARY;

	// Append end of message sequence <CR><LF>.<CR><LF>
	os << CRLF << '.' << CRLF;
}

bool MailERRORState::ProduceMsg(Anything &context, std::ostream &os) {
	os << "QUIT";
	os << CRLF;

	StatTrace(MailDAImpl.ProduceMsg, "Produce MailERRORState", Coast::Storage::Current());
	return (!!os);
}

SMTPState *MailERRORState::HandleState(Anything context, std::iostream &Ios) {
	DoProduceMsg(context, Ios);
	return SMTPState::FindSMTPState(NextOkState(context["Result"]));
}

bool MailQUITState::ProduceMsg(Anything &context, std::ostream &os) {
	os << "QUIT";
	os << CRLF;

	return (!!os);
}

SMTPState *MailQUITState::HandleState(Anything context, std::iostream &Ios) {
	DoProduceMsg(context, Ios);
	return SMTPState::FindSMTPState(NextOkState(context["Result"]));
}

RegCacheImpl(SMTPState)
//--- MailDAImpl -----------------------------------------------------
RegisterDataAccessImpl(MailDAImpl);

bool MailDAImpl::Exec(Context &ctx, ParameterMapper *in, ResultMapper *out) {
	StartTrace(MailDAImpl.Exec);

	// Config dynamically from MetaData
	String address = ctx.Lookup("Mailhost", "127.0.0.1");
	long port = ctx.Lookup("MailhostPort", 25L);
	long timeout = ctx.Lookup("Timeout", 10L) * 1000;

	bool result;
	Trace("Address<" << address << "," << port << ">");

	Connector csc(address, port, timeout);
	std::iostream *Ios = csc.GetStream();
	if (Ios) {
		result = SMTPState::SendMail(*Ios, ctx, in, out);
	} else {
		// Could not open Connection
		String errorMsg("Could not open Connection to ");
		errorMsg << address << " " << port;
		out->Put("Error", errorMsg, ctx);
		result = false;
	}
	return result;
}
