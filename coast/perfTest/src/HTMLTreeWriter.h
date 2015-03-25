/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLTreeWriter_H
#define _HTMLTreeWriter_H

#include "HTMLParser.h"

class String;
class Anything;

class HTMLTreeWriter : public AAT_HTMLWriter
{
public:
	HTMLTreeWriter();
	virtual ~HTMLTreeWriter() { }

	virtual void Tag(int type, const char *tag);
	virtual void Argument(const String &key, const String &value);
	virtual void Comment(const String &comment);
	virtual void Put(char c);
	virtual void Put(Unicode c);

	Anything PublishResult();

protected:
	void addText ();
	void addAttrs (Anything &posToAdd);
	Anything GetParseTree();
	void NewTag(String &tagname);
	void EndTag(String &tagname);
	String StripWhiteSpace(String &text);
	void GetThisLevelTagName(String &thisLevelTagname);

private:
	Anything fStructure;
	Anything fStructPos;
	Anything fAttrStore;
	Anything fStack;
	String fContentText;
};

#endif		//not defined _HTMLTreeWriter_H
