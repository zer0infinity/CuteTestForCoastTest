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
namespace Coast {
	namespace URLUtils {
		enum NormalizeTag {
			eUpshift, eDownshift, eUntouched,
		};
		enum URLCheckStatus {
			eOk, eSuspiciousChar
		};
		//! remove \\r\\n or only \\r at end of line
		void TrimENDL(String &line);
		//!removes blanks from start or end
		void TrimBlanks(String &str, bool front = true);
		//!removes leading chars at front or end
		void TrimChars(String &str, bool front = true, char c = ' ');

		//! split key value pair with delimiter "delim" and add to Anything
		//! sample:
		//! buf: fooKey=fooVal, delim: '='  results in  /fooKey fooVal
		//! if delim is not found buf will be appended to any
		void Pair(const char *buf, char delim, Anything &any, Coast::URLUtils::NormalizeTag norm = Coast::URLUtils::eUntouched);

		//! split a list of delimiter separated key value pairs and add them
		//! to the Anything, the key value pairs are appended using Pair
		//! PS: use char for delim2
		void Split(const char *buf, char delim, Anything &out, char delim2 = '=', Coast::URLUtils::NormalizeTag norm =
				Coast::URLUtils::eUntouched);

		//! encodes the given string by expanding problematic characters into %XX escapes
		String MSUrlEncode(const String &str);
		//! RFC1808 compliant url encoder, allows chars to be excluded from escaping
		String urlEncode(const String &str, const String &exclusionSet = String());

		//! URL decodes the given string, (\%xx, \%uxxxx) Only values <= xFF are valid
		String urlDecode(const String &str, Coast::URLUtils::URLCheckStatus &eUrlCheckStatus, bool replacePlusByBlank = true);
		String urlDecode(const String &str, bool replacePlusByBlank = true);

		//! HTML decodes the given string, (\&\#xffff - \&\#xf, \&\#dddd - \&\#d). Only values <= xFF are valid
		String HTMLDecode(const String &str);

		//! Decode URL until no more escape sequences are left
		String ExhaustiveUrlDecode(const String &instr, Coast::URLUtils::URLCheckStatus &eUrlCheckStatus, bool replacePlusByBlank = true);
		String ExhaustiveUrlDecode(const String &instr, bool replacePlusByBlank = true);

		//! Decode HTML until no more escape sequences are left
		String ExhaustiveHTMLDecode(const String &instr);

		//! decode all elements of this anything
		void DecodeAll(Anything &a);

		//! check URL Path Encoding according to RFC1738
		bool CheckUrlEncoding(const String &str, const String &override = String());

		//! check URL Args Encoding according to RFC1738
		bool CheckUrlArgEncoding(String &str, const String &override = String());

		//! check URL Path char to be safe according to RFC1738
		//! You may pass in your own set of unsafe chars (overrideUnsafe
		//! If asciiExtended is enabled, you may define chars which are in the extended set which are ignored
		//! by this check.
		//! The default applies to RFC1738
		bool CheckUrlPathContainsUnsafeChars(String &str, const String &overrideUnsafe = String(), const String &overrideAscii = String(),
				bool asciiExtended = true);

		//! takes a full uri and decomposes it into a query anything
		//! \param query resulting anything structure
		//! \param uri input string from http server
		void HandleURI(Anything &query, const String &uri);

		//! takes a full uri and decomposes it into a query anything with known slotnames, certain slots will not be written e.g. "localhost" in domain
		/*! \param query resulting anything structure
		 \param uri input string from http server
		 \param baseURL if specified, will be prepended to uri */
		void HandleURI2(Anything &query, const String &uri, const char *baseURL = "");

		//! takes an anything containing keys and single values and forms a form content string out of them
		/*! \param kVPairs Anything containing key value pairs to compose a form string of
		 \return encoded string composed of key values pairs in kVPairs */
		String EncodeFormContent(Anything &kVPairs);

		//!remove '\'' and '"' at front and end
		void RemoveQuotes(String &myStr);

		//!up or downshift str according to normKey in case of eUntouched it is a no op
		void Normalize(String &str, Coast::URLUtils::NormalizeTag normKey);

		//!appends value to any under the tag key if key.Length>0; it appends to key if key already has a value
		void AppendValueTo(Anything &any, const String &key, const char *value);

		//!escape all not alnum chars in string with "&#<int value>"
		String HTMLEscape(const String &toEscape);

		//!remove the characters contained in badOnes from instr
		String RemoveUnwantedChars(const String &instr, const String &badOnes);

		//!cleans up request uri according to RFC 1808
		String CleanUpUriPath(String value);
	}
}

#endif
