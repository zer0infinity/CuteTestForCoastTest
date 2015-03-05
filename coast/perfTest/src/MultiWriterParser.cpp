/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MultiWriterParser.h"

MultiWriterParser::MultiWriterParser(AAT_HTMLReader &reader, AAT_HTMLWriter &writer1, AAT_HTMLWriter &writer2) :
	AAT_StdHTMLParser(reader, writer1), fSecondWriter(writer2) {
}

void MultiWriterParser::IntFlush() {
	fSecondWriter.Flush();
	AAT_StdHTMLParser::IntFlush();
}

void MultiWriterParser::IntPushNode(Anything &node) {
	fSecondWriter.PushNode(node);
	AAT_StdHTMLParser::IntPushNode(node);
}

void MultiWriterParser::IntPut(Unicode c) {
	fSecondWriter.Put(c);
	AAT_StdHTMLParser::IntPut(c);
}
void MultiWriterParser::IntComment(const String &comment) {
	fSecondWriter.Comment(comment);
	AAT_StdHTMLParser::IntComment(comment);
}
void MultiWriterParser::IntTag(int type, const char *tag) {
	fSecondWriter.Tag(type, tag);
	AAT_StdHTMLParser::IntTag(type, tag);
}
void MultiWriterParser::IntArgument(const String &key, const String &value) {
	fSecondWriter.Argument(key, value);
	AAT_StdHTMLParser::IntArgument(key, value);
}
void MultiWriterParser::IntError(long, const String &msg) {
	fSecondWriter.Error(fLine, msg);
	AAT_StdHTMLParser::IntError(fLine, msg);
}
