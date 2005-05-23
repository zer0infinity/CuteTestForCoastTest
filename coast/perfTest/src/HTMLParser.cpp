/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

// if &copy etc is to be translated to local character set
// SPECIAL_CHAR_CONVERSION to be defined
#undef CONTROL_CHAR_CHECK // iscntrl is locale specific, it is avoided if this define is set
#define SPECIAL_CHAR_CONVERSION // should you need to turn conversion of &copy etc into local characters

//--- interface include --------------------------------------------------------
#include "HTMLParser.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "SysLog.h"
#include "DiffTimer.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
#include <ctype.h>
#endif

class MethodInfoCollector
{
public:
	MethodInfoCollector();
	~MethodInfoCollector();

	Anything fResults;
};

class MethodMeter
{
public:
	MethodMeter(const char *, Anything &result);
	~MethodMeter();

protected:
	Anything &fResult;
	const char *fName;
	DiffTimer fTimer;
};

static MethodInfoCollector gMeter;

//#define METHOD_METER
#ifdef METHOD_METER
#define Meter(key)	MethodMeter mme(_QUOTE_(key), gMeter.fResults);
#else
#define Meter(key)
#endif

//---- Lookup Tags -----------------------------------------------------------------------------
HTMLParser::TagInfo HTMLParser::TagTable[] = {		// keep table sorted!
	{ "a", 		HTMLParser::eANCHOR },
	{ "abbrev",	HTMLParser::eABBREV },
	{ "acronym", HTMLParser::eACRONYM },
	{ "address", HTMLParser::eADDRESS },
	{ "applet",	HTMLParser::eAPPLET },
	{ "area",	HTMLParser::eAREA },		// ??
	{ "arg",	HTMLParser::eARGUMENT },
	{ "au",		HTMLParser::eAUTHOR },
	{ "b",		HTMLParser::eBOLD },
	{ "banner", HTMLParser::eBANNER },
	{ "base", 	HTMLParser::eBASE },
	{ "basefont", 	HTMLParser::eBASEFONT },
	{ "big", 	HTMLParser::eBIG },
	{ "blink",	HTMLParser::eBLINK },
	{ "blockquote", HTMLParser::eBLOCKQUOTE },	// ??
	{ "body",	HTMLParser::eBODY },
	{ "bq",		HTMLParser::eBLOCKQUOTE },
	{ "br", 	HTMLParser::eLINEBREAK },
	{ "caption", HTMLParser::eCAPTION },
	{ "center",	HTMLParser::eCENTER },	// ??
	{ "cite",	HTMLParser::eCITATION },
	{ "cmd",	HTMLParser::eCOMMAND },
	{ "code",	HTMLParser::eCODE },
	{ "col",	HTMLParser::eCOLUMN },
	{ "credit",	HTMLParser::eCREDIT },
	{ "dd", 	HTMLParser::eDEF_DEF },
	{ "del", 	HTMLParser::eDELETED },
	{ "dfn", 	HTMLParser::eDEFINITION },
	{ "dir", 	HTMLParser::eDIRECTORY },	// ??
	{ "div",	HTMLParser::eDIV },
	{ "division", HTMLParser::eDIVISION },	// ??
	{ "dl", 	HTMLParser::eDEF_LIST },
	{ "doctype", HTMLParser::eDOCTYPE },
	{ "dt", 	HTMLParser::eDEF_TERM },
	{ "em",		HTMLParser::eEMPHASIZED },
	{ "embed",	HTMLParser::eEMBEDDED },
	{ "fig",	HTMLParser::eFIGURE },
	{ "fn",		HTMLParser::eFOOTNOTE },
	{ "font",	HTMLParser::eFONT },
	{ "form", 	HTMLParser::eFORM },
	{ "frame", 	HTMLParser::eFRAME },
	{ "frameset",	HTMLParser::eFRAMESET },
	{ "h1",		HTMLParser::eHEADER_1 },
	{ "h2",		HTMLParser::eHEADER_2 },
	{ "h3",		HTMLParser::eHEADER_3 },
	{ "h4",		HTMLParser::eHEADER_4 },
	{ "h5",		HTMLParser::eHEADER_5 },
	{ "h6",		HTMLParser::eHEADER_6 },
	{ "head", 	HTMLParser::eHEAD },
	{ "hr", 	HTMLParser::eHRULE },
	{ "html",	HTMLParser::eHTML },
	{ "i",		HTMLParser::eITALIC },
	{ "img", 	HTMLParser::eIMAGE },
	{ "input", 	HTMLParser::eINPUT },
	{ "ins", 	HTMLParser::eINSERTED },
	{ "isindex", HTMLParser::eISINDEX },
	{ "kbd",	HTMLParser::eKEYBOARD },
	{ "lang", 	HTMLParser::eLANGUAGE },
	{ "lh", 	HTMLParser::eLISTHEADER },
	{ "li", 	HTMLParser::eLIST_ITEM },
	{ "link",	HTMLParser::eLINK },
	{ "listing", HTMLParser::ePREFORMAT },	// ??
	{ "map", 	HTMLParser::eMAP },			// ??
	{ "marquee", HTMLParser::eMARQUEE },
	{ "math", 	HTMLParser::eMATH },
	{ "menu", 	HTMLParser::eMENU },		// obsolete
	{ "meta", 	HTMLParser::eMETA },		// ??
	{ "nextid", HTMLParser::eNEXTID },
	{ "nobr", 	HTMLParser::eNOBR },
	{ "note", 	HTMLParser::eNOTE },
	{ "object", HTMLParser::eOBJECT },
	{ "ol", 	HTMLParser::eORDERED_LIST },
	{ "option", HTMLParser::eOPTION },
	{ "overlay", HTMLParser::eOVERLAY },
	{ "p", 		HTMLParser::ePARAGRAPH },
	{ "param",	HTMLParser::ePARAM },
	{ "person",	HTMLParser::ePERSON },
	{ "pre", 	HTMLParser::ePREFORMAT },
	{ "q", 		HTMLParser::eQUOTATION_MARK },
	{ "samp",	HTMLParser::eSAMPLE },
	{ "script", HTMLParser::eSCRIPT },		// ??
	{ "select", HTMLParser::eSELECT },
	{ "small",	HTMLParser::eSMALL },
	{ "strike", HTMLParser::eSTRIKEOUT },
	{ "strong", HTMLParser::eSTRONG },
	{ "style", 	HTMLParser::eSTYLE },		// ??
	{ "sub", 	HTMLParser::eSUBSCRIPT },
	{ "sup", 	HTMLParser::eSUPERSCRIPT },
	{ "tab",	HTMLParser::eTAB },
	{ "table",	HTMLParser::eTABLE },
	{ "tbody",	HTMLParser::eTBODY },
	{ "td",		HTMLParser::eTABLECELL },
	{ "textarea", HTMLParser::eTEXTAREA },
	{ "textflow", HTMLParser::eTEXTFLOW },	// ??
	{ "tfoot", 	HTMLParser::eTABLEFOOTER },	// ??
	{ "th",		HTMLParser::eTABLEHEADER },
	{ "thead",	HTMLParser::eTABLEHEAD },
	{ "title",	HTMLParser::eTITLE },
	{ "tr", 	HTMLParser::eTABLEROW },
	{ "tt", 	HTMLParser::eTELETYPE },
	{ "u",		HTMLParser::eUNDERLINE },
	{ "ul", 	HTMLParser::eUNORDERED_LIST },
	{ "var",	HTMLParser::eVARIABLE },
	{ "wbr", 	HTMLParser::eWBR },
	{ "xmp",	HTMLParser::ePREFORMAT },	// ??
	{ 0,		HTMLParser::eUNKNOWN }
};

//---- special characters ---------------------------------------------------------------

static struct SpecialCharInfo {
	const char *fName;
	unsigned char fCode;
} SpecialCharTable[] = {
	{ "AElig", 	198 },
	{ "Aacute", 193 },
	{ "Acirc", 	194 },
	{ "Agrave", 192 },
	{ "Aring", 	197 },
	{ "Atilde", 195 },
	{ "Auml", 	196 },
	{ "Ccedil", 199 },
	{ "ETH", 	208 },
	{ "Eacute", 201 },
	{ "Ecirc", 	202 },
	{ "Egrave", 200 },
	{ "Euml", 	203 },
	{ "Iacute", 205 },
	{ "Icirc", 	206 },
	{ "Igrave", 204 },
	{ "Iuml", 	207 },
	{ "Ntilde", 209 },
	{ "Oacute", 211 },
	{ "Ocirc", 	212 },
	{ "Ograve", 210 },
	{ "Oslash", 216 },
	{ "Otilde", 213 },
	{ "Ouml", 	214 },
	{ "THORN", 	222 },
	{ "Uacute", 218 },
	{ "Ucirc", 	219 },
	{ "Ugrave", 217 },
	{ "Uuml", 	220 },
	{ "Yacute", 221 },
	{ "aacute", 225 },
	{ "acirc", 	226 },
	{ "aelig", 	230 },
	{ "agrave", 224 },
	{ "amp", 	38 },
	{ "aring", 	229 },
	{ "atilde", 227 },
	{ "auml", 	228 },
	{ "ccedil", 231 },
	{ "copy", 	169 },
	{ "eacute", 233 },
	{ "ecirc", 	234 },
	{ "egrave", 232 },
	{ "eth", 	240 },
	{ "euml", 	235 },
	{ "gt", 	62 },
	{ "iacute", 237 },
	{ "icirc", 	238 },
	{ "igrave", 236 },
	{ "iuml", 	239 },
	{ "lt", 	60 },
	{ "nbsp", 	32 },
	{ "ntilde", 241 },
	{ "oacute", 243 },
	{ "ocirc", 	244 },
	{ "ograve", 242 },
	{ "oslash", 248 },
	{ "otilde", 245 },
	{ "ouml", 	246 },
	{ "quot", 	34 },
	{ "reg", 	174 },
	{ "shyp", 	45 },
	{ "szlig", 	223 },
	{ "thorn", 	254 },
	{ "uacute", 250 },
	{ "ucirc", 	251 },
	{ "ugrave", 249 },
	{ "uuml", 	252 },
	{ "yacute", 253 },
	{ "yuml", 	255 }
};

//---- Aligns -------------------------------------------------------

static struct Aligns {
	const char *tag;
	EAlign align;
} AlignTags[] = {
	{ "top",		eTop },
	{ "texttop",	eTop },
	{ "middle",		eCenter },
	{ "absmiddle",	eCenter },
	{ "bottom",		eBottom },
	{ "baseline",	eBase },
	{ "absbottom",	eBottom },
	{ "center",		eCenter },
	{ "right",		eRight },
	{ "left",		eLeft },
	{ 0, 			eTop }
};

//---- HTMLParser -----------------------------------------------------------
HTMLParser::HTMLParser()
	: fLine(1), fExitParser(false)
{
}

void HTMLParser::Exit()
{
	fExitParser = true;
}

void HTMLParser::Put(const String &s)
{
	Meter(HTMLParser.PutString);
	//	int c;
	IntPut(s);
}
void HTMLParser::PutSpecial(Unicode c, const String &)
{
	Meter(HTMLParser.PutSpecial);
	IntPut(c);
}

Unicode HTMLParser::Get()
{
	Meter(HTMLParser.Get);
	Unicode c = IntGet();
	if (c == '\015') {	// CR
		Unicode cc = IntGet();
		if (cc != '\012') {	// LF
			IntPutBack(cc);
		}
		c = '\n';
	}
	if (c == '\012') {
		fLine++;
	}
	return c;
}

Unicode HTMLParser::ParseName(char *name, Unicode c)
{
	Meter(HTMLParser.ParseName);
	StartTrace(HTMLParser.ParseName);
	for (int i = 0; ; i++) {
		if (i >= 72) {
			Error(String("name too long (> 72)") << (char)c);
		} else {
			*name++ = tolower(c);
		}
		c = Get();
		if (c == EOF) {
			Error("unexpected EOF in name");
			break;
		}
		if (! (isalnum(c) || c == '.' || c == '-' || c == '_')) {	// XXX added "_"
			break;
		}
	}
	while (isspace(c)) {
		c = Get();
	}
	*name = '\0';
	return c;
}

void HTMLParser::ParseComment()
{
	Meter(HTMLParser.ParseComment);
	char c;
	String comment, endtry;

	for (;;) {
		c = Get();
		if (c == '-') {
			endtry.Append(c);
			c = Get();
			if (c == '-') {
				endtry.Append(c);
				c = Get();
				while ( c == '-' ) {
					comment.Append('-');
					c = Get();
				}
				if (c == '>') {
					break;
				} else {
					endtry.Append(c);
				}
			}
		}
		if (c == EOF) {
			Error("unexpected EOF in comment");
			break;
		}
		if (endtry.Length() > 0) {
			comment.Append(endtry);
			endtry = "";
		}
		comment.Append((char)c);
	}
	IntComment(comment);
}

bool HTMLParser::TagHasArguments(const char *tag)
{
	Meter(HTMLParser.TagHasArguments);
	if (strcmp(tag, "doctype") == 0) {
		return false;
	}
	return true;
}

void HTMLParser::ParseTag(int type, Unicode c)
{
// PS: limit is checked in ParseName (<72)
	Meter(HTMLParser.ParseTag);
	StartTrace(HTMLParser.ParseTag);
	MetaThing node;

	if (type != '/') {
		// if not an end tag, increase stack size in ..Writer
		IntPushNode(node);
	}
	char tag[80];

	c = ParseName(tag, c);

	if (c != '>') {
		if (type == '/') {
			Error("expected > at end of tag");
		}
		IntPutBack(c);

		ParseArguments(tag, node);
	}

	IntTag(type, tag );

	if (strcmp(tag, "script") == 0)	{// special treatment of scripts
		ParseScript(type, c, tag);
	}
}

void HTMLParser::ParseScript(int /* type */, Unicode c, char *tag)
{
	Meter(HTMLParser.ParseScript);
	// parse the remainer of the script (incl. end tag)
	for (;;) {
		switch (c = Get()) {

			case EOF:
				goto out2;
			case '<':					// find </SCRIPT> tag
				c = Get();
				switch (c) {
					case EOF:
						goto out2;
					case '/':
						c = Get();
						if (isalpha(c)) {
							c = ParseName(tag, c);
							Anything dummy;
							IntTag('/', tag );

							if (strcmp(tag, "script") == 0) {	// done with script
								goto out2;
							}

						} else {
							// text
							IntPut('<');
							IntPut('/');
							IntPutBack(c);
						}
						break;
					default:
						IntPut('<');
						IntPutBack(c);
						break;
				}
				break;
			default:
				// copy anything between the <SCRIPT> tags
				IntPut(c);
		}
	}
out2:
	return;
}

void HTMLParser::ParseCharacterEntity()
{
	// parse number and special character formats

	Meter(HTMLParser.ParseCharacterEntity);
	Unicode c = Get();

	switch (c) {
			// end of file
		case EOF:
			break;

			// a number format
		case '#':
			c = Get();
			if (isdigit(c)) {
				String num;
				for (;;) {
					num.Append((char)c);
					c = Get();
					if (!isdigit(c)) {
						if (c != ';') {
							IntPutBack(c);
						}
						break;
					}
				}
				PutSpecial(num.AsLong(-1), num);
			} else {
				// no number follows quote #
				IntPut('&');
				IntPut('#');
				IntPutBack(c);
			}
			break;

		default:
			if (isalpha(c)) {
				Unicode cc = ' ';
				String name;

				for (;;) {
					name.Append((char)c);

					c = Get();
					if (!isalpha(c)) {
						if (c != ';') {
							IntPutBack(c);
						} else {
							if (name.Length() >= 2) {
								cc = LookupSpecial(name);
							}
						}
						break;
					}
				}
				if (cc < 0) {
					SysLog::WriteToStderr(String("can't find special character: ") << name << "\n");
					IntPut('&');
					Put(name);
					if (c == ';') {
						IntPut(c);
					}
				} else {
					PutSpecial(cc, name);
				}
			} else {
				IntPut('&');
				IntPutBack(c);
			}
			break;
	}
}

long HTMLParser::IntParse()
{
	Meter(HTMLParser.IntParse);
	Unicode c;

	fExitParser = false;

	for (;;) {
		switch (c = Get()) {

				// end of file
			case EOF:
				goto out;

				// end of line
			case 9:		// Hor Tab
			case 10:	// LF
			case 13:	// CR
				IntPut(c);
				break;
#ifdef SPECIAL_CHAR_CONVERSION
			case '&':
				ParseCharacterEntity();
				break;
#endif
			case '<':
				c = Get();
				switch (c) {
					case EOF:
						return EOF;
					case '/':
						c = Get();
						if (isalpha(c)) {
							ParseTag('/', c);	// end tag
							if (fExitParser) {
								goto out;
							}
						} else {
							// text
							IntPut('<');
							IntPut('/');
							IntPutBack(c);
						}
						break;
					case '!':
						c = Get();
						switch (c) {
							case EOF:
								return EOF;
							case '-':
								c = Get();
								if (c == '-') {
									ParseComment();
								} else {
									IntPut('<');
									IntPut('!');
									IntPut('-');
									IntPutBack(c);
								}
								break;
							case '>': {
								// empty comment
								String comment;
								IntComment(comment);
							}
							break;
							default:
								if (isalpha(c)) {
									ParseTag('!', c);
									if (fExitParser) {
										goto out;
									}
								} else {
									// no comment
									IntPut('<');
									IntPut('!');
									IntPutBack(c);
								}
								break;
						}
						break;
					case '?':
						break;
					default:
						if (isalpha(c)) {
							ParseTag(' ', c);	// start tag
							if (fExitParser) {
								goto out;
							}
						} else {
							IntPut('<');
							IntPutBack(c);
						}
						break;
				}
				break;
			default:
				// setlocale( LC_CTYPE,"switzerland");
#ifdef CONTROL_CHAR_CHECK
				if (!iscntrl(c))
#endif
					IntPut(c);
				break;
		}
	}
out:
	IntFlush();
	return 0;
}

bool IsOtherAcceptableChar(Unicode c)
{
	if ((c == '-') || (c == '_') || (c == '#') ||
		(c == '%') || (c == '+') || (c == '=')) {
		return true;
	}
	return false;
}

Unicode HTMLParser::NextToken(String &token, bool withDelims, bool acceptEqual)
{
	Meter(HTMLParser.NextToken);
	int i;
	Unicode c, enddelim;

	do {
		c = Get();
	} while (isspace(c));

	switch (c) {

		case '\0':
			return EOF;
		case '>':
			return '>';

		case '"':
		case '\'':
			enddelim = c;
			token = "";

			if (withDelims) {
				token.Append((char)c);
			}

			for (i = 0; i < 1025; i++) {
				if (i >= 1024) {
					Error("value too long (> 1024)");
				}
				c = Get();
				if (c == enddelim) {
					break;
				}
				token.Append((char)c);
			}
			if (withDelims) {
				token.Append((char)c);
			}

			return 's';

		default:
			if (!acceptEqual) {
				if (c == '=') {
					return '=';
				}
			}
			token = "";
			{
				bool name = true;

				for (;;) {
					if (!isalnum(c) && !IsOtherAcceptableChar(c)) {
						name = false;
					}
					token.Append((char)c);
					c = Get();
					if (isspace(c) || c == '>' || (!acceptEqual && (c == '='))) {
						IntPutBack(c);
						break;
					}
				}
				if (name) {
					return 'a';
				}
				return 's';
			}
	}
}

void HTMLParser::ParseArguments(const char *tag, Anything &node)
{
	Meter(HTMLParser.ParseArguments);
	StartTrace(HTMLParser.ParseArguments);
	String token, key, value;
	int ix = 0;

	bool hasArgs = TagHasArguments(tag);

	Unicode t = NextToken(key, false);
	for (; t != EOF && t != '>' ; ) {
		if (!hasArgs) {
			// tag contains arbitrary argument string
			node["__unstructured"].Append(key);
			t = NextToken(key, true);
		} else {
			// tag has regular key/value type arguments
			if (t == 'a') {
				t = NextToken(token, false);
				if (t == EOF || t == '>') {
					break;
				}
				if (t == '=') {
					// no we know that the first token was the key
					t = NextToken(value, false, true);
					if (t == EOF || t == '>') {
						break;
					}
					if (t == 'a' || t == 's') {
						key.ToLower();
						IntArgument(key, value);
						ix++;
					} else {
						Error("arg error>");
					}
				} else if (t == 'a') {
					String key2 = key;
					key2.ToLower();
					IntArgument(key2, key);
					key = token;
					continue;
				} else {
					String msg("unexpected <");
					msg << token << "> in argument list (1)";
					Error(msg);
				}
			} else {
				TraceAny(node, "node");
				String msg("unexpected <");
				msg << token << "> in argument list (2) for tag <" << tag << ">";
				Error(msg);
			}
			t = NextToken(key, false);
		}
	}
}

void HTMLParser::Error(const String &msg)
{
	SYSERROR(msg);
}

//---- Overrides ----
Unicode HTMLParser::IntGet()
{
	return EOF;
}
void HTMLParser::IntPutBack(Unicode)
{
}
void HTMLParser::IntFlush()
{
}
void HTMLParser::IntPut(Unicode)
{
}
void HTMLParser::IntPut(const String &)
{
}
void HTMLParser::IntComment(const String &)
{
}
void HTMLParser::IntPushNode(Anything &)
{
}
void HTMLParser::IntTag(int, const char *)
{
}
void HTMLParser::IntArgument(const String &, const String &)
{
}
void HTMLParser::IntError(long line, const String &msg)
{
}

//--- static
HTMLParser::TagType HTMLParser::LookupTag(const char *name)
{
	Meter(HTMLParser.LookupTag);
	register int position;
	int last = sizeof(TagTable) / sizeof(TagInfo) - 1, base = 0, result;
	if (name) {
		while (last >= base) {
			position = (base + last) >> 1;
			if ((result = strcmp(TagTable[position].fName, name)) == 0) {
				return TagTable[position].fCode;
			}
			if (result > 0) {
				last = position - 1;
			} else {
				base = position + 1;
			}
		}
	}
	return eUNKNOWN;
}

void HTMLParser::VerifyTagTable()
{
	Meter(HTMLParser.VerifyTagTable);
	for (int i = 1; TagTable[i].fName; i++) {
		if (strcmp(TagTable[i-1].fName, TagTable[i].fName) >= 0) {
			String logMsg;
			logMsg << TagTable[i-1].fName << " > " << TagTable[i].fName;
			SYSERROR(logMsg);
			break;
		}
	}
}

Unicode HTMLParser::LookupSpecial(const char *name)
{
	Meter(HTMLParser.LookupSpecial);
	register int position;
	int last = sizeof(SpecialCharTable) / sizeof(SpecialCharInfo) - 1, base = 0, result;
	if (name) {
		while (last >= base) {
			position = (base + last) >> 1;
			if ((result = strcmp(SpecialCharTable[position].fName, name)) == 0) {
				return SpecialCharTable[position].fCode;
			}
			if (result > 0) {
				last = position - 1;
			} else {
				base = position + 1;
			}
		}
	}
	return -1;
}

EAlign HTMLParser::LookupAlign(const char *key)
{
	Meter(HTMLParser.LookupAlign);
	for (int i = 0; AlignTags[i].tag; i++)
		if (String::CaselessCompare(AlignTags[i].tag, key) == 0) {
			return AlignTags[i].align;
		}
	String logMsg("align ");
	logMsg << key << " not found";
	SYSERROR(logMsg);
	return eTop;
}

//---- AAT_HTMLReader ---------------------------------------------------------------------------
AAT_HTMLReader::AAT_HTMLReader(istream *fp)
	: fFile(fp)
{
}

int AAT_HTMLReader::Get()
{
	return fFile ? fFile->get() : EOF;
}

void AAT_HTMLReader::PutBack(char c)
{
	fFile ? fFile->putback(c) : (*fFile);
}

//---- AAT_HTMLWriter ---------------------------------------------------------------------------
void AAT_HTMLWriter::Put(Unicode)
{
}
void AAT_HTMLWriter::Put(char)
{
}
void AAT_HTMLWriter::Put(const String &)
{
}
void AAT_HTMLWriter::Flush()
{
}
void AAT_HTMLWriter::Comment(const String &)
{
}
void AAT_HTMLWriter::PushNode(Anything &)
{
}
void AAT_HTMLWriter::Tag(int, const char *)
{
}
void AAT_HTMLWriter::Argument(const String &, const String &)
{
}
void AAT_HTMLWriter::Error(long line, const String &msg)
{
	StartTrace(AAT_HTMLWriter.Error);
	String errorMsg;
	errorMsg << line << " " << msg;
	Trace(errorMsg);
	SYSERROR(errorMsg);
}

//--------------------------------------------------
AAT_StdHTMLParser::AAT_StdHTMLParser(AAT_HTMLReader &reader, AAT_HTMLWriter &writer)
	: fReader(reader), fWriter(writer)
{
}

Unicode AAT_StdHTMLParser::IntGet()
{
	return fReader.Get();
}

void AAT_StdHTMLParser::IntPutBack(Unicode c)
{
	fReader.PutBack(c);
}

void AAT_StdHTMLParser::IntFlush()
{
	fWriter.Flush();
}

void AAT_StdHTMLParser::IntPushNode(Anything &node)
{
	fWriter.PushNode(node);
}

void AAT_StdHTMLParser::IntPut(const String &str)
{
	fWriter.Put(str);
}

void AAT_StdHTMLParser::IntPut(Unicode c)
{
	fWriter.Put(c);
}

void AAT_StdHTMLParser::IntComment(const String &comment)
{
	fWriter.Comment(comment);
}

void AAT_StdHTMLParser::IntTag(int type, const char *tag)
{
	Meter(AAT_StdHTMLParser.IntTag);
	fWriter.Tag(type, tag);
}

void AAT_StdHTMLParser::IntArgument(const String &key, const String &value)
{
	Meter(AAT_StdHTMLParser.IntArgument);
	fWriter.Argument(key, value );
}

void AAT_StdHTMLParser::IntError(long line, const String &msg)
{
	Meter(AAT_StdHTMLParser.IntError);
	fWriter.Error(line, msg);
}

void MyHTMLWriter::Put(char c )
{
	Meter(MyHTMLWriter.Put);
	Put( (Unicode) c );
}

void MyHTMLWriter::Put(Unicode c )
{
	Meter(MyHTMLWriter.Put);
	StartTrace(MyHTMLWriter.Put);
	if (fInScript == true ) {
		return;
	}

	if ( fStoreTitle ) {
		fTitle.Append( (char) c );
		if ( isalnum(c) ) {
			fAllStringsInPage.Append( (char)c );
		}
	} else {
		if ( fNodeStack.GetSize() > 0 ) {
			String myString(fNodeStack[fNodeStack.GetSize()-1]["String"].AsCharPtr());
			if ( ( (c != ' ') && (c != '\n') && (c != '\t') && (c != '\r') ) || ( ( myString.Length() > 0 ) && (c == ' ') )	 ) {
				myString.Append( (char)c );
				// only add text for valid tags
				// - should not add text for __unstructured tag anymore
				if ( fNodeStack[fNodeStack.GetSize()-1].IsDefined("Tag") ) {
					fAllStringsInPage.Append((char)c );
					fNodeStack[fNodeStack.GetSize()-1]["String"] = myString;
				}
			}
		}
	}
}

void MyHTMLWriter::Put(const String &str)
{
	Meter(MyHTMLWriter.PutString);
	StartTrace(MyHTMLWriter.PutString);
	if (fInScript == true ) {
		return;
	};

	if ( fStoreTitle ) {
		fTitle << str;
		fAllStringsInPage << str;
	} else {

		if ( fNodeStack.GetSize() > 0 ) {
			fNodeStack[fNodeStack.GetSize()-1]["String"] = str;
			fAllStringsInPage << str;
		}
	}
}

void MyHTMLWriter::Flush()
{
	Meter(MyHTMLWriter.Flush);
	fUrls["Tokens"] = fAllStringsInPage;
}

void MyHTMLWriter::Comment(const String &)
{
	Meter(MyHTMLWriter.Comment);
}

void MyHTMLWriter::Tag(int t, const char *s )
{
	Meter(MyHTMLWriter.Tag);
	StartTrace(MyHTMLWriter.Tag);
	Trace( "Int is->" << (long)t << "[" << (char)t << "] String Tag is [" << s << "]" );
	if ( fNodeStack.GetSize() > 0L ) {
		Anything node = fNodeStack[fNodeStack.GetSize()-1];
		if (t == ' ') {
			node["Tag"] = s;
		}
		TraceAny( node, "current node is" );
		Trace("fFormNr is-->" << fFormNr );

		HTMLParser::TagType tt = HTMLParser::LookupTag(s);

		switch (tt) {
			case HTMLParser::eANCHOR:
				RenderATag(t, s, node);
				break;
			case HTMLParser::eIMAGE:
				RenderImageTag(t, s, node);
				break;
			case HTMLParser::eMETA:
				RenderMetaTag(t, s, node);
				break;
			case HTMLParser::eSELECT:
				RenderSelectTag(t, s, node);
				break;
			case HTMLParser::eSCRIPT:
				RenderScriptTag(t, s, node);
				break;
			case HTMLParser::eFRAME:
				RenderFrameTag(t, s, node);
				break;
			case HTMLParser::eFORM:
				RenderFormTag(t, s, node);
				break;
			case HTMLParser::eINPUT:
				RenderInputTag(t, s, node);
				break;
			case HTMLParser::eTITLE:
				RenderTitleTag(t, s, node);
				break;
			case HTMLParser::eBASE:
				RenderBASETag(t, s, node);
				break;
			case HTMLParser::eFONT:
			case HTMLParser::eBOLD:
			case HTMLParser::eITALIC:
			case HTMLParser::eTELETYPE:
			case HTMLParser::eUNDERLINE:
			case HTMLParser::eSMALL:
			case HTMLParser::eSTRIKEOUT:
			case HTMLParser::eSTRONG:
			case HTMLParser::eSUBSCRIPT:
			case HTMLParser::eSUPERSCRIPT:
			case HTMLParser::eBLINK:
				RenderFontTag(t, s, node);
			default:
				if (!strcmp(s, "s")) {
					RenderFontTag(t, s, node); // promote text to containing node
				} else {
					if (t == '/') {
						TraceAny(fNodeStack, "NodeStack");
						String thisNodesTag, oldLevel, levelOfFailure;
						Anything anyStructPos = node;
						long stackTop = fNodeStack.GetSize() - 1;
						while ( stackTop >= 0 ) {
							node = fNodeStack[stackTop];
							oldLevel = levelOfFailure;
							thisNodesTag.Trim(0);
							if ( node.IsDefined("Tag") ) {
								thisNodesTag = node["Tag"].AsCharPtr("");
							}
							Trace("This Level ->" << thisNodesTag );
							if ( thisNodesTag == s ) {
								break;
							}
							levelOfFailure = thisNodesTag;
							if ( oldLevel.Length() ) {
								levelOfFailure << ".";
							}
							levelOfFailure << oldLevel;
							Trace("levelOfFailure [" << levelOfFailure << "]");
							stackTop--;
						}
						Trace("stackTop:" << stackTop);
						node = anyStructPos;
						if ( stackTop < 0 && thisNodesTag != s ) {
							SYSWARNING("endtag [" << s << "] without begin tag at [" << levelOfFailure << "]");
							TraceAny(fNodeStack, "Nodestack");
						} else {
							stackTop = fNodeStack.GetSize() - 1;
							while ( stackTop >= 0 ) {
								node = fNodeStack[stackTop];
								thisNodesTag = node["Tag"].AsCharPtr("");
								node.Remove("Tag");
								if ( node.IsDefined("String") ) {
									node.Remove("String");
								}
								fNodeStack.Remove(stackTop);
								Trace("POP - stack, new size " << stackTop );
								if ( thisNodesTag == s ) {
									break;
								}
								stackTop--;
							}
						}
					}
				}
		}
	} else {
		SYSINFO( "Int is->" << (long)t << "[" << (char)t << "] String Tag is [" << s << "]" );
	}
}

void MyHTMLWriter::RenderScriptTag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderScriptTag);
	if (t == ' ') {
		// start tag
		fInScript = true;
	} else {
		fInScript = false;
	}
}

void MyHTMLWriter::RenderImageTag(int t, const char *s, Anything &node, const char *wrap)
{
	Meter(MyHTMLWriter.RenderImageTag);
	if (t == ' ') {
		// start tag
		if ( node.IsDefined("src") ) {
			String strSrc(node["src"].AsCharPtr(""));
			long i;
			// check whether this image is already in the list
			for ( i = 0; i < fUrls["Imgs"].GetSize(); i++ ) {
				if ( ! strSrc.Compare( fUrls["Imgs"][i]["src"].AsCharPtr("") ) ) {
					// already in the list
					break;
				}
			}
			// add this image to the list of to be loaded images
			if ( i >= fUrls["Imgs"].GetSize() ) {
				fUrls["Imgs"].Append( node );
			}
		}
	} else {
		// end tag.. needs not be modified
	}
}

void MyHTMLWriter::RenderATag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderATag);
	StartTrace1(MyHTMLWriter.RenderATag, NotNull(s)); // won't work with nested links, but they can't happen right(?)
	TraceAny(fUrls["Links"], "Links so far" );

	if (t == ' ') {
		// start tag
		// if node contains href keep this link
		Anything converter(fUrls["Links"].GetSize());
		String index(converter.AsCharPtr("IllegalNumber"));
		if ( node.IsDefined("href") ) {
			fUrls["Links"][index] = node;
		}
	} else {
		// end tag.. needs not be modified
		Anything converter(fUrls["Links"].GetSize() - 1);
		String index(converter.AsCharPtr("IllegalNumber"));

		if ( fUrls["Links"][index].IsDefined("String") ) {
			fUrls["LinksNameMap"][fUrls["Links"][index]["String"].AsCharPtr()] = index;
			fUrls["Links"][index].Remove("String");
		}
	}
	TraceAny(fUrls["Links"], "Links after" );
}

void MyHTMLWriter::RenderFontTag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderFontTag);
	if (t != ' ') {
		// END tag - propagate text "physical text attribute node" to containing node
		if (fNodeStack.GetSize() > 1 ) {
			if ( fNodeStack[fNodeStack.GetSize()-1].IsDefined("String") ) {
				fNodeStack[fNodeStack.GetSize()-2]["String"] = fNodeStack[fNodeStack.GetSize()-1]["String"];
			}
		}
	}
}

void MyHTMLWriter::RenderBASETag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderBASETag);
	if (t == ' ') {
		// start tag
		// if BASE tag contains href keep this base link
		if ( node.IsDefined("href") ) {
			String strHREF(node["href"].AsCharPtr(""));
			//strHREF.TrimFront(7); // eliminate 'HTTP://' // MIKE, don't! leave HTTP in there
			fUrls["BASE"] = strHREF;
		}
	} else {
		// end tag.. needs not be modified
	}
}

void MyHTMLWriter::RenderMetaTag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderMetaTag);
	StartTrace1(MyHTMLWriter.RenderMetaTag, NotNull(s));
	Trace( "Int is->" << (long)t << " String Tag is->" << s );
	TraceAny( node, " Node is" );

	if (t == ' ') {
		// start tag
		// if BASE tag contains href keep this base link
		if ( node.IsDefined("http-equiv") ) {
			fUrls["Meta"][node["http-equiv"].AsCharPtr("")] =  node["content"].AsCharPtr("");
		}
	} else {
		// end tag.. needs not be modified
	}
}

void MyHTMLWriter::RenderTitleTag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderTitleTag);
	// searches in title for wdgatewayComm error messages
	if (t == ' ') {
		// start tag; store comment between start and end tag
		fStoreTitle = true;
		fTitle = "";
	} else {
		// end tag; compare title
		fStoreTitle = false;
		if ( fTitle.CompareN( "wdgatewayComm", 13, 0 ) == 0 ) {
			fRequestFailed = true;
		}
	}
}

void MyHTMLWriter::RenderSelectTag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderSelectTag);
	StartTrace1(MyHTMLWriter.RenderSelectTag, NotNull(s));
	Trace( "Int is->" << (long)t << " String Tag is->" << s );
	if (t == ' ') {
		Anything fNr(fFormNr);

		fUrls["Forms"][fNr.AsCharPtr("")]["Fields"][node["name"].AsCharPtr("")] = node["value"];
	} else {
		// end tag.. needs not be modified
	}
}

void MyHTMLWriter::RenderInputTag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderInputTag);
	// Input field in a form
	StartTrace1(MyHTMLWriter.RenderInputTag, NotNull(s));
	Trace( "Int is->" << (long)t << " String Tag is->" << s );
	TraceAny( node, "<-- input Node is" );
	if (t == ' ') {
		// start tag; keep this form specification
		// Anything converter(fUrls["Forms"].GetSize());
		// String index=converter.AsString("IllegalNumber");
		// TYPE="HIDDEN" or "TEXT" etc.
		// NAME="<x>" - field name
		// VALUE="<y>" - to be sent back
		String nodeType(node["type"].AsCharPtr());
		nodeType.ToUpper();

		Trace( "Node type is->" << nodeType );

		// what about TEXTAREA and SELECT -> See RenderSelectTag RenderTextAreaTag
		// checked-ness of buttons still needs to be done...
		//		<input type="checkbox" name="chk_checkbox" value="chedckbox" checked> ..or..
		//		<input type="checkbox" name="chk_checkbox" value="chedckbox" >
		//
		Anything fNr(fFormNr);

		if ( nodeType.IsEqual("SUBMIT")) {
			// render submit button(s) other way around so they can be more easily found by stresser browser...
			// only one of these buttons is sent back as form selected button to the server... also notice rendered in a separate
			// anything from other fields
			fUrls["Forms"][fNr.AsCharPtr("")]["Buttons"][node["value"].AsCharPtr("")] = node["name"];
			fUrls["Forms"][fNr.AsCharPtr("")]["Buttons"][node["name"].AsCharPtr("")] = node["name"];
		} else if (  nodeType.IsEqual("IMAGE") ) {
			fUrls["Forms"][fNr.AsCharPtr("")]["ImageButtons"][node["name"].AsCharPtr("")] = "IMAGETYPE";
		} else {		// planned Mike:
			fUrls["Forms"][fNr.AsCharPtr("")]["Fields"][node["name"].AsCharPtr("")] = node["value"];
		}
	} else {
		// end tag.. needs not be modified
	}
}

void MyHTMLWriter::RenderFrameTag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderFrameTag);
	if (t == ' ') {
		// start tag; keep this form specification
		Anything converter(fUrls["Frames"].GetSize());
		String index(converter.AsCharPtr("IllegalNumber"));

		if ( node.IsDefined("src") ) {
			fUrls["Frames"][index] = node;
		}

		if ( node.IsDefined("name") ) {
			//String s = node["src"].AsString("");
			fUrls["FramesNameMap"][node["name"].AsCharPtr("")] = index;
		}
	} else {
		// end tag.. needs not be modified
	}
}

void MyHTMLWriter::RenderFormTag(int t, const char *s, Anything &node)
{
	Meter(MyHTMLWriter.RenderFormTag);
	StartTrace(MyHTMLWriter.RenderFormTag);
	Trace("fFormNr is-B4 ->" << fFormNr );

	if (t == ' ') {
		fFormNr++;
		// start tag; keep this form specification
		Anything converter(fUrls["Forms"].GetSize());
		String index(converter.AsCharPtr("IllegalNumber"));
		fUrls["Forms"][index] = node;
	}
	Trace("fFormNr is-after ->" << fFormNr );
}

void MyHTMLWriter::PushNode(Anything &node)
{
	Meter(MyHTMLWriter.PushNode);
	StartTrace(MyHTMLWriter.PushNode);
	TraceAny(node, "<--------------------push this node");
	fNodeStack[fNodeStack.GetSize()] = node;
	Trace("Stack size now -->" << fNodeStack.GetSize() );
}

void MyHTMLWriter::Argument(const String &key, const String &value)
{
	Meter(MyHTMLWriter.Argument);
	fNodeStack[fNodeStack.GetSize()-1][key] = value;
}

void MyHTMLWriter::Error(long line, const String &msg)
{
	Meter(MyHTMLWriter.Error);
	StartTrace(MyHTMLWriter.Error);
	SYSERROR(line);
	Trace(line);
}

bool MyHTMLWriter::GetReqFailed( void )
{
	Meter(MyHTMLWriter.GetReqFailed);
	return fRequestFailed;
}

MethodMeter::MethodMeter(const char *name, Anything &result)
	:	fResult(result),
		fName(name),
		fTimer(0)
{
	fTimer.Start();
}

MethodMeter::~MethodMeter()
{
	fResult[fName][0L] = fResult[fName][0L].AsLong(0) + 1;
	long ul = fResult[fName][1L].AsLong(0);
	ul += fTimer.Reset();
	fResult[fName][1L] = ul;
}

MethodInfoCollector::MethodInfoCollector()
{
}

MethodInfoCollector::~MethodInfoCollector()
{
	long sz = fResults.GetSize();
	for (long i = 0; i < sz; i++) {
		u_long ul = fResults[i][1L].AsLong(1);
		HRTIME t = ul;
		SysLog::WriteToStderr(String(fResults.SlotName(i)) << ":[");
		SysLog::WriteToStderr(String() << fResults[i][0L].AsLong(1) << ", ");
		SysLog::WriteToStderr(String() << (long)(t * 1000L / DiffTimer::TicksPerSecond()) << "]\n");
	}
}
