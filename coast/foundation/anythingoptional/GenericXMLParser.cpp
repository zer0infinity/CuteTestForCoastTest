/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "GenericXMLParser.h"
#include "Dbg.h"
#include "StringStream.h"
#include "SystemLog.h"

//---- GenericXMLParser ----------------------------------------------------------------
Anything GenericXMLParser::Parse(std::istream &reader, const char *filename, long startline, Allocator *a)
{
	StartTrace(GenericXMLParser.Parse);
	fReader = &reader;
	fFileName = filename;
	fLine = startline;
	fParseTree = Anything();
	fParseTree.SetAllocator(a);
	DoParse("", fParseTree);
	return fParseTree;
}

void GenericXMLParser::DoParse(String theTag, Anything &tag)
{
	StartTrace(GenericXMLParser.DoParse);
	int c, lookahead;
	Anything &tagbody = tag; // now just append, allow for different things like tagbody = tag["body"]
	String reply;

	while ((c = Get()) != 0 && c != EOF) {
		switch (c) {
			case '<':
				Store(tagbody, reply);
				lookahead = Peek();
				switch (lookahead) {
					case '!':
						Store(tagbody, reply);
						Store(tagbody, ParseCommentCdataOrDtd());
						break;
					case '/': {
						Get();
						String tagname = ParseName();
						c = Peek();
						if ('>' == c && tagname == theTag) {
							Get();
							return; // done
							//reply.Append("</").Append(tagname).Append(char(c));
						} else {
							// a potential syntax error...
							reply.Append("</").Append(tagname);
							String msg("Unexpected character <x");
							msg.AppendAsHex((unsigned char)c).Append('>');
							msg.Append(" or mismatched tag: ").Append(tagname);
							msg.Append(" expected: ").Append(theTag);
							Error(msg);
						}
						break;
					}
					case '?': {
						Store(tagbody, ParseXmlOrProcessingInstruction());
						break;
					}
					default:
						if (IsValidNameChar(lookahead)) {
							String tagname;
							Anything subTag;
							Anything attrs;
							bool hasbody = ParseTag(tagname, attrs);
							subTag[tagname] = attrs;
							if (hasbody) {
								DoParse(tagname, subTag);
							}
							Store(tagbody, subTag);
							//if (hasbody)
							//Store (ProcessTag(tagname,attributes));
							//else
							//Store(RenderTagAsLiteral(tagname,attributes));
						} else {
							// it cannot be a tag, so just append the '<'
							reply << (char)c;
						}
				}
				break;
			case '\x0D':// normalize line feeds
				if ('\x0A' == Peek()) {
					Get();
				}
				c = '\x0A' ;
				// Fall Through...
			default:
				reply << ((char)c);
		}
	}
	Store(tagbody, reply);
}
String GenericXMLParser::SkipToClosingAngleBracket()
{
	StartTrace(GenericXMLParser.SkipToClosingAngleBracket);
	int c;
	String result;
	while ((c = Get()) != 0 && c != '>' && c != EOF) {
		result.Append(char(c));
	}
	return result;
}
Anything GenericXMLParser::ParseXmlOrProcessingInstruction()
{
	StartTrace(GenericXMLParser.ParseXmlOrProcessingInstruction);
	int c;
	String key;
	key.Append((char)Get()); // this is always a '?'
	key.Append(ParseName());
	String value;
	while ((c = Get()) != 0 && c != EOF) {
		if ('?' == c && '>' == Peek()) {
			// done.
			Get();
			Anything result;
			result[key] = value;
			return result;
		}
		value.Append(char(c));
	}
	// this is an Error
	String msg("unexpected EOF in processing instruction ");
	msg.Append(key);
	Error(msg);
	SkipToClosingAngleBracket();
	return Anything();
}
Anything GenericXMLParser::ParseCommentCdataOrDtd(bool withindtd)
{
	StartTrace(GenericXMLParser.ParseCommentCdataOrDtd);
	int c;
	c = Get();// read the '!'
	String key("!");
	Anything result;
	switch (Peek()) {
		case '-':
			return ParseComment();
		case '[':
			if (!withindtd) {
				return ParseCdata();    // ignore include or ignore for the moment
			} else {
				goto error;
			}
		case 'D':
			if (!withindtd) {
				return ParseDtd();    // DOCTYPE
			} else {
				goto error;
			}
		case 'E': // ELEMENT or ENTITY
		case 'A': // ATTLIST
		case 'N': // NOTATION
			if (withindtd) {
				key.Append(ParseName());
				break;
			}
		default://lint !e616
		error: {
				String msg("unexpected character in <! element: ");
				msg.Append(char(c));
				Error(msg);
			}
	}
	result[key] = SkipToClosingAngleBracket();
	return result;
}
Anything GenericXMLParser::ParseDtd()
{
	StartTrace(GenericXMLParser.ParseDtd);
	// we got already <! following a D
	String doctype("!");
	doctype.Append(ParseName());
	Anything result;
	Anything externalid;
	if (doctype != "!DOCTYPE") {
		Error("invalid <! tag");
		result[doctype] = SkipToClosingAngleBracket(); // best effort
		return result;
	}
	SkipWhitespace();
	String rootnodename = ParseName();
	result[doctype] = rootnodename;
	SkipWhitespace();
	int c = Peek();
	if ('S' == c || 'P' == c) {
		externalid = ParseExternalId();
	}
	result.Append(externalid);
	SkipWhitespace();
	if ('[' == Peek()) {
		result.Append(ParseDtdElements());
	}
	SkipWhitespace();
	if ('>' != Peek()) {
		Error("DTD syntax error");
		result.Append(SkipToClosingAngleBracket());
	} else {
		Get(); // read the end >
	}
	return result;
}
Anything GenericXMLParser::ParseExternalId()
{
	StartTrace(GenericXMLParser.ParseExternalId);
	String systemOrPublic = ParseName();
	SkipWhitespace();
	Anything result;
	if (systemOrPublic == "PUBLIC") {
		result[systemOrPublic] = ParseQuotedString();
	}
	result[systemOrPublic].Append(ParseQuotedString());
	return result;
}
Anything GenericXMLParser::ParseDtdElements()
{
	StartTrace(GenericXMLParser.ParseDtdElements);
	Anything result;
	int c = Get();// read the [
	while (!IsEof() && ']' != c) {
		SkipWhitespace();
		c = Get();
		switch (c) {
			case '%' : { // a PE Reference
				String peref("%");
				peref.Append(ParseToSemicolon());
				peref.Append(';');
				result.Append(peref);
				break;
			}
			case '<': // a processing instruction, comment or dtd decl
				switch (Peek()) {
					case '?':
						result.Append(ParseXmlOrProcessingInstruction());
						break;
					case '!': // a comment only allowed
						result.Append(ParseCommentCdataOrDtd(true));
						break;
					default:
						Error("invalid < character seqence within DTD");
						result.Append(SkipToClosingAngleBracket());
						break;
				}
				break;
			case ']':
				break; // done
			default:
				Error("invalid character within DTD ignored");
				break;

		}
	}
	return result;
}
Anything GenericXMLParser::ParseCdata()
{
	StartTrace(GenericXMLParser.ParseCdata);
	String key("![");
	Anything result;
	Get(); // read '['
	SkipWhitespace();
	key.Append(ParseName());
	SkipWhitespace();
	if ('[' == Get()) {
		if ("![CDATA" == key) {
			result[key] = SkipToCdataClosing(); // ]]>
			return result;
		}
//		else if ("![INCLUDE" == key)
//		{
//		}
//		else if ("![IGNORE" == key)
//		{
//		}
		else {
			String msg("wrong key (not CDATA) in <![:");
			msg.Append(key);
			Error(msg);
		}
	} else {
		Error("unexpected characer in <![ (not [)");
	}
	result[key] = SkipToClosingAngleBracket();
	return result;
}
String GenericXMLParser::SkipToCdataClosing()
{
	StartTrace(GenericXMLParser.SkipToCdataClosing);
	String result;
	int c;
	while ((c = Get()) != 0 && c != EOF) {
		if (']' == c) {
			if (']' == Peek()) {
				// yes, ]] found
				Get(); // read "]]"
				while (']' == (c = Get())) { // skip and remember exceeding ]
					result.Append(char(c));
				}
				if ('>' == c) {
					// done, found "]]>"
					return result;
				} else {
					// not done, just found "]]"
					result.Append(']').Append(']');
				}
			}
		}
		result.Append(char(c));
	}
	Error("unexpected EOF in <![CDATA[");
	return result;
}
Anything GenericXMLParser::ParseComment()
{
	StartTrace(GenericXMLParser.ParseComment);
	int c;
	// keep comments for test cases relying on them
	String comment;
#define GSR ((c = Get()),comment.Append(char(c)))

	c = Get();
	if (c == '-') {
		c = Get();
		if (c == '-') {
			// now we are inside a comment

			// skip whitespace
			for (;;) {
				GSR;
				if (!isspace( (unsigned char) c)) {
					break;
				}
			}
			// skip comment
			for (;;) {
				if (c == EOF || c == 0) {
					break;
				}
				if (c == '-') {
					GSR;
					if (c == '-') {
						GSR;
						while ( c == '-' ) {
							GSR;
						}
						if (c == '>') {
							// end of comment
							break;
						}
					}
				}
				GSR;
			}
			comment.Trim(comment.Length() - 3); // cut -->
			Anything result;
			result["!--"] = comment; // /"!--" marks the comment
			return result;
		}
	}
	String msg("Unexpected character or EOF in Comment: ");
	msg.Append((char)c);
	Error(msg);
	if ('>' != c) {
		comment.Append(SkipToClosingAngleBracket());
	}
	Anything result;
	result["!--"] = comment;
	return result;
}

Anything GenericXMLParser::ProcessArgs(const String &renderer, const String &args)
{
	StartTrace1(GenericXMLParser.ProcessArgs, "renderer: <" << renderer << ">");
	Anything aargs;
	Anything result;
	if (args.Length() > 0) {
		Trace("parsing args:" << args);
		Anything readit;
		IStringStream is(args);
		readit.Import(is);
		// now we need to adjust it to become an array if it isn't already:
		if (AnyArrayType != readit.GetType()) {
			aargs[0L] = readit;
		} else {
			aargs = readit;
		}
	}
//!@FIXME
	result[renderer] = aargs;
	TraceAny(aargs, "aargs");
	return result;
}

void GenericXMLParser::Store(Anything &cache, String &literal)
{
	StartTrace(GenericXMLParser.Store);
	if (literal.Length() > 0) {
		cache.Append(Anything(literal));
		literal = "";
	}
}
void GenericXMLParser::Store(Anything &cache, const String &literal)
{
	StartTrace(GenericXMLParser.Store);
	if (literal.Length() > 0) {
		cache.Append(Anything(literal));
	}
}

void GenericXMLParser::Store(Anything &cache, const Anything &body)
{
	StartTrace(GenericXMLParser.Store2);
	if (!body.IsNull()) {
		cache.Append(body);
	}
}

void GenericXMLParser::SkipWhitespace()
{
	StartTrace(GenericXMLParser.SkipWhitespace);
	int c;
	while (!IsEof() && (c = Peek()) != 0 && c != EOF) {
		if (!isspace(c)) {
			return;
		}
		c = Get();
	}
}
bool GenericXMLParser::IsValidNameChar(int c)
{
	StartTrace(GenericXMLParser.IsValidNameChar);
	return (isalpha(c) || '-' == c || '_' == c || '.' == c || ':' == c);
}
String GenericXMLParser::ParseToSemicolon()
{
	StartTrace(GenericXMLParser.ParseToSemicolon);
	String value;
	int c;
	while (!IsEof() && ';' != (c = Get())) {
		value.Append(char(c));
	}
	return value;
}

String GenericXMLParser::ParseName()
{
	StartTrace(GenericXMLParser.ParseName);
	String theName;
	int c; // Unicode?
	while (!IsEof() && (c = Peek()) != EOF && c != 0) {
		if (IsValidNameChar(c)
			|| (isdigit(c) && theName.Length() > 0)) {
			theName.Append((char)Get());
		} else {
			break;
		}
	}
	return theName;
}

int GenericXMLParser::Get()
{
	StartTrace(GenericXMLParser.Get);
	int c = 0;
	if (fReader && !fReader->eof()) {
		c = fReader->get();
		if ('\n' == c) {
			++fLine;
		}
	}
	return c;
}
int GenericXMLParser::Peek()
{
	StartTrace(GenericXMLParser.Peek);
	if (fReader && !fReader->eof()) {
		return fReader->peek();
	}
	return 0;
}
void GenericXMLParser::PutBack(char c)
{
	StartTrace(GenericXMLParser.PutBack);
	if (fReader) {
		fReader->putback(c);
	}
	if ('\n' == c) {
		--fLine;
	}
}

bool GenericXMLParser::IsEof()
{
	StartTrace(GenericXMLParser.IsEof);
	return !fReader || !fReader->good();
}

bool GenericXMLParser::ParseTag(String &tag, Anything &tagAttributes)
{
	StartTrace(GenericXMLParser.ParseTag);
	tag = ParseName();
	Trace("tag = " << tag);
	while (!IsEof()) {
		SkipWhitespace();
		int c = Peek();
		switch (c) {
			case '>': // done with tag
				c = Get();
				return true;//lint !e438
			case '/': // an empty tag? i.e. <br />
				c = Get();
				if ('>' == Peek()) {
					c = Get();
					return false;//lint !e438
				}
				// an error occured, ignore '/' silently
				PutBack(c);
			default: {//lint !e616
				String name;
				String value;
				if (ParseAttribute(name, value)) {
					tagAttributes[name] = value;
				} else {
					// non-well formed XML...no value given
					if (name.Length() > 0) {
						tagAttributes.Append(name);
					} else {
						String msg("Unexpected character <");
						c = Get();
						msg.AppendAsHex((unsigned char)c).Append("> near ").Append(tag);
						Error(msg);
						tagAttributes.Append(String(char(c)));
					}
				}
			}
		}
	}
	Error("unexpected EOF in Tag");
	return false; // no body to expect
}
bool GenericXMLParser::ParseAttribute(String &name, String &value)
{
	StartTrace(GenericXMLParser.ParseAttribute);
	name = ParseName();
	name.ToLower(); // XHTML conformance all attribute names are lower case
	Trace("attribute name:" << name);
	value = "";
	SkipWhitespace();
	int c = Peek();
	if (c != '=') {
		return false;
	}
	c = Get();
	(void)c;
	SkipWhitespace();
	value = ParseValue();
	return true;
	// otherwise it is a syntax error, we just ignore silently for now.
}
String GenericXMLParser::ParseQuotedString()
{
	StartTrace(GenericXMLParser.ParseValue);
	String value;
	SkipWhitespace();
	int	quote = Get();
	int c;
	while (!IsEof() && (c = Get()) != quote) {
		value.Append((char)c);
	}
	return value;
}
String GenericXMLParser::ParseValue()
{
// Might need some fine tuning. allow non-well-formed i.e. unquoted values
	StartTrace(GenericXMLParser.ParseValue);
	String value;
	int c = Peek();
	if ('\'' == c || '\"' == c) { // a quoted value
		value = ParseQuotedString();
	} else if (IsValidNameChar(c)) { // a legacy non-quoted value
		value = ParseName();
	} else if (isdigit(c)) {
		do {
			value.Append(char(Get()));
			c = Peek();
		} while (isdigit(c) || '%' == c); // allow for legacy percentage values, without quotes
	} else if ('#' == c) { // allow for legacy color attributes without quotes
		do {
			value.Append(char(Get()));
		} while (isxdigit(Peek()));
	}
	return value;
}

String GenericXMLParser::ParseAsStringUpToEndTag(String &tagName)
{
	StartTrace(GenericXMLParser.ParseAsStringUpToEndTag);
	String result;
	int c;
	while (!IsEof() && (c = Get()) != 0) {
		if ('<' == c) {
			if ('/' == Peek()) {
				c = Get();
				String foundname = ParseName();
				if (String::CaselessCompare(tagName, foundname) == 0 && '>' == Peek()) {
					Get();
					return result;
				}
				result.Append("</");
				result.Append(foundname);
				continue;
			}
		}
		result.Append(char(c));
	}
	Error("unexpected EOF");
	return result;
}

void GenericXMLParser::Error(const char *msg)
{
	StartTrace(GenericXMLParser.Error);
	String m(" ");
	m.Append(fFileName).Append(".xml:").Append(fLine).Append(" ").Append(msg);
	SystemLog::Warning(m);
	fParseTree["Errors"].Append(m);
	Trace(m);
	m << "\n";
	SystemLog::WriteToStderr(m);
}
void GenericXMLPrinter::PrintXml(std::ostream &os, ROAnything domany)
{
	for (long i = 0, sz = domany.GetSize(); i < sz; ++i) {
		if (!domany.SlotName(i)) {
			// do not go down for /Errors slot
			DoPrintXml(os, domany[i]);
		}
	}
}
void GenericXMLPrinter::DoPrintXml(std::ostream &os, ROAnything subdomany)
{
	if (subdomany.GetType() == AnyArrayType) {
		String tag = subdomany.SlotName(0L);
		if (tag.Length()) {
			if ('?' == tag[0L]) {
				DoPrintXmlPI(os, tag, subdomany);
			} else if (tag == "!--") {
				DoPrintXmlComment(os, subdomany);
			} else if (tag == "![CDATA") {
				DoPrintXmlCdata(os, subdomany);
			} else if (tag == "!DOCTYPE") {
				DoPrintXmlDtd(os, subdomany);
			} else {
				DoPrintXmlTag(os, tag, subdomany[0L]);
				for (long i = 1, sz = subdomany.GetSize(); i < sz; ++i) {
					DoPrintXml(os, subdomany[i]);
				}
				os << "</" << tag << '>';
			}
		} else {
			// this is an error...
			os << "<!-- error in DOM anything input, missing tag -->";
		}
	} else if (subdomany.GetType() != AnyNullType) {
		os << subdomany.AsString();
	}
}
void GenericXMLPrinter::DoPrintXmlTag(std::ostream &os, const String &tag, ROAnything attributes)
{
	os << '<' << tag ;
	for (long i = 0L, sz = attributes.GetSize(); i < sz; ++i) {
		os << ' ' << attributes.SlotName(i) << "=\"" << attributes[i].AsString() << '"';
	}
	os << '>';
}

void GenericXMLPrinter::DoPrintXmlPI(std::ostream &os, const String &pitag, ROAnything subdomany)
{
	os << '<' << pitag << subdomany[0L].AsString() << "?>";
	if (subdomany.GetSize() > 1L) {
		os << "<!-- error in DOM anything input, processing instruction additional slots -->";
	}
}
void GenericXMLPrinter::DoPrintXmlComment(std::ostream &os, ROAnything subdomany)
{
	os << "<!--" << subdomany[0L].AsString() << "-->";
	if (subdomany.GetSize() > 1L) {
		os << "<!-- error in DOM anything input, comment additional slots -->";
	}
}
void GenericXMLPrinter::DoPrintXmlCdata(std::ostream &os, ROAnything subdomany)
{
	os << "<![CDATA[" << subdomany[0L].AsString() << "]]>";
	if (subdomany.GetSize() > 1L) {
		os << "<!-- error in DOM anything input, CDATA additional slots -->";
	}
}
void GenericXMLPrinter::DoPrintXmlDtd(std::ostream &os, ROAnything subdomany)
{
	os << "<!DOCTYPE " << subdomany[0L].AsString(); // root node
	if (!subdomany[1L].IsNull()) {
		os << " " << subdomany[1L].SlotName(0L); // public or system
		os << " \"" << subdomany[1L][0L].AsString() << "\" ";
		if (subdomany[1L].GetSize() >= 2) {
			os << " \"" << subdomany[1L][1L].AsString() << "\" ";
		}
	}
	if (subdomany.GetSize() <= 2L) {
		os << '>'; // done, external id
	} else { // DTD elements given
		os << " [";
		for (long i = 0L, sz = subdomany[2L].GetSize(); i < sz; ++i) {
			DoPrintXmlSubDtd(os, subdomany[2L][i]);
		}
		os << "]>";
	}
}

void GenericXMLPrinter::DoPrintXmlSubDtd(std::ostream &os, ROAnything subdomany)
{
	if (subdomany.GetType() == AnyArrayType) {
		String tag = subdomany.SlotName(0L);
		if (tag.Length()) {
			if ('?' == tag[0L]) {
				DoPrintXmlPI(os, tag, subdomany);
			} else if (tag == "!--") {
				DoPrintXmlComment(os, subdomany);
			} else if (tag[0L] == '!') {
				os << '<' << tag << subdomany[0L].AsString() << '>';
			} else {
				os << "<!-- error in DOM DTD anything input, wrong tag -->";
			}
		} else {
			// this is an error...
			os << "<!-- error in DOM anything input, missing tag -->";
		}
	} else if (subdomany.GetType() != AnyNullType) {
		os << subdomany.AsString();
	}
}
