/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ParserTest_h_
#define _ParserTest_h_

#include "TestCase.h"
#include "Anything.h"
//---- ParserTest -----------------------------------------------------------
//!testcases for AnythingParser
class ParserTest : public TestCase
{
protected:
	Anything	emptyAny, anyTemp0, anyTemp1,
				anyTemp2, anyTemp3, anyTemp4;

	void 		slashSlotnames();

	Anything	storeAndReload( Anything );
	void		writeResult( String *input , long nrOfElt, char *path, char *ext );
	void		checkImportExport( Anything any, String fileName );
	void		scanAnything( Anything );
	Anything	anyOutput;
	long		lineCounter;
	void 		assertParsedAsDouble(const char *in, double val, int id);
	void 		assertParsedAsLong(const char *in, long val, int id);
	void 		assertParsedAsString(const char *in, int id);

public:
	ParserTest (TString name); // : TestCase (name) {}
	virtual void			setUp ();
	static Test				*worksuite ();
	static Test				*suite ();

	void	anythingTests();
	void		parseSimpleTypeLong();
	void 		IntParseSimpleTypeLong(const String &inp, long exp);
	void		parseSimpleTypeDouble();
	void		parseSimpleTypeNull();
	void		parseSimpleTypeCharPtr();
	void		parseMixedAnything ();
	void		parseNumber();
	void		parseOctal();
	void		parseBinary();
	void		importEmptyStream();
	void		readWriteAnything();
	void		parseTestFiles();
	void	testSemanticAnything();
	void		testSemantic0();
	void		testSemantic1();
	void		testSemantic2();
	void		testSemantic3();
	void		testSemantic4();
	void		testSemantic5();
	void		testSemantic6();
	void		testSemantic7();
	void		testSemantic8();
	void		testSemantic9();
	void		testSemantic10();
	void		testSemantic11();
	void		testSemantic12();
	void		testSemantic13();
	void		testSemantic14();
	void		testSemantic15();
	void		testSemantic16();
	void		testSemantic17();
	void		testSemantic18();
	void		testSemantic19();
	void		testSemantic20();
	void		testSemantic21();
	void		testSemantic22();
	void		testSemantic23();
	void		testSemantic24();
	void		testSemantic25();
	void		testSemantic26();
	void		testSemantic27();
	void		testSemantic28Prep( Anything *any );
	void		testSemantic28();
	void		testSemantic29Prep( Anything *any );
	void		testSemantic29();
	void		testSemantic30Prep( Anything &any );
	void		testSemantic30();
	void		testSemantic31();
	void		testSemantic32();
	void		testSemantic33();

	void        testQuotedSlotname();
};
#endif
