/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLParser_H
#define _HTMLParser_H

#ifndef HTMLParser_First
#define HTMLParser_First

#include "config_perftest.h"
#include "Anything.h"

typedef int Unicode;

enum EAlign {
	eLeft,
	eTop = eLeft,
	eCenter,
	eBase,
	eRight,
	eBottom = eRight,
	eJustified
};

//---- HTMLParser -----------------------------------------------------------

// extremely simple parser for HTML files: does not consider hierachical
// structure of HTML... only at the tag level... (special treatment of
// <SCRIPT> tag  and comments)
class EXPORTDECL_PERFTEST HTMLParser
{
public:

	enum TagType {
		eUNKNOWN = -1, eNONE = 0,

		// document structure
		eHTML, eBODY, eHEAD, eDOCTYPE, eBANNER, eDIV, eFOOTNOTE,

		// head section
		eTITLE,	eISINDEX, eMETA, eLINK,	eBASE, eSCRIPT, eSTYLE, eNEXTID,

		// block-level elements
		// headings
		eHEADER_1, eHEADER_2, eHEADER_3, eHEADER_4, eHEADER_5, eHEADER_6,
		// text containers
		ePARAGRAPH,	ePREFORMAT,	eBLOCKQUOTE, eADDRESS, eCREDIT,
		// lists
		eUNORDERED_LIST, eORDERED_LIST, eDIRECTORY, eMENU, eLIST_ITEM,
		eDEF_LIST, eDEF_TERM, eDEF_DEF, eLISTHEADER,
		// others
		eDIVISION, eCENTER, eHRULE,
		eFORM, eINPUT, eSELECT, eOPTION, eTEXTAREA,
		eTABLE, eCAPTION, eTABLECELL, eTABLEHEADER, eTABLEROW,
		eCOLUMN, eTABLEHEAD, eTBODY, eTABLEFOOTER,
		eFRAME, eFRAMESET,
		eNOTE,

		// text-level elements
		// logical markup
		eEMPHASIZED, eSTRONG, eDEFINITION, eCODE, eSAMPLE,
		eKEYBOARD, eVARIABLE, eCITATION, eCOMMAND, eARGUMENT,
		eACRONYM, eDELETED, ePERSON, eINSERTED, eAUTHOR,
		eABBREV, eQUOTATION_MARK, eLANGUAGE, eBLINK,
		// physical markup
		eTELETYPE, eITALIC, eUNDERLINE, eSTRIKEOUT, eBOLD,
		eBIG, eSMALL, eSUBSCRIPT, eSUPERSCRIPT,
		eFONT, eBASEFONT,
		// embedded object
		eEMBEDDED, eFIGURE, eOVERLAY, eIMAGE, eMATH, eOBJECT,
		// special markup
		eANCHOR,
		eAPPLET, ePARAM, eTEXTFLOW, eMARQUEE,
		eLINEBREAK, eNOBR, eWBR, eTAB,
		eMAP, eAREA
	};

	struct TagInfo {
		const char *fName;
#if defined(WIN32)
		TagType fCode;
#else
		HTMLParser::TagType fCode;
#endif
	};

	HTMLParser();

	virtual long IntParse();

	virtual Unicode IntGet();
	virtual void IntPutBack(Unicode c);

	virtual void IntPut(const String &);
	virtual void IntPut(Unicode c);
	void Put(const String &s);
	void PutSpecial(Unicode c, const String &name);
	virtual void IntComment(const String &comment);
	virtual void IntPushNode(Anything &);
	virtual void IntTag(int type, const char *tag);
	virtual void IntArgument(const String &key, const String &value);

	virtual void IntFlush();
	virtual void IntError(long line, const String &msg);

	static TagType LookupTag(const char *name);
	static Unicode LookupSpecial(const char *name);
	static EAlign LookupAlign(const char *value);
	static void VerifyTagTable();

protected:
	void ParseTag(int type, Unicode c);
	Unicode ParseName(char *name, Unicode c);
	bool TagHasArguments(const char *tag);
	void ParseComment();
	void ParseScript(int type, Unicode c, char *tag);
	void ParseArguments(const char *tag, Anything &node);
	void ParseUnstructuredArg(Anything &node);
	void ParseCharacterEntity();
	Unicode NextToken(String &token, bool withDelims = false, bool accEqual = false);

	void Exit();

	Unicode Get();

	void Error(const String &msg);

	long fLine;
private:
	long fExitCode;
	bool fExitParser;

	static TagInfo TagTable[];
};
//---- AAT_HTMLReader ---------------------------------------------------------------------------
class EXPORTDECL_PERFTEST AAT_HTMLReader
{
public:
	AAT_HTMLReader(istream *fp = 0);

	virtual Unicode Get();
	virtual void PutBack(char c);

protected:
	istream *fFile;

};
//---- AAT_HTMLWriter ---------------------------------------------------------------------------
class EXPORTDECL_PERFTEST AAT_HTMLWriter
{
public:
	AAT_HTMLWriter()
	{ }

	virtual void Put(char c);
	virtual void Put(Unicode c);
	virtual void Put(const String &);

	virtual void Flush();

	virtual void Tag(int type, const char *tag);
	virtual void Argument(const String &key, const String &value);
	virtual void PushNode(Anything &node);
	virtual void Comment(const String &comment);

	virtual void Error(long line, const String &msg);
};
//---- AAT_StdHTMLParser -----
class EXPORTDECL_PERFTEST AAT_StdHTMLParser : public HTMLParser
{
public:
	AAT_StdHTMLParser(AAT_HTMLReader &reader, AAT_HTMLWriter &writer);

protected:
	Unicode IntGet();
	void IntPutBack(Unicode c);
	void IntFlush();
	void IntPut(const String &);
	void IntPut(Unicode c);
	void IntComment(const String &comment);
	void IntPushNode(Anything &);
	void IntTag(int type, const char *tag );
	void IntArgument(const String &key, const String &value);
	void IntError(long line, const String &msg);

private:
	AAT_HTMLReader &fReader;
	AAT_HTMLWriter &fWriter;
};

// copy - paste - delete reuse of Juergens HTMLParser
// we don't need a reply here only some special tags like hrefs, imgs, forms ...
//---- MyHTMLWriter ---------------------------------------------------------------------------
class EXPORTDECL_PERFTEST MyHTMLWriter : public AAT_HTMLWriter
{

public:
	MyHTMLWriter( Anything &urls ) : fUrls(urls), fStoreTitle(0), fRequestFailed(0), fFormNr(-1), fTitle(""), fAllStringsInPage(""), fInScript(false)
	{ }
	virtual ~MyHTMLWriter() { }

	virtual void Put(char c);
	virtual void Put(Unicode c);
	virtual void Put(const String &);

	virtual void Flush();

	virtual void Tag(int type, const char *tag);
	virtual void Argument(const String &key, const String &value);
	virtual void PushNode(Anything &node);

	virtual void Comment(const String &comment);

	virtual void Error(long line, const String &msg);
	virtual bool GetReqFailed( void );

protected:
	void RenderImageTag(int t, const char *s, Anything &node, const char *wrap = "src");
	void RenderATag(int t, const char *s, Anything &node);
	void RenderFontTag(int t, const char *s, Anything &node);
	void RenderBASETag(int t, const char *s, Anything &node);
	void RenderScriptTag(int t, const char *s, Anything &node);
//	void RenderBodyTag(int t, const char*s, Anything &node);
	void RenderTitleTag(int t, const char *s, Anything &node);
	void RenderMetaTag(int t, const char *s, Anything &node);
	void RenderFrameTag(int t, const char *s, Anything &node);
//	void RenderFramesetTag(int t, const char*s, Anything &node);
	void RenderFormTag(int t, const char *s, Anything &node);
	void RenderInputTag(int t, const char *s, Anything &node);
	void RenderSelectTag(int t, const char *s, Anything &node);
//	bool Expand2AbsRequest(String &req, String &expanded);
//	void RenderPBoWHeader();
//	void RenderPBoWFooter();
//	void RenderImageLink(String &req);
protected:

	Anything &fUrls;
	bool fStoreTitle;
	bool fRequestFailed;
	long fFormNr; // form nesting level
	String fTitle;
	String fAllStringsInPage;
	bool fInScript;
	int fFramesetNr; // frameset nesting level (happens?)
	Anything fNodeStack;
};

#endif
#endif		//not defined _HTMLParser_H
