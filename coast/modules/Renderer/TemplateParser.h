/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TemplateParser_H
#define _TemplateParser_H

#include "config_renderer.h"
#include "Anything.h"

//---- TemplateParser ----------------------------------------------------------
//! Used to parse HTML-Templates during TemplatesCache initialization
/*!
\par Configuration
\code
{
	/Tags {				Anything	optional, tag specific settings
		/AttrNameCase	String		optional, [lower|upper|preserve], default lower, specify if attribute name should be case adjusted
		...
		/<tagname> {				optional, override above 'global' settings with tag specific setting
			/<setting>	String		optional, use same setting-names as above, but only for specific tagname
			...
		}
		...
	}
}
\endcode
*/
class EXPORTDECL_RENDERER TemplateParser
{
public:
	TemplateParser() {}
	virtual ~TemplateParser() {}
	Anything Parse(istream &reader, const char *filename = "NO_FILE", long startline = 1L, Allocator *a = Storage::Current(), const ROAnything roaParserConfig = ROAnything());

protected:
	virtual void DoParse();
	Anything OldStyleComment();
	Anything Macro();

	bool ParseTag(String &tagName, Anything &attributes);
	Anything ParseValue();
	bool ParseAttribute(const String &tagName, String &name, Anything &value);
	virtual Anything ProcessTag(const String &tagName, Anything &tagAttributes);
	virtual Anything ProcessSpecialTag(String &tagName, Anything &tagAttributes, bool mustrender);
	virtual Anything ProcessFormTag(const String &tagName, Anything &tagAttributes, const String &body, long startline);
	virtual Anything ProcessScriptTag(const String &tagName, Anything &tagAttributes, const String &body, long startline);
	Anything RenderTagAsLiteral(String &tagName, Anything &tagAttributes);
	virtual bool IsSpecialTag(String &tagName, Anything &tagAttributes);
	void ParseAnything(int endChar);
	bool IsEmptyOrWd();
	String ReadHTMLAsString(int &endChar);
	String ParseAsStringUpToEndTag(String &tagName);
	void Error(const char *msg);

	Anything ProcessArgs(const String &renderer, const String &args);
	virtual void Store(String &htmlBlock);
	void StoreInto(Anything &cache, String &htmlBlock);
	virtual void Store(const Anything &args);
	void CompactHTMLBlocks(Anything &cache);

	String ParseName();
	String ParseUpToWhitespaceOrMacroEnd();
	void SkipWhitespace();
	bool IsValidNameChar(int c);

	int Get();
	int Peek();
	bool IsGood();
	void PutBack(char c);

	Anything fCache;
	istream *fReader;
	String	fFileName;
	long	fLine;
	ROAnything froaConfig;
};

//---- FormTemplateParser ----------------------------------------------------------
//! Used to parse Form-specific HTML-Template content
class EXPORTDECL_RENDERER FormTemplateParser: public TemplateParser
{
protected:
	bool IsSpecialTag(String &tagName, Anything &tagAttributes);
	Anything ProcessSpecialTag(String &tagName, Anything &tagAttributes, bool mustrender);
};

//---- ScriptTemplateParser ----------------------------------------------------------
//! Used to parse script-specific HTML-Template content
class EXPORTDECL_RENDERER ScriptTemplateParser: public TemplateParser
{
protected:
	void DoParse();
};

#endif
