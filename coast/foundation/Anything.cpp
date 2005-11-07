/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Anything.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SysLog.h"
#include "StringStream.h"
#include "Dbg.h"
#include "AnyVisitor.h"
#include "AnyComparers.h"

//--- c-library modules used ---------------------------------------------------
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static const String fgStrEmpty(Storage::Global()); //avoid temporary
static const Anything fgAnyEmpty(Storage::Global()); // avoid temporary

//--- auxiliary calculating hash value and the length of the key
long IFAHash(const char *key, long &len, char stop1 = '\0', char stop2 = '\0')
{
	register long h = 0;
	register u_long g;
	register const unsigned char *p = (const unsigned char *)key;

	if (key) {
		while (*p && *p != stop1 && *p != stop2) {
			h = (h << 4) + *p++;
			if ((g = (h & 0xf0000000))) {
				h = (h ^ (g >> 24)) ^ g;
			}
		}
	}
	len = (long)(p) - (long)(key);
	return h;
}

//---- InputContext -------------------------------------------------------------
class EXPORTDECL_FOUNDATION InputContext
{
public:
	// constructor
	InputContext(istream &is, const char *fname = 0) : fIs(is), fLine(1), fFileName(fname) { }

	// destructor
	~InputContext() {}

	// input
	void SkipToEOL();    // for reading comments by parser
	bool Get(char &c)				{
		return (
#if defined(WIN32)
				   !fIs.get(c).fail()
#else
				   fIs.get(c).good()
#endif
			   ) || (c = 0);
	}
	// the last assignment is a trick for sunCC weakness of storing EOF(-1) in c
	void Putback(char c)			{
		fIs.putback(c);
	}
	bool IsGood() 					{
		return fIs.good();
	}

	istream &fIs;
	long fLine;
	String fFileName;
};

//---- the following class is used for lexical analysis of
//---- any-files or input. It will represent the next
//---- character or symbol read from the stream
class EXPORTDECL_FOUNDATION AnythingToken
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
	int       fToken; // enum above or character value
	String    fText; // the characters that form the token, lex would say yytext[]
	AnythingToken(InputContext &context); // read next token from is
	static bool isNameDelimiter(char c); // implement single place where we check for delims
private:
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
	// isprint( (unsigned char) c) shouldn't be used because of umlauts äüö and signed chars
	// may be double quotes " should also be considered delimiters
	return isspace((unsigned char) c) || '/' == c || '#' == c || '&' == c || '*' == c
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
					while (context.Get(c) && isspace((unsigned char)c)) {// consume spaces
						// adjust line count!
						if ('\n' == c || '\r' == c) {
							context.fLine++;
						}
						c = 0;
					}
					if (isdigit( (unsigned char) c)) {
						DoReadNumber(context, c);
						if (fToken == AnythingToken::eDecimalNumber) {
							fToken = AnythingToken::eObject;
							return;
						}
					}// anything else is a syntax error!
					// consume invalid characters up to a whitespace
					do {
						fText.Append(c);
					} while (!isspace( (unsigned char) c) && context.Get(c));
					if ('\n' == c || '\r' == c) {
						context.fLine++;
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
							//                      if (isalnum( (unsigned char) c) || '_' == c || '-' == c) {
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
					break;
				case '#': // start of a comment skip to eol
					context.SkipToEOL();
					break;
				default:
					if (isalnum( (unsigned char) c) || '_' == c) { // do we need to allow more?
						// a name or string read it
						c = DoReadName(context, c);
						// should check for delimiter
						// this character should be ignored safely
						if (c) {
							context.Putback(c);
						}
					} else if (isspace( (unsigned char) c)) {
						// ignore it but count line changes
						if ('\n' == c || '\r' == c) {
							context.fLine++;
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
		context.Putback(firstchar);    // leave double quote on the stream
	}
	long linebreakswithinstring =  fText.IntReadFrom(context.fIs, firstchar);
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
	context.fLine += linebreakswithinstring;
}

void AnythingToken::DoReadBinBuf(InputContext &context)
{
	// context's line count is not adjusted in the case of binary buffers
	// first read in the length of the buffer
	long length = 0;
	context.fIs >> length; // we do not need DoReadNumber here.
	// we need to check a formatting error on the stream here!
	if (!context.fIs.good()) { // a syntax error
		fToken = AnythingToken::eError;
		context.fIs.clear(); // try again
	}
	// now we read ';' as a separator
	char c = 0;
	context.Get(c);
	if (';' != c) {
		// this is an error
		// skip to ] or eof and set token to eError
		fToken = AnythingToken::eError;
		while (']' != c && context.Get(c));
		return;
	}
	// now read in the binary buffer, this can be done via fText
	fText.Trim(0); // to ensure nothing nasty happens
	fText.Append(context.fIs, length);
	if (fText.Length() != length) {
		// we have a premature EOF.
		fToken = AnythingToken::eError;
	}
	context.Get(c);
	if (']' != c) {
		fToken = AnythingToken::eError;
		while (']' != c && context.Get(c));
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
		//        if (isalnum( (unsigned char) c) || '_' == c || '-' == c) { // should we allow more? YES!
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
		if (isdigit( (unsigned char) c)) {
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
				if (isxdigit( (unsigned char) c)) {
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
	bool sign = false, intpart = false, fraction = false,
		 exponent = false, echar = false;
	if (context.IsGood()) {
		if ('+' == firstchar || '-' == firstchar) {
			// found optional sign character :
			fText.Append(firstchar);
			sign = true;
			context.Get(firstchar);
		}
		// processed [+-]? follow: [0-9]*\.?[0-9]*([eE][+-]?[0-9]+)?
		if (isdigit( (unsigned char) firstchar)) {
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
			if (isdigit( (unsigned char) firstchar)) {
				exponent = true;
				fText.Append(firstchar);
				firstchar = DoReadDigits(context);
			}
		}
	} else {
		// special case one digit number
		fText.Append(firstchar);
		if (isdigit( (unsigned char) firstchar)) {
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

class  EXPORTDECL_FOUNDATION AnyXrefHandler
{
protected:
	String	fParseLevel;
	Anything	fXrefs;
public:
	AnyXrefHandler() {};
	String	GetCurrentLevel() {
		return fParseLevel;
	}
	void	ResetLevel(const String &s) {
		fParseLevel = s;
	}
	void	PushKey(const String &key) {
		if (fParseLevel.Length() > 0) {
			fParseLevel.Append('.');
		}
		fParseLevel.Append(key);
	}
	void	PushIndex(long lIdx) {
		fParseLevel.Append(':').Append(lIdx);
	}
	void Pop() {
		long icolon = fParseLevel.StrRChr(':');
		long idot   = fParseLevel.StrRChr('.');
		long trim = icolon > idot ? icolon : idot;
		fParseLevel.Trim(trim);
	}
};
class PrinterXrefHandler: public AnyXrefHandler
{
	String	ToId(long id) {
		return String().Append(id);
	}
public:
	bool	IsDefined(long id) {
		return fXrefs.IsDefined(ToId(id));
	}
	void	DefineBackRef(long id) {
		fXrefs[ToId(id)] = fParseLevel;
	}
	String	GetBackRef(long id) {
		return fXrefs[ToId(id)].AsString();
	}

};
class ParserXrefHandler: public AnyXrefHandler
{
public:
	void DefinePatchRef(long lIdx) {
		Anything patch;
		patch[0L] = fParseLevel;
		patch[1L] = lIdx;
		fXrefs.Append(patch);
	}
	void	DefinePatchRef(const String &reference) {
		Anything patch;
		patch[0L] = fParseLevel;
		patch[1L] = reference;
		fXrefs.Append(patch);
	}
	void	Patch(Anything &any) {
		for (long i = 0; i < fXrefs.GetSize(); i++) {
			Anything preslot, slot, ref, anyIdx;
			String strSlotName;
			strSlotName = fXrefs[i][0L].AsString();
			anyIdx = fXrefs[i][1L];
			// Slot
			if (!strSlotName.Length()) {
				preslot = any;
			} else if (!any.LookupPath(preslot, strSlotName)) {
				String m;
				m << "lookup of slotname [" << strSlotName << "] failed!" << "\n";
				SysLog::WriteToStderr(m);
			}
			if (!preslot.IsNull()) {
				if (anyIdx.GetType() == Anything::eLong) {
					slot = preslot[anyIdx.AsLong(0L)];
				} else {
					slot = preslot[anyIdx.AsString()];
				}
				String strRefName;
				strRefName = slot.AsString();
				if (!any.LookupPath(ref, strRefName)) {
					String m;
					m << "lookup of reference [" << strRefName << "] failed!" << "\n";
					SysLog::WriteToStderr(m);
				} else {
					if (anyIdx.GetType() == Anything::eLong) {
						preslot[anyIdx.AsLong(0L)] = ref;
					} else {
						preslot[anyIdx.AsString()] = ref;
					}
				}
			}
		}
	}
};

class EXPORTDECL_FOUNDATION AnythingParser
{
	// really implement the grammar of Anythings
	// needs to be friend of Anything to set Anything's internals
public:
	AnythingParser(InputContext &c): fContext(c) {}
	bool    DoParse(Anything &a); // returns false if there was a syntax error
	bool    DoParseSequence(Anything &a, ParserXrefHandler &xrefs);
	bool    DoParseArrayKey(Anything &a);
	bool    MakeSimpleAny(AnythingToken &tok, Anything &a);
private:
	void 	ImportIncludeAny(Anything &element, String &url);
	void    Error(const char *text, const char *what);
	InputContext &fContext;
};

//---- AnyImpl --------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyImpl
{
public:
	AnyImpl(Allocator *a)
		: fRefCount(1),
		  fAllocator((a) ? a : Storage::Current())			{  }
	virtual ~AnyImpl() 								{
		Assert(fRefCount <= 0);
	}
	virtual Anything::EType GetType() const = 0;
	virtual long GetSize()							{
		return 1;
	}
	virtual long Contains(const char *) 				{
		return -1;
	}

	virtual long AsLong(long dflt) = 0;
	virtual IFAObject *AsIFAObject(IFAObject *dflt) const {
		return dflt;
	}
	virtual double AsDouble(double dflt) = 0;
	virtual const char *AsCharPtr(const char *dflt) const = 0;
	virtual const char *AsCharPtr(const char *dflt, long &buflen) const = 0;
	virtual String AsString(const char *dflt) const = 0;
	virtual bool IsEqual(AnyImpl *) const = 0;
	virtual AnyImpl *DeepClone(Allocator *a, Anything &xreftable);
	virtual AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable) = 0;
	virtual void Accept(AnyVisitor &v, long lIdx, const char *slotname) const = 0;

	void Ref() {
		fRefCount++;
	}
	void Unref() {
		if (--fRefCount <= 0) {
			delete this;
		}
	}
	long RefCount() {
		return fRefCount;
	}
	inline String ThisToHex(Allocator *a = Storage::Current()) const {
		String hexStr(a);
		return hexStr.Append((long)this);
	}
	static void *operator new(size_t size, Allocator *a);
	static void operator delete(void *d);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete(void *d, Allocator *a);
#endif
private:
	AnyImpl(const AnyImpl &);
	long		fRefCount;
public:
	Allocator	*fAllocator;
};

//---- AnyLongImpl -----------------------------------------------------------------
#define IsLongImpl(anyimpl) ((anyimpl) && (Anything::eLong)==(anyimpl)->GetType())
#define LongImpl(anyimpl) ((AnyLongImpl*)(anyimpl))

class EXPORTDECL_FOUNDATION AnyLongImpl : public AnyImpl
{
	long fLong;
	String fBuf;

	AnyLongImpl(long l, const String &buf, Allocator *a) : AnyImpl(a), fLong(l), fBuf(buf, a) { }

public:
	AnyLongImpl(long l, Allocator *a) : AnyImpl(a), fLong(l), fBuf(a) {
		OStringStream out(&fBuf);
		out << fLong;
	}
	~AnyLongImpl()								{  }
	Anything::EType GetType() const				{
		return Anything::eLong;
	}
	long AsLong(long)							{
		return fLong;
	}
	double AsDouble(double) 					{
		return (double) fLong;
	}
	const char *AsCharPtr(const char *) const;
	const char *AsCharPtr(const char *, long &buflen) const;
	bool IsEqual(AnyImpl *fAnyImp) const		{
		return fLong == fAnyImp->AsLong(-1);
	}
	String AsString(const char *) const;
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable) {
		AnyImpl *ret =  new ((a) ? a : Storage::Current()) AnyLongImpl(this->fLong, this->fBuf, a);
		return ret;
	}
	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
		v.VisitLong(fLong, this, lIdx, slotname);
	}
};

//---- AnyObjectImpl -----------------------------------------------------------------
// convenience macros for AnyImpl simplification
#define IsObjectImpl(anyimpl) ((anyimpl) && (Anything::eObject)==(anyimpl)->GetType())
#define ObjectImpl(anyimpl) ((AnyObjectImpl*)(anyimpl))
static const char *gcObjectText = "IFAObject";

class EXPORTDECL_FOUNDATION AnyObjectImpl : public AnyImpl
{
	IFAObject *fObject;
public:
	AnyObjectImpl(IFAObject *o, Allocator *a) : AnyImpl(a), fObject(o) { }
	Anything::EType GetType() const				{
		return Anything::eObject;
	}
	long AsLong(long) 							{
		return (long) fObject;
	}
	IFAObject *AsIFAObject(IFAObject *) const	{
		return fObject;
	}
	const char *AsCharPtr(const char *) const	{
		return gcObjectText;
	}
	const char *AsCharPtr(const char *, long &buflen) const;
	String AsString(const char *) const			{
		return gcObjectText;
	}
	bool IsEqual(AnyImpl *) const				{
		return false;
	}
	double AsDouble(double dft) 				{
		return dft;
	}
	// PS,JW: check if we need fObject->Clone() instead
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable) {
		AnyImpl *ret = new ((a) ? a : Storage::Current()) AnyObjectImpl(this->fObject, a);
		return ret;
	}
	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
		v.VisitObject(fObject, this, lIdx, slotname);
	}
};

//---- AnyDoubleImpl -----------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyDoubleImpl : public AnyImpl
{
	double fDouble;
	String fBuf;

	AnyDoubleImpl(double d, const String &buf, Allocator *a) : AnyImpl(a), fDouble(d), fBuf(buf, a) { }

public:
	AnyDoubleImpl(double d, Allocator *a) : AnyImpl(a),	fDouble(d), fBuf(a) {
		OStringStream out(&fBuf);
		out.precision(20); // safety margin, 16 should be OK for doubles
		out << fDouble;
	}
	~AnyDoubleImpl()							{ }
	Anything::EType GetType() const				{
		return Anything::eDouble;
	}
	long AsLong(long) 							{
		return (long) fDouble;
	}
	double AsDouble(double) 					{
		return fDouble;
	}
	bool IsEqual(AnyImpl *fAnyImp) const		{
		return fDouble == fAnyImp->AsDouble(-1);
	}
	const char *AsCharPtr(const char *dflt) const;
	const char *AsCharPtr(const char *dflt, long &buflen) const;
	String AsString(const char *) const;
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable) {
		AnyImpl *ret = new ((a) ? a : Storage::Current()) AnyDoubleImpl(this->fDouble, this->fBuf, a);
		return ret;
	}
	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
		v.VisitDouble(fDouble, this, lIdx, slotname);
	}
};

//---- AnyBinaryBufImpl -----------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyBinaryBufImpl : public AnyImpl
{
	String fBuf;
public:
	AnyBinaryBufImpl(void *buf, long len, Allocator *a) : AnyImpl(a), fBuf(buf, len, a) { }
	~AnyBinaryBufImpl()								{  }
	Anything::EType GetType() const					{
		return Anything::eVoidBuf;
	}
	const char *AsCharPtr(const char *) const		{
		return fBuf.Capacity() > 0 ? (const char *)fBuf : 0;
	}
	const char *AsCharPtr(const char *, long &buflen) const;
	long AsLong(long) 								{
		return (long)(const char *)fBuf;
	}
	double AsDouble(double dft) 					{
		return dft;
	}
	String AsString(const char *) const				{
		return fBuf;
	}
	bool IsEqual(AnyImpl *impl) const				{
		return ((AnyImpl *)this == impl);
	}
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable) {
		AnyImpl *ret = new ((a) ? a : Storage::Current()) AnyBinaryBufImpl((void *)(const char *)this->fBuf, this->fBuf.Length(), a);
		return ret;
	}
	virtual void Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
		v.VisitVoidBuf(fBuf, this, lIdx, slotname);
	}
};

//---- AnyStringImpl -----------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyStringImpl : public AnyImpl
{
	String fString;
public:
	AnyStringImpl(const char *s, long l, Allocator *a) : AnyImpl(a), fString(s, l, a) { }
	AnyStringImpl(const String &s, Allocator *a) : AnyImpl(a),  fString(s, a) { }
	Anything::EType GetType() const					{
		return Anything::eCharPtr;
	}
	long Contains(const char *k)					{
		return Compare(k);
	}
	long AsLong(long dflt);
	double AsDouble(double dflt);
	bool IsEqual(AnyImpl *anyImpl) const			{
		return fString.IsEqual(anyImpl->AsCharPtr(0)) ;
	} //String("")); }
	const char *AsCharPtr(const char *) const;
	const char *AsCharPtr(const char *, long &buflen) const;
	String AsString(const char *) const;
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable) {
		AnyImpl *ret = new ((a) ? a : Storage::Current()) AnyStringImpl(this->fString, a);
		return ret;
	}
	virtual void Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
		v.VisitCharPtr(fString, this, lIdx, slotname);
	}

protected:
	long Compare(const char *other);
};

//---- AnyKeyAssoc --------------------------------------------------
class EXPORTDECL_FOUNDATION AnyKeyAssoc
{
public:
	AnyKeyAssoc(const Anything &value, const char *key = 0);
	AnyKeyAssoc(const AnyKeyAssoc &aka);
	AnyKeyAssoc();
	~AnyKeyAssoc();
	void Init(Allocator *a);
	Anything &Value();
	const Anything &Value() const;
	const String &Key();
	const String &Key() const;
	void SetKey(const char *key);
	void SetValue(const Anything &val);
	AnyKeyAssoc &operator=(const AnyKeyAssoc &aka);

	// new[] is needed to properly initialize instance variables (i.e. Anything)
#if !defined(OPERATOR_NEW_ARRAY_NOT_SUPPORTED)
	static void *operator new[](size_t size, Allocator *a);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete[](void *ptr, Allocator *a);
#endif
	static void operator delete[](void *ptr);
#endif
protected:
	Anything 	fValue;
	String 		fKey;
	Allocator 	*fAllocator;
};

inline Anything &AnyKeyAssoc::Value()
{
	return fValue;
}
inline const Anything &AnyKeyAssoc::Value() const
{
	return fValue;
}
inline const String &AnyKeyAssoc::Key()
{
	return fKey;
}
inline const String &AnyKeyAssoc::Key() const
{
	return fKey;
}

//---- AnyKeyTable --------------------------------------------------
class EXPORTDECL_FOUNDATION AnyKeyTable
{
public:
	enum { cInitCapacity = 17 };

	AnyKeyTable(AnyArrayImpl *table, long initCapacity = cInitCapacity);
	~AnyKeyTable();

	void Clear();

	long Append(const char *key, long lIdx);
	void Update(long fromIndex);

	long At(const char *key, long hashhint = -1, u_long hashhint1 = 0	) const;

	void PrintHash();

	static void *operator new(size_t size, Allocator *a);
	static void operator delete(void *d);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete(void *d, Allocator *a);
#endif

protected:
	void InitTable(long cap);
	long DoHash(const char *key, bool append = false, long sizehint = -1, u_long hashhint = 0) const;
	void Rehash(long newCap);

	AnyArrayImpl *fKeyTable;	// shared with AnyArrayImpl
	long *fHashTable;
	long fThreshold, fCapacity;
	Allocator *fAllocator;
};

//---- AnyIndTable --------------------------------------------------
class EXPORTDECL_FOUNDATION AnyIndTable
{
public:

	AnyIndTable(long initCapacity, Allocator *a);
	~AnyIndTable();

	long At(long);
	// return -1 if slot could not be found
	long FindAt(long slot);
	void Remove(long slot);

	static void *operator new(size_t size, Allocator *a);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete(void *d, Allocator *a);
#endif
	static void operator delete(void *d);
	void Swap(long l, long r);
	void SetIndex(long slot, long index);

protected:
	void InitTable(long cap);
	void InitIndices(long slot, long *ot);
	void InitEmpty(long oldCap, long newCap);
	void Expand(long slot);
	void Clear();
	void PrintTable();

	long *fIndexTable;
	long *fEmptyTable;
	long fCapacity;
	long fSize;
	Allocator *fAllocator;
};

//---- AnyArrayImpl -----------------------------------------------------------------
// convenience macros for AnyImpl simplification
#define IsArrayImpl(anyimpl) ((anyimpl) && (Anything::eArray)==(anyimpl)->GetType())
#define ArrayImpl(anyimpl) ((AnyArrayImpl*)(anyimpl))

static const char *gcArrayText = "AnyArrayImpl";

class EXPORTDECL_FOUNDATION AnyArrayImpl : public AnyImpl
{
public:
	AnyKeyAssoc **fContents;
	AnyKeyTable *fKeys;
	AnyIndTable *fInd;
	long fCapacity, fSize;
	long fBufSize, fNumOfBufs;
public:
	AnyArrayImpl(Allocator *a);
	~AnyArrayImpl();
	Anything &At(long i);
	Anything &operator[](long i)						{
		return At(i);
	}
	void Expand(long c);
	void Remove(long slot);
	long GetSize()										{
		return fSize;
	}
	long Contains(const char *k);
	long FindIndex(const char *k, long sizehint = -1, u_long hashhint = 0);
	long FindIndex(const long lIdx);
	Anything &At(const char *key);
	Anything &operator[](const char *key)				{
		return At(key);
	}
	const char *AsCharPtr(const char *) const			{
		return gcArrayText;
	}
	const char *AsCharPtr(const char *dflt, long &buflen) const;
	String AsString(const char *) const					{
		return gcArrayText;
	}
	double AsDouble(double dflt) 						{
		return dflt;
	}
	long AsLong(long dflt)								{
		return dflt;
	}
	Anything::EType GetType() const						{
		return Anything::eArray;
	}
	const char *SlotName(long slot);
	const String &VisitSlotName(long slot);
	virtual bool IsEqual(AnyImpl *) const				{
		return false;
	}
	void PrintKeys();
	void PrintHash();

	long IntAt(long at)			{
		return fInd->At(at);
	}
	long IntAtBuf(long at)		{
		return at / fBufSize;
	}
	long IntAtSlot(long at)		{
		return at % fBufSize;
	}
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable);
	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
		ROAnything wrapit((AnyImpl *)this); //is there a nicer way
		Assert(wrapit.fAnyImp == (AnyImpl *)this);		// check for auto conversion by compiler
		v.VisitArray(wrapit, this, lIdx, slotname);
	}

	// new from SOP
	//!reorder Array using sort order of the keys
	void SortByKey();
	//! this method is only relevant for legacy behaviour of SlotNameSorter.
	//! We do not need to sort descending, since we always can iterate from the end
	void SortReverseByKey();
	//!reorder Array using sort order defined by the AnyComparer
	void SortByAnyComparer(const AnyComparer &comparer);

	//!rebuild hash map after sorting O(fSize)
	void RecreateKeyTabe();
	//!similar to SlotName(at)
	const String &Key(long slot) ;
	//!works with index into fContents
	const Anything &IntValue(long at);
	//!works with index into fContents
	const String &IntKey(long at) ;

	//!interface for internal comparing during sort
	class EXPORTDECL_FOUNDATION AnyIntCompare
	{
	public:
		virtual ~AnyIntCompare() {};
		virtual int	Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
			return 0;
		}
	};
	void MergeSortByComparer(long left, long right, const AnyIntCompare &comparer);
	void MergeByComparer(long left, long right, long middle, const AnyIntCompare &comparer);
#if 0
	void Qsort(long left, long right);
	void BuildHeap();
	void DownHeap(long start, long end);
	void HeapSort();
	int CompareKeys(long i, long j) ;
#endif
	void Swap(long l, long r) {
		fInd->Swap(l, r);
	}
	void AllocMemory();
	long AdjustCapacity(long cap) {
		return ((cap + fBufSize - 1) / fBufSize) * fBufSize;    // make it a multiple of fBufSize
	}

	static class EXPORTDECL_FOUNDATION AnyIntKeyCompare : public AnyIntCompare
	{
	public:
		virtual int	Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
			return that.IntKey(leftInt).Compare(that.IntKey(rightInt));
		}
	}	theKeyComparer;
	static class EXPORTDECL_FOUNDATION AnyIntReverseKeyCompare : public AnyIntCompare
	{
	public:
		virtual int	Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
			return - that.IntKey(leftInt).Compare(that.IntKey(rightInt));
		}
	}	theReverseKeyComparer;
	class EXPORTDECL_FOUNDATION AnyIntComparerCompare : public AnyIntCompare
	{
		const AnyComparer	&ac;
	public:
		AnyIntComparerCompare(const AnyComparer &theComparer): ac(theComparer) {}
		virtual int	Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
			return ac.Compare(that.IntValue(leftInt), that.IntValue(rightInt));
		}
	};
}; // AnyArrayImpl
AnyArrayImpl::AnyIntKeyCompare AnyArrayImpl::theKeyComparer;
AnyArrayImpl::AnyIntReverseKeyCompare AnyArrayImpl::theReverseKeyComparer;

//-- AnyImpl implementation
void *AnyImpl::operator new(size_t size, Allocator *a)
{
	if (a) {
		return a->Calloc(1, size);
	} else {
		return ::operator new(size);
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void AnyImpl::operator delete(void *d, Allocator *a)
{
	if (d) {
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
	}
}
#endif

void AnyImpl::operator delete(void *d)
{
	if (d) {
		Allocator *a = ((AnyImpl *)d)->fAllocator;
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		AnyImpl::operator delete(d, a);
#else
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
#endif
	}
}

AnyImpl *AnyImpl::DeepClone(Allocator *a, Anything &xreftable)
{
	return this->DoDeepClone(a, xreftable);
}

//--- AnyArrayImpl implementation ----------------------------------

AnyArrayImpl::AnyArrayImpl(Allocator *a)
	: AnyImpl(a)
	, fContents(0)
	, fKeys(0)
	, fInd(0)
	, fCapacity(4)
	, fSize(0)
	, fBufSize(4)
	, fNumOfBufs(0)
{
	// allocate a default size structure
	// without keys
	fCapacity = AdjustCapacity(fCapacity);
	AllocMemory();
}

AnyArrayImpl::~AnyArrayImpl()
{
	if (fContents) {
		// free the buffers themselves
		for (int j = 0; j < fNumOfBufs; j++) {
#if defined(OPERATOR_NEW_ARRAY_NOT_SUPPORTED)
			for ( int k = 0; k < fBufSize; k++) {
				fContents[j][k].~AnyKeyAssoc();
			}
			fAllocator->Free(fContents[j]);
#else
			delete [] (fContents[j]);
#endif
		}
		// free the buffer Ptr
		fAllocator->Free(fContents);

		// free the key array
		if ( fKeys ) {
			delete fKeys;
		}

		// free the index array
		delete fInd;

		// reset everything
		fInd = 0;
		fContents = 0;
		fKeys = 0;
		fSize = fCapacity = 0;
	}
}

const char *AnyArrayImpl::AsCharPtr(const char *, long &buflen) const
{
	buflen = strlen(gcArrayText);
	return gcArrayText;
}

Anything &AnyArrayImpl::At(long slot)
{
	// return an address of an anything
	// residing at slot
	// expand the buffers as necessary to fullfill
	// the request

	long newsz = slot + 1;

	// check for logical expansion
	if ( (newsz > fSize) ) {
		// reset the size
		fSize = newsz;

		// check for physical expansion
		if ( newsz >= fCapacity ) {
			Expand(newsz);
		}
	}

	// calculate the adress of the anything
	long at = IntAt(slot);
	return fContents[IntAtBuf(at)][IntAtSlot(at)].Value();
}

Anything &AnyArrayImpl::At(const char *key)
{
	// calculate the adress of an anything
	// given its key

	long slot = -1;
	if ( !fKeys ) {
		// no keys exist yet
		fKeys = new(fAllocator) AnyKeyTable(this);
	} else {
		// find index of key or return -1
		slot = fKeys->At(key);
	}

	long at;

	if (slot < 0) {
		// key doesn't exist so append this key in the key array
		// with the according slot
		slot = fKeys->Append(key, fSize);
		at = IntAt(slot);
		// set the key in the any key assoc structure
		fContents[IntAtBuf(at)][IntAtSlot(at)].SetKey(key);

		// return the element found
		// this creates a new element
		return At(fSize);
	}
	// the element already exists the slot is an internal slot
	at = IntAt(slot);
	return fContents[IntAtBuf(slot)][IntAtSlot(slot)].Value();
}

long AnyArrayImpl::FindIndex(const char *key, long sizehint, u_long hashhint)
{
	// find the index of an anything given
	// its key. It returns -1 if not defined

	if ( !fKeys ) {
		return -1;
	}

	// return index from keyarray
	return fKeys->At(key, sizehint, hashhint);
}

long AnyArrayImpl::FindIndex(const long lIdx)
{
	// find the index of an anything given
	// its index. It returns -1 if not defined
	if ( !fInd ) {
		return -1L;
	}

	// return index from indexarray
	return fInd->FindAt(lIdx);
}

long AnyArrayImpl::Contains(const char *k)
{
	// search the value in the array
	// assume an array of simple strings
	// otherwise the result could be misleading
	// e.g. an AnyArrayImpl returns always AnyArrayImpl

	Assert(k);	// require a valid search string

	long i; 	// external slot index
	long at;	// internal buffer index
	for (i = 0; i < fSize; i++) {
		at = IntAt(i);	// calculate the internal index
		Assert(at >= 0 && at < fCapacity);
		if ( strcmp(fContents[IntAtBuf(at)][IntAtSlot(at)].Value().AsCharPtr(""), k) == 0 ) {
			return i;
		}
	}
	return -1;
}

void AnyArrayImpl::Remove(long slot)
{
	// remove an anything from the internal
	// structures

	// check the slot range
	if (slot >= 0 && slot < fSize) {
		// delete the internal key assoc
		// at slot index
		long at = IntAt(slot);
//#if defined(PURE_LEAK) || defined(__linux__)
//		fAllocator ? fAllocator : Storage::Current();
//#endif
		fContents[IntAtBuf(at)][IntAtSlot(at)] = AnyKeyAssoc(fAllocator);	// reset it to initial empty assoc

		// remove the slot from the index array
		fInd->Remove(slot);

		// update the key array if it exists
		if ( fKeys ) {
			fKeys->Update(slot);
		}

		// update the size
		fSize--;
	} else {
		String msg;
		SYSERROR(msg.Append("index ").Append(slot).Append(" out of range"));
	}
}
const String &AnyArrayImpl::Key(long slot)
{
	if (slot >= 0 && slot < fSize) {
		long at = IntAt(slot);
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Key();
	}
	return fgStrEmpty;
}
const String &AnyArrayImpl::IntKey(long at)
{
	if (at >= 0 && at < fCapacity) {
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Key();
	}
	return fgStrEmpty;
}
const Anything &AnyArrayImpl::IntValue(long at)
{
	if (at >= 0 && at < fCapacity) {
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Value();
	}
	return fgAnyEmpty;
}
const char *AnyArrayImpl::SlotName(long slot)
{
	// calculate the slot name given an
	// index
	const String &k = Key(slot);
	return (k.Length() > 0) ? (const char *)k : (const char *)0;
}
const String &AnyArrayImpl::VisitSlotName(long slot)
{
	// calculate the slot name given an
	// index

	// first check the range
	if (slot >= 0 && slot < fSize) {
		long at = IntAt(slot);
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Key();
	}
	return fgStrEmpty;
}

void AnyArrayImpl::Expand(long newsize)
{
	bool allocOk = true;
	long oldCap = fCapacity;

	// set the new capacity
	if (newsize > fCapacity * 2) {
		fCapacity = AdjustCapacity(newsize);
	} else {
		fCapacity = AdjustCapacity(fCapacity * 2);
	}

	// check for the range of the capacity
	Assert((fCapacity % fBufSize) == 0);

	// calculate the number of buffers needed for the expansion
	long numOfExistingBufs = fNumOfBufs;
	long numOfNewBufs = fCapacity / fBufSize; //fCapacity / fBufSize + 1;

	Assert(numOfNewBufs *fBufSize >= newsize);

	// allocate new ptr buffer if necessary
	if ( numOfNewBufs > fNumOfBufs ) {
		// expand the number of needed ptr buffers
		fNumOfBufs = numOfNewBufs;

		// allocate the new size
		AnyKeyAssoc **old = fContents;
		fContents = (AnyKeyAssoc **)fAllocator->Calloc(fNumOfBufs, sizeof(AnyKeyAssoc *));
		if (fContents) {
			for (long bufs = 0; bufs < numOfExistingBufs; bufs++) {
				fContents[bufs] = old[bufs];
			}
			fAllocator->Free(old); // frees the old ptr buffer array not the contents buffer
		} else {
			static const char crashmsg[] = "FATAL: AnyArrayImpl::Expand calloc failed (increasing pointer buffer). I will crash :-(\n";
			SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));

			fContents = old;
			fCapacity = oldCap;
			allocOk = false;
		}
	}

	// allocate the needed buffers
	if (allocOk) {
		for (long i = numOfExistingBufs; i < fNumOfBufs && allocOk; i++) {
			Assert(fAllocator != 0);
			//#if defined(PURE_LEAK) || defined(__linux__)
			//			fAllocator = fAllocator ? fAllocator : Storage::Current();
			//#endif
#if defined(OPERATOR_NEW_ARRAY_NOT_SUPPORTED)
			fContents[i] = (AnyKeyAssoc *)fAllocator->Calloc(fBufSize, sizeof(AnyKeyAssoc));
#else
			fContents[i] = new (fAllocator) AnyKeyAssoc[fBufSize];
#endif
			if (fContents[i] == 0) {
				static const char crashmsg[] = "FATAL: AnyArrayImpl::Expand calloc failed (assigning memory to increased pointer buffers).\nI will crash :-(\n";
				SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));

				allocOk = false;
			} else {
				for ( long keyAssocKeyCnt = 0L; keyAssocKeyCnt < fBufSize; keyAssocKeyCnt++ ) {
					fContents[i][keyAssocKeyCnt].Init(fAllocator);
				}
			}
		}
	}
}

void AnyArrayImpl::AllocMemory()
{
	// calculate the number of needed buffers
	// allocate at least fBufSize buffers of the size fBufSize
	fNumOfBufs = fCapacity / fBufSize; // round to the next multiple
	//(fCapacity / fBufSize > fBufSize) ? fCapacity/fBufSize : fBufSize;
	fContents = (AnyKeyAssoc **) fAllocator->Calloc(fNumOfBufs, sizeof(AnyKeyAssoc *));

	// allocate the index table
	fInd = new (fAllocator) AnyIndTable(fCapacity, fAllocator);

	// allocate the buffers holding the
	// Any Key Assocs
	for (long i = 0; i < fNumOfBufs; i++) {
		// must not use calloc to ensure proper initialization of Anything instance variables
		Assert(fAllocator != 0);
//#if defined(PURE_LEAK) || defined(__linux__)
//		fAllocator = fAllocator ? fAllocator : Storage::Current();
//#endif

#if defined(OPERATOR_NEW_ARRAY_NOT_SUPPORTED)
		fContents[i] = (AnyKeyAssoc *)fAllocator->Calloc(fBufSize, sizeof(AnyKeyAssoc));
#else
		fContents[i] = new (fAllocator) AnyKeyAssoc[fBufSize];		// (fAllocator);
#endif

		if ( fContents[i] == 0 ) {
			String msg("Memory allocation failed!");
			SYSERROR(msg);
		} else {
			for ( long keyAssocKeyCnt = 0L; keyAssocKeyCnt < fBufSize; keyAssocKeyCnt++ ) {
				fContents[i][keyAssocKeyCnt].Init(fAllocator);
			}
		}
	}
}

void AnyArrayImpl::PrintKeys()
{
	long hash = -1;
	for (long i = 0; i < fSize; i++) {
		long at = IntAt(i);
		if (fKeys) {
			hash = 	fKeys->At(fContents[IntAtBuf(at)][IntAtSlot(at)].Key());
		}
		String m;
		m << "[" << i << "]<" << NotNullStr(fContents[IntAtBuf(at)][IntAtSlot(at)].Key()) << ">(" << hash << ")" << "\n";
		SysLog::WriteToStderr(m);
	}
}

void AnyArrayImpl::PrintHash()
{
	if (fKeys) {
		fKeys->PrintHash();
	} else {
		SysLog::WriteToStderr("*", 1);
	}
}

AnyImpl *AnyArrayImpl::DoDeepClone(Allocator *a, Anything &xreftable)
{
	String adr(ThisToHex(), a);
	Anything &refEntry = xreftable[adr];
	AnyImpl *res = (AnyImpl *)refEntry.AsIFAObject(0);
	if (res != NULL) {
		res->Ref(); // do not forget to count
		return res;
	}

	AnyArrayImpl *ret = new ((a) ? a : Storage::Current()) AnyArrayImpl(a);
	refEntry = (IFAObject *)ret;
	long count = this->GetSize();

	for (long i = 0 ; i < count; i++) {
		ret->At(this->SlotName(i)) = this->At(i).DeepClone(a, xreftable);
	}
	return ret;
}
#if 0
void AnyArrayImpl::Qsort(long left, long right)
{
	// taken from wirth
	long i = left, j = right;
	long median = (left + right) / 2;
	const String &mediankey = Key(median);
	do {
		while (mediankey.Compare(Key(i)) > 0) {
			i++;
		}
		while (mediankey.Compare(Key(j)) < 0) {
			j--;
		}
		if (i <= j) {
			Swap(i, j);
			i++;
			j--;
		}
	} while (i <= j);
	if (left < j) {
		Qsort(left, j);
	}
	if (right > i) {
		Qsort(i, right);
	}
}
int AnyArrayImpl::CompareKeys(long i, long j)
{
	return Key(i).Compare(Key(j));
}
void AnyArrayImpl::DownHeap (long v, long n)
{
	long w = 2 * v + 1;           // first descendant of v
	while (w < n) {
		if (w + 1 < n)           // is there a second descendant?
			if (CompareKeys(w + 1, w) > 0) {
				w++;
			}
		// w is the descendant of v with maximum label

		if (CompareKeys(v, w) >= 0) {
			return;    // v has the heap property
		}
		// otherwise
		Swap(v, w);;      // exchange the labels of v and w
		v = w;
		w = 2 * v + 1;      // continue
	}
}
void AnyArrayImpl::BuildHeap ()
{
	long n = GetSize();
	for (long v = n / 2 - 1; v >= 0; v--) {
		DownHeap(v, n);
	}
}
void AnyArrayImpl::HeapSort()
{
	BuildHeap();
	long n = GetSize();
	for (long v = n - 1; v > 0; v--) {
		Swap(0, v);
		DownHeap(0, v);
	}
}
#endif

void AnyArrayImpl::MergeByComparer(long lo, long hi, long m, const AnyIntCompare &comparer)
{
	if (hi < m + 1 || lo > m) {
		return;    // nothing to merge
	}
	long i, j, k;
	const long sz = m - lo + 1;
#if defined(WIN32) && (_MSC_VER <= 1200) // VC6 or lower
	long *a = new long[sz];		// temporary array of lower half
#else
	long a[sz];					// temporary array of lower half
#endif
	for (k = 0, i = lo; i <= m && k < sz; i++, k++) {
		a[k] = IntAt(i);
	}
	Assert(k == sz);
	Assert(i > m);
	j = m + 1;
	k = 0;
	i = lo;
	while (k < sz && j <= hi) {
		Assert(i < j);
		if (comparer.Compare(*this, a[k], IntAt(j)) <= 0) {
			fInd->SetIndex(i, a[k]);
			k++;
		} else {
			fInd->SetIndex(i, IntAt(j));
			j++;
		}
		i++;
	}
	// copy the remainder
	while ( k < sz && i < j ) {
		Assert(j > hi);
		fInd->SetIndex(i, a[k]);
		i++;
		k++;
	}
	Assert(i == j);
	Assert(k == sz);
#if defined(WIN32) && (_MSC_VER <= 1200) // VC6 or lower
	delete[] a;
#endif
}

void AnyArrayImpl::MergeSortByComparer(long low, long high, const AnyIntCompare &comparer)
{
	if (low >= high) {
		return;
	}
	long middle = (low + high) / 2;
	MergeSortByComparer(low, middle, comparer);
	MergeSortByComparer(middle + 1, high, comparer);
	MergeByComparer(low, high, middle, comparer);
}

void AnyArrayImpl::SortByKey()
{
	//HeapSort();
	//Qsort(0,GetSize()-1);
	MergeSortByComparer(0, GetSize() - 1, theKeyComparer);
	RecreateKeyTabe();
}

void AnyArrayImpl::SortReverseByKey()
{
	MergeSortByComparer(0, GetSize() - 1, theReverseKeyComparer);
	RecreateKeyTabe();
}

void AnyArrayImpl::SortByAnyComparer(const AnyComparer &comparer)
{
	MergeSortByComparer(0, GetSize() - 1, AnyIntComparerCompare(comparer));
	RecreateKeyTabe();
}

void AnyArrayImpl::RecreateKeyTabe()
{
	if (fKeys) {
		fKeys->Clear();
		for (long i = 0; i < fSize; i++) {
			const char *sn = SlotName(i);
			if (sn) {
				fKeys->Append(sn, i);
			}
		}
	}
}

//---- Anything ---------------------------------------------------------------------
Anything::Anything(Allocator *a) 		: fAnyImp(0)
{
	SetAllocator(a);
}
Anything::Anything(AnyImpl *ai) 			: fAnyImp(ai)
{
	SetAllocator(ai ? ai->fAllocator : Storage::Current());
}
Anything::Anything(int i, Allocator *a) 			: fAnyImp(new ((a) ? a : Storage::Current()) AnyLongImpl(i, a))
{
	SetAllocator(a);
}
#if !defined(BOOL_NOT_SUPPORTED)
Anything::Anything(bool b, Allocator *a) 		: fAnyImp(new ((a) ? a : Storage::Current()) AnyLongImpl(b, a))
{
	SetAllocator(a);
}
#endif
Anything::Anything(long i, Allocator *a) 			: fAnyImp(new ((a) ? a : Storage::Current()) AnyLongImpl(i, a))
{
	SetAllocator(a);
}
Anything::Anything(float f, Allocator *a)  			: fAnyImp(new ((a) ? a : Storage::Current()) AnyDoubleImpl(f, a))
{
	SetAllocator(a);
}
Anything::Anything(double d, Allocator *a) 			: fAnyImp(new ((a) ? a : Storage::Current()) AnyDoubleImpl(d, a))
{
	SetAllocator(a);
}
Anything::Anything(IFAObject *o, Allocator *a) 				: fAnyImp(new ((a) ? a : Storage::Current()) AnyObjectImpl(o, a))
{
	SetAllocator(a);   // PS: Only for transient pointers NO checking!!
}
Anything::Anything(const String &s, Allocator *a) 			: fAnyImp(new ((a) ? a : Storage::Current()) AnyStringImpl(s, a))
{
	SetAllocator(a);
}
Anything::Anything(const char *s, long len, Allocator *a)	: fAnyImp(new ((a) ? a : Storage::Current()) AnyStringImpl(s, len, a))
{
	SetAllocator(a);
}
Anything::Anything(void *buf, long len, Allocator *a)		: fAnyImp(new ((a) ? a : Storage::Current()) AnyBinaryBufImpl(buf, len, a))
{
	SetAllocator(a);
}

Anything::Anything(const Anything &any, Allocator *a) : fAnyImp(0)
{
	SetAllocator(a);
	if (GetAllocator() == any.GetAllocator()) {
		// add reference
		fAnyImp = any.GetImpl();
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

Anything::~Anything()
{
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

Anything::EType Anything::GetType() const
{
	return GetImpl() ? GetImpl()->GetType() : Anything::eNull;
}

void Anything::EnsureArrayImpl(Anything &anyToEnsure)
{
	StartTrace(Anything.EnsureArrayImpl);
	if ( anyToEnsure.GetType() != Anything::eArray ) {
		Trace("is not array");
		// behave friendly if current slot is not an array impl, eg don't lose current entry
		MetaThing expander(anyToEnsure.GetAllocator());
		if ( !anyToEnsure.IsNull() ) {
			Trace("was not Null");
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
		return (LongImpl(GetImpl())->AsLong((long) dflt) != 0);
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

void Anything::Expand() const
{
	if (GetType() != eArray) {
		Anything *nonConstThis = (Anything *)this;
		Allocator *al = nonConstThis->GetAllocator();
		Assert(al != 0);
		AnyArrayImpl *a = new ((al) ? al : Storage::Current()) AnyArrayImpl(al);
		if (a && GetType() != eNull) {
			a->At(0L) = *this; // this semantic is different from the Java version
		}
		if (nonConstThis->GetImpl()) {
			nonConstThis->GetImpl()->Unref();
		}
		nonConstThis->fAnyImp = a;
		if (0 == a) {
			nonConstThis->SetAllocator(al);    // remember allocator in case of insanity or no memory
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

Anything &Anything::DoAt(long i) const
{
	Assert(i >= 0);
	if ( i > 0 || GetType() == eNull ) {
		// check whether Expansion really needed
		// Expand if simple type and index > 0
		// or type is still null
		Expand();
	}
	return DoGetAt(i);
}

Anything &Anything::DoGetAt(long i) const
{
	if ( GetType() != eArray ) {
		// we have an index == 0
		// and the type is not null
		// so just return this
		return *(Anything *)this;
	}
	// double check for the index range since in a productive version
	// Assert does expand to nothing
	return ArrayImpl(GetImpl())->At((i >= 0) ? i : 0);
}

Anything &Anything::DoAt(const char *k) const
{
	long i;
	if (k && (*k != 0)) {
		if ( (i = FindIndex(k)) == -1L) {
			Expand();
			Assert(Anything::eArray == GetImpl()->GetType());
			return ArrayImpl(GetImpl())->At(k);
		} else {
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

void Anything::Remove(long slot)
{
	if (IsArrayImpl(GetImpl())) {
		ArrayImpl(GetImpl())->Remove(slot);
	}
}

void Anything::Remove(const char *k)
{
	if (IsArrayImpl(GetImpl())) {
		long slot = FindIndex(k);
		if (slot >= 0) {
			ArrayImpl(GetImpl())->Remove(slot);
		}
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

Anything &Anything::operator= (int i)
{
	Allocator *a = GetAllocator();
	if (GetImpl()) {
		GetImpl()->Unref();
	}
	fAnyImp = new ((a) ? a : Storage::Current()) AnyLongImpl(i, a);
	if (!GetImpl()) {
		SetAllocator(a);    // make it sane in case of allocation errors
	}
	return *this;
}

#if !defined(BOOL_NOT_SUPPORTED)
Anything &Anything::operator= (bool i)
{
	Allocator *a = GetAllocator();
	if (GetImpl()) {
		GetImpl()->Unref();
	}
	fAnyImp = new ((a) ? a : Storage::Current()) AnyLongImpl(i, a);
	if (!GetImpl()) {
		SetAllocator(a);    // make it sane in case of allocation errors
	}
	return *this;
}
#endif

Anything &Anything::operator= (long l)
{
	Allocator *a = GetAllocator();
	if (GetImpl()) {
		GetImpl()->Unref();
	}
	fAnyImp = new ((a) ? a : Storage::Current()) AnyLongImpl(l, a);
	if (!GetImpl()) {
		SetAllocator(a);    // make it sane in case of allocation errors
	}
	return *this;
}

Anything &Anything::operator= (float f)
{
	Allocator *a = GetAllocator();
	if (GetImpl()) {
		GetImpl()->Unref();
	}

	fAnyImp = new ((a) ? a : Storage::Current()) AnyDoubleImpl(f, a);
	if (!GetImpl()) {
		SetAllocator(a);    // make it sane in case of allocation errors
	}
	return *this;
}

Anything &Anything::operator= (double d)
{
	Allocator *a = GetAllocator();
	if (GetImpl()) {
		GetImpl()->Unref();
	}
	fAnyImp = new ((a) ? a : Storage::Current()) AnyDoubleImpl(d, a);
	if (!GetImpl()) {
		SetAllocator(a);    // make it sane in case of allocation errors
	}
	return *this;
}

Anything &Anything::operator= (IFAObject *obj)
{
	Allocator *a = GetAllocator();
	if (GetImpl()) {
		GetImpl()->Unref();
	}
	fAnyImp = new ((a) ? a : Storage::Current()) AnyObjectImpl(obj, a);
	if (!GetImpl()) {
		SetAllocator(a);    // make it sane in case of allocation errors
	}
	return *this;
}

Anything &Anything::operator= (const char *s)
{
	Allocator *a = GetAllocator();
	AnyImpl *oldImpl = GetImpl();
	fAnyImp = new ((a) ? a : Storage::Current()) AnyStringImpl(s, ((s) ? strlen(s) : 0), a);
	if (oldImpl) {
		oldImpl->Unref();
	}
	if (!GetImpl()) {
		SetAllocator(a);    // make it sane in case of allocation errors
	}
	return *this;
}
Anything &Anything::operator= (const String &s)
{
	Allocator *a = GetAllocator();
	AnyImpl *oldImpl = GetImpl();

	fAnyImp = new ((a) ? a : Storage::Current()) AnyStringImpl(s, a);
	if (oldImpl) {
		oldImpl->Unref();
	}
	if (!GetImpl()) {
		SetAllocator(a);    // make it sane in case of allocation errors
	}
	return *this;
}
Anything &Anything::operator= (const Anything &a)
{
	if (GetImpl() != a.GetImpl()) {
		Allocator *al = GetAllocator();
		// do a deepclone if allocators don't match
		AnyImpl *oldImpl = GetImpl();
		if (a.GetAllocator() == al) {
			fAnyImp = a.GetImpl();
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
}

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
	ostream &fOs;
	void PrintKey(const String &s) { // no copy for efficiency
		bool needquote = false;

		fOs << '/';

		if (isdigit( (unsigned char) s[0L])) {
			needquote = true;    // quote all numbers
		} else
			for (long i = s.Length(); i-- > 0 && ! needquote;) {
				needquote = AnythingToken::isNameDelimiter(s[i]);
			}

		if (needquote) {
			s.IntPrintOn(fOs, '\"');
		} else {
			fOs << s ;
		}
		fOs << ' ';
	}
	void ArrayBefore(const ROAnything value, const AnyImpl *id, long index, const char *slotname) {
		fOs << '{';
	}
	void ArrayBeforeElement(long lIdx, const String &key) {
		if (key.Length() > 0) {
			PrintKey(key);
		}
	}
	//void ArrayAfterElement(long index, const String &key){}
	void ArrayAfter(const ROAnything value, const AnyImpl *id, long index, const char *slotname) {
		fOs << '}';
	}

public:
	SimpleAnyPrinter(ostream &os): fOs(os) {}
	virtual void	VisitNull(long lIdx, const char *slotname) {
		fOs.put('*');
	}
	virtual void	VisitCharPtr(const String &value, const AnyImpl *id, long lIdx, const char *slotname) {
		value.IntPrintOn(fOs);
	}
	virtual void	VisitLong(long value, const AnyImpl *id, long lIdx, const char *slotname) {
		if ( id->GetType() == Anything::eLong ) {
			fOs << id->AsString("");
		} else {
			// this section is just for the impossible case where...
			fOs << value;
		}
	}
	virtual void	VisitDouble(double value, const AnyImpl *id, long lIdx, const char *slotname) {
		if ( id->GetType() == Anything::eDouble ) {
			fOs << id->AsString("");
		} else {
			// this section is just for the impossible case where...
			// keep track of precision, so we can read in our
			// numbers anyway
			int prec = fOs.precision();
			fOs.precision(20); // safety margin, 16 should be OK for doubles
			fOs << value;
			fOs.precision(prec); // restore default
		}
	}
	virtual void	VisitVoidBuf(const String &value, const AnyImpl *, long lIdx, const char *slotname) {
		fOs << '[' << value.Length() << ';'; // separator
		fOs.write((const char *)value, value.Length());
		fOs << ']';

	}
	virtual void	VisitObject(IFAObject *value, const AnyImpl *id, long lIdx, const char *slotname) {
		fOs << '&' << (long) value;
	}
};

class PrettyAnyPrinter: public SimpleAnyPrinter
{
protected:
	long	fLevel;
	void Tab() {
		for (long i = 0; i < fLevel; i++) {
			fOs.put(' ').put(' ');
		}
	}
	void ArrayBefore(const ROAnything value, const AnyImpl *id, long index, const char *slotname) {
		fOs << "{\n"; // } trick sniff
		fLevel++;
	}
	void ArrayBeforeElement(long lIdx, const String &key) {
		Tab();
		SimpleAnyPrinter::ArrayBeforeElement(lIdx, key);
	}
	void ArrayAfterElement(long lIdx, const String &key) {
		fOs << '\n';
	}
	void ArrayAfter(const ROAnything value, const AnyImpl *id, long index, const char *slotname) {
		fLevel--;
		Tab(); // { trick sniff
		fOs << '}';
	}

public:
	PrettyAnyPrinter(ostream &os, long level = 0): SimpleAnyPrinter(os), fLevel(level) {}
};
class XrefAnyPrinter : public PrettyAnyPrinter
{
protected:
	PrinterXrefHandler fXref;
	bool PrintAsXref(const AnyImpl *ai) {
		if (!fXref.IsDefined((long)ai)) {
			fXref.DefineBackRef((long)ai); // remember symbolic slot name
			return false;
		} else {
			fOs << "%\"" << fXref.GetBackRef((long)ai) << "\"";
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
	XrefAnyPrinter(ostream &os, long level = 0): PrettyAnyPrinter(os, level) {
	}
	// FIXME: take mechanics from IntPrintOnWithRef to this class.
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
ostream &Anything::PrintOn(ostream &os, bool pretty) const
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

void Anything::Export(ostream &os, int level) const
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

bool Anything::Import(istream &is, const char *fname)
{
	if (! !is) {
		InputContext context(is, fname);
		AnythingParser p(context);
		if (!p.DoParse(*this)) {
			// there has been a syntax error
			String m("Anything::Import "), strFName(context.fFileName);
			bool bHasExt = true;
			if ( !strFName.Length() && fname != NULL ) {
				strFName << fname;
				bHasExt = (strFName.SubString(strFName.Length() - 4L) == ".any");
			} else {
				strFName << "<NoName>";
			}
			m << strFName << (bHasExt ? ":" : ".any");
			m.Append(": syntax error");
			SysLog::WriteToStderr(m << "\n");
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
		// calculate key values into anything; cache hasvalues and size information
		// assume we have at least one delimSlot in path
		register const char *tokPtr = path;
		if (!tokPtr || *tokPtr == delimSlot) {
			return false;
		}

		const Anything *c = this; // pointers are faster!
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
					return false;    // not a valid number
				}
				// check if index is defined
				if (lIdx >= c->GetSize() || (*c)[lIdx].IsNull()) { // caution: auto-enlargement!
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
				if ((lIdx = c->FindIndex(tokPtr, keylen, h)) < 0) {
					return false;
				}
				tokPtr += keylen;
			} else {
				return false;
			}
			c = &(c->DoGetAt(lIdx));
		} while (c && *tokPtr != '\0');
		// we got it
		if (c) {
			result = *c;
			return true;
		}
	}
	return false;
}

Allocator *Anything::GetAllocator() const
{
	if (GetImpl()) {
		return GetImplAllocator();
	}
	return (Allocator *)(bits&~0x01);
}

bool Anything::SetAllocator(Allocator *a)
{
	if ( !GetImpl() || !fAlloc ) {
		fAlloc = (a) ? a : Storage::Current();
		bits |= 0x01;
		return (a != 0);
	}
	return false;
}

Allocator *Anything::GetImplAllocator()const
{
	if (GetImpl()) {
		return GetImpl()->fAllocator;
	}
	return 0;
}

AnyImpl *Anything::GetImpl()const
{
	if (bits & 0x01) {
		return 0;
	} else {
		return fAnyImp;
	}
}
void Anything::Accept(AnyVisitor &v, long lIdx, const char *slotname) const
{
	if (GetImpl()) {
		GetImpl()->Accept(v, lIdx, slotname);
	} else {
		v.VisitNull(lIdx, slotname);
	}
}

// metathing is always an array
MetaThing::MetaThing(Allocator *a) : Anything(new ((a) ? a : Storage::Current()) AnyArrayImpl(a))
{
}

MetaThing::~MetaThing() { }
TrickyThing::TrickyThing(Allocator *a) : Anything(new ((a) ? a : Storage::Current()) AnyArrayImpl(a))
{
}

//---- ROAnything ---------------------------------------------------------------------

ROAnything::ROAnything()
{
	fAnyImp = 0;
}

ROAnything::ROAnything(const Anything &a)
	: fAnyImp(a.GetImpl())
{
	// no ref necessary
	// ROAnything don't do any
	// ref counting
}

ROAnything::ROAnything(const ROAnything &a)
	: fAnyImp(a.fAnyImp)
{
	// no ref necessary
	// ROAnything don't do any
	// ref counting
}

ROAnything::~ROAnything()
{
	// no unref necessary
	// ROAnything don't do any
	// ref counting
}

Anything ROAnything::DeepClone(Allocator *a) const
{
	Anything xref(a);
	if (fAnyImp) {
		return fAnyImp->DeepClone(a, xref);
	} else {
		return a;
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
	fAnyImp = a.fAnyImp;
	return *this;
}

ROAnything &ROAnything::operator= (const Anything &a)
{
	fAnyImp = a.GetImpl();
	return *this;
}

Anything::EType ROAnything::GetType() const
{
	if (fAnyImp) {
		return fAnyImp->GetType();
	} else {
		return Anything::eNull;
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
	if IsLongImpl(fAnyImp) {
		return (LongImpl(fAnyImp)->AsLong((long) dflt) != 0);
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
		if ( GetType() != Anything::eArray ) {
			// we have an index == 0
			// and the type is not null
			// so just return this
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

ostream &ROAnything::PrintOn(ostream &os, bool pretty) const
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

void ROAnything::Export(ostream &os, int level) const
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
					return false;    // not a valid number
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

//--- AnyImpls -------------------------------------------------------------

//--- AnyLongImpl ----------------------------------------------------------
String AnyLongImpl::AsString(const char *) const
{
	return fBuf;
}

const char *AnyLongImpl::AsCharPtr(const char *) const
{
	return (const char *)fBuf;
}

const char *AnyLongImpl::AsCharPtr(const char *, long &buflen) const
{
	buflen = fBuf.Length();
	return (const char *)fBuf;
}

//--- AnyObjectImpl ----------------------------------------------------------
const char *AnyObjectImpl::AsCharPtr(const char *, long &buflen) const
{
	buflen = strlen(gcObjectText);
	return gcObjectText;
}

//---- AnyDoubleImpl -----------------------------------------------------------------
String AnyDoubleImpl::AsString(const char *) const
{
	return fBuf;
//	OStringStream out;
////		out.precision(20); // safety margin, 16 should be OK for doubles
//	out << fDouble << flush;
//	return out.str();
}

const char *AnyDoubleImpl::AsCharPtr(const char *dflt) const
{
	return (const char *)fBuf;
//	return dflt;
}

const char *AnyDoubleImpl::AsCharPtr(const char *dflt, long &buflen) const
{
	buflen = fBuf.Length();
	return (const char *)fBuf;
//	buflen=0L;
//	return dflt;
}

//---- AnyBinaryBufImpl -----------------------------------------------------------------
const char *AnyBinaryBufImpl::AsCharPtr(const char *, long &buflen) const
{
	if (fBuf.Capacity() > 0) {
		buflen = fBuf.Length();
		return (const char *)fBuf;
	} else {
		buflen = 0;
		return 0;
	}
}

//---- AnyStringImpl -----------------------------------------------------------------
long AnyStringImpl::Compare(const char *other)
{
	if ( fString.Compare(other) == 0 ) {
		return 0;
	}
	return -1;
}

long AnyStringImpl::AsLong(long dflt)
{
	return fString.AsLong(dflt);
}

double AnyStringImpl::AsDouble(double dflt)
{
	return fString.AsDouble(dflt);
}

String AnyStringImpl::AsString(const char *) const
{
	return fString;
}

const char *AnyStringImpl::AsCharPtr(const char *) const
{
	return (const char *)fString; // PS: fString.AsCharPtr(dft); use operator const char * instead
}

const char *AnyStringImpl::AsCharPtr(const char *, long &buflen) const
{
	buflen = fString.Length();
	return (const char *)fString;
}

//---- InputContext -----------------------------------------------------------------
void InputContext::SkipToEOL()
{
	char c = ' ';

	while (Get(c) && c != '\n' && c != '\r');

	if (c == '\n' || c == '\r') {
		fLine ++; // count contexts lines
		// we should treat DOS-convention of CRLF nicely by reading it
		char crnl;
		if (Get(crnl) &&
			((crnl != '\r' && c == '\n') ||
			 (c == '\r' && crnl != '\n')) ) {
			Putback(crnl); // no crlf lfcr sequence
		}
	} // we have reached EOF just do nothing
}

//--- AnyKeyAssoc -------------------------------------------------------
#if !defined(OPERATOR_NEW_ARRAY_NOT_SUPPORTED)
void *AnyKeyAssoc::operator new[](size_t size, Allocator *a)
{
	if (a) {
		void *mem = a->Calloc(1, size + sizeof(Allocator *));
		((Allocator **)mem)[0L] = a;				// remember address of responsible Allocator
		return (char *)mem + sizeof(Allocator *); // needs cast because of pointer arithmetic
	} else { //SOP: untested, make compiler happy
		size += sizeof(Allocator *);
		return ((char *)::operator new(size + sizeof(Allocator *)));
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void AnyKeyAssoc::operator delete[](void *ptr, Allocator *a)
{
	if (ptr) {
		void *realPtr = (char *)ptr - sizeof(Allocator *);
		if (a) {
			a->Free(realPtr);
		} else {
			::operator delete(realPtr);    //SOP: untested
		}
	}
	return;
}
#endif

void AnyKeyAssoc::operator delete[](void *ptr)
{
	if (ptr) {
		void *realPtr = (char *)ptr - sizeof(Allocator *);
		Allocator *a = ((Allocator **)realPtr)[0L];	// retrieve Allocator
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		AnyKeyAssoc::operator delete[](ptr, a);
#else
		a->Free(realPtr);
#endif
	}
	return;
}
#endif
AnyKeyAssoc::AnyKeyAssoc(const Anything &value, const char *key)
	: fValue(value), fKey(key, -1, value.GetAllocator()),  fAllocator(value.GetAllocator())
{
}

AnyKeyAssoc::AnyKeyAssoc(const AnyKeyAssoc &aka)
	: fValue(aka.fValue), fKey(aka.fKey), fAllocator(aka.fValue.GetAllocator())
{
}

// used when allocating arrays... CAUTION: elements then must be initialized manually with Init()!
AnyKeyAssoc::AnyKeyAssoc()
	: fValue((Allocator *)0), fKey((Allocator *)0), fAllocator(0)
{
}

AnyKeyAssoc::~AnyKeyAssoc()
{
}

void AnyKeyAssoc::Init(Allocator *a)
{
	fAllocator = a;
	fValue.SetAllocator(a);
	fKey.SetAllocator(a);
}

void AnyKeyAssoc::SetKey(const char *key)
{
	fKey = key;
}

void AnyKeyAssoc::SetValue(const Anything &val)
{
	fValue = val;
}

AnyKeyAssoc &AnyKeyAssoc::operator=(const AnyKeyAssoc &aka)
{
	fValue = aka.Value();
	if ( aka.Key() ) {
		fKey = aka.Key();
	}
	return *this;
}

//---- AnyKeyTable ------------------------------------------------------

#define LOADFACTOR 0.75

long IFANextPrime(long x)
{
	if (x <= 3) {
		return 3;
	}

	if (x % 2 == 0) {
		x++;
	}

	for (;;) {
		long n;
		for (n = 3; (n *n <= x) && ((x % n) != 0); n += 2)
			;
		if (n * n > x) {
			return x;
		}
		x += 2;
	}
}

AnyKeyTable::AnyKeyTable(AnyArrayImpl *table, long initCapacity)
	: fKeyTable(table),
	  fHashTable(0),
	  fThreshold(0),
	  fCapacity(0),
	  fAllocator(fKeyTable->fAllocator)
{
	InitTable(initCapacity);
}

AnyKeyTable::~AnyKeyTable()
{
	if (fHashTable) {
		Clear();
		fAllocator->Free(fHashTable);
		fKeyTable = 0;
		fHashTable = 0;
	}
}

void AnyKeyTable::InitTable(long cap)
{
	if (cap < cInitCapacity) {
		cap = cInitCapacity;
	}
	fCapacity = IFANextPrime(cap);
	fHashTable = (long *)fAllocator->Malloc(fCapacity * sizeof(long));
	fThreshold = (3 * fCapacity) / 4;
	Clear();
}

void AnyKeyTable::Clear()
{
	// only works with 2 complement binary arithmetic!
	memset(fHashTable, -1, sizeof(long) * fCapacity);
}

long AnyKeyTable::DoHash(const char *key, bool append, long sizehint, u_long hashhint) const
{
	// calculate some index into fHashTable
	long keylen = sizehint;
	long hashval = (hashhint) ? hashhint : IFAHash(key, keylen);
	long hash = hashval % fCapacity;

	// look for next free slot
	// do wrap around search
	long bufSz = fKeyTable->fBufSize;

	long i = hash;
	do {
		long lIdx = fHashTable[i];
		switch (lIdx) {
			case -1:
				return i;
			case -2:// slot is deleted
				if (append) {
					return i;
				} else {
					break;
				}
			default: {
				long at = fKeyTable->IntAt(lIdx);
				const String &keyAtVal = fKeyTable->fContents[at/bufSz][at%bufSz].Key(); // might be null
				if (keylen == keyAtVal.Length()) {
					const char *keyPtr = key, *keyAtValPtr = keyAtVal;
					const char *eptr = keyPtr + keylen;
#if 0 /* Optimization which does not increase speed on Linux */
					const long *kl = (const long *) keyPtr;
					const long *kavl = (const long *)keyAtValPtr;
					const long iter = keylen / sizeof(long);
					for (int j = 0; j < iter; j++) {
						if (*kl++ != *kavl++) {
							goto loop;
						}
					}
					// compare remainder
					keyPtr += iter * sizeof(long);
					keyAtValPtr += iter * sizeof(long);
#endif
					for ( ; keyPtr < eptr; ++keyPtr, ++keyAtValPtr)
						if ( *keyPtr != *keyAtValPtr ) {
							goto loop;
						}
					return i; // we found the key
				}
			}
		}
	loop:
		// increment and modulo capacity -- NO modulo is slow!
		// ensure we wrap arround in the table
		if (++i >= fCapacity) {
			i -= fCapacity;
		}
	} while ( i != hash ); // finish loop if wrapped around
	return -1;
}

long AnyKeyTable::Append(const char *key, long atIndex)
{
	if ( key ) {
		if ( fKeyTable->fSize > fThreshold) {
			// everything gets a new place in the hashtable
			// so we're done with it
			Rehash(fCapacity * 2);
		}
		// calculate hash index and put key table index
		// into it
		fHashTable[DoHash(key, true)] = atIndex;
	}
	return atIndex;
}
void AnyKeyTable::Update(long fromIndex)
{
	for (long i = 0; i < fCapacity; i++) {
		long lIdx = fHashTable[i];
		if ( lIdx == fromIndex ) {
			fHashTable[i] = -2;    // mark as deleted
		} else if ( lIdx > fromIndex ) {
			fHashTable[i] = lIdx - 1;    // update position in keytable
		}
	}
}

long AnyKeyTable::At(const char *key, long sizehint, u_long hashhint) const
{
	// returns valid external index into key table
	// or -1= not found
	long lIdx = DoHash(key, false, sizehint, hashhint);
	if ( lIdx > -1 ) {
		return fHashTable[lIdx];
	}
	return lIdx;
}

void AnyKeyTable::Rehash(long newCap)
{
	long oldCapacity = fCapacity;
	long *ot = fHashTable;

	// allocate new table with new capacity
	// table may expand or shrink
	InitTable(newCap);

	// iterate over the old table and rehash
	// values
	for ( long i = 0; i < oldCapacity; i++ ) {
		register long slot = ot[i];

		if (slot > -1) {	// assumption: we found an index for a key
			long at = fKeyTable->IntAt(slot);
			register const char *key = fKeyTable->fContents[fKeyTable->IntAtBuf(at)][fKeyTable->IntAtSlot(at)].Key();
			Assert(key);
			long lIdx = DoHash(key);
			Assert(lIdx > -1 && lIdx < fCapacity);
			fHashTable[lIdx] = slot;
		}
	}
	// free old table
	fAllocator->Free(ot);
}

void AnyKeyTable::PrintHash()
{
	for (long i = 0; i < fCapacity; i++) {
		if ( fHashTable[i] > -1 ) {
			String m;
			m << "[" << i << "]<" << fHashTable[i] << "> ";
			SysLog::WriteToStderr(m);
		}
	}
	SysLog::WriteToStderr("\n", 1);
}

void *AnyKeyTable::operator new(size_t size, Allocator *a)
{
	if (a) {
		return a->Calloc(1, size);
	} else {
		return ::operator new(size);
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void AnyKeyTable::operator delete(void *d, Allocator *a)
{
	if (d) {
//#if defined(PURE_LEAK) || defined(__linux__)
//		a = (a ? a : Storage::Current());
//#endif
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
	}
}
#endif

void AnyKeyTable::operator delete(void *d)
{
	if (d) {
		Allocator *a = ((AnyKeyTable *)d)->fAllocator;
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		AnyKeyTable::operator delete(d, a);
#else
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
#endif
	}
}

//--- AnyIndTable ------------------------------------------------------
AnyIndTable::AnyIndTable(long initCapacity, Allocator *a) :
	fIndexTable(0), fEmptyTable(0), fCapacity(initCapacity), fSize(0), fAllocator(a)
{
	fEmptyTable = 0;
	InitTable(initCapacity);
	InitEmpty(0, fCapacity);
}

AnyIndTable::~AnyIndTable()
{
	if (fIndexTable) {
		Clear();
		fAllocator->Free(fIndexTable);
		fAllocator->Free(fEmptyTable);

		fIndexTable = 0;
	}
}

void AnyIndTable::InitTable(long cap)
{
	fCapacity = cap;
	fIndexTable = (long *)fAllocator->Malloc(fCapacity * sizeof(long));
	Clear();
}

void AnyIndTable::Clear()
{
	memset(fIndexTable, -1, sizeof(long) * fCapacity);
}

void AnyIndTable::Expand(long slot)
{
	Assert( slot >= fCapacity );

	// save old index table and capacity
	long *old = fIndexTable;
	long oldCap = fCapacity;

	// check for sufficient new capacity
	if ( slot >= fCapacity * 2 ) {
		fCapacity = slot;
	}

	// initalize index table with new capacity
	InitTable(fCapacity * 2);

	// initialize the empty table
	InitEmpty(oldCap, fCapacity);

	// initalize the indices in the index table
	InitIndices(slot, old);

	fAllocator->Free(old);
//	delete [] old;
}

void AnyIndTable::InitEmpty(long oldCap, long newCap)
{
	long *old = fEmptyTable;
	long sz = 0;
	long i = 0;

	// allocate the new size of the empty table
	fEmptyTable = (long *)fAllocator->Malloc(newCap * sizeof(long));

	// calculate the size to be copied
	if ( oldCap < newCap )	{
		sz = oldCap;
	} else	{
		sz = newCap;    // the size has shrunk
	}

	// copy the empty index array
	for (i = 0; i < sz; i++)	{
		fEmptyTable[i] = old[i];
	}

	// fill in straight index (never used so far)
	if ( sz < newCap )
		for (i = sz; i < newCap; i++) {
			fEmptyTable[i] = i;
		}

	fAllocator->Free(old);
}

void AnyIndTable::InitIndices(long slot, long *ot)
{
	Assert( fSize <= slot );

	long i = 0;

	// copy old indices if they exist
	if ( ot ) {
		for (i = 0; i < fSize; i++) {
			fIndexTable[i] = ot[i];
		}
	}

	// reuse deleted slots if they exist
	for (i = fSize; i <= slot; i++) {
		fIndexTable[i] = fEmptyTable[i];
	}
	fSize = slot + 1;
}

long AnyIndTable::At(long slot)
{
	Assert( slot >= 0 );

	// check for capacity overflow
	if ( slot >= fCapacity ) {
		Expand(slot);
	}

	// check for size overflow
	if ( slot >= fSize ) {
		InitIndices(slot, 0);
	}

	// return the index
	Assert( slot < fSize );
	return fIndexTable[slot];
}

long AnyIndTable::FindAt(long slot)
{
	Assert( slot >= 0 );

	// check for capacity overflow
	if ( slot < fCapacity && slot < fSize) {
		return fIndexTable[slot];
	}
	return -1L;
}

void AnyIndTable::Remove(long slot)
{
	Assert( slot >= 0 && slot < fSize );

	// save the index for later reuse
	fEmptyTable[fSize-1] = fIndexTable[slot];

	// shift the table down
	for (long i = slot; i < fSize - 1; i++) {
		fIndexTable[i] = fIndexTable[i+1];
	}

	fSize--;
}
void AnyIndTable::Swap(long l, long r)
{
	Assert( l >= 0 && l < fSize );
	Assert( r >= 0 && r < fSize );
	long t = fIndexTable[l];
	fIndexTable[l] = fIndexTable[r];
	fIndexTable[r] = t;
}
void AnyIndTable::SetIndex(long slot, long index)
{
	Assert( slot >= 0 && slot < fSize );
	if (slot >= 0 && slot < fSize) {
		fIndexTable[slot] = index;
	} else {
		cerr << "OOPS, slot = " << slot << " index = " << index << " fSize = " << fSize << endl;
	}
}
void AnyIndTable::PrintTable()
{
	String m("IndexTable: \n");
	SysLog::WriteToStderr(m);
	for ( long i = 0; i < fSize; i++) {
		String m1;
		m1 << "[" << i << "]<" << fIndexTable[i] << ">" << "\n";
		SysLog::WriteToStderr(m);
	}
}

void *AnyIndTable::operator new(size_t size, Allocator *a)
{
	if (a) {
		return a->Calloc(1, size);
	} else {
		return ::operator new(size);
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void AnyIndTable::operator delete(void *d, Allocator *a)
{
	if (d) {
//#if defined(PURE_LEAK) || defined(__linux__)
//		a = (a ? a : Storage::Current());
//#endif
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
	}
}
#endif

void AnyIndTable::operator delete(void *d)
{
	if (d) {
		Allocator *a = ((AnyIndTable *)d)->fAllocator;
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		AnyIndTable::operator delete(d, a);
#else
		if (a) {
			a->Free(d);
		} else {
			::operator delete(d);
		}
#endif
	}
}

// expect
// Any:     '{' sequence_including_}
//     |    eString | eDecimalNumber | eOctalNumber | eHexNumber | eFloatNumber
//     |  '*' | '&' eDecimalNumber | eBinaryBufImpl | eError
bool AnythingParser::DoParse(Anything &any)
{
//	StartTrace(AnythingParser.DoParse);
	// free old impl
	Allocator *a = any.GetAllocator();
	any = Anything((a) ? a : Storage::Current()); // assignment should be OK, but we keep it safe

	ParserXrefHandler xrefs;
	AnythingToken tok(fContext);
	bool ok = false;
	if ( '{' == tok.fToken) {
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
//	StartTrace(AnythingParser.DoParseSequence);
	Allocator *a = (any.GetAllocator()) ? any.GetAllocator() : Storage::Current();
	bool ok = true;
	// we need to make it an array
	any = MetaThing(a);
	do {
		Anything element(a);
		String key;
		bool lastok = true;

		AnythingToken tok(fContext);
//		Trace("Tok[" << tok.fText << "]");
	restart:    // for behaving nicely in case of a syntax error
		switch (tok.fToken) {
			case '}' : // '{' this is to cheat sniff
				return ok; // we are done...
			case AnythingToken::eError:
				lastok = false; // try to resync
				Error("syntax error invalid token", tok.fText);
				break;
			case AnythingToken::eNullSym:
				// premature EOF
				return false;

			case AnythingToken::eInclude : {
				ImportIncludeAny(element, tok.fText);
				any.Append(element);
				break;
			}
			case AnythingToken::eRef : {
				// a unnamed slot with a reference
//			Trace("AnythingToken::eRef");
				// we use '%' to devide the slotNamePath and the index of the unnamed slot
				// to keep the order of the inserted slots as requested we need
				// to add a dummy slot which will be linked in a second step
				element = tok.fText;
				long lIdx = any.Append(element);
				// add temporary for resolving reference after anything is fully parsed
				xrefs.DefinePatchRef(lIdx);
				break;
			}
			case AnythingToken::eIndex :
//			Trace("AnythingToken::eIndex");
				key = tok.fText; // remember index
//            Trace("key:" << key);
				if (key.Length() > 0) {
					if (any.IsDefined(key)) {
						// double definition, should be a warning....
						Error("Anything index double defined, last takes precedence", tok.fText);
						// key = "";
						// leave ok OK, because we haven't ok = false;
					}
				}
				tok = AnythingToken(fContext); // get next one
				if (AnythingToken::eRef == tok.fToken) {
					// link it to the given slotname (must exist)
					element = tok.fText;
					any[key] = element;

					// add temporary for resolving reference after anything is fully parsed
					xrefs.DefinePatchRef(key);
					// to keep the order of the inserted slots as requested we need
					// to add a dummy slot which will be linked in a second step
					break;
				} else if (AnythingToken::eInclude == tok.fToken) {
					ImportIncludeAny(element, tok.fText);
					any[key] = element;
					break;
				} else if (AnythingToken::eIndex == tok.fToken) {
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
				if ('{' == tok.fToken) {
					long lIdx = 0;
					String marklevel = xrefs.GetCurrentLevel();
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
				} else if (element.GetType() != Anything::eNull || lastok) {
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
	Allocator *a = (any.GetAllocator()) ? any.GetAllocator() : Storage::Current();
	Assert(a != 0);
	switch (tok.fToken) {
		case '*' :
			any = Anything(a);
			break;
		case AnythingToken::eStringError:
			// notify user about error but behave nicely if string is not empty
			Error("syntax error: invalid string constant (missing quote)", tok.fText);
			if (tok.fText.Length() <= 0) {
				// ignore empty strings otherwise fall through!
				return false;
			}
		case AnythingToken::eString: // string impl
			any = tok.fText;
			break;
			// long impl.
		case AnythingToken::eDecimalNumber:
			any = atol(tok.fText);
			break;
		case AnythingToken::eOctalNumber:
			any = static_cast<long>(strtoul((const char *)tok.fText, 0, 8)); // AB: use explicit cast to make g++ happy
			// we do not check for conversion errors here
			break;
		case AnythingToken::eHexNumber:
			any = static_cast<long>(strtoul((const char *)tok.fText, 0, 16)); // AB: use explicit cast to make g++ happy
			// we do not check for conversion errors here
			break;
		case AnythingToken::eFloatNumber:
			any = atof(tok.fText);
			// we do not check for conversion errors here
			break;
		case AnythingToken::eBinaryBuf:
			// oops we cannot yet assign a binary-buf impl ?
			// but a temporary anything should be sufficient
			any = Anything((void *)(const char *)tok.fText, tok.fText.Length(), a);
			break;
		case AnythingToken::eObject:
			// make it an AnyObjectImpl
			any = Anything((IFAObject *)atol(tok.fText), a);
			break;
		default:
			// this is an error
			Error("syntax error invalid token", tok.fText);
			return false;
	}
	return true;
}

void AnythingParser::ImportIncludeAny(Anything &element, String &url)
{
//	StartTrace(AnythingParser.ImportIncludeAny);
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

		iostream *pStream = System::OpenStream(fileName, "");
		if (pStream) {
			element.Import(*pStream, fileName);
			delete pStream;

			if (queryString.Length() > 0 && !element.LookupPath(element, queryString) ) {
				// query not found
				element = Anything();
			}
		} else {
			Error("cannot open included Anything at", url);
		}
	} else {
		Error("protocol not yet supported", url);
	}
}

void AnythingParser::Error(const char *msg, const char *toktext)
{
	// put a space in front to give poor Sniff a chance
	String m(fContext.fFileName);
	bool bHasExt = true;
	if ( !m.Length() ) {
		m << "<NoName>";
	} else {
		bHasExt = (m.SubString(m.Length() - 4L) == ".any");
	}
	m << (bHasExt ? ":" : ".any:") << fContext.fLine << " " << msg << " [" << toktext << "]";
	SYSWARNING(m);
	m << "\n";
	SysLog::WriteToStderr(m);
}

//---------------- test case support ------------

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

//-- SlotFinder ---------------------------------------------------------------
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
				dest[destSlotname] = MetaThing(dest.GetAllocator());
			}
			dest = dest[destSlotname];
		} else {
			if (!dest.IsDefined(destIdx)) {
				Trace("adding indexslot [" << destIdx << "]");
				dest[destIdx] = MetaThing(dest.GetAllocator());
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
			if (IntOperate(dest, s, destIdx)) {
				// ensure that we have a valid anything
				if (dest[s].GetType() == Anything::eNull) {
					dest[s] = MetaThing(dest.GetAllocator());
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
			if (dest[destIdx].GetType() == Anything::eNull) {
				dest[destIdx] = MetaThing(dest.GetAllocator());
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
				dest[k] = MetaThing(dest.GetAllocator());
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

//-- SlotPutter ---------------------------------------------------------------

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
	Trace("sourceImpl:" << (long)source.GetType() << " destImpl:" << (long)dest.GetType());
	Trace("source any alloc:" << (long)source.GetAllocator() << " dest.alloc:" << (long)dest.GetAllocator());
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

//-- SlotCleaner ---------------------------------------------------------------

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

//-- SlotCopier ---------------------------------------------------------------
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

//----- SlotnameSorter -------------------------------------
void SlotnameSorter::Sort(Anything &toSort, EMode mode)
{
	StartTrace(SlotnameSorter.Sort);

	if (mode == SlotnameSorter::asc) {
		toSort.SortByKey();
	} else {
		toSort.SortReverseByKey();
	}
}
