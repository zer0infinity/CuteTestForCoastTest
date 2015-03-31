/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnyImpls.h"
#include "SystemBase.h"
#include "SystemFile.h"
#include "SystemLog.h"
#include "StringStream.h"
#include "Tracer.h"
#include "AnyVisitor.h"
#include "AnyComparers.h"

using namespace coast;

#include <cstring>
#include <algorithm>
#if defined(COAST_TRACE)
#define anyStatTrace(trigger, msg, allocator) 	StatTrace(trigger, msg, allocator)
#define anyStartTrace(trigger)					StartTrace(trigger)
#define anyStartTrace1(trigger, msg)			StartTrace1(trigger, msg)
#define anyTrace(msg)							Trace(msg);
#define anyTraceAny(any, msg)					TraceAny(any, msg);
#else
#define anyStatTrace(trigger, msg, allocator)
#define anyStartTrace(trigger)
#define anyStartTrace1(trigger, msg)
#define anyTrace(msg)
#define anyTraceAny(any, msg)
#endif

static const String fgStrEmpty(coast::storage::Global()); //avoid temporary

//--- auxiliary calculating hash value and the length of the key
long IFAHash(const char *key, long &len, char stop1, char stop2)
//long DoIFAHash(const unsigned char *key, long &len, unsigned char stop1, unsigned char stop2)
{
	register long h = 0;
	register u_long g;
	const unsigned char *const keyp = reinterpret_cast<const unsigned char *>(key);
	register const unsigned char *p = keyp;
	if (key) {
		while (*p && *p != stop1 && *p != stop2) {
			h = (h << 4) + *p++;
			if ((g = (h & 0xf0000000))) {
				h = (h ^ (g >> 24)) ^ g;
			}
		}
	}
	len = p - keyp;
	return h;
}

class InputContext
{
public:
	// constructor
	InputContext(std::istream &is, const char *fname = 0)
		: fIs(is)
		, fLine(1)
		, fFileName(fname) { }

	// destructor
	~InputContext() {}

	// input
	void SkipToEOL();    // for reading comments by parser
	bool Get(char &c)				{
		return (fIs.get(c).good()) || (c = 0);//lint !e506
	}
	// the last assignment is a trick for sunCC weakness of storing EOF(-1) in c
	void Putback(char c)			{
		fIs.putback(c);
	}
	bool IsGood() 					{
		return fIs.good();
	}

	long &LineRef() {
		return fLine;//lint !e1536
	}
	std::istream &StreamRef() {
		return fIs;//lint !e1536
	}
	const String &FileName() {
		return fFileName;
	}

private:
	std::istream &fIs;
	long fLine;
	String fFileName;
};

//---- the following class is used for lexical analysis of
//---- any-files or input. It will represent the next
//---- character or symbol read from the stream
class AnythingToken
{
public:
	enum Tok { eNullSym = 256, // this means no valid stuff found or eof
			   // numeric values
			   eDecimalNumber, eOctalNumber, eHexNumber, eFloatNumber,
			   // string or names or arrayindex or ifaobject
			   eString, eBinaryBuf, eIndex, eRef, eInclude, eObject,
			   // we have found a lexical or syntax error
			   eStringError, eError
			 };
	AnythingToken(InputContext &context); // read next token from is
	static bool isNameDelimiter(char c); // implement single place where we check for delims

	const int &Token() {
		return fToken;
	}
	const String &Text() {
		return fText;
	}

private:
	int       fToken; // enum above or character value
	String    fText; // the characters that form the token, lex would say yytext[]

	char    DoReadOctalOrHex(InputContext &context);
	char    DoReadNumber(InputContext &context, char firstchar);
	char    DoReadDigits(InputContext &context);
	void    DoReadString(InputContext &context, char firstchar);
	char    DoReadName(InputContext &context, char firstchar);
	void    DoReadBinBuf(InputContext &context);
};

bool AnythingToken::isNameDelimiter(char c)
{
	// alternative Impl: return strchr(" \t\n\r\v\"/#{}[&*",c) != 0;
	// isprint( static_cast<unsigned char>(c)) shouldn't be used because of umlauts äüö and signed chars
	// may be double quotes " should also be considered delimiters
	return isspace(static_cast<unsigned char>(c)) || '/' == c || '#' == c || '&' == c || '*' == c
		   || '{' == c || '}' == c || '[' == c //|| ']' == c
		   || '\"' == c || 0 == c || '%' == c || '!' == c;
}

AnythingToken::AnythingToken(InputContext &context) : fToken(0)
{
	char c = 0;
	do {
		if (!context.Get(c)) {
			// we have reached eof
			if (0 == fToken) {
				fToken = AnythingToken::eNullSym;
			}
			break;
		} else {
			switch (c) {
					// single character tokens
				case '{': // an lbrace,
				case '}': // an rbrace,
				case '*': // a null Anything indicator
					fText.Append(c);
					fToken = c;
					return;
				case '&': // a object indicator a number must follow
					while (context.Get(c) && isspace(static_cast<unsigned char>(c))) {// consume spaces
						// adjust line count!
						if ('\n' == c || '\r' == c) {
							++context.LineRef();
						}
						c = 0;
					}
					c = DoReadNumber(context, c);
					if (fToken == AnythingToken::eDecimalNumber) {
						fToken = AnythingToken::eObject;
						context.Putback(c);
						return;
					}
					// consume invalid characters up to a whitespace
					do {
						fText.Append(c);
					} while (!isspace( static_cast<unsigned char>(c)) && context.Get(c));
					if ('\n' == c || '\r' == c) {
						++context.LineRef();
					}
					fToken = AnythingToken::eError;
					return;
				case '/': // a key indicator : string or name follows
				case '%': // a Ref indicator : string with reference follows
				case '!': { // a Include indicator : string with url to include follows
					Tok theToken = ('/' == c) ? eIndex : ('%' == c) ? eRef : eInclude;
					c = 0;
					if (context.Get(c)) {
						if ('"' == c) {
							DoReadString(context, c);
						} else if (!isNameDelimiter(c)) {
							//                      if (isalnum( static_cast<unsigned char>(c)) || '_' == c || '-' == c) {
							// should we allow more? YES! Note this code corresponts to
							// DoReadName below
							c = DoReadName(context, c);
							if (c) {
								context.Putback(c);
							}
						}  else {
							if (c) {
								context.Putback(c);    // re-interpret delimiters
							}
						}
						// ignore string errors here
						if ((AnythingToken::eStringError == fToken ||
							 AnythingToken::eString == fToken) && fText.Length() > 0) {
							fToken = theToken;
							return;
						}
					} // anything else  is an error.
					fToken = AnythingToken::eError;
					return;
				}
				case '"': // a string value, read it
					DoReadString(context, c);
					return;
				case '[': // a binary buff impl, read it
					DoReadBinBuf(context);
					// should not occur in config files but might occur in ipc
					break;
					// numeric values:
				case '0': // need to check octal or hex
					c = DoReadOctalOrHex(context);
					// should check if c is a delimiter = ( WS / & { } [ ] " )
					if (!isNameDelimiter(c)) {
						// PT: used the same delimiter list as below
						// if it is no delimiter assume a strangely formatted number
						// this is treated as a AnyStringimpl.
						c = DoReadName(context, c);
					}
					if (c) {
						context.Putback(c);    // putback a single char should be always supported.
					}
					return;
				case '+':
				case '-': // a signed number
				case '.': // a decimal number
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					// read a decimal number
					c = DoReadNumber(context, c);
					// should check if c is a delimiter = ( WS # / & { } [ ] " ) PT: added # in comment
					if (!isNameDelimiter(c)
						// PT: added '/', '&' and '}' to condition to account for packed Anythings
						|| fToken == AnythingToken::eError) {
						// if it is no delimiter assume a strangely formatted number
						// this is treated as a AnyStringimpl.
						c = DoReadName(context, c);
					}
					if (c) {
						context.Putback(c);
					}
					return;
				case '#': // start of a comment skip to eol
					context.SkipToEOL();
					break;
				default:
					if (isalnum( static_cast<unsigned char>(c)) || '_' == c) { // do we need to allow more?
						// a name or string read it
						c = DoReadName(context, c);
						// should check for delimiter
						// this character should be ignored safely
						if (c) {
							context.Putback(c);
						}
					} else if (isspace( static_cast<unsigned char>(c))) {
						// ignore it but count line changes
						if ('\n' == c || '\r' == c) {
							++context.LineRef();
						}
					} else {
						// this is a lexical error
						fText.Append(c);
						fToken = AnythingToken::eError;
						// ignore character
					}
			} // switch
		} // if
	} while (0 == fToken);
}

void AnythingToken::DoReadString(InputContext &context, char firstchar)
{
	if (firstchar) {
		context.Putback(firstchar); // leave double quote on the stream
	}
	long linebreakswithinstring =  fText.IntReadFrom(context.StreamRef(), firstchar);
	if (linebreakswithinstring < 0) {
		// if someone puts "hello""
		// in an any file the mismatched double quote opens a string only
		// until the end of line. This is detected by IntReadFrom
		// post a separate error, because it might not be obvious
		// what was the reason otherwise.
		linebreakswithinstring = 0 - linebreakswithinstring;
		fToken = AnythingToken::eStringError;

	} else {
		fToken = AnythingToken::eString;
	}
	// adjust line number automatically
	context.LineRef() += linebreakswithinstring;
}

void AnythingToken::DoReadBinBuf(InputContext &context)
{
	// context's line count is not adjusted in the case of binary buffers
	// first read in the length of the buffer
	long length = 0;
	context.StreamRef() >> length; // we do not need DoReadNumber here.
	// we need to check a formatting error on the stream here!
	if ( !context.StreamRef().good() ) {
		// a syntax error
		fToken = AnythingToken::eError;
		context.StreamRef().clear(); // try again
	}
	// now we read ';' as a separator
	char c = 0;
	context.Get(c);
	if ( ';' != c ) {
		// this is an error
		// skip to ] or eof and set token to eError
		fToken = AnythingToken::eError;
		while ( ']' != c && context.Get(c) ) {
		}
		return;
	}
	// now read in the binary buffer, this can be done via fText
	fText.Trim(0); // to ensure nothing nasty happens
	fText.Append(context.StreamRef(), length);
	if ( fText.Length() != length ) {
		// we have a premature EOF.
		fToken = AnythingToken::eError;
	}
	context.Get(c);
	if ( ']' != c ) {
		fToken = AnythingToken::eError;
		while ( ']' != c && context.Get(c) ) {
		}
	}
	if (fToken != AnythingToken::eError) {
		fToken = AnythingToken::eBinaryBuf;
	}
}

char AnythingToken::DoReadName(InputContext &context, char firstchar)
{
	// reads letters and digits of a name and collects them in fText
	// returns 0 in case of eof or the delimiting character instead
	char c = 0;
	fText.Append(firstchar);
	fToken = AnythingToken::eString;
	while (context.Get(c)) {
		//        if (isalnum( static_cast<unsigned char>(c)) || '_' == c || '-' == c) { // should we allow more? YES!
		if (!isNameDelimiter(c)) {
			// collect almost all printable chars except comments and nested anys
			fText.Append(c);
		} else {
			return c; // we are done
		}
		c = 0; // to mask non-read chars in case eof condition is not determined safely
	}
	// huh eof, but not critical
	return 0;
}

char AnythingToken::DoReadDigits(InputContext &context)
{
	// reads decimal digits returns character that follows last digit
	// collects digits in fText
	// returns 0 in case of eof
	char c = 0;
	while (context.Get(c)) {
		if (isdigit( static_cast<unsigned char>(c))) {
			fText.Append(c);
		} else {
			return c; // we are done
		}
		c = 0; // to mask non-read chars in case eof condition is not determined safely
	}
	// huh eof, but not critical
	return 0;
}

char AnythingToken::DoReadOctalOrHex(InputContext &context)
{
	// determines octal or hex number and then
	// reads decimal digits returns character that follows last digit
	// collects digits in fText
	// returns 0 in case of eof
	// might chain DoReadNumber if we encounter a '.' or a
	// 8 or 9 after the initial 0
	fToken = AnythingToken::eOctalNumber;
	char c = 0;
	if (context.Get(c)) {
		if ('X' == toupper(c)) {
			// we have got a hex number, collect it to string
			// without leading x;
			char saveXcase = c;
			fToken = AnythingToken::eHexNumber;
			while (context.Get(c)) {
				if (isxdigit( static_cast<unsigned char>(c))) {
					fText.Append(c);
				} else if (!isNameDelimiter(c)) {
					// we might have some problem here if c is not a delim
					// assume we are reading a name instead of a number
					// therefore re-insert 0x or 0X
					fText = String("0") << saveXcase << fText;
					return c;
				} else {
					return c; // we are done
				}
				c = 0; // to mask non-read chars in case eof condition is not determined safely
			}
		} else if ('0' <= c && '7' >= c) {
			// assume for now it is an octal number
			fText.Append('0'); // save 0
			do { // collect the octal digits
				fText.Append(c);
				c = 0;
				if (context.Get(c)) {
					if ('8' == c || '9' == c || '.' == c || 'E' == toupper(c) ) {
						// oops we found a decimal instead of octal
						// this might lead to an error if we input 007.
						// because the int-part might be already parsed
						return DoReadNumber(context, c);
					} // else assume its OK and a valid octal number
					// or a delimiter.
				} else {
					return 0; // we have reached EOF.
				}
			} while ('0' <= c && '7' >= c);
			return c;
		} else if ('8' == c || '9' == c || '.' == c || 'E' == toupper(c) ) {
			// oops we found a decimal instead of octal
			// this might lead to an error if we input "0."
			// because the int-part might be already parsed
			// therefore putback c
			// nevertheless decimal numbers beginning with 08 or 09
			// should be considered a syntax error, but we accept them now.
			context.Putback(c);
			return DoReadNumber(context, '0');
		} else { // we have found '0', therefore fall through down below:
			fText.Append('0'); // save 0
		}
	} else {
		c = 0; // solve sunCCs problem of passing EOF through a char
		// definitely set c to 0 if we fail to Get
	}
	// we have found 0 or some other error
	// should check that c is a valid delimiter in the outside part
	return c;
}

char AnythingToken::DoReadNumber(InputContext &context, char firstchar)
{
	// collect numeric characters from context input and look if it might
	// be a floating point number
	// this code is a little bit tricky, since we have a multi-state automaton
	// determining lexically correct floating point numbers
	fToken = AnythingToken::eDecimalNumber; // might become eFloatNumber
	bool intpart = false, fraction = false,
		 exponent = false, echar = false;
	if (context.IsGood()) {
		if ('+' == firstchar || '-' == firstchar) {
			// found optional sign character :
			fText.Append(firstchar);
			context.Get(firstchar);
		}
		// processed [+-]? follow: [0-9]*\.?[0-9]*([eE][+-]?[0-9]+)?
		if (isdigit( static_cast<unsigned char>(firstchar))) {
			// collect decimal digits
			intpart = true;
			fText.Append(firstchar);
			firstchar = DoReadDigits(context);
		}
		// processed [+-]?[0-9]* follow: \.?[0-9]*([eE][+-]?[0-9]+)?
		if ('.' == firstchar) {
			fraction = true;
			fText.Append(firstchar);
			firstchar = DoReadDigits(context);
		}
		// processed [+-]?[0-9]*\.?[0-9]* follow: ([eE][+-]?[0-9]+)?
		if ('E' == toupper(firstchar)) {
			// tricky part sign might follow
			fText.Append(firstchar);
			firstchar = 0;
			echar = true;
			context.Get(firstchar);
			if ('+' == firstchar || '-' == firstchar) {
				// found sign
				fText.Append(firstchar);
				firstchar = 0;
				context.Get(firstchar);
			}
			// processed [+-]?[0-9]*\.?[0-9]*([eE][+-]? follow: [0-9]+)?
			if (isdigit( static_cast<unsigned char>(firstchar))) {
				exponent = true;
				fText.Append(firstchar);
				firstchar = DoReadDigits(context);
			}
		}
	} else {
		// special case one digit number
		fText.Append(firstchar);
		if (isdigit( static_cast<unsigned char>(firstchar))) {
			intpart = true;
		} else {
			fToken = AnythingToken::eError;
		}
		firstchar = 0;
	}
	// now we check our bools for valid numbers
	if ((! intpart && !fraction && !exponent) || (echar && !exponent)) {
		fToken = AnythingToken::eError;
	} else if (fraction || exponent) {
		fToken = AnythingToken::eFloatNumber;
	} // otherwise it should be a decimal number
	return firstchar;
}

namespace {
	char const fgPathDelim = '.';
	char const fgIndexDelim = ':';
	char const fgEscapeChar = '\\';

	bool leftCharNotEscape(char const l, char const r) {
		return ( l != fgEscapeChar && ( r == fgPathDelim || r == fgIndexDelim ) );
	}
	bool leftCharEscape(char const l, char const r) {
		return ( l == fgEscapeChar && ( r == fgPathDelim || r == fgIndexDelim ) );
	}
	struct unescapeString {
		String result;
		void operator()(const char c) {
			if ( result.Length() && leftCharEscape(result[result.Length()-1L], c) ) {
				result.Trim(result.Length()-1L);
			}
			result.Append(c);
		}
	};
	struct escapeString {
		String result;
		void operator()(const char c) {
			if ( c == fgPathDelim || c == fgIndexDelim ) {
				result.Append(fgEscapeChar);
			}
			result.Append(c);
		}
	};

	struct appendAnyLevelToString : public std::unary_function<Anything const&,void> {
		String fStr;
		void operator()(Anything const& anyValue) {
			if ( anyValue.GetType() == AnyCharPtrType ) {
				if ( fStr.Length() ) fStr.Append(fgPathDelim);
			} else if ( anyValue.GetType() == AnyLongType ) {
				fStr.Append(fgIndexDelim);
			}
			String strTok = anyValue.AsString();
			fStr.Append(std::for_each(strTok.cstr(), strTok.cstr()+strTok.Length(),escapeString()).result);
		}
	};//lint !e1509
	struct resolveToAnyLevel : public std::unary_function<Anything const&,void> {
		Anything result;
		resolveToAnyLevel(Anything const& anySource) : result(anySource) {}
		void operator()(Anything const& anyValue) {
			anyStartTrace1(resolveToAnyLevel.operator, "key [" << anyValue.AsString() << "], success: " << (result.IsNull()?"false":"true"));
			if ( !result.IsNull() ) {
				long lIdx = -1L;
				if ( anyValue.GetType() == AnyCharPtrType ) {
					lIdx = result.FindIndex(anyValue.AsString());
					anyTrace("char key to idx: " << lIdx);
				} else if ( anyValue.GetType() == AnyLongType ) {
					lIdx = result.FindIndex(anyValue.AsLong(-1L));
					anyTrace("long key to idx: " << lIdx);
				}
				if ( lIdx == -1L ) {
					result = Anything();
					return;
				}
				result = result[lIdx];
				anyTraceAny(result, "retrieved reference so far")
			}
		}
	};//lint !e1509
	Anything escapedQueryStringToAny(String const& query) {
		anyStartTrace(AnythingParser.escapedQueryStringToAny);
		char const *pStart = query.cstr(), *pEnd = pStart + query.Length(), *pPos = pStart;
		Anything anyLevel;
		for (; pPos != pEnd; ) {
			pPos = std::adjacent_find(pStart, pEnd, leftCharNotEscape);
			long lLen = pPos-pStart;
			if ( pPos != pEnd ) ++lLen;
			String strTok(pStart, lLen);
			if ( *pStart == fgIndexDelim || *pStart == fgPathDelim ) {
				strTok.TrimFront(1);
			}
			if ( *pStart == fgIndexDelim ) {
				anyTrace("indextoken [" << strTok << "]")
				anyLevel.Append(strTok.AsLong(-1L));
			} else {
				anyTrace("pathtoken [" << strTok << "]")
				anyLevel.Append(std::for_each(strTok.cstr(), strTok.cstr()+strTok.Length(),unescapeString()).result);
			}
			if ( pPos != pEnd ) {
				pStart = ++pPos;
			}
		}
		anyTraceAny(anyLevel, "sequence");
		return anyLevel;
	}
}

class AnyXrefHandler {
	Anything fParseLevel;
protected:
	Anything fXrefs;
public:
	Anything ParseLevel() const {
		return fParseLevel.DeepClone();
	}
	String ParseLevelAsString() {
		return std::for_each(fParseLevel.begin(), fParseLevel.end(), appendAnyLevelToString()).fStr;
	}
	void ResetLevel(Anything const &any) {
		fParseLevel = any;
	}
	void PushKey(const String &key) {
		fParseLevel.Append(key);
	}
	void PushIndex(long lIdx) {
		fParseLevel.Append(lIdx);
	}
	void Pop() {
		fParseLevel.Remove(fParseLevel.GetSize() - 1);
	}
};

class PrinterXrefHandler: public AnyXrefHandler {
	String ToId(long id) const {
		return String().Append(id);
	}
public:
	bool IsDefined(long id) const {
		return fXrefs.IsDefined(ToId(id));
	}
	void DefineBackRef(long id) {
		fXrefs[ToId(id)] = ParseLevel();
	}
	String GetBackRef(long id) {
		return std::for_each(fXrefs[ToId(id)].begin(), fXrefs[ToId(id)].end(), appendAnyLevelToString()).fStr;
	}
};
//lint !e1509

class ParserXrefHandler : public AnyXrefHandler
{
public:
	void DefinePatchRef(long lIdx) {
		anyStartTrace1(ParserXrefHandler.DefinePatchRef, "parseLevel:" << ParseLevelAsString() << " idx:" << lIdx);
		Anything patch;
		patch[0L] = ParseLevel();
		patch[1L] = lIdx;
		fXrefs.Append(patch);
	}
	void DefinePatchRef(const String &reference) {
		anyStartTrace1(ParserXrefHandler.DefinePatchRef, "parseLevel:" << ParseLevelAsString() << " ref [" << reference << "]");
		Anything patch;
		patch[0L] = ParseLevel();
		patch[1L] = reference;
		fXrefs.Append(patch);
	}
	void Patch(Anything &any) {
		anyStartTrace(ParserXrefHandler.Patch);
		Anything preslot, anyReferenced, ref, anyLevel, anyIdx;
		for (long i = 0, lSize = fXrefs.GetSize(); i < lSize; ++i) {
			anyLevel = fXrefs[i][0L];
			anyIdx = fXrefs[i][1L];
			anyTraceAny(anyLevel, "strLevel [" << std::for_each(anyLevel.begin(), anyLevel.end(), appendAnyLevelToString()).fStr << "] for reference [" << anyIdx.AsString() << "]");
			if (anyLevel.IsNull()) { // root level node
				preslot = any;
			} else {
				preslot = std::for_each(anyLevel.begin(), anyLevel.end(), resolveToAnyLevel(any)).result;
			}
			anyTraceAny(preslot, "replacement reference");
			if ( preslot.IsNull() ) {
				String m;
				m << "lookup of slotname [" << std::for_each(anyLevel.begin(), anyLevel.end(), appendAnyLevelToString()).fStr << "] failed!" << "\n";
				SystemLog::WriteToStderr(m);
			} else {
				if (anyIdx.GetType() == AnyLongType) {
					anyReferenced = preslot[anyIdx.AsLong(0L)];
				} else {
					anyReferenced = preslot[anyIdx.AsString()];
				}
				ref = std::for_each(anyReferenced.begin(), anyReferenced.end(), resolveToAnyLevel(any)).result;
				anyTraceAny(ref, "referenced any as string [" << std::for_each(anyReferenced.begin(), anyReferenced.end(), appendAnyLevelToString()).fStr << "]");
				if ( ref.IsNull() ) {
					String m;
					m << "lookup of reference [" << std::for_each(anyReferenced.begin(), anyReferenced.end(), appendAnyLevelToString()).fStr << "] failed!" << "\n";
					SystemLog::WriteToStderr(m);
				} else {
					if (anyIdx.GetType() == AnyLongType) {
						preslot[anyIdx.AsLong(0L)] = ref;
					} else {
						preslot[anyIdx.AsString()] = ref;
					}
				}
			}
		}
	}
};//lint !e1509

class AnythingParser {
	// really implement the grammar of Anythings
	// needs to be friend of Anything to set Anything's internals
public:
	AnythingParser(InputContext &c) :
			fContext(c) {
	}
	bool DoParse(Anything &a); // returns false if there was a syntax error
	bool DoParseSequence(Anything &a, ParserXrefHandler &xrefs);
	bool MakeSimpleAny(AnythingToken &tok, Anything &a);

private:
	void ImportIncludeAny(Anything &element, const String &url);
	void Error(String const &msg, String const &toktext);
	InputContext &fContext;
};

Anything::Anything(Allocator *a) :
		fAnyImp(0) {
	SetAllocator(a);
}
Anything::Anything(AnyImpl *ai) :
		fAnyImp(ai) {
	SetAllocator(ai ? ai->MyAllocator() : coast::storage::Current());
}
Anything::Anything(int i, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyLongImpl(i, a)) {
	SetAllocator(a);
}
#if !defined(BOOL_NOT_SUPPORTED)
Anything::Anything(bool b, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyLongImpl(b, a)) {
	SetAllocator(a);
}
#endif
Anything::Anything(long i, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyLongImpl(i, a)) {
	SetAllocator(a);
}
Anything::Anything(float f, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyDoubleImpl(f, a)) {
	SetAllocator(a);
}
Anything::Anything(double d, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyDoubleImpl(d, a)) {
	SetAllocator(a);
}
Anything::Anything(IFAObject *o, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyObjectImpl(o, a)) {
	SetAllocator(a);   // PS: Only for transient pointers NO checking!!
}
Anything::Anything(const String &s, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyStringImpl(s, a)) {
	SetAllocator(a);
}
Anything::Anything(const char *s, long len, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyStringImpl(s, len, a)) {
	SetAllocator(a);
}
Anything::Anything(void *buf, long len, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyBinaryBufImpl(buf, len, a)) {
	SetAllocator(a);
}

Anything::Anything(const Anything &any, Allocator *a) :
		fAnyImp(0) {
	SetAllocator(a);
	if (GetAllocator() == any.GetAllocator()) {
		// add reference
		fAnyImp = const_cast<AnyImpl*>(any.GetImpl());
		if (GetImpl()) {
			GetImpl()->Ref();
		}
	} else {
		// copy memory
		Anything xref;
		if (any.GetImpl()) {
			fAnyImp = any.GetImpl()->DeepClone(GetAllocator(), xref);
		}
	}
	if (!GetImpl()) {
		SetAllocator(a);    // remember allocator or make it sane in case of errors
	}
}
Anything::Anything(ArrayMarker m, Allocator *a) :
		fAnyImp(new ((a) ? a : coast::storage::Current()) AnyArrayImpl(a)) {
	SetAllocator(a);
}

Anything::~Anything() {
	if (GetImpl()) {
		GetImpl()->Unref();
		fAnyImp = 0;
	}
}

Anything Anything::DeepClone(Allocator *a) const
{
	Anything xref(a);
	if (GetImpl()) {
		return GetImpl()->DeepClone(a, xref);
	} else {
		return Anything(a);    // just set the allocator
	}
}

Anything Anything::DeepClone(Allocator *a, Anything &xref) const
{
	if (GetImpl()) {
		return GetImpl()->DeepClone(a, xref);
	} else {
		return Anything(a);    // just set the allocator
	}
}

long Anything::GetSize() const
{
	return GetImpl() ? GetImpl()->GetSize() : 0L;
}

AnyImplType Anything::GetType() const
{
	return GetImpl() ? GetImpl()->GetType() : AnyNullType;
}

void Anything::EnsureArrayImpl(Anything &anyToEnsure)
{
	anyStartTrace(Anything.EnsureArrayImpl);
	if ( anyToEnsure.GetType() != AnyArrayType ) {
		anyTrace("is not array");
		// behave friendly if current slot is not an array impl, eg don't lose current entry
		Anything expander = Anything(Anything::ArrayMarker(),anyToEnsure.GetAllocator());
		if ( !anyToEnsure.IsNull() ) {
			anyTrace("was not Null");
			expander.Append(anyToEnsure);
		}
		anyToEnsure = expander;
	}
}

long Anything::AsLong(long dflt) const
{
	if (GetImpl()) {
		return GetImpl()->AsLong(dflt);
	}
	return dflt;
}

bool Anything::AsBool(bool dflt) const
{
	if IsLongImpl(GetImpl()) {
		return (LongImpl(GetImpl())->AsLong(static_cast<long>(dflt)) != 0);
	}
	return dflt;
}

double Anything::AsDouble(double dflt) const
{
	if (GetImpl()) {
		return GetImpl()->AsDouble(dflt);
	}
	return dflt;
}

IFAObject *Anything::AsIFAObject(IFAObject *dflt) const
{
	if (IsObjectImpl(GetImpl())) {
		return ObjectImpl(GetImpl())->AsIFAObject(dflt);
	}
	return dflt;
}

const char *Anything::AsCharPtr(const char *dflt) const
{
	if (GetImpl()) {
		return GetImpl()->AsCharPtr(dflt);
	}
	return dflt;
}

const char *Anything::AsCharPtr(const char *dflt, long &buflen) const
{
	if (GetImpl()) {
		return GetImpl()->AsCharPtr(dflt, buflen);
	}
	if (dflt) {
		buflen = strlen(dflt);
	}
	return dflt;
}

String Anything::AsString(const char *dflt) const
{
	if (GetImpl()) {
		return GetImpl()->AsString(dflt);
	}
	return dflt;
}

void Anything::Expand()
{
	if (GetType() != AnyArrayType) {
		Allocator *al = this->GetAllocator();
		Assert(al != 0);
		AnyArrayImpl *a = new ((al) ? al : coast::storage::Current()) AnyArrayImpl(al);
		if ( a && GetType() != AnyNullType ) {
			a->At(0L) = *this; // this semantic is different from the Java version
		}
		if (this->GetImpl()) {
			this->GetImpl()->Unref();
		}
		this->fAnyImp = a;
		if (0 == a) {
			this->SetAllocator(al); // remember allocator in case of insanity or no memory
		}
	}
}

const Anything &Anything::At(long i) const
{
	return DoAt(i);
}

Anything &Anything::At(long i)
{
	return DoAt(i);
}
Anything &Anything::At(const char *k)
{
	return DoAt(k);
}
const Anything &Anything::At(const char *k) const
{
	return DoAt(k);
}

Anything  &Anything::DoAt(long i)
{
	Assert(i >= 0);
	if ( i > 0 || GetType() == AnyNullType ) {
		// check whether Expansion really needed
		// Expand if simple type and index > 0
		// or type is still null
		Expand();
	}
	return DoGetAt(i);
}
Anything const &Anything::DoAt(long i) const
{
	Assert(i >= 0);
	return DoGetAt(i);
}

Anything  &Anything::DoGetAt(long i)
{
	if ( GetType() != AnyArrayType ) {
		// if the type is not an AnyArrayType
		// just return this
		return *this;
	}
	// double check for the index range since in a productive version
	// Assert does expand to nothing --> should use size_t instead of long for indices
	return ArrayImpl(GetImpl())->At((i >= 0) ? i : 0);
}
Anything const &Anything::DoGetAt(long i) const
{
	if ( GetType() != AnyArrayType ) {
		// if the type is not an AnyArrayType
		// just return this
		return *this;
	}
	// double check for the index range since in a productive version
	// Assert does expand to nothing --> should use size_t instead of long for indices
	return ArrayImpl(GetImpl())->At((i >= 0) ? i : 0);
}

Anything &Anything::DoAt(const char *k)
{
	long i;
	if (k && (*k != 0)) {
		if ( (i = FindIndex(k)) == -1L) {
			Expand();
			Assert(AnyArrayType == GetImpl()->GetType());
			return ArrayImpl(GetImpl())->At(k);
		} else {
			return this->DoAt(i);
		}
	}
	return this->DoAt(GetSize());
}
Anything const &Anything::DoAt(const char *k)const
{
	long i;
	if (k && (*k != 0)) {
		if ( (i = FindIndex(k)) != -1L) {
			return this->DoAt(i);
		}
	}
	return this->DoAt(GetSize());
}

const char *Anything::SlotName(long slot) const
{
	if (IsArrayImpl(GetImpl())) {
		return ArrayImpl(GetImpl())->SlotName(slot);
	}
	return 0;
}

bool Anything::Remove(long slot)
{
	if (IsArrayImpl(GetImpl())) {
		ArrayImpl(GetImpl())->Remove(slot);
		return true;
	} else if ( slot == 0 && !IsNull() ) {
		clear();
		return true;
	}
	return false;
}

bool Anything::Remove(const char *k)
{
	if (IsArrayImpl(GetImpl())) {
		long slot = FindIndex(k);
		if (slot >= 0) {
			ArrayImpl(GetImpl())->Remove(slot);
			return true;
		}
	}
	return false;
}

void Anything::InsertReserve(long slot, long sz)
{
	if (IsArrayImpl(GetImpl())) {
		ArrayImpl(GetImpl())->InsertReserve(slot, sz);
	}
}

bool Anything::IsEqual(const Anything &other) const
{
	if (GetImpl() == other.GetImpl()) {
		return true;
	}
	if (GetImpl() && other.GetImpl()) {
		return GetImpl()->IsEqual(other.GetImpl());
	}
	return false;
}

bool Anything::IsEqual(const char *other) const
{
	if (GetImpl()) {
		if (other) {
			return (strcmp(AsCharPtr(""), other) == 0);
		}
	} else if (!other) {
		return true;
	}
	return false;
}

long Anything::FindIndex(const char *k, long sizehint, u_long hashhint) const
{
	Assert(k);
	if (IsArrayImpl(GetImpl())) {
		return ArrayImpl(GetImpl())->FindIndex(k, sizehint, hashhint);
	}
	return -1L;
}

long Anything::FindIndex(const long lIdx) const
{
	Assert(lIdx >= 0);
	if (IsArrayImpl(GetImpl())) {
		return ArrayImpl(GetImpl())->FindIndex(lIdx);
	}
	return -1L;
}

long Anything::FindValue(const char *k) const
{
	if (GetImpl()) {
		return GetImpl()->Contains(k);
	}
	return -1L;
}

bool Anything::Contains(const char *k) const
{
	if (GetImpl()) {
		return (GetImpl()->Contains(k) >= 0);
	}
	return false;
}

long Anything::Append(const Anything &a)
{
	long s = GetSize();
	At(s) = a;
	return s;
}

Anything &Anything::operator= (const Anything &a)
{
	if (GetImpl() != a.GetImpl()) {
		Allocator *al = GetAllocator();
		// do a deepclone if allocators don't match
		AnyImpl *oldImpl = GetImpl();
		if (a.GetAllocator() == al) {
			fAnyImp = const_cast<AnyImpl*>(a.GetImpl());
			if (GetImpl()) {
				GetImpl()->Ref();
			}
		} else if (a.GetImpl()) {
			Assert(al != 0);
			Anything xref;
			fAnyImp = a.GetImpl()->DeepClone(al, xref);
		} else {// empty any
			fAnyImp = 0;
		}
		if (!GetImpl()) {
			SetAllocator(al);
		}
		// make it sane if we remain empty
		if (oldImpl) {
			oldImpl->Unref();
		}
	}
	return *this;
}//lint !e1529

void Anything::SortByKey()
{
	if (IsArrayImpl(GetImpl())) {
		ArrayImpl(GetImpl())->SortByKey();
	}
}

void Anything::SortReverseByKey()
{
	if (IsArrayImpl(GetImpl())) {
		ArrayImpl(GetImpl())->SortReverseByKey();
	}
}

void Anything::SortByStringValues()
{
	if (IsArrayImpl(GetImpl())) {
		ArrayImpl(GetImpl())->SortByAnyComparer(AnyStringValueComparer());
	}
}

void Anything::SortByAnyComparer(const AnyComparer &ac)
{
	if (IsArrayImpl(GetImpl())) {
		ArrayImpl(GetImpl())->SortByAnyComparer(ac);
	}
}

class SimpleAnyPrinter: public AnyVisitor
{
protected:
	std::ostream &fOs;
	void PrintKey(const String &s) { // no copy for efficiency
		bool needquote = false;

		fOs << '/';

		if (isdigit( static_cast<unsigned char>(s[0L]))) {
			needquote = true;	// quote all numbers
		} else {
			for (long i = s.Length(); --i >= 0 && !needquote; ) {
				needquote = AnythingToken::isNameDelimiter(s[i]);
			}
		}

		if (needquote) {
			s.IntPrintOn(fOs, '\"');
		} else {
			fOs << s ;
		}
		fOs << ' ';
	}
	void ArrayBefore(const ROAnything , const AnyImpl *, long , const char *) {
		fOs << '{';
	}
	void ArrayBeforeElement(long lIdx, const String &key) {
		if (key.Length() > 0) {
			PrintKey(key);
		}
	}
	//void ArrayAfterElement(long index, const String &key){}
	void ArrayAfter(const ROAnything, const AnyImpl *, long , const char *) {
		fOs << '}';
	}

public:
	SimpleAnyPrinter(std::ostream &os): fOs(os) {}
	virtual void	VisitNull(long lIdx, const char *slotname) {
		fOs.put('*');
	}
	virtual void	VisitCharPtr(const String &value, const AnyImpl *id, long lIdx, const char *slotname) {
		value.IntPrintOn(fOs);
	}
	virtual void	VisitLong(long value, const AnyImpl *id, long lIdx, const char *slotname) {
		if ( id->GetType() == AnyLongType ) {
			fOs << id->AsString("");
		} else {
			// this section is just for the impossible case where...
			fOs << value;
		}
	}
	virtual void	VisitDouble(double value, const AnyImpl *id, long lIdx, const char *slotname) {
		if ( id->GetType() == AnyDoubleType ) {
			fOs << id->AsString("");
		} else {
			// this section is just for the impossible case where...
			// keep track of precision, so we can read in our
			// numbers anyway
			String strBuf;
			String::DoubleToString(value, strBuf);
			fOs << strBuf;
		}
	}
	virtual void	VisitVoidBuf(const String &value, const AnyImpl *, long lIdx, const char *slotname) {
		fOs << '[' << value.Length() << ';'; // separator
		fOs.write(value.cstr(), value.Length());
		fOs << ']';

	}
	virtual void	VisitObject(IFAObject *value, const AnyImpl *id, long lIdx, const char *slotname) {
		fOs << '&' << reinterpret_cast<unsigned long>(value);
	}
};

class PrettyAnyPrinter: public SimpleAnyPrinter
{
protected:
	long	fLevel;
	void Tab() {
		for (long i = 0; i < fLevel; ++i) {
			fOs.put(' ').put(' ');
		}
	}
	void ArrayBefore(const ROAnything , const AnyImpl *, long , const char *) {
		fOs << "{\n"; // } trick sniff
		++fLevel;
	}
	void ArrayBeforeElement(long lIdx, const String &key) {
		Tab();
		SimpleAnyPrinter::ArrayBeforeElement(lIdx, key);
	}
	void ArrayAfterElement(long lIdx, const String &key) {
		fOs << '\n';
	}
	void ArrayAfter(const ROAnything , const AnyImpl *, long , const char *) {
		--fLevel;
		Tab(); // { trick sniff
		fOs << '}';
	}

public:
	PrettyAnyPrinter(std::ostream &os, long level = 0): SimpleAnyPrinter(os), fLevel(level) {}
};
class XrefAnyPrinter : public PrettyAnyPrinter
{
protected:
	PrinterXrefHandler fXref;
	bool PrintAsXref(const AnyImpl *ai) {
		if (!fXref.IsDefined(reinterpret_cast<long>(ai))) {
			fXref.DefineBackRef(reinterpret_cast<long>(ai)); // remember symbolic slot name
			return false;
		} else {
			fOs << "%\"" << fXref.GetBackRef(reinterpret_cast<long>(ai)) << "\"";
			return true;
		}

	}
	void ArrayBeforeElement(long lIdx, const String &key) {
		PrettyAnyPrinter::ArrayBeforeElement(lIdx, key);
		if (key.Length() > 0) {
			fXref.PushKey(key);
		} else {
			fXref.PushIndex(lIdx);
		}
	}
	void ArrayAfterElement(long lIdx, const String &key) {
		fXref.Pop();
		PrettyAnyPrinter::ArrayAfterElement(lIdx, key);
	}

public:
	XrefAnyPrinter(std::ostream &os, long level = 0): PrettyAnyPrinter(os, level) {
	}
	//!@FIXME take mechanics from IntPrintOnWithRef to this class.
	virtual void	VisitCharPtr(const String &value, const AnyImpl *id, long lIdx, const char *slotname) {
		if (!PrintAsXref(id)) {
			PrettyAnyPrinter::VisitCharPtr(value, id, lIdx, slotname);
		}
	}
	//!trick to avoid leaking AnyArrayImpl class to the outside use ROAnything instead
	virtual void	VisitArray(const ROAnything value, const AnyImpl *id, long lIdx, const char *slotname) {
		if (!PrintAsXref(id)) {
			PrettyAnyPrinter::VisitArray(value, id, lIdx, slotname);
		}
	}
	virtual void	VisitLong(long value, const AnyImpl *id, long lIdx, const char *slotname) {
		if (!PrintAsXref(id)) {
			PrettyAnyPrinter::VisitLong(value, id, lIdx, slotname);
		}
	}
	virtual void	VisitDouble(double value, const AnyImpl *id, long lIdx, const char *slotname) {
		if (!PrintAsXref(id)) {
			PrettyAnyPrinter::VisitDouble(value, id, lIdx, slotname);
		}
	}
	virtual void	VisitVoidBuf(const String &value, const AnyImpl *id, long lIdx, const char *slotname) {
		if (!PrintAsXref(id)) {
			PrettyAnyPrinter::VisitVoidBuf(value, id, lIdx, slotname);
		}
	}
	virtual void	VisitObject(IFAObject *value, const AnyImpl *id, long lIdx, const char *slotname) {
		if (!PrintAsXref(id)) {
			PrettyAnyPrinter::VisitObject(value, id, lIdx, slotname);
		}
	}
};
std::ostream &Anything::PrintOn(std::ostream &os, bool pretty) const
{
	if (pretty) {
		PrettyAnyPrinter p(os);
		this->Accept(p);
	} else {
		SimpleAnyPrinter p(os);
		this->Accept(p);
	}
	return os;
}

void Anything::Export(std::ostream &os, int level) const
{
	if (! ! os) {
		XrefAnyPrinter pp(os, level);
		this->Accept(pp);
		os.flush(); // export should really flush it.
	}
}

long Anything::RefCount() const
{
	return (GetImpl()) ? GetImpl()->RefCount() : 0L;
}

bool Anything::Import(std::istream &is, const char *fname)
{
	if (! !is) {
		InputContext context(is, fname);
		AnythingParser p(context);
		if ( !p.DoParse(*this) ) {
			// there has been a syntax error
			String m("Anything::Import "), strFName(context.FileName());
			bool bHasExt = true;
			if ( !strFName.Length() && fname != NULL ) {
				strFName << fname;
				bHasExt = (strFName.SubString(strFName.Length() - 4L) == ".any");
			} else {
				strFName << "<NoName>";
			}
			m << strFName << (bHasExt ? ":" : ".any");
			m.Append(": syntax error");
			SYSERROR(m);
			return false;
		}

	} else {
		Allocator *a = GetAllocator();

		if (GetImpl()) {
			GetImpl()->Unref();
		}
		fAnyImp = 0;
		SetAllocator(a); // remember allocator
		return false;
	}
	return true;
}

bool Anything::LookupPath(Anything &result, const char *path, char delimSlot, char delimIdx) const
{
	// do some shortcut if delimSlot does not exist in path
	if (delimSlot == '\000' || delimIdx == '\000' || (!strchr(NotNull(path), delimSlot) && !strchr(NotNull(path), delimIdx)) ) {
		// '\000' is not a valid delimiter, we do not use it, fast special case
		long lIdx = FindIndex(path);
		if (lIdx >= 0) {
			result = DoGetAt(lIdx);
			return true;
		}
	} else {
		// calculate key values into anything; cache hash values and size information
		// assume we have at least one delimSlot in path
		register const char *tokPtr = path;
		if (!tokPtr || *tokPtr == delimSlot) {
			return false;
		}
		Anything c = *this;
		do {
			register long lIdx = -1;
			if (*tokPtr == delimIdx) {
				if (! *++tokPtr || *tokPtr == delimIdx || *tokPtr == delimSlot) {
					return false;
				}
				lIdx = 0;
				while (isdigit(*tokPtr)) { //*tokPtr != '\0' &&  *tokPtr != delimSlot && *tokPtr != delimIdx)
					lIdx *= 10;
					lIdx += (*tokPtr++ -'0');
				}
				if (*tokPtr != '\0' &&  *tokPtr != delimSlot && *tokPtr != delimIdx) {
					return false; // not a valid number
				}
				// check if index is defined
				if (lIdx >= c.GetSize() || c[lIdx].IsNull()) {
					return false;
				}
			} else if (*tokPtr) {
				if (*tokPtr == delimSlot) {
					++tokPtr;
					if (*tokPtr == '\0' || *tokPtr == delimSlot || *tokPtr == delimIdx) {
						return false;
					}
				}
				// calculate hash value while iterating for next delimSlot
				long keylen = 0;
				long h = IFAHash(tokPtr, keylen, delimSlot, delimIdx);
				// find the index with precalculated hash and sizes
				if ((lIdx = c.FindIndex(tokPtr, keylen, h)) < 0) {
					return false;
				}
				tokPtr += keylen;
			} else {
				return false;
			}
			c = c.DoGetAt(lIdx);
		} while (*tokPtr != '\0');
		// we got it
		result = c;
		return true;
	}
	return false;
}

Allocator *Anything::GetAllocator() const {
	if (GetImpl()) {
		return GetImplAllocator();
	}
	return reinterpret_cast<Allocator *>(bits & ~0x01);
}

bool Anything::SetAllocator(Allocator *a) {
	if (!GetImpl() || !fAlloc) {
		fAlloc = (a) ? a : coast::storage::Current();
		bits |= 0x01;
		return (a != 0);
	}
	return false;
}

Allocator *Anything::GetImplAllocator() const {
	if (GetImpl()) {
		return GetImpl()->MyAllocator();
	}
	return 0;
}

AnyImpl const *Anything::GetImpl() const {
	if (bits & 0x01) {
		return 0;
	} else {
		return fAnyImp;
	}
}
AnyImpl *Anything::GetImpl() {
	if (bits & 0x01) {
		return 0;
	} else {
		//!@TODO: silently throws away constness!!!!
		return const_cast<AnyImpl*>(fAnyImp);
	}
}

void Anything::Accept(AnyVisitor &v, long lIdx, const char *slotname) const
{
	if (GetImpl()) {
		GetImpl()->Accept(v, lIdx, slotname);//lint !e613
	} else {
		v.VisitNull(lIdx, slotname);
	}
}

//-- implement STL support functionality of class Anything

Anything::iterator Anything::begin()
{
	return Anything_iterator(*this, 0);
}
Anything::iterator Anything::end()
{
	return Anything_iterator(*this, GetSize());
}
Anything::const_iterator Anything::begin()const
{
	return Anything_const_iterator(*this, 0);
}
Anything::const_iterator Anything::end() const
{
	return Anything_const_iterator(*this, GetSize());
}

Anything::reverse_iterator Anything::rbegin()
{
	return reverse_iterator(end());
}
Anything::reverse_iterator Anything::rend()
{
	return reverse_iterator(begin());
}
Anything::const_reverse_iterator Anything::rbegin()const
{
	return const_reverse_iterator(end());
}
Anything::const_reverse_iterator Anything::rend() const
{
	return const_reverse_iterator(begin());
}
void Anything::clear()
{
	// may be not most efficient, but working
	*this = Anything(GetAllocator());
}
Anything::iterator Anything::erase(Anything::iterator pos)
{
	if (&pos.a == this) {
		if (pos.position >= 0 && pos.position < this->GetSize()) {
			Remove(pos.position);
			return pos;
		}
	}
	return this->end(); // should throw, but stay robust
}
Anything::iterator Anything::erase(Anything::iterator from, Anything::iterator to)
{
	if (&from.a == this && &to.a == this) {
		if (from.position >= 0 && from.position <= to.position && to.position <= this->GetSize()) {
			while (--to >= from) {
				Remove(to.position);
			}
			return from;
		}
	}
	return this->end(); // should throw, but stay robust
}
void Anything::swap(Anything &that)
{
	// we use an anonymous union of pointers and bits. this could be dangerous....
	Assert(sizeof(bits) == sizeof(fAnyImp));
	Assert(sizeof(fAnyImp) == sizeof(fAlloc));
	std::swap(this->fAnyImp,that.fAnyImp);
}

Anything::iterator Anything::do_insert(iterator pos, size_type n, const value_type &v)
{
	if (&(pos.a) != this || pos > end()) {
		return end();    // no op
	}
	// handles empty Anything as well as appending, where no adjustment is needed
	// need to deal with a special case of inserting 1 element into a null anything
	if (pos >= end() || n < 1) {
		long res = GetSize();
		if (n == 1 && IsNull()) {
			*this = v;
		} else {
			while (n-- > 0) {
				Append(v);
			}
		}
		return Anything::iterator(*this, res);
	}
	// handles simple Anythings with insertion in the front
	if ( GetType() != AnyArrayType ) {
		Expand();
		*(pos + n) = *pos;
	} else {
		// deal with insertion into arrays
		InsertReserve(pos.position, n);
	}
	while (n-- > 0) {
		*(pos + n) = v;
	}
	// now we need to forward it to the array-impl and keep track of the hash
	// otherwise we need to make space available.... not yet done
	return pos;
}

ROAnything::ROAnything() : fAnyImp(0)
{
}

ROAnything::ROAnything(const Anything &a)
	: fAnyImp(a.GetImpl())
{
	// no ref necessary
	// ROAnything don't do any
	// ref counting
}

ROAnything::ROAnything(const ROAnything &a)
	: fAnyImp(a.fAnyImp)//lint !e1554
{
	// no ref necessary
	// ROAnything don't do any
	// ref counting
}

Anything ROAnything::DeepClone(Allocator *a) const
{
	anyStartTrace(ROAnything.DeepClone);
	Anything xref(a);
	if (fAnyImp) {
		return fAnyImp->DeepClone(a, xref);
	} else {
		return Anything(a);
	}
}

long ROAnything::GetSize() const
{
	if (fAnyImp) {
		return fAnyImp->GetSize();
	}
	return 0L;
}

ROAnything &ROAnything::operator= (const ROAnything &a)
{
	fAnyImp = a.fAnyImp;//lint !e1555
	return *this;
}//lint !e1529

ROAnything &ROAnything::operator= (const Anything &a)
{
	fAnyImp = a.GetImpl();
	return *this;
}

AnyImplType ROAnything::GetType() const
{
	if (fAnyImp) {
		return fAnyImp->GetType();
	} else {
		return AnyNullType;
	}
}

long ROAnything::AsLong(long dflt) const
{
	if (fAnyImp) {
		return fAnyImp->AsLong(dflt);
	}
	return dflt;
}

bool ROAnything::AsBool(bool dflt) const
{
	if (IsLongImpl(fAnyImp)) {
		return (LongImpl(fAnyImp)->AsLong(static_cast<long>(dflt)) != 0);
	}
	return dflt;
}

double ROAnything::AsDouble(double dflt) const
{
	if (fAnyImp) {
		return fAnyImp->AsDouble(dflt);
	}
	return dflt;
}

IFAObject *ROAnything::AsIFAObject(IFAObject *dflt) const
{
	if (IsObjectImpl(fAnyImp)) {
		return ObjectImpl(fAnyImp)->AsIFAObject(dflt);
	}
	return dflt;
}

const char *ROAnything::AsCharPtr(const char *dflt) const
{
	if (fAnyImp) {
		return fAnyImp->AsCharPtr(dflt);
	}
	return dflt;
}

const char *ROAnything::AsCharPtr(const char *dflt, long &buflen) const
{
	if (fAnyImp) {
		return fAnyImp->AsCharPtr(dflt, buflen);
	}
	if (dflt) {
		buflen = strlen(dflt);
	}
	return dflt;
}

String ROAnything::AsString(const char *dflt) const
{
	if (fAnyImp) {
		return fAnyImp->AsString(dflt);
	}
	return dflt;
}

long ROAnything::AssertRange(long i) const
{
	if ( (i < 0)  || (i >= GetSize()) ) {
		return -1;
	}
	return i;
}

long ROAnything::AssertRange(const char *k) const
{
	return AssertRange(FindIndex(k));
}

const ROAnything ROAnything::At(long i) const
{
	if ( AssertRange(i) != -1 ) {
		if ( GetType() != AnyArrayType ) {
			// if the type is not an AnyArrayType
			// just return this
			return *this;
		}
		return ROAnything(ArrayImpl(fAnyImp)->At(i));
	}
	return ROAnything();
}

const ROAnything ROAnything::At(const char *k) const
{
	long lIdx;
	if ( (lIdx = AssertRange(k)) != -1 ) {
		return this->At(lIdx);
	}
	return ROAnything();
}

const char *ROAnything::SlotName(long slot) const
{
	if (IsArrayImpl(fAnyImp)) {
		return ArrayImpl(fAnyImp)->SlotName(slot);
	}
	return 0;
}
const String &ROAnything::VisitSlotName(long slot) const
{
	if (IsArrayImpl(fAnyImp)) {
		return ArrayImpl(fAnyImp)->VisitSlotName(slot);
	}
	return fgStrEmpty;
}

bool ROAnything::IsEqual(const ROAnything &other) const
{
	if (fAnyImp == other.fAnyImp) {
		return true;
	}
	if (fAnyImp && other.fAnyImp) {
		return fAnyImp->IsEqual(other.fAnyImp);
	}
	return false;
}

bool ROAnything::IsEqual(const Anything &other) const
{
	if (fAnyImp == other.GetImpl()) {
		return true;
	}
	if (fAnyImp && other.GetImpl()) {
		return fAnyImp->IsEqual(other.GetImpl());
	}
	return false;
}

bool ROAnything::IsEqual(const char *other) const
{
	if (fAnyImp) {
		if (other) {
			return (strcmp(AsCharPtr(""), other) == 0);
		}
	} else if (!other) {
		return true;
	}
	return false;
}

long ROAnything::FindIndex(const char *k, long sizehint, u_long hashhint) const
{
	Assert(k);
	if (IsArrayImpl(fAnyImp)) {
		return ArrayImpl(fAnyImp)->FindIndex(k, sizehint, hashhint);
	}
	return -1L;
}

long ROAnything::FindIndex(const long lIdx) const
{
	Assert(lIdx >= 0);
	if (IsArrayImpl(fAnyImp)) {
		return ArrayImpl(fAnyImp)->FindIndex(lIdx);
	}
	return -1L;
}

long ROAnything::FindValue(const char *k) const
{
	if (fAnyImp) {
		return fAnyImp->Contains(k);
	}
	return -1L;
}

bool ROAnything::Contains(const char *k) const
{
	if (fAnyImp) {
		return (fAnyImp->Contains(k) >= 0);
	}
	return false;
}

std::ostream &ROAnything::PrintOn(std::ostream &os, bool pretty) const
{
	if (pretty) {
		PrettyAnyPrinter p(os);
		this->Accept(p);
	} else {
		SimpleAnyPrinter p(os);
		this->Accept(p);
	}
	return os;
}

void ROAnything::Export(std::ostream &os, int level) const
{
	if (! ! os) {
		XrefAnyPrinter pp(os, level);
		this->Accept(pp);
		os.flush(); // export should really flush it.
	}
}

bool ROAnything::LookupPath(ROAnything &result, const char *path, char delimSlot, char delimIdx) const
{
	// do some shortcut if delimSlot does not exist in path
	if (delimSlot == '\000' || delimIdx == '\000' || (!strchr(NotNull(path), delimSlot) && !strchr(NotNull(path), delimIdx)) ) {
		// '\000' is not a valid delimiter, we do not use it, fast special case
		long lIdx = FindIndex(path);
		if (lIdx >= 0) {
			result = At(lIdx);
			return true;
		}
	} else {
		// calculate key values into anything; cache hasvalues and size information
		// assume we have at least one delimSlot in path
		//String key(path); // do not need global allocator here!
		register const char *tokPtr = path; //(const unsigned char *)(const char*)key;
		if (!tokPtr || *tokPtr == delimSlot) {
			return false;
		}

		ROAnything c(*this); // pointers are faster! but ROAnythings are pointers...
		do {
			register long lIdx = -1;
			if (*tokPtr == delimIdx) {
				if (! *++tokPtr || *tokPtr == delimIdx || *tokPtr == delimSlot) {
					return false;
				}
				lIdx = 0;
				while (isdigit(*tokPtr)) { //*tokPtr != '\0' &&  *tokPtr != delimSlot && *tokPtr != delimIdx)
					lIdx *= 10;
					lIdx += (*tokPtr++ -'0');
				}
				if (*tokPtr != '\0' &&  *tokPtr != delimSlot && *tokPtr != delimIdx) {
					return false; // not a valid number
				}
				// check if index is defined
				if (lIdx >= c.GetSize() || c[lIdx].IsNull()) { // caution: auto-enlargement!
					return false;
				}
			} else if (*tokPtr) {
				if (*tokPtr == delimSlot) {
					++tokPtr;
					if (*tokPtr == '\0' || *tokPtr == delimSlot || *tokPtr == delimIdx) {
						return false;
					}
				}
				// calculate hashvalue while iterating for next delimSlot
				long keylen = 0;
				long h = IFAHash(tokPtr, keylen, delimSlot, delimIdx);
				// find the index with precalculated hash and sizes
				if ((lIdx = c.FindIndex(tokPtr, keylen, h)) < 0) {
					return false;
				}
				tokPtr += keylen;
			} else {
				return false;
			}
			c = c.At(lIdx);
		} while (*tokPtr != '\0');
		// we got it
		result = c;
		return true;
	}
	return false;
}

void ROAnything::Accept(AnyVisitor &v, long lIdx, const char *slotname) const
{
	if (fAnyImp) {
		fAnyImp->Accept(v, lIdx, slotname);
	} else {
		v.VisitNull(lIdx, slotname);
	}
}

void InputContext::SkipToEOL()
{
	char c = ' ';

	while (Get(c) && c != '\n' && c != '\r') { }
	if (c == '\n' || c == '\r') {
		++fLine; // count contexts lines
		// we should treat DOS-convention of CRLF nicely by reading it
		char crnl;
		if (Get(crnl) &&
			((crnl != '\r' && c == '\n') ||
			 (c == '\r' && crnl != '\n')) ) {
			Putback(crnl); // no crlf lfcr sequence
		}
	} // we have reached EOF just do nothing
}

// expect
// Any:     '{' sequence_including_}
//     |    eString | eDecimalNumber | eOctalNumber | eHexNumber | eFloatNumber
//     |  '*' | '&' eDecimalNumber | eBinaryBufImpl | eError
bool AnythingParser::DoParse(Anything &any)
{
	anyStartTrace(AnythingParser.DoParse);
	// free old impl
	Allocator *a = any.GetAllocator();
	any = Anything((a) ? a : coast::storage::Current()); // assignment should be OK, but we keep it safe

	ParserXrefHandler xrefs;
	AnythingToken tok(fContext);
	bool ok = false;
	if ( '{' == tok.Token()) {
		ok = DoParseSequence(any, xrefs);
	} else {
		ok = MakeSimpleAny(tok, any);
	}
	xrefs.Patch(any);
	// here we would check for complete input- the next token should be eNullSym
	// but reading from a socket blocks us here. This check won't work.
	// it might be possible to only check it if we are reading a file
	// this is the case if fContext.fIs && fContext.fFileName != ""
//    tok = AnythingToken(fContext);
//    if (tok.fToken != AnythingToken::eNullSym) {
//        Error("too much input:",tok.fText);
//        ok = false;
//    }
	return ok;
}

bool AnythingParser::DoParseSequence(Anything &any, ParserXrefHandler &xrefs)
{
	anyStartTrace(AnythingParser.DoParseSequence);
	Allocator *a = (any.GetAllocator()) ? any.GetAllocator() : coast::storage::Current();
	bool ok = true;
	// we need to make it an array
	any = Anything(Anything::ArrayMarker(),a);
	do {
		Anything element(a);
		String key;
		bool lastok = true;
		AnythingToken tok(fContext);
		anyTrace("Token [" << tok.Token() << "] Text [" << tok.Text() << "]");
	restart:    // for behaving nicely in case of a syntax error
		switch (tok.Token()) {
			case '}' : // '{' this is to cheat sniff
				return ok; // we are done...
			case AnythingToken::eError: {
				lastok = false; // try to resync
				String strError("syntax error invalid token:");
				strError.Append(static_cast<long>(tok.Token()));
				Error(strError, tok.Text().DumpAsHex(tok.Text().Length()));
				break;
			}
			case AnythingToken::eNullSym:
				Error("premature EOF token encountered", tok.Text());
				return false;

			case AnythingToken::eInclude : {
				ImportIncludeAny(element, tok.Text());
				any.Append(element);
				break;
			}
			case AnythingToken::eRef : {
				// a unnamed slot with a reference
				anyTrace("AnythingToken::eRef");
				// we use '%' to divide the slotNamePath and the index of the unnamed slot
				// to keep the order of the inserted slots as requested we need
				// to add a dummy slot which will be linked in a second step
				element = escapedQueryStringToAny(tok.Text());
				anyTraceAny(element,"eRef");
				long lIdx = any.Append(element);
				// add temporary for resolving reference after anything is fully parsed
				xrefs.DefinePatchRef(lIdx);
				break;
			}
			case AnythingToken::eIndex :
				anyTrace("AnythingToken::eIndex");
				key = tok.Text(); // remember index
				anyTrace("key:" << key);
				if (key.Length() > 0) {
					if (any.IsDefined(key)) {
						// double definition, should be a warning....
						Error("Anything index double defined, last takes precedence", tok.Text());
						// key = "";
						// leave ok OK, because we haven't ok = false;
					}
				}
				tok = AnythingToken(fContext); // get next one
				if ( AnythingToken::eRef == tok.Token() ) {
					// link it to the given slotname (must exist)
					element = escapedQueryStringToAny(tok.Text());
					anyTraceAny(element,"eRef within eIndex");
					any[key] = element;

					// add temporary for resolving reference after anything is fully parsed
					xrefs.DefinePatchRef(key);
					// to keep the order of the inserted slots as requested we need
					// to add a dummy slot which will be linked in a second step
					break;
				} else if (AnythingToken::eInclude == tok.Token()) {
					ImportIncludeAny(element, tok.Text());
					any[key] = element;
					break;
				} else if (AnythingToken::eIndex == tok.Token()) {
					// this is an error
					Error("missing Anything at index ", key);
					if (key.Length() > 0) {
						any[key] = Anything(a); // behave fault tolerant
						key.Trim(0); // forget key
					}
					// we need a goto here to behave friendly
					goto restart;
				}
				// No break here! Fall through, read simple Anything
			default:
				if ( '{' == tok.Token() ) {
					long lIdx = 0;
					Anything marklevel = xrefs.ParseLevel();
					if (key.Length() > 0) {
						xrefs.PushKey(key);
					} else {
						lIdx = any.Append(element);
						xrefs.PushIndex(lIdx);
					}
					lastok = DoParseSequence(element, xrefs);
					xrefs.ResetLevel(marklevel);
					if (key.Length() == 0) {
						any[lIdx] = element;
						break;
					}
				} else {
					lastok = MakeSimpleAny(tok, element);
				}
				if (key.Length() > 0) {
					any[key] = element;
				} else if (element.GetType() != AnyNullType || lastok) {
					any.Append(element);
				} // do not append empty anything in case of a syntax error
		} // switch
		ok = ok && lastok;
	} while (fContext.IsGood()) ; // may be we should check for i < somemaximum
	// we have a premature EOF...
	return false;
}

// sets a.fAnyImp according to tok for simple values
bool AnythingParser::MakeSimpleAny(AnythingToken &tok, Anything &any)
{
	Allocator *a = (any.GetAllocator()) ? any.GetAllocator() : coast::storage::Current();
	Assert(a != 0);
	switch ( tok.Token() ) {
		case '*' :
			any = Anything(a);
			break;
		case AnythingToken::eStringError:
			// notify user about error but behave nicely if string is not empty
			Error("syntax error: invalid string constant (missing quote)", tok.Text());
			if (tok.Text().Length() <= 0) {
				// ignore empty strings otherwise fall through!
				return false;
			}
			//fall through
		case AnythingToken::eString: // string impl//lint !e616
			any = tok.Text();
			break;
			// long impl.
		case AnythingToken::eDecimalNumber:
			{
				long number = 0;
				if ( coast::system::StrToL(number, tok.Text().cstr()) ) {
					any = number;
				}
				break;
			}
		case AnythingToken::eOctalNumber:
			{
				unsigned long number = 0;
				if ( coast::system::StrToUL(number, tok.Text().cstr(), 8) ) {
					any = static_cast<long>(number);
				}
				break;
			}
		case AnythingToken::eHexNumber:
			{
				unsigned long number = 0;
				if ( coast::system::StrToUL(number, tok.Text().cstr(), 16) ) {
					any = static_cast<long>(number);
				}
				break;
			}
		case AnythingToken::eFloatNumber:
			{
				double number = 0.0;
				if ( coast::system::StrToD(number, tok.Text().cstr()) ) {
					any = number;
				}
				break;
			}
			break;
		case AnythingToken::eBinaryBuf:
			// oops we cannot yet assign a binary-buf impl ?
			// but a temporary anything should be sufficient
			any = Anything(reinterpret_cast<void *>(const_cast<char *>(tok.Text().cstr())), tok.Text().Length(), a);
			break;
		case AnythingToken::eObject:
			{
				unsigned long number = 0;
				if ( coast::system::StrToUL(number, tok.Text().cstr(), 10) ) {
					any = Anything(reinterpret_cast<IFAObject *>(number), a);
				}
				break;
			}
		case AnythingToken::eNullSym:
			Error("unexpected EOF token encountered", "");
			return false;
		default: {
			// this is an error
			String strError("syntax error invalid token:");
			strError.Append(static_cast<long>(tok.Token()));
			Error(strError, tok.Text().DumpAsHex(tok.Text().Length()));
			return false;
		}
	}
	return true;
}

void AnythingParser::ImportIncludeAny(Anything &element, const String &url)
{
	anyStartTrace(AnythingParser.ImportIncludeAny);
	Anything query;

	long colonpos = url.StrChr(':');
	String protocol = colonpos > 0 ? url.SubString(0, colonpos) : String();
	String fileName = url.SubString(colonpos + 1);
	protocol.ToUpper();

	// we support only file protocol
	if (0 == protocol.Length() || protocol == "FILE") {
		// look for a query
		long 	queryIdx    = fileName.StrChr('?');
		String  queryString;

		if ( queryIdx >= 0 ) {
			queryString = fileName.SubString(queryIdx + 1);
			fileName.Trim(queryIdx);
		}
		if (0 == fileName.Contains("///")) {
			// squash URL syntax
			fileName.TrimFront(3);
		}

		std::iostream *pStream = system::OpenStream(fileName, "");
		if (pStream) {
			if ( element.Import(*pStream, fileName) && queryString.Length() > 0 ) {
				Anything anyLevel = escapedQueryStringToAny(queryString);
				element = std::for_each(anyLevel.begin(), anyLevel.end(), resolveToAnyLevel(element)).result;
				if ( element.IsNull() ) {
					String m;
					m << "lookup of " << fileName << " slotname [" << std::for_each(anyLevel.begin(), anyLevel.end(), appendAnyLevelToString()).fStr << "] failed!" << "\n";
					SystemLog::WriteToStderr(m);
				}
			}
			delete pStream;
		} else {
			Error("cannot open included Anything at", url);
		}
	} else {
		Error("protocol not yet supported", url);
	}
}

void AnythingParser::Error(String const &msg, String const &toktext)
{
	// put a space in front to give poor Sniff a chance
	String m(fContext.FileName());
	bool bHasExt = true;
	if ( !m.Length() ) {
		m << "<NoName>";
	} else {
		bHasExt = (m.SubString(m.Length() - 4L) == ".any");
	}
	m << (bHasExt ? ":" : ".any:") << fContext.LineRef() << " " << msg << " [" << toktext << "]";
	SYSWARNING(m);
}

String Anything::CompareForTestCases(const ROAnything &expected, const ROAnything &actual, bool &result)
{
	String sexp, act;
	String retval;
	OStringStream oexp(&sexp), oact(&act);
	expected.Export(oexp, false);
	actual.Export(oact, false);
	result = (sexp == act);
	if (!result) {
		retval << "expected: " << sexp << " but was: " << act;
	}
	return retval;
}

void SlotFinder::Operate(Anything &source, Anything &dest, const Anything &config)
{
	StartTrace(SlotFinder.Operate);
	ROAnything ROconfig(config);
	Operate(source, dest, ROconfig);
}

void SlotFinder::Operate(Anything &source, Anything &dest, const ROAnything &config)
{
	StartTrace(SlotFinder.Operate);
	TraceAny(config, "Config");
	Operate(source, dest, config["Slot"].AsString(""), config["Delim"].AsCharPtr(".")[0L], config["IndexDelim"].AsCharPtr(":")[0L]);
}
void SlotFinder::Operate(Anything &source, Anything &dest, String destSlotname, char delim, char indexdelim )
{
	StartTrace(SlotFinder.Operate);
	Trace("Destination slotname [" << destSlotname << "]");
	SubTraceAny(TraceAny, source, "Source");
	dest = source;
	long destIdx = -1L;
	if ( IntOperate(dest, destSlotname, destIdx, delim, indexdelim) ) {
		if (destSlotname.Length()) {
			if ( !dest.IsDefined(destSlotname) || dest[destSlotname].IsNull() ) {
				Trace("adding slot [" << destSlotname << "]");
				dest[destSlotname] = Anything(Anything::ArrayMarker(),dest.GetAllocator());
			}
			dest = dest[destSlotname];
		} else {
			if (!dest.IsDefined(destIdx)) {
				Trace("adding indexslot [" << destIdx << "]");
				dest[destIdx] = Anything(Anything::ArrayMarker(),dest.GetAllocator());
			}
			dest = dest.At(destIdx);
		}
	}
	SubTraceAny(TraceAny, dest, "Destination is finally:");
}
bool SlotFinder::IntOperate(Anything &dest, String &destSlotname, long &destIdx, char delim, char indexdelim)
{
	StartTrace(SlotFinder.IntOperate);
	Trace("processing[" << destSlotname << "]");
	long lIdxDelim;
	String s;
	if ( (lIdxDelim = destSlotname.StrChr(indexdelim)) != -1 ) {
		s = destSlotname.SubString(0, lIdxDelim);
		if (s.Length()) {
			Trace("part before index[" << s << "]");
			if ( IntOperate(dest, s, destIdx, delim, indexdelim) ) {
				// ensure that we have a valid anything
				if (dest[s].GetType() == AnyNullType) {
					dest[s] = Anything(Anything::ArrayMarker(),dest.GetAllocator());
				}
				dest = dest[s];
				SubTraceAny(TraceAny, dest, "dest so far");
			}
		}
		destSlotname.TrimFront(lIdxDelim + 1);
		Trace("remaining string[" << destSlotname << "]");

		// here we want to get the index token
		StringTokenizer2 tokenizer(destSlotname, String() << delim << indexdelim);

		// get the index now
		if (tokenizer.NextToken(s)) {
			destIdx = s.AsLong(-1L);
			Trace("index:" << destIdx);
			destSlotname = tokenizer.GetRemainder(true);
			if (destSlotname.Length() && destSlotname[0L] == delim) {
				destSlotname.TrimFront(1);
			}
		}
		Trace("remaining string[" << destSlotname << "]");
		if (destSlotname.Length()) {
			// ensure that we have a valid anything
			if (dest[destIdx].GetType() == AnyNullType) {
				dest[destIdx] = Anything(Anything::ArrayMarker(),dest.GetAllocator());
			}
			// adjusting dest anything with slotindex
			dest = dest[destIdx];
			SubTraceAny(TraceAny, dest, "dest so far");
			Trace("calling IntOperate(" << destSlotname << ")");
			return IntOperate(dest, destSlotname, destIdx, delim, indexdelim);
		} else {
			// we are done, no more slots to resolve
			return true;
		}
	}
	StringTokenizer st(destSlotname, delim);
	bool keepOn = st.NextToken(s);

	while (keepOn) {
		String k = s;
		keepOn = st.NextToken(s);
		Trace("k is " << k);
		if (keepOn) {
			if ( !dest.IsDefined(k) || dest[k].IsNull() ) {
				// insert non-existing slots on the fly
				dest[k] = Anything(Anything::ArrayMarker(),dest.GetAllocator());
			}
		} else {
			destSlotname = k;
			return true;
		}
		dest = dest[k];
		SubTraceAny(TraceAny, dest, "dest so far");
	}
	return false;
}

void SlotPutter::Operate(Anything &source, Anything &dest, const Anything &config)
{
	StartTrace(SlotPutter.Operate);
	ROAnything ROconfig(config);
	Operate(source, dest, ROconfig);
}

void SlotPutter::Operate(Anything &source, Anything &dest, const ROAnything &config)
{
	StartTrace(SlotPutter.Operate);
	TraceAny(config, "Config");
	TraceAny(source, "Source");
	SubTraceAny(TraceAny, dest, "Destination");
	Operate(source, dest, config["Slot"].AsString(""), config["Append"].AsBool(false), config["Delim"].AsCharPtr(".")[0L], config["IndexDelim"].AsCharPtr(":")[0L] );
}

void SlotPutter::Operate(Anything &source, Anything &dest, String destSlotname, bool append, char delim, char indexdelim )
{
	StartTrace(SlotPutter.Operate);
	Trace("Destination slotname[" << destSlotname << "]");
	Trace("sourceImpl:" << static_cast<long>(source.GetType()) << " destImpl:" << static_cast<long>(dest.GetType()));
	Trace("source any alloc:" << reinterpret_cast<long>(source.GetAllocator()) << " dest.alloc:" << reinterpret_cast<long>(dest.GetAllocator()));
	SubTraceAny(TraceAny, source, "source");
	// ensure that the destination anything is real
	Anything::EnsureArrayImpl(dest);

	Anything work(dest, dest.GetAllocator());
	long destIdx = -1L;
	if (SlotFinder::IntOperate(work, destSlotname, destIdx, delim, indexdelim)) {
		if (append) {
			if (destSlotname.Length()) {
				Trace("appending source to dest[" << destSlotname << "]");
				work[destSlotname].Append(source);
			} else {
				Trace("appending source to dest[" << destIdx << "]");
				work[destIdx].Append(source);
			}
		} else {
			if (destSlotname.Length()) {
				Trace("replacing dest[" << destSlotname << "] with source");
				work[destSlotname] = source;
			} else {
				Trace("replacing dest[" << destIdx << "] with source");
				work[destIdx] = source;
			}
		}
	}
	SubTraceAny(TraceAny, dest, "destination after");
}

void SlotCleaner::Operate(Anything &dest, const Anything &config)
{
	StartTrace(SlotCleaner.Operate);
	Operate(dest, ROAnything(config));
}

void SlotCleaner::Operate(Anything &dest, const ROAnything &config)
{
	StartTrace(SlotCleaner.Operate);
	TraceAny(config, "Config");
	Operate(dest, config["Slot"].AsString(""), (config["RemoveLast"].AsLong(0L) == 1L), config["Delim"].AsCharPtr(".")[0L], config["IndexDelim"].AsCharPtr(":")[0L]);
}

void SlotCleaner::Operate(Anything &dest, String slotName, bool removeLast, char delim, char indexdelim)
{
	StartTrace(SlotCleaner.Operate);

	if (slotName.Length()) {
		Trace("Destination slotname [" << slotName << "]");
		// first of all, get the correct store
		Anything anyParent(dest, dest.GetAllocator());

		// test if the path to be deleted exists in the store, avoids creation of nonexisting slot
		Anything anySlotTest(dest.GetAllocator());
		if (anyParent.LookupPath(anySlotTest, slotName, delim, indexdelim)) {
			// use SlotFinders IntOperate to get the correct parent anything and the slotname/index
			// to remove from
			long slotIndex = -1L;
			if (SlotFinder::IntOperate(anyParent, slotName, slotIndex, delim, indexdelim)) {
				if (slotName.Length()) {
					long sz = anyParent[slotName].GetSize();
					if (removeLast && sz > 1) {
						TraceAny(anyParent[slotName], "removing slot in this any");
						Trace("removing last slot");
						anyParent[slotName].Remove(sz - 1);
					} else {
						TraceAny(anyParent, "removing slot in this any");
						Trace("removing named slot [" << slotName << "]");
						anyParent.Remove(slotName);
					}
				} else if (slotIndex != -1L) {
					long sz = anyParent[slotIndex].GetSize();
					if (removeLast && sz > 1) {
						TraceAny(anyParent[slotIndex], "removing slot in this any");
						Trace("removing last slot");
						anyParent[slotIndex].Remove(sz - 1);
					} else {
						TraceAny(anyParent, "removing slot in this any");
						Trace("removing index slot [" << slotIndex << "]");
						anyParent.Remove(slotIndex);
					}
				}
			}
		} else {
			Trace("path to be deleted not found! [" << slotName << "]");
			// as we do not have to delete anything we return true anyway
		}
	}
}

void SlotCopier::Operate(Anything &source, Anything &dest, const Anything &config, char delim, char indexdelim)
{
	StartTrace(SlotCopier.Operate);
	ROAnything ROconfig(config);
	Operate(source, dest, ROconfig, delim, indexdelim);
}

void SlotCopier::Operate(Anything &source, Anything &dest, const ROAnything &config, char delim, char indexdelim)
{
	StartTrace(SlotCopier.Operate);
	TraceAny(config, "Config");

	for (long i = 0, sz = config.GetSize(); i < sz; ++i) {
		String sourceSlot = config.SlotName(i);
		String destSlot = config[i].AsCharPtr(0);
		Trace("copying [" << sourceSlot << "] to [" << destSlot << "]");
		Anything content(dest.GetAllocator());
		if ( sourceSlot && destSlot && source.LookupPath(content, sourceSlot, delim, indexdelim ) ) {
			dest[destSlot] = content;
		}
	}
}

void SlotnameSorter::Sort(Anything &toSort, EMode mode)
{
	StartTrace(SlotnameSorter.Sort);

	if (mode == SlotnameSorter::asc) {
		toSort.SortByKey();
	} else {
		toSort.SortReverseByKey();
	}
}
