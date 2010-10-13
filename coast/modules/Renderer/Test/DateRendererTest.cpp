/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DateRendererTest.h"

//--- module under test --------------------------------------------------------
#include "DateRenderer.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "SystemBase.h"

using namespace Coast;

DateRendererTest::DateRendererTest (TString tname)
	: RendererTest(tname)
{
}

DateRendererTest::~DateRendererTest()
{
}

void DateRendererTest::setUp ()
{
	RendererTest::setUp();
	fConfig["Format"] = "%C";
}

void DateRendererTest::CompareHelper(const char *format, long offset)
{
	time_t now;
	struct tm *tt, res;
	time(&now);
	now += offset * 86400;
	tt = System::LocalTime(&now, &res);
	const int maxsize(100);
	char date[maxsize];

	strftime(date, maxsize, format, tt);
	assertEqual(date, fReply.str());
}

void DateRendererTest::simpleFormat()
// test the date renderer with simple formatting strings
{
	DateRenderer dateRenderer("");

	// set up the configuration
	// render the configuration
	ROAnything roConfig = fConfig;
	dateRenderer.RenderAll(fReply, fContext, roConfig);

	CompareHelper("%C");
}

void DateRendererTest::DifferentFormats1()
// test the date renderer with simple formatting strings
{
	DateRenderer dateRenderer("");

	const char *format = "%d.%m.%y";

	// set up the configuration
	// render the configuration
	fConfig["Format"] = format;
	ROAnything roConfig = fConfig;
	dateRenderer.RenderAll(fReply, fContext, roConfig);

	CompareHelper(format);
}

void DateRendererTest::DifferentFormats2()
// test the date renderer with simple formatting strings
{
	DateRenderer dateRenderer("");

	const char *format = "%m/%d/%y";

	// set up the configuration
	// render the configuration
	fConfig["Format"] = format;
	ROAnything roConfig = fConfig;
	dateRenderer.RenderAll(fReply, fContext, roConfig);

	CompareHelper(format);
}

void DateRendererTest::DifferentFormats3()
// test the date renderer with simple formatting strings
{
	DateRenderer dateRenderer("");

	const char *format = "%y%m%d";

	// set up the configuration
	// render the configuration
	fConfig["Format"] = format;
	ROAnything roConfig = fConfig;
	dateRenderer.RenderAll(fReply, fContext, roConfig);

	CompareHelper(format);
}

void DateRendererTest::RelativeTimeFormat()
{
	DateRenderer dateRenderer("");
	const char *format = "%d.%m.%y";

	// set up the configuration
	fConfig["Format"] = format;
	fConfig["Offset"] = "1d";

	// render the configuration
	ROAnything roConfig = fConfig;
	dateRenderer.RenderAll(fReply, fContext, roConfig);

	CompareHelper(format, 1);
}

void DateRendererTest::AbsolutTimeFormat()
// test the date renderer with simple formatting strings
{
	DateRenderer dateRenderer("");
	const int maxsize = 100;
	time_t absolut = 870912000;		// seconds
	const char *format = "%d.%m.%y";

	// set up the configuration
	fConfig["Format"] = format;
	fConfig["Date"] = absolut;

	// render the configuration
	ROAnything roConfig = fConfig;
	dateRenderer.RenderAll(fReply, fContext, roConfig);

	struct tm *tt, res;
	tt = System::LocalTime(&absolut, &res);
	char date[maxsize];

	strftime(date, maxsize, format, tt);
	assertEqual(date, fReply.str());
}

void DateRendererTest::GMTTime()
// test the date renderer with simple formatting strings
{
	DateRenderer dateRenderer("");
	const int maxsize = 100;

	const char *format = "%a, %d %b %Y %T GMT";

	// set up the configuration
	// render the configuration
	fConfig["RFC2616Date"] = 1;
	ROAnything roConfig = fConfig;
	dateRenderer.RenderAll(fReply, fContext, roConfig);

	time_t now;
	time(&now);						// use current time
	struct tm *tt, res;
	tt = System::GmTime(&now, &res);
	char date[maxsize];

	strftime(date, maxsize, format, tt);
	assertEqual(date, fReply.str());
}

void DateRendererTest::emptyConfig1()
// test the date renderer with simple formatting strings
{
	DateRenderer dateRenderer("");
	const int maxsize = 100;

	// set up the configuration
	// render the configuration
	fConfig.Remove( "Format" );

	ROAnything roConfig = fConfig;
	// We try try up to 4 times to do the test within the same second.
	for (int i = 0; i <= 4; i++) {
		dateRenderer.RenderAll(fReply, fContext, roConfig);

		// assert the result
		time_t now;
		struct tm *tt, res;
		time(&now);						// use current time
		tt = System::LocalTime(&now, &res);
		char date[maxsize];

#if defined(WIN32)
		strftime(date, maxsize, "%c", tt);
#else
		strftime(date, maxsize, "%C", tt);
#endif
		if (date == fReply.str() || i != 4) {
			assertEqual(date, fReply.str());
			break;
		} else {
			if (i == 4) {
				// now the test really failed
				assertEqual(date, fReply.str());
			}
		}
	}
}

void DateRendererTest::emptyConfig2()
// test the date renderer with simple formatting strings
{
	DateRenderer dateRenderer("");
	const int maxsize = 100;

	ROAnything roConfig;
	for (int i = 0; i <= 4; i++) {
		dateRenderer.RenderAll(fReply, fContext, roConfig);

		// assert the result
		time_t now;
		struct tm *tt, res;
		time(&now);						// use current time
		tt = System::LocalTime(&now, &res);
		char date[maxsize];

#if defined(WIN32)
		strftime(date, maxsize, "%c", tt);
#else
		strftime(date, maxsize, "%C", tt);
#endif
		if (date == fReply.str() || i != 4) {
			assertEqual(date, fReply.str());
			break;
		} else {
			if (i == 4) {
				// now the test really failed
				assertEqual(date, fReply.str());
			}
		}
	}
}

Test *DateRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, DateRendererTest, simpleFormat);
	ADD_CASE(testSuite, DateRendererTest, DifferentFormats1);
	ADD_CASE(testSuite, DateRendererTest, DifferentFormats2);
	ADD_CASE(testSuite, DateRendererTest, DifferentFormats3);
	ADD_CASE(testSuite, DateRendererTest, RelativeTimeFormat);
	ADD_CASE(testSuite, DateRendererTest, AbsolutTimeFormat);
	ADD_CASE(testSuite, DateRendererTest, GMTTime);
	ADD_CASE(testSuite, DateRendererTest, emptyConfig1);
	ADD_CASE(testSuite, DateRendererTest, emptyConfig2);
	return testSuite;
}
