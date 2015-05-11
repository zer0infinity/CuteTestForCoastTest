/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringTestExtreme.h"
#include "SystemFile.h"

using namespace coast;

StringTestExtreme::StringTestExtreme() : extremelyLongString(""), trueString("") {
	char s;
	std::iostream *is = system::OpenStream("longString", "txt", std::ios::in);
	while (is && !is->eof()) {
		if (is->read(&s, 1)) {
			extremelyLongString.Append(s);
			trueString.append(1, s);
		}
	}
}

void StringTestExtreme::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(StringTestExtreme, extremeLength));
}

void StringTestExtreme::extremeLength() {
	// Init a string without parameters
	ASSERT(extremelyLongString.Length()>0);
	ASSERT_EQUAL(trueString.length(), extremelyLongString.Length());
}
