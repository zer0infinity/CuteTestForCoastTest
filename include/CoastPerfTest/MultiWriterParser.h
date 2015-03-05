/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MultiWriterParser_H
#define _MultiWriterParser_H

#include "HTMLParser.h"

//!Parser that drives two writers
class MultiWriterParser : public AAT_StdHTMLParser
{
	AAT_HTMLWriter &fSecondWriter;
	MultiWriterParser();
	MultiWriterParser(const MultiWriterParser &);
	MultiWriterParser &operator=(const MultiWriterParser &);
public:
	MultiWriterParser(AAT_HTMLReader &reader,
					  AAT_HTMLWriter &writer1,
					  AAT_HTMLWriter &writer2);
	void Flush() {
		IntFlush();
	};

protected:
	void IntFlush();
	void IntPut(Unicode c);
	void IntComment(const String &comment);
	void IntPushNode(Anything &);
	void IntTag(int type, const char *tag );
	void IntArgument(const String &key, const String &value);
	void IntError(long line, const String &msg);
};

#endif
