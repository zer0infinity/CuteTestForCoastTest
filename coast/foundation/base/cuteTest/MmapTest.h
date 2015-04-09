/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MmapTest_h_
#define _MmapTest_h_

/* Purpose: Test the interface of the MmapStream
   Initial Autor: Peter
*/

#include "cute.h"

class MmapTest {
public:
	static void runAllTests(cute::suite &s);

	void LargeWrite();
	void SimpleWrite();
	void SimpleRead();
	void EmptyFileRead();
	void EmptyFilePutback();
	void FStreamEmptyFilePutback();
	void SimpleReadOnly();
	void SimpleTruncate();
	void SimplePutback();
	void SimpleROPutback();
	void SimpleAppend();
	void SimpleAtEnd();
	void SimpleSeek();
	void SimulatedLogAppend();
	void OperatorShiftLeftWithReadBuf();

	void TestMagicFlags();

	void IntSimpleWrite(int openflag = 0640);

	static const char *const fgcContent;
	static const char *const fgcFilename;
};

#endif
