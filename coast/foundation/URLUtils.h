/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URLUTILS_H
#define _URLUTILS_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "Anything.h"

//---- URLUtils ----------------------------------------------------------------------
//! some utility methods that simplify url string handling
class EXPORTDECL_FOUNDATION URLUtils
{
public:
	enum NormalizeTag {
		eUpshift,
		eDownshift,
		eUntouched,
	};
	enum URLCheckStatus {
		eOk,
		eSuspiciousChar
	};
	//! split key value pair with delimiter "delim" and add to Anything
	//! sample:
	//! buf: fooKey=fooVal, delim: '='  results in  /fooKey fooVal
	//! if delim is not found buf will be appended to any
	static void Pair(const char *buf, char delim, Anything &any, NormalizeTag norm = URLUtils::eUntouched);

	//! split a list of delimiter separated key value pairs and add them
	//! to the Anything, the key value pairs are appended using Pair
	//! PS: use char for delim2
	static void Split(const char *buf, char delim, Anything &out, char delim2 = '=', NormalizeTag norm = URLUtils::eUntouched);

	//! encodes the given string by expanding problematic characters into %XX escapes
	static String urlEncode(const String &str);
	//! RFC1808 compliant url encoder, allows chars to be excluded from escaping
	static String urlEncode(const String &str, String &exclusionSet);

	//! URL decodes the given string, (%xx, %uxxxx) Only values <= xFF are valid
	static String urlDecode(const String &str, URLUtils::URLCheckStatus &eUrlCheckStatus, bool replacePlusByBlank = true);
	static String urlDecode(const String &str, bool replacePlusByBlank = true);

	//! HTML decodes the given string, (&#xffff - &#xf, &#dddd - &#d). Only values <= xFF are valid
	static String HTMLDecode(const String &str);

	//! Decode URL until no more escape sequences are left
	static String ExhaustiveUrlDecode(const String &instr, URLUtils::URLCheckStatus &eUrlCheckStatus, bool replacePlusByBlank = true);
	static String ExhaustiveUrlDecode(const String &instr, bool replacePlusByBlank = true);

	//! Decode HTML until no more escape sequences are left
	static String ExhaustiveHTMLDecode(const String &instr);

	//! decode all elements of this anything
	static void DecodeAll(Anything &a);

	//! check URL Path Encoding according to RFC1738
	static bool CheckUrlEncoding(String &str, const String override = String());

	//! check URL Args Encoding according to RFC1738
	static bool CheckUrlArgEncoding(String &str);

	//! check URL Path char to be safe according to RFC1738
	//! You may pass in your own set of unsafe chars (overrideUnsafe
	//! If asciiExtended is enabled, you may define chars which are in the extended set which are ignored
	//! by this check.
	//! The default applies to RFC1738
	static bool CheckUrlPathContainsUnsafeChars(String &str, const String overrideUnsafe = String(),
			const String overrideAscii = String(), bool asciiExtended = true);

	//! takes a full uri and decomposes it into a query anything
	//! \param query resulting anything structure
	//! \param uri input string from http server
	static void HandleURI(Anything &query, const String &uri);

	//! takes a full uri and decomposes it into a query anything with known slotnames, certain slots will not be written e.g. "localhost" in domain
	//! \param query resulting anything structure
	//! \param uri input string from http server
	static void HandleURI2(Anything &query, const String &uri, const char *baseURL = "" );

	//! takes an in simple anything containing keys and single values and forms a form content string from them
	//! \param query resulting anything structure
	//! \param uri input string from http server
	static String EncodeFormContent(Anything &kVPairs);

	//!remove '\'' and '"' at front and end
	static void RemoveQuotes(String &myStr);

	//!removes blanks from start or end
	static void TrimBlanks(String &str, bool front = true);

	//!removes leading chars at front or end
	static void TrimChars(String &str, bool front = true, char c = ' ');

	//!up or downshift str according to normKey in case of eUntouched it is a no op
	static void Normalize(String &str, NormalizeTag normKey);

	//!appends value to any under the tag key if key.Length>0; it appends to key if key already has a value
	static void AppendValueTo(Anything &any, const String &key, const char *value);

	//!escape all not alnum chars in string with "&#<int value>"
	static String HTMLEscape(const String &toEscape);

	//!remove the characters contained in badOnes from instr
	static String RemoveUnwantedChars(const String &instr, const String &badOnes);

	//!cleans up request uri according to RFC 1808
	static String CleanUpUriPath(String value);

private:
	//!helper method handling the speial characters
	static char DecodeSpecialChars(const String &str, char c, long &lPos, long offset);
	static void DecodeSpecialHTMLChars(const String &str, String &res, long &lPos);
	static void ExtractDecimal(const String &str, String &res, long &lPos, long delta);
	static void ExtractHex(const String &str, String &res, long &lPos, long delta);

};

#endif
