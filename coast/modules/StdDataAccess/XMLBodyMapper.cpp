/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "XMLBodyMapper.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

// Utilities ---
class EXPORTDECL_STDDATAACCESS TagToken
{
public:
	TagToken(String Tag): fTag(Tag) {};
	~TagToken() {};

	bool IsValid() {
		return (fTag.Length() != 0);
	}
	bool IsEndTag() {
		if (!IsValid()) {
			return false;
		}
		return fTag[0L] == '/';
	};
	String GetTag() {
		if (IsEndTag())	{
			return fTag.SubString(1);
		}
		return fTag;
	};
	bool MatchesTag(String OtherTag) {
		OtherTag.ToUpper();
		String thisTag = GetTag();
		thisTag.ToUpper();
		return (thisTag == OtherTag);
	};

private:
	String fTag;
};

class EXPORTDECL_STDDATAACCESS XMLTagParser
{
public:
	XMLTagParser() {};
	~XMLTagParser() {};

	Anything DoParse(std::istream &Is);

private:
	void ProcessElement(std::istream &Is, TagToken &Tag, Anything Result);
	Anything ProcessContent(std::istream &Is, TagToken &Tag);
	TagToken ReadNextTag(std::istream &Is, String &LeadingText);
	bool ParseTag(std::istream &Is, String &Tag);
	bool ReadToExpectedChar(char ExpectedChar, std::istream &Is, String &Content);

};

Anything XMLTagParser::DoParse(std::istream &Is)
{
	StartTrace(XMLTagParser.DoParse);

	String garbageTilTheFirstTag;
	TagToken tagToken = ReadNextTag(Is, garbageTilTheFirstTag);

	MetaThing result;
	if (tagToken.IsValid() && !tagToken.IsEndTag()) {
		ProcessElement(Is, tagToken, result);
		TraceAny(result, "DoParse Result");
	}
	return result;
}

void XMLTagParser::ProcessElement(std::istream &Is, TagToken &Tag, Anything Result)
{
	StartTrace(XMLTagParser.ProcessElement);

	Anything content = ProcessContent(Is, Tag);
	String tag = Tag.GetTag();
	if ( Result.IsDefined(tag)) {
		if (Result[tag].SlotName(0)) {
			// pack first item
			Anything firstItem = Result[tag];
			Anything dummy;
			Result[tag] = dummy;
			Result[tag].Append(firstItem);
		}
		Result[tag].Append(content);
	} else {
		Result[tag] = content;
	}
	TraceAny(Result, "Result");
}

Anything XMLTagParser::ProcessContent(std::istream &Is, TagToken &Tag)
{
	StartTrace(XMLTagParser.ProcessContent);

	Anything result;
	TagToken nextToken("");
	do {
		String content;
		nextToken = ReadNextTag(Is, content);

		if (!nextToken.IsValid()) {
			return result;
		}

		if (nextToken.IsEndTag()) {
			if (content.Length() && result.IsNull()) {
				result = content;
			}
		} else {
			if (result.IsNull()) {
				MetaThing anyArray;
				result = anyArray;
			}
			ProcessElement(Is, nextToken, result);
			TraceAny(result, "Nestedresult");
		}
	} while (!nextToken.IsEndTag());

	TraceAny(result, "Result");
	return result;

}

TagToken XMLTagParser::ReadNextTag(std::istream &Is, String &LeadingText)
{
	StartTrace(XMLTagParser.ReadNextTag);

	if (!ReadToExpectedChar('<', Is, LeadingText)) {
		return TagToken("");
	}
	String tag;
	ParseTag(Is, tag);
	Trace("Parsed Tag:" << tag << "||");
	return TagToken(tag);
}

bool XMLTagParser::ParseTag(std::istream &Is, String &Tag)
{
	StartTrace(XMLTagParser.ParseTag);
	String tagContent;
	if (ReadToExpectedChar('>', Is, tagContent)) {
		Trace("Tag content:" << tagContent << "||");
		StringTokenizer2 tok(tagContent);
		return (tok.NextToken(Tag));
	}
	return false;
}

bool XMLTagParser::ReadToExpectedChar(char ExpectedChar, std::istream &Is, String &Content)
{
	StartTrace(XMLTagParser.ReadToExpectedChar);
	char c;

	while ( Is.get(c).good() ) {
		if (c == ExpectedChar) {
			return true;
		}
		Content.Append(c);
	}
	// Comment here we could throw an UnexpectedEndOfFile Exception for easier errorhandling
	return false;
}

//--- XMLBodyMapper ---------------------------
RegisterResultMapper(XMLBodyMapper);

// --- Put API
bool XMLBodyMapper::DoFinalPutStream(const char *key, std::istream &is, Context &ctx)
{
	StartTrace(XMLBodyMapper.DoPutStream);

	XMLTagParser parser;
	Anything result = parser.DoParse(is);
	return DoFinalPutAny(key, result, ctx);
}

