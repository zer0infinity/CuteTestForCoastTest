/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _GenericXMLParser_H
#define _GenericXMLParser_H

//---- GenericXMLParser ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "Anything.h"
//---- forward declaration -----------------------------------------------

//! <B>construct a simple DOM representation using Anything from an XML file</B>
/*!
construct an anything representation of DOM parsing the XML
*/
class EXPORTDECL_FOUNDATION GenericXMLParser
{
public:
	GenericXMLParser() {}
	virtual ~GenericXMLParser() {}
	//! do the parsing,
	//! \return the constructed Anything using the given Allocator
	//! \param reader the input source
	//! \param filename for giving convenient error messages when reading from a real file
	//! \param startline the line number when starting the parsing for convenient error messages
	//! \param a the allocator to use, provide Storage::Global() for config data
	Anything Parse(istream &reader, const char *filename = "NO_FILE", long startline = 1L, Allocator *a = Storage::Current());
protected:
	virtual void DoParse(String endTag, Anything &tag);
	virtual Anything ParseComment();
	virtual Anything ParseXmlOrProcessingInstruction();
	virtual bool ParseTag(String &tag, Anything &attributes);
	virtual String ParseValue();
	virtual bool ParseAttribute(String &name, String &value);
	virtual Anything ParseCommentCdataOrDtd(bool withindtd = false);
	virtual Anything ParseDtd();
	virtual Anything ParseExternalId();
	virtual Anything ParseDtdElements();
	virtual String SkipToClosingAngleBracket();
	virtual Anything ParseCdata();
	virtual String SkipToCdataClosing();

	virtual String ParseAsStringUpToEndTag(String &tagName);
	virtual void Error(const char *msg);

	virtual Anything ProcessArgs(const String &renderer, const String &args);
	virtual void Store(Anything &cache, String &htmlBlock);
	virtual void Store(Anything &cache, const String &htmlBlock);
	virtual void Store(Anything &cache, const Anything &args);

	virtual String ParseName();
	virtual String ParseQuotedString();
	virtual	String ParseToSemicolon();

	virtual void SkipWhitespace();
	virtual bool IsValidNameChar(int c);

	virtual int Get();
	virtual int Peek();
	virtual bool IsEof();
	virtual void PutBack(char c);
	Anything fParseTree;
	istream *fReader;
	String	fFileName;
	long	fLine;
};

class EXPORTDECL_FOUNDATION GenericXMLPrinter
{
public:
	static void PrintXml(ostream &os, ROAnything domany);
	static void DoPrintXml(ostream &os, ROAnything domany);
	static void DoPrintXmlTag(ostream &os, const String &tag, ROAnything attributes);
	static void DoPrintXmlPI(ostream &os, const String &pitag, ROAnything subdomany);
	static void DoPrintXmlComment(ostream &os, ROAnything subdomany);
	static void DoPrintXmlCdata(ostream &os, ROAnything subdomany);
	static void DoPrintXmlDtd(ostream &os, ROAnything subdomany);
	static void DoPrintXmlSubDtd(ostream &os, ROAnything subdomany);
};

#endif
