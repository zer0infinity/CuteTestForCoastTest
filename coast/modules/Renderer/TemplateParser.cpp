/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TemplateParser.h"
#include "Renderer.h"
#include "Page.h"
#include "Role.h"
#include "Registry.h"
#include "Session.h" /* for field prefixes, may be should be refactored, i.e. to Renderer */
#include "StringStream.h"
#include <cstring>

Anything TemplateParser::Parse(std::istream &reader, const char *filename, long startline, Allocator *a, const ROAnything roaParserConfig)
{
	StartTrace(TemplateParser.Parse);
	fReader = &reader;
	fFileName = filename;
	fLine = startline;
	fCache = Anything();
	fCache.SetAllocator(a);
	froaConfig = roaParserConfig;
	TraceAny(froaConfig, "ParserConfig");
	DoParse();
	CompactHTMLBlocks(fCache);
	return fCache;
}

void TemplateParser::DoParse()
{
	StartTrace(TemplateParser.DoParse);
	int c, lookahead;
	String reply;
	Anything tag;

	while ((c = Get()) != 0 && c != EOF) {
		Trace("char [" << (c ? (char)c : ' ') << "]");
		switch (c) {
			case '<':
				Store(reply);
				lookahead = Peek();
				switch (lookahead) {
					case '!': {
						int cE = Get(), cN = Peek();
						Trace("currently at <" << (char)cE << (char)cN);
						PutBack(cE);
						// do not parse DOCTYPE tag as comment!
						if ( cN == '-' ) {
							Store(reply);
							Store(OldStyleComment());
						} else {
							reply << (char)c;
						}
						break;
					}
					case '/': {
						Get();
						String tagname = ParseName();
						c = Peek();
						if ('>' == c) {
							Get();
							reply.Append("</").Append(tagname).Append(char(c));
						} else {
							// a potential syntax error...
							reply.Append("</").Append(tagname);
							String msg("Unexpected character <x");
							msg.AppendAsHex((unsigned char)c).Append('>');
							Error(msg);
						}
						break;
					}
					case '?':
					case '%' : {
						int cT = Get();
						if (IsEmptyOrWd()) {
							ParseAnything(cT);
							break;
						} // else fall through, it was a unknown tag...
						PutBack(cT);
					}
					default:
						if (IsValidNameChar(lookahead)) {
							String tagname;
							Anything attributes;
							bool mustrender = ParseTag(tagname, attributes);
							if (IsSpecialTag(tagname, attributes)) {
								Store (ProcessSpecialTag(tagname, attributes, mustrender));
							} else {
								if (mustrender) {
									Store (ProcessTag(tagname, attributes));
								} else {
									Store(RenderTagAsLiteral(tagname, attributes));
								}
							}
						} else {
							// it cannot be a tag, so just append the '<'
							reply << (char)c;
						}
				}
				break;
			case '[':
				lookahead = Peek();
				if ( lookahead == '[' ) {
					Store(reply);
					Store(Macro());
					break;
				}
			default:
				reply << ((char)c);
		}
	}
	Store(reply);
}

Anything TemplateParser::OldStyleComment()
{
	StartTrace(TemplateParser::OldStyleComment);
	int c;
	// keep comments for test cases relying on them
	String comment("<");
#define GSR ((c = Get()),comment.Append(char(c)))

	GSR;
	if (c == '!') {
		GSR;
		if (c == '-') {
			GSR;
			if (c == '-') {
				// now we are inside a comment

				// skip whitespace
				for (;;) {
					GSR;
					if (!isspace( (unsigned char) c)) {
						break;
					}
				}

				String macro;
				if (c == '#') {
					for (;;) {
						GSR;
						if (!isalnum( (unsigned char) c)) {
							break;
						}
						macro.Append((char)c);
					}
				}

				if ( macro == "wd" ) {	// our format ?
					String key, args;

					for (;;) {
						GSR;
						if (!isalnum( (unsigned char) c)) {
							break;
						}
						key.Append((char)c);
					}

					// skip whitespace
					while (isspace( (unsigned char) c)) {
						GSR;
					}

					// read arguments into buffer
					for (;;) {
						if (c == EOF || c == 0) {
							break;
						}
						args.Append((char)c);
						if (c == '-') {
							GSR;
							args.Append((char)c);
							if (c == '-') {
								GSR;
								args.Append((char)c);
								while ( c == '-' ) {
									GSR;
									args.Append(char(c));
								}
								if (c == '>') {
									// end of comment
									break;
								}
							}
						}
						GSR;
					}
					Error("Old style macro comment used");
					return ProcessArgs(key, args);
				} else {
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
				}
			}
		}
	}
	return Anything(comment);
}

Anything TemplateParser::ProcessArgs(const String &renderer, const String &args)
{
	StartTrace1(TemplateParser.ProcessArgs, "renderer: <" << renderer << ">");
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
	// PS make it safe for unknown renderers!
	// might be too strict, if we intend to use [[#wd foo bar]] to create an anything within a tag further interpreted
	// but, in the moment keep it as simple as possible
	Renderer *r = Renderer::FindRenderer(renderer);
	if (r) { // all is OK
		result[renderer] = aargs;
	} else {
		if (renderer.Length() == 0) { // allow for anonymous anythings to be rendered later on by Renderer
			result.Append(aargs);
		} else {
			// produce an error message analogous to Renderer::Render
			String errmsg("**** TemplateParser: Renderer ");
			errmsg << renderer << " not found ****";
			result = errmsg;
			Error(errmsg);
		}
	}
	TraceAny(aargs, "aargs");
	return result;
}

Anything TemplateParser::Macro( )
{
	StartTrace(TemplateParser.Macro);
	String reply("[");
	int c;
	c = Get();
	reply << char(c);
	if (c == '[') {
		c = Get();
		reply << char(c);
		if (c == '#') {
			c = ' ';
			String interpreter = ParseName();
			interpreter.ToLower();
			if (interpreter == "wd") {
				String key, args;

				SkipWhitespace();
				if ('/' == Peek()) {
					Get();    // convenience, allow for / like in all other places
				}
				key = ParseUpToWhitespaceOrMacroEnd(); // ParseName might be too stringent
				SkipWhitespace();
				// read arguments into buffer, look unconditionally for ]]
				while (!IsGood() && (c = Get()) != 0 && c != EOF) {
					if (c == ']' && ']' == Peek()) {
						c = Get();
						break;
					} else {
						args.Append((char)c);
					}
				}
				return ProcessArgs( key, args);
			} else {
				String msg("Unknown macro interpreter <");
				msg.Append(interpreter).Append("> expected [[#wd ");
				Error(msg);
				reply  << interpreter ;
			}
		}
	}
	return reply;

}

void TemplateParser::Store(String &literal)
{
	StoreInto(fCache, literal);
}

void TemplateParser::StoreInto(Anything &cache, String &literal)
{
	StartTrace1(TemplateParser.StoreInto, "literal [" << literal << "]");
	if (literal.Length() > 0) {
		cache.Append(Anything(literal));
		literal = "";
	}
}

void TemplateParser::Store(const Anything &rendererspec)
{
	StartTrace(TemplateParser.StoreAny);
	if (!rendererspec.IsNull()) {
		fCache.Append(rendererspec);
	}
}

void TemplateParser::CompactHTMLBlocks(Anything &cache)
{
	StartTrace(TemplateParser.CompactHTMLBlocks);
	TraceAny(cache, "Cache:");
	Anything a(cache.GetAllocator());
	Anything compactedCache(cache.GetAllocator());
	String htmlBlock;
	for (long i = 0, sz = cache.GetSize(); i < sz; ++i) {
		a = cache[i];
		if (a.GetType() == AnyCharPtrType) {
			htmlBlock.Append(a.AsCharPtr(""));
		} else {
			// otherwise it is a renderer spec in an AnyArray
			StoreInto( compactedCache, htmlBlock );
			htmlBlock = "";
			compactedCache.Append(a);
		}
	}
	if ( htmlBlock.Length() > 0) {
		StoreInto( compactedCache, htmlBlock );
	}

	TraceAny(compactedCache, "Compacted:");
	cache = compactedCache;
}

void TemplateParser::SkipWhitespace()
{
	StartTrace(TemplateParser.SkipWhitespace);
	int c;
	while (!IsGood() && (c = Peek()) != 0 && c != EOF) {
		if (!isspace(c)) {
			return;
		}
		c = Get();
	}
}

bool TemplateParser::IsValidNameChar(int c)
{
	bool bIsValid = (isalpha(c) || '-' == c || '_' == c || '.' == c || ':' == c);
	StatTrace(TemplateParser.IsValidNameChar, "is " << (bIsValid ? "" : "not ") << "valid", coast::storage::Current());
	return bIsValid;
}

String TemplateParser::ParseName()
{
	StartTrace(TemplateParser.ParseName);
	String theName;
	int c; // Unicode?
	while (!IsGood() && (c = Peek()) != EOF && c != 0) {
		if ( IsValidNameChar(c) || (isdigit(c) && theName.Length() > 0) ) {
			theName.Append((char)Get());
		} else {
			break;
		}
	}
	return theName;
}
String TemplateParser::ParseUpToWhitespaceOrMacroEnd()
{
	StartTrace(TemplateParser.ParseUpToWhitespaceOrMacroEnd);
	String theName;
	int c;
	while (!IsGood() && (c = Peek()) != EOF && c != 0 && !isspace(c)) {
		if (']' == c) {
			c = Get();
			if (']' == Peek()) {
				PutBack(c);
				Error("unexpected macro end");
				return theName;
			}
		}
		theName.Append((char)Get());
	}
	return theName;
}

int TemplateParser::Get()
{
	int c = 0;
	if (fReader && fReader->good()) {
		c = fReader->get();
		if ('\n' == c) {
			++fLine;
		}
	}
	StatTrace(TemplateParser.Get, "char [" << (c ? (char)c : ' ') << "]", coast::storage::Current());
	return c;
}

int TemplateParser::Peek()
{
	int c = 0;
	if (fReader && fReader->good()) {
		c = fReader->peek();
	}
	StatTrace(TemplateParser.Peek, "char [" << (c ? (char)c : ' ') << "]", coast::storage::Current());
	return c;
}

void TemplateParser::PutBack(char c)
{
	StatTrace(TemplateParser.PutBack, "char [" << (char)c << "]", coast::storage::Current());
	if (fReader) {
		fReader->putback(c);
	}
	if ('\n' == c) {
		--fLine;
	}
}

bool TemplateParser::IsGood()
{
	return !(fReader && fReader->good());
}

bool TemplateParser::ParseTag(String &tagName, Anything &tagAttributes)
{
	StartTrace(TemplateParser.ParseTag);
	bool needsrendering = false;
	tagName = ParseName();
	Trace("tag = <" << tagName << ">");
	while (!IsGood()) {
		SkipWhitespace();
		int c = Peek();
		switch (c) {
			case '[' : {
				Get();
				Anything a = Macro();
				needsrendering = true;
				SkipWhitespace();
				if ('=' == Peek()) {
					Get();
					a.Append("=");
					a.Append(ParseValue());
				}
				tagAttributes.Append(a);
			}
			break;
			case '>': // done with tag
				c = Get();
				return needsrendering;
			case '/': // an empty tag? i.e. <br />
				c = Get();
				if ('>' == Peek()) {
					c = Get();
					tagAttributes.Append("/");//tagAttributes[WASEMPTYTAG] = 1L;
					// should remember /> as closing...
					return needsrendering;
				}
				// an error occured, ignore '/' silently
				PutBack(c);
			default: {
				String name;
				Anything value;
				if ( ParseAttribute(tagName, name, value) ) {
					// found "attr=value"
					if (AnyArrayType == value.GetType()) {
						needsrendering = true;
					}
					tagAttributes[name] = value;
				} else {
					// non-value attribute
					if (name.Length() > 0) {
						tagAttributes.Append(name);
					} else {
						String msg("Unexpected character <");
						c = Get();
						msg.AppendAsHex((unsigned char)c).Append("> near ").Append(tagName);
						Error(msg);
						tagAttributes.Append(String(char(c)));
					}
				}
			}
		}
	}
	return needsrendering;
}

bool TemplateParser::ParseAttribute(const String &tagName, String &name, Anything &value)
{
	StartTrace(TemplateParser.ParseAttribute);
	ROAnything roaTagsConfig;
	name = ParseName();
	Trace("non  adjusted name [" << name << "]");
	if ( froaConfig.LookupPath(roaTagsConfig, "Tags") ) {
		TraceAny(roaTagsConfig, "Tags configuration");
		// check if tag specific settings are defined, otherwise use global settings
		// LookupPath preserves roaTagsConfig if not found
		if ( roaTagsConfig.LookupPath(roaTagsConfig, tagName) ) {
			TraceAny(roaTagsConfig, String(tagName) << " configuration");
		}
	}
	// found specific tag settings
	String strCase = roaTagsConfig["AttrNameCase"].AsString("lower").ToLower();
	if ( !strCase.Length() || strCase.IsEqual("lower") ) {
		// default, XHTML conformance all attribute names are lower case
		name.ToLower();
	} else if ( strCase.IsEqual("upper") ) {
		name.ToUpper();
	}
	// else preserve case
	Trace("case adjusted name [" << name << "]");
	value = "";
	SkipWhitespace();
	int c = Peek();
	if (c != '=') {
		return false;
	}
	c = Get();
	value = ParseValue();
	return true;
	// otherwise it is a syntax error, we just ignore silently for now.
}

Anything TemplateParser::ParseValue()
{
	StartTrace(TemplateParser.ParseValue);
	String value;
	int c = Peek();
	if ('[' == c) { // a wd macro
		Get();
		return Macro();
	} else if ('\'' == c || '\"' == c) { // a quoted value
		int	quote = Get();
		Anything result;
		while (!IsGood() && (c = Get()) != quote) {
			if ('[' == c && '[' == Peek()) {
				StoreInto(result, value);
				Anything subvalue = Macro();
				result.Append(subvalue);
			} else {
				value.Append((char)c);
			}
			// should we allow for macros within quotes to please HTML editors?
		}
		if (!result.IsNull()) { // we had a macro
			StoreInto(result, value);
			return result;
		}
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

//--- processing of tags
Anything TemplateParser::ProcessTag(const String &tagName, Anything &tagAttributes)
{
	StartTrace(TemplateParser.ProcessTag);
	// this is a tag that needs to be rendered
	Anything result;
	String start("<");
	start.Append(tagName);
	result.Append(start);
	result["OptionsPrinter"] = tagAttributes;
	result.Append(">");
	return result;
}
Anything TemplateParser::ProcessSpecialTag(String &tagName, Anything &tagAttributes, bool mustrender)
{
	StartTrace(TemplateParser.ProcessSpecialTag);
	// for now, only <form> is special
	long startline = fLine;
	tagName.ToLower();
	if (tagName == "script") {
		return ProcessScriptTag(tagName, tagAttributes, ParseAsStringUpToEndTag(tagName), startline);
	} else {
		return ProcessFormTag(tagName, tagAttributes, ParseAsStringUpToEndTag(tagName), startline);
	}
}
Anything TemplateParser::RenderTagAsLiteral(String &tagName, Anything &tagAttributes)
{
	StartTrace(TemplateParser.RenderTagAsLiteral);
	// otherwise, we can place it literally
	String tag("<");
	tag.Append(tagName);
	for (long i = 0, sz = tagAttributes.GetSize(); i < sz; ++i) {
		tag.Append(' ');
		if (tagAttributes.SlotName(i)) {
			tag.Append(tagAttributes.SlotName(i));
			if (AnyCharPtrType == tagAttributes[i].GetType()) {
				tag.Append("=\"");
				tag.Append(tagAttributes[i].AsCharPtr(""));
				tag.Append('"');
			}
		} else {
			tag.Append(tagAttributes[i].AsCharPtr("")); // just take the value
		}
	}
	tag.Append('>');
	Trace("rendered tag [" << tag << "]");
	return tag;
}

Anything TemplateParser::ProcessFormTag(const String &tagName, Anything &tagAttributes, const String &body, long startline)
{
	StartTrace(TemplateParser.ProcessFormTag);
	Anything result = Anything(Anything::ArrayMarker());
	Anything &formrenderer = result["FormRenderer"];
	TraceAny(tagAttributes, "given attributes");
#define MVATTR(name,def) \
	{ String lowername(#name); lowername.ToLower();\
		if (tagAttributes.IsDefined(lowername))\
		{\
			formrenderer[#name]=tagAttributes[lowername];\
			tagAttributes.Remove(lowername);\
		}\
		else if (strlen(def)>0)\
		{\
			formrenderer[#name]=def;\
			tagAttributes.Remove(lowername);\
		}\
	}
	MVATTR(Method, "GET");
	MVATTR(Action, "");
	MVATTR(EncType, "");
	MVATTR(Target, "");
	MVATTR(BaseAddr, "");
	if (!tagAttributes.IsNull()) {
		formrenderer["Options"] = tagAttributes;
	}
	FormTemplateParser fp;
	IStringStream is(body);
	formrenderer["Layout"] = fp.Parse(is, fFileName, startline, result.GetAllocator()); // parse separately, use /Layout, if preprocessed...
	return result;
}
Anything TemplateParser::ProcessScriptTag(const String &tagName, Anything &tagAttributes, const String &body, long startline)
{
	StartTrace(TemplateParser.ProcessScriptTag);
	ScriptTemplateParser sp;
	IStringStream is(body);
	Anything result = ProcessTag(tagName, tagAttributes);
	result.Append(sp.Parse(is, fFileName, startline)); // parse separately
	String endtag("</");
	endtag.Append(tagName).Append('>');
	result.Append(endtag);
	return result;
}

bool TemplateParser::IsSpecialTag(String &tagName, Anything &tagAttributes)
{
	StartTrace(TemplateParser.IsSpecialTag);
	if ( String::CaselessCompare(tagName, "script") == 0 ) {
		return true;
	} else {
		if ( (String::CaselessCompare(tagName, "form") == 0) && tagAttributes.IsDefined("action") && tagAttributes["action"].AsString().Length() ) {
			// several locations to check for a valid 'action'
			// 1. Real registered Action
			// 2. Role-Page-Token-Map entry in Role.any
			// 3. Page specific slot defined -> transitionToken
			// case 1:
			const char *pAction = tagAttributes["action"].AsCharPtr(0);
			Trace("given action [" << pAction << "]");
			if ( Action::FindAction(pAction) ) {
				Trace("found valid real Action");
				return true;
			}
			Trace("Filename of template is [" << fFileName << "]");
			// returns -1 if no match so the following if should work
			long lLastSlashIdx = fFileName.StrRChr('/') + 1L;
			if ( lLastSlashIdx > 0L ) {
				String strPageName = fFileName.SubString(lLastSlashIdx);
				// case 2:
				// because we do not know the correct Role at the moment of parsing this file (startup)
				// we need to iterate over all registered roles to find a possible match although this
				// could lead to misinterpretations
				Registry *reg = MetaRegistry::instance().GetRegistry("Role");
				RegistryIterator ri(reg);
				while ( ri.HasMore() ) {
					String roleName("null");
					Role *pRole = (Role *)ri.Next(roleName);
					Trace("role found <" << roleName << ">");
					if (pRole) {
						String transition(pAction);
						Trace("checking for entry in [" << roleName << "]");
						ROAnything roaEntry;
						if ( pRole->GetNextActionConfig( roaEntry, transition, strPageName) || pRole->IsStayOnSamePageToken(transition) ) {
							Trace("matching action-token found");
							return true;
						}
					}
				}

				// case 3:
				Trace("Trying to find page with name [" << strPageName << "]");
				Page *pPage = Page::FindPage(strPageName);
				if ( pPage ) {
					Trace("Page found, now checking for slot which could match");
					ROAnything roaConfig;
					if ( pPage->Lookup(pAction, roaConfig) ) {
						TraceAny(roaConfig, "Config of actionscript on page");
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool TemplateParser::IsEmptyOrWd()
{
	int c = Peek();
	StatTrace(TemplateParser.IsEmptyOrWd, "char [" << (char)c << "]", coast::storage::Current());
	if (!isspace(c)) {
		if (tolower(c) == 'w') {
			Get();
			if (tolower(Peek()) == 'd') {
				Get();
				if (isspace(Peek())) {
					return true;
				}
				PutBack('d');
			}
			PutBack('w');
		}
		return false;
	}
	return true;
}

void TemplateParser::ParseAnything(int endChar)
{
	StartTrace(TemplateParser.ParseAnything);
	int c;
	long bl = 0;
	String collectany("{");
	while (!IsGood() && (c = Get()) != 0) {
		if (endChar == c && '>' == Peek()) {
			Get();
			if (0 == bl) {
				break;
			} else if (bl > 0) {
				String res = ReadHTMLAsString(endChar);
				OStringStream os;// make sub-expr a string
				res.IntPrintOn(os, '"'); // take care of anything conformant masking
				collectany.Append(os.str());
				continue;
			} else {
				Error("Too many closing }");
				break;
			}
		}
		switch (c) {
			case '{':
				++bl;
				break;
			case '}':
				--bl;
				break;
			case '"': // skip string without considering braces
				collectany.Append(char(c));
				while (!IsGood() && (c = Get()) != 0 && '"' != c) {
					collectany.Append(char(c));
					if ('\\' == c) {
						collectany.Append(char(Get()));
					}
				}
				break;
			case '#': // skip comment to eol
				collectany.Append(char(c));
				while (!IsGood() && (c = Get()) != 0 && '\n' != c) {
					collectany.Append(char(c));
				}
				break;
		}
		collectany.Append(char(c));
	}
	collectany.Append('}');
	IStringStream is(collectany);
	Anything result;
	result.Import(is);
	Store(result);
}

String TemplateParser::ReadHTMLAsString(int &endChar)
{
	StartTrace(TemplateParser.ReadHTMLAsString);
	String result;
	int c;
	while (!IsGood() && (c = Get()) != 0 && c != EOF) {
		if ('<' == c) {
			int lookahead = Peek();
			if ('?' == lookahead || '%' == lookahead) {
				c = Get();
				if (IsEmptyOrWd()) {
					endChar = c;
					return result;
				}
				result.Append('<');
				result.Append(char(c));
				continue;
			}
		}
		result.Append(char(c));
	}
	Error("unexpected EOF");
	return result;
}
String TemplateParser::ParseAsStringUpToEndTag(String &tagName)
{
	StartTrace(TemplateParser.ParseAsStringUpToEndTag);
	String result;
	int c;
	while (!IsGood() && (c = Get()) != 0) {
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

void TemplateParser::Error(const char *msg)
{
	StartTrace(TemplateParser.Error);
	String m(" ");
	m.Append(fFileName).Append(".html:").Append(fLine).Append(" ").Append(msg);
	SystemLog::Warning(m);
	Trace(m);
	m << "\n";
	SystemLog::WriteToStderr(m);
}

// --- FormTemplateParser ---
bool FormTemplateParser::IsSpecialTag(String &tagName, Anything &tagAttributes)
{
	StartTrace(FormTemplateParser.IsSpecialTag);
	String tag = tagName;
	tag.ToLower();
	return tag == "input"
		   || tag == "textarea"
		   || tag == "select"
		   || tag == "button";
}
Anything FormTemplateParser::ProcessSpecialTag(String &tagName, Anything &tagAttributes, bool mustrender)
{
	StartTrace(FormTemplateParser.ProcessSpecialTag);
	String tag = tagName;
	tag.ToLower();
	if ((tag == "input" || tag == "textarea" || tag == "select" ) &&
		tagAttributes.IsDefined("name") && 0 != tagAttributes["name"].AsString().Contains(Session::FR_FIELDPREFIX)) {
		String theName(Session::FR_FIELDPREFIX);
		if (tagAttributes.IsDefined("type")) {
			if (tagAttributes["type"].AsString().ToLower() == "submit") {
				theName = Session::FR_BUTTONPREFIX;
			} else if (tagAttributes["type"].AsString().ToLower() == "image") {
				theName = Session::FR_IMAGEPREFIX;    // check why and where that is needed? -> .x.y coordinates
			}
		}
		theName.Append(tagAttributes["name"].AsString());
		tagAttributes["name"] = theName;
	} else if (tag == "button" && tagAttributes.IsDefined("name") && 0 != tagAttributes["name"].AsString().Contains(Session::FR_BUTTONPREFIX)) {
		String theName(Session::FR_BUTTONPREFIX);
		theName.Append(tagAttributes["name"].AsString());
		tagAttributes["name"] = theName;
	}
	return mustrender ? ProcessTag(tag, tagAttributes) : RenderTagAsLiteral(tag, tagAttributes);
}

void ScriptTemplateParser::DoParse()
{
	StartTrace(ScriptTemplateParser.DoParse);
	int c, lookahead;
	String reply;
	Anything tag;

	while ((c = Get()) != 0 && c != EOF) {
		switch (c) {
			case '<':
				Store(reply);
				lookahead = Peek();
				switch (lookahead) {
					case '?':
					case '%' : {
						int cT = Get();
						if (IsEmptyOrWd()) {
							ParseAnything(cT);
							break;
						} // else fall through, it was a unknown tag...
						PutBack(cT);
					}
					default:
						// it cannot be a tag, so just append the '<'
						reply << (char)c;
				}
				break;
			case '[':
				lookahead = Peek();
				if ( lookahead == '[' ) {
					Store(reply);
					Store(Macro());
					break;
				}
			default:
				reply << ((char)c);
		}
	}
	Store(reply);
}
