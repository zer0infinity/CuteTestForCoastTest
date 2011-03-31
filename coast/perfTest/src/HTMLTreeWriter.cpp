/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTMLTreeWriter.h"
#include "Dbg.h"
#include "SystemLog.h"

//--- HTMLTreeWriter -----------------------------------------------------
HTMLTreeWriter::HTMLTreeWriter()
	: fContentText("")
{
	fStructure = Anything(Anything::ArrayMarker());
	fStructPos["Tree"] = fStructure;
	fStructPos["Tag"] = "";
}

void HTMLTreeWriter::Put(char c )
{
	fContentText.Append(c);
}

void HTMLTreeWriter::Put(Unicode c )
{
	Put( (char) c );
}

void HTMLTreeWriter::addText()
{
	StartTrace(HTMLTreeWriter.addText);
	fContentText = StripWhiteSpace(fContentText);
	if (fContentText != "") {
		Trace(fContentText);
		fStructPos["Tree"]["Content"].Append(fContentText);
		fContentText = "";
	}
}

String HTMLTreeWriter::StripWhiteSpace(String &text)
{
	StartTrace1(HTMLTreeWriter.StripWhiteSpace, "Text in: >" << text << "<" );

	String result;
	String whiteSpace = " \t\r\n";

	long textLen = text.Length();
	long i = 0;

	//skip leading whitespace
	while (i < textLen && whiteSpace.StrChr(text[i]) >= 0) {
		i++;
	}

	while (i < textLen) {
		while (i < textLen && whiteSpace.StrChr(text[i]) < 0) {
			result << text[i];
			i++;
		}
		while (i < textLen && whiteSpace.StrChr(text[i]) >= 0) {
			i++;
		}
		if (i < textLen) {
			result << " ";
		}
	}
	Trace("Text out: >" << result << "<" );
	return result;
}

void HTMLTreeWriter::addAttrs(Anything &posToAdd)
{
	StartTrace(HTMLTreeWriter.addAttrs);
	if (!fAttrStore.IsNull()) {
		TraceAny(fAttrStore, "Attributes");
		posToAdd["Attr"] = fAttrStore;
		fAttrStore = Anything();
	}
}

Anything HTMLTreeWriter::PublishResult()
{
	Anything result;
	result["Tree"] = fStructure;
	return result;
}

void HTMLTreeWriter::NewTag(String &tagname)
{
	StartTrace1(HTMLTreeWriter.NewTag, "Tag: " << tagname);
	if (tagname) {
		String thisLevelTagName;
		GetThisLevelTagName(thisLevelTagName);

		Trace("This Level>" << thisLevelTagName);

		if (thisLevelTagName == tagname) {
			// No nesting of same tags
			EndTag(tagname);
		}

		addText();
		Anything theNew;
		theNew[tagname] = Anything(Anything::ArrayMarker());
		addAttrs(theNew[tagname]);

		fStructPos["Tree"]["Content"].Append(theNew);
		Anything toStack;

		fStack.Append(fStructPos);
		fStructPos = Anything(Anything::ArrayMarker());
		fStructPos["Tree"] = theNew[tagname];
		fStructPos["Tag"] = tagname;
	}
}

void HTMLTreeWriter::GetThisLevelTagName(String &thisLevelTagname)
{
	StartTrace(HTMLTreeWriter.GetThisLevelTagName);
	thisLevelTagname.Trim(0);
	if ( fStructPos.IsDefined("Tag") ) {
		thisLevelTagname = fStructPos["Tag"].AsString("");
	}
}

void HTMLTreeWriter::EndTag(String &tagname)
{
	StartTrace1(HTMLTreeWriter.EndTag, "Tag: " << tagname);

	addText();

	String thisLevelTagName, levelOfFailure, oldLevel;
	long stackTop = fStack.GetSize() - 1;
	if (stackTop < 0) {
		return;
	}

	Anything anyStructPos = fStructPos;
	do {
		oldLevel = levelOfFailure;
		GetThisLevelTagName(thisLevelTagName);
		Trace("This Level ->" << thisLevelTagName );
		levelOfFailure = thisLevelTagName;
		if ( oldLevel.Length() ) {
			levelOfFailure << ".";
		}
		levelOfFailure << oldLevel;
		Trace("levelOfFailure [" << levelOfFailure << "]");
		fStructPos = fStack[stackTop];
		stackTop--;
	} while (stackTop >= 0 && thisLevelTagName != tagname);
	Trace("stackTop:" << stackTop);
	fStructPos = anyStructPos;
	if ( stackTop < 0 && thisLevelTagName != tagname ) {
		SYSWARNING("endtag [" << tagname << "] without begin tag at [" << levelOfFailure << "]");
	} else {
		stackTop = fStack.GetSize() - 1;
		do {
			GetThisLevelTagName(thisLevelTagName);
			Trace("This Level ->" << thisLevelTagName );
			fStructPos = fStack[stackTop];
			fStack.Remove(stackTop);
			stackTop = fStack.GetSize() - 1;
		} while (stackTop >= 0 && thisLevelTagName != tagname);
	}
}

void HTMLTreeWriter::Argument(const String &key, const String &value)
{
	StartTrace1(HTMLTreeWriter.Argument, "key: >" << key << "< Value: >" << value << "<");
	if (key != "") {
		fAttrStore[key] = value;
	}
}

Anything HTMLTreeWriter::GetParseTree()
{
	return fStructure;
}

void HTMLTreeWriter::Tag (int type, const char *tagname)
{
	if (tagname) {
		String tn(tagname);
		if (type == '/') { // End tag
			EndTag(tn);
		} else {
			NewTag(tn);
		}
	}
}

void HTMLTreeWriter::Comment(const String &comment)
{
	StartTrace1(HTMLTreeWriter.Comment, "Comment: " << comment);

	String thisLevelTagName;
	GetThisLevelTagName(thisLevelTagName);
	if (thisLevelTagName == "title") {
		// special treatment of title tag
		fContentText << "<!--" << comment << "-->";
	}
}

