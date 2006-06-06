/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <time.h>
#endif

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "Context.h"
#include "Renderer.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "FormRenderer.h"

//--- interface include --------------------------------------------------------
#include "FieldRendererTest.h"

FieldRendererTest::FieldRendererTest (TString tname) : RendererTest(tname), fFieldRenderer(0) {};
FieldRendererTest::~FieldRendererTest() {};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void FieldRendererTest::setUp ()
{
	RendererTest::setUp();
	fCurrentTestMethod = "Test-Method has been forgotten";
}

Test *FieldRendererTest::suite ()
/* what: return the whole suite of tests for renderers, we could add the suites
		 of the derived classes here, but we would need to include all the headers
		 of the derived classes which is not nice.
		 Add cases for the generic Renderer here (if you find any that is);
*/
{
	TestSuite *testSuite = new TestSuite;
	return testSuite;
}
/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void FieldRendererTest::RenderConf()
{
	ROAnything roConfig = fConfig;
	fFieldRenderer->RenderAll(fReply, fContext, roConfig);
	// assert the result
//	this->PrintResult();
}

void FieldRendererTest::PrintResult()
{
	cerr << endl << (const char *)fCurrentTestMethod << " :   " << (const char *)fReply.str() << endl;
}

void FieldRendererTest::ConfigureField0()
{
//	1) Textfieldrenderer
//	2) TextAreaRendererTest
//	3) PulldownMenuRendererTest
//	4) SelectBoxRendererTest
//																1	2	3	4

	fConfig["Name"] 		= "the name of field";			//	x	x
	fConfig["Label"] 		= "the label of field";			//
	fConfig["Value"] 		= "the value of field";			//	x	x
	fConfig["Src"] 			= "the source of field";		//
	fConfig["Multiple"] 	= 1L;
	fConfig["Checked"] 		= "the checked of field";
	fConfig["Unreadable"] 	= 1;							//	x
	fConfig["Size"] 		= 50;							//	x
	fConfig["Maxlength"] 	= 40;							//	x
	fConfig["Width"]		= 30;							//		x
	fConfig["Height"]		= 20;							//		x
	fConfig["Wrap"]			= 10;							//		x

	fConfig["Options"] = "the options of field";			//	x
}

void FieldRendererTest::TestField0()
{
	// set up the configuration
	this->ConfigureField0();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureField1()
{
	fConfig["Name"] = "the name of field";
	fConfig["Label"] = "the label of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = "the source of field";
	fConfig["Multiple"] 	= 1L;
	fConfig["Checked"] 		= "the checked of field";
	fConfig["Unreadable"] 	= 1;
	fConfig["Size"] 		= 50;
	fConfig["Maxlength"] 	= 40;
	fConfig["Width"]		= 30;
	fConfig["Height"]		= 20;
	fConfig["Wrap"]			= 10;
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = "the option nr. 2 of field";
}

void FieldRendererTest::TestField1()
{
	// set up the configuration
	this->ConfigureField1();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureFieldWithRenderer()
{
	Anything optionRenderer;
	Anything nameRenderer;
	Anything srcRenderer;

	optionRenderer["StringRenderer"]["Default"] = "String rendered Option";
	nameRenderer["StringRenderer"]["Default"] = "String rendered Name";
	srcRenderer["StringRenderer"]["Default"] = "String rendered Source";

	fConfig["Name"] = nameRenderer;
	fConfig["Label"] = "the name of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = srcRenderer;
	fConfig["Multiple"] = 1L;
	fConfig["Checked"] = "the checked of field";
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = optionRenderer;
}

void FieldRendererTest::TestConfigureFieldWithRenderer()
{
	// set up the configuration
	this->ConfigureFieldWithRenderer();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureField2()
{
	fConfig["Name"] = "the name of field";
	fConfig["Label"] = "the label of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = "the source of field";
	fConfig["Multiple"] = 1L;
	fConfig["Checked"] = "the checked of field";
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"].Append("the option nr. 1 of field");
	fConfig["Options"]["2"] = "the option nr. 2 of field";
}

void FieldRendererTest::TestField2()
{
	// set up the configuration
	this->ConfigureField2();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureField3()
{
	fConfig["Name"] = "the name of field";
	fConfig["Label"] = "the label of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = "the source of field";
	fConfig["Multiple"] = 1L;
	fConfig["Checked"] = "the checked of field";
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"].Append("the option nr. 1 of field");
	fConfig["Options"]["2"] = "the option nr. 2 of field";
}

void FieldRendererTest::TestField3()
{
	// set up the configuration
	this->ConfigureField3();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureFieldOptionRendererOld()
{
	time_t absolut = 870912000;		// seconds
	const char *format = "%d.%m.%y";

	Anything Config4RendererAny;
	Anything RendererAny;
	Anything EmptyAny;

	Config4RendererAny["Format"] = format;
	Config4RendererAny["Date"] = absolut;

	RendererAny["Type"] = "DateRenderer";
	RendererAny["Data"] = Config4RendererAny;

	ConfigureField1();

	fConfig["Options"]["1"] = RendererAny;
}

void FieldRendererTest::TestFieldOptionRendererOld()
{
	// set up the configuration
	this->ConfigureFieldOptionRendererOld();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureFieldOptionRenderer()
{
	time_t absolut = 870912000;		// seconds
	const char *format = "%d.%m.%y";

	Anything Config4RendererAny;
	Anything RendererAny;

	Config4RendererAny["Format"] = format;
	Config4RendererAny["Date"] = absolut;

	RendererAny["DateRenderer"] = Config4RendererAny;

	ConfigureField1();

	fConfig["Options"]["1"] = RendererAny;
}

void FieldRendererTest::ConfigureFieldOptionSelectUnselectRenderer()
{
	fConfig["Name"] = "the name of field";
	fConfig["Size"]	= "1";
	fConfig["List"][0L]["Text"]["Type"] = "StringRenderer";
	fConfig["List"][0L]["Text"]["Default"] = "Text 1";
	fConfig["List"][0L]["Options"]["Value"] = "Value 1";
	fConfig["List"][0L]["Options"]["Selected"] = "1";
	fConfig["List"][1L]["Text"]["Type"] = "StringRenderer";
	fConfig["List"][1L]["Text"]["Default"] = "Text 2";
	fConfig["List"][1L]["Options"]["Value"] = "Value 2";
	fConfig["List"][1L]["Options"]["Selected"] = "0";

	this->RenderConf();
}

void FieldRendererTest::TestFieldOptionRenderer()
{
	// set up the configuration
	this->ConfigureFieldOptionRenderer();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureFieldFaultOptionRendererOld()
{
	time_t absolut = 870912000;		// seconds
	const char *format = "%d.%m.%y";

	Anything Config4RendererAny;
	Anything RendererAny;
	Anything EmptyAny;

	Config4RendererAny["Format"] = format;
	Config4RendererAny["Date"] = absolut;

	RendererAny["Type"] = "FaultRenderer";
	RendererAny["Data"] = Config4RendererAny;

	ConfigureField1();

	fConfig["Options"]["1"] = RendererAny;
}

void FieldRendererTest::TestFieldFaultOptionRendererOld()
{
	// set up the configuration
	this->ConfigureFieldFaultOptionRendererOld();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::TestFieldOptionSelectUnselectRenderer()
{
	// set up the configuration
	this->ConfigureFieldOptionSelectUnselectRenderer();
}

void FieldRendererTest::ConfigureFieldFaultOptionRenderer()
{
	time_t absolut = 870912000;		// seconds
	const char *format = "%d.%m.%y";

	Anything Config4RendererAny;
	Anything RendererAny;

	Config4RendererAny["Format"] = format;
	Config4RendererAny["Date"] = absolut;

	RendererAny["FaultRenderer"] = Config4RendererAny;

	ConfigureField1();

	fConfig["Options"]["1"] = RendererAny;
}

void FieldRendererTest::TestFieldFaultOptionRenderer()
{
	// set up the configuration
	this->ConfigureFieldFaultOptionRenderer();
	// render the configuration
	this->RenderConf();
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void FieldRendererTest::ConfigureEmptyConf()
{
	Anything any;
	fConfig = any;
}

void FieldRendererTest::TestEmptyConf()
{
	// set up the configuration
	this->ConfigureEmptyConf();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::TestFieldWithoutName()
{
	// fConfig["Name"] is mandatory
	// set up the configuration
	this->ConfigureField0();
	fConfig.Remove("Name");
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::TestFieldWithoutLabel()
{
	// fConfig["Label"] is not mandatory
	// set up the configuration
	this->ConfigureField0();
	fConfig.Remove("Label");
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::TestFieldWithoutValue()
{
	// fConfig["Value"] is not mandatory
	// set up the configuration
	this->ConfigureField0();
	fConfig.Remove("Value");
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::TestFieldWithoutSource()
{
	// fConfig["Source"] is not mandatory
	// set up the configuration
	this->ConfigureField0();
	fConfig.Remove("Value");
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::TestFieldWithoutMultiple()
{
	// fConfig["Multiple"] is not mandatory
	// set up the configuration
	this->ConfigureField0();
	fConfig.Remove("Multiple");
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::TestFieldWithoutChecked()
{
	// fConfig["Checked"] is not mandatory
	// set up the configuration
	this->ConfigureField0();
	fConfig.Remove("Checked");
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::TestFieldWithoutOptions()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->ConfigureField0();
	fConfig.Remove("Options");
	// render the configuration
	this->RenderConf();
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void FieldRendererTest::ConfigureWrongField()
{
	fConfig["Name"] = "";
	fConfig["Label"] = "";
	fConfig["Value"] = "";
	fConfig["Src"] = "";
	fConfig["Multiple"] = "";
	fConfig["Checked"] = "";
	fConfig["Options"] = "";
}

void FieldRendererTest::TestWrongField()
{
	// Can the reply be interpreted by HTML?
	// set up the configuration
	this->ConfigureField0();
	// render the configuration
	this->RenderConf();
}

/*---------------------------------------------------------------------------------------*/
/* Following configurations and tests are important ONLY for PulldownMenuRenderer and    */
/* SelectBoxRenderer and are applied only in such cases.                                 */
/* It is of course possible to use following configurations for all other FieldRenderers */
/*---------------------------------------------------------------------------------------*/
void FieldRendererTest::ConfigureFieldWithList()
{
	fConfig["Name"] = "the name of field";
	fConfig["Label"] = "the label of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = "the source of field";
	fConfig["Multiple"] = 1L;
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = "the option nr. 2 of field";

	Anything any0, any1, any2, any3, any4, any5, any6, any7;

	any0["Options"]["Value"]    = "the value from list[0]";
	any0["Options"]["Selected"] = "the selected from list[0]";
	any0["Text"]                = "the text from list[0]";

	any1["Options"]["Value"]    = "the value from list[1]";
	any1["Options"]["Selected"] = "the selected from list[1]";

	any2["Options"]["Value"]    = "the value from list[2]";
	any2["Text"]                = "the text from list[2]";

	any3["Options"]["Selected"] = "the selected from list[3]";
	any3["Text"]                = "the text from list[3]";

	any4["Options"]["Value"]    = "the value from list[4]";

	any5["Options"]["Selected"] = "the selected from list[5]";

	any6["Text"]                = "the text from list[6]";

	// any7 must be empty !!!

	fConfig["List"].Append(any0);
	fConfig["List"].Append(any1);
	fConfig["List"].Append(any2);
	fConfig["List"].Append(any3);
	fConfig["List"].Append(any4);
	fConfig["List"].Append(any5);
	fConfig["List"].Append(any6);
	fConfig["List"].Append(any7);
}

void FieldRendererTest::TestFieldWithList()
{
	// fConfig["List"] is not mandatory
	// set up the configuration
	this->ConfigureFieldWithList();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureFieldWithLookupList()
{
	fConfig["Name"] = "the name of field";
	fConfig["Label"] = "the label of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = "the source of field";
	fConfig["Multiple"] = 1L;
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = "the option nr. 2 of field";

	Anything any0, any1, any2, any3, any4, any5, any6, any7;

	any0["Options"]["Value"]    = "the value from list[0]";
	any0["Options"]["Selected"] = "the selected from list[0]";
	any0["Text"]                = "the text from list[0]";

	any1["Options"]["Value"]    = "the value from list[1]";
	any1["Options"]["Selected"] = "the selected from list[1]";

	any2["Options"]["Value"]    = "the value from list[2]";
	any2["Text"]                = "the text from list[2]";

	any3["Options"]["Selected"] = "the selected from list[3]";
	any3["Text"]                = "the text from list[3]";

	any4["Options"]["Value"]    = "the value from list[4]";

	any5["Options"]["Selected"] = "the selected from list[5]";

	any6["Text"]                = "the text from list[6]";

	// any7 must be empty !!!

	Anything anyList;
	anyList.Append(any0);
	anyList.Append(any1);
	anyList.Append(any2);
	anyList.Append(any3);
	anyList.Append(any4);
	anyList.Append(any5);
	anyList.Append(any6);
	anyList.Append(any7);

	fConfig["LookupList"] = "KeyInContext";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["KeyInContext"] = anyList;
}

void FieldRendererTest::TestFieldWithLookupList()
{
	// fConfig["LookupList"] is not mandatory
	// set up the configuration
	this->ConfigureFieldWithLookupList();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureFieldWithListAndLookupList()
{
	fConfig["Name"] = "the name of field";
	fConfig["Label"] = "the label of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = "the source of field";
	fConfig["Multiple"] = 1L;
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = "the option nr. 2 of field";

	Anything any0, any1;

	any0["Options"]["Value"]    = "the value from LookupList[0]";
	any0["Options"]["Selected"] = "the selected from LookupList[0]";
	any0["Text"]                = "the text from LookupList[0]";

	// LookupList
	Anything anyList;
	anyList.Append(any0);
	fConfig["LookupList"] = "KeyInContext";
	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["KeyInContext"] = anyList;

	// List
	any1["Options"]["Value"]    = "the value from list[0]";
	any1["Options"]["Selected"] = "the selected from list[0]";
	any1["Text"]                = "the text from list[0]";
	fConfig["List"].Append(any1);
}

void FieldRendererTest::TestFieldWithListAndLookupList()
{
	// fConfig["LookupList"] is not mandatory
	// set up the configuration
	this->ConfigureFieldWithListAndLookupList();
	// render the configuration:  LookupList is stronger than List
	this->RenderConf();
}

void FieldRendererTest::ConfigureFieldWithListAndLookupListSkipped()
{
	fConfig["Name"] = "the name of field";
	fConfig["Label"] = "the label of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = "the source of field";
	fConfig["Multiple"] = 1L;
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = "the option nr. 2 of field";

	Anything any0, any1;

	any0["Options"]["Value"]    = "the value from LookupList[0]";
	any0["Options"]["Selected"] = "the selected from LookupList[0]";
	any0["Text"]                = "the text from LookupList[0]";

	// LookupList
	Anything anyList;
	anyList.Append(any0);
	fConfig["LookupList"] = "KeyInContext";
	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["XXX"] = anyList;	// LookupList is not to be found

	// List
	any1["Options"]["Value"]    = "the value from list[0]";
	any1["Options"]["Selected"] = "the selected from list[0]";
	any1["Text"]                = "the text from list[0]";
	fConfig["List"].Append(any1);
}

void FieldRendererTest::TestFieldWithListAndLookupListSkipped()
{
	// fConfig["LookupList"] is not mandatory
	// set up the configuration
	this->ConfigureFieldWithListAndLookupListSkipped();
	// render the configuration:  LookupList not found in Context,  List ignored
	// --> RenderConf renders without List and without LookupList
	this->RenderConf();
}

void FieldRendererTest::ConfigureFieldWithUndefList()
{
	fConfig["Name"] = "the name of field";
	fConfig["Label"] = "the label of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = "the source of field";
	fConfig["Multiple"] = 1L;
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = "the option nr. 2 of field";

	Anything any0;
	fConfig["List"].Append(any0);
}

void FieldRendererTest::TestFieldWithUndefList()
{
	// fConfig["List"] is not mandatory
	// set up the configuration
	this->ConfigureFieldWithUndefList();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureFieldWithListAndUndefLookupList()
{
	fConfig["Name"] = "the name of field";
	fConfig["Label"] = "the label of field";
	fConfig["Value"] = "the value of field";
	fConfig["Src"] = "the source of field";
	fConfig["Multiple"] = 1L;
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = "the option nr. 2 of field";

	Anything any0, any1;

	// LookupList (Undef)
	Anything anyList;
	anyList.Append(any0);
	fConfig["LookupList"] = "KeyInContext";
	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["KeyInContext"] = anyList;

	// List
	any1["Options"]["Value"]    = "the value from list[0]";
	any1["Options"]["Selected"] = "the selected from list[0]";
	any1["Text"]                = "the text from list[0]";
	fConfig["List"].Append(any1);
}

void FieldRendererTest::TestFieldWithListAndUndefLookupList()
{
	// fConfig["LookupList"] is not mandatory
	// set up the configuration
	this->ConfigureFieldWithListAndUndefLookupList();
	// render the configuration:  LookupList is stronger than List + LookupList is empty
	// --> RenderConf renders without List and without LookupList
	this->RenderConf();
}

void FieldRendererTest::ConfigureAllAttributes()
{
	Anything Opt1, Opt2, Opt3, list;

	fConfig["Name"] = "EinName";
	fConfig["Size"] = 10;
	fConfig["Multiple"] = 1L;

	Opt1["Options"]["Value"] = 11;
	Opt1["Options"]["Selected"] = 1;
	Opt1["Text"] = "Das ist der Text1.";

	Opt2["Options"]["Value"] = 22;
	Opt2["Options"]["Selected"] = 2;
	Opt2["Text"] = "Das ist der Text2.";

	Opt3["Options"]["Value"] = 33;
	Opt3["Options"]["Selected"] = 3;
	Opt3["Text"] = "Das ist der Text3.";

	list.Append( Opt1 );
	list.Append( Opt2 );
	list.Append( Opt3 );

	fConfig["List"] = list;
}

void FieldRendererTest::TestAllAttributes()
{
	// set up the configuration
	this->ConfigureAllAttributes();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureNameAndMultiple()
{
	PulldownMenuRenderer pulldownMenuRenderer("");

	fConfig["Name"] = "EinName";
	fConfig["Multiple"] = 1L;
}

void FieldRendererTest::TestNameAndMultiple()
{
	// set up the configuration
	this->ConfigureNameAndMultiple();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureListWithUndefOptions()
{
	Anything Opt1, EmptyOpt;
	fConfig["Name"] = "EinName";
	fConfig["List"]["Options"] = EmptyOpt;
}

void FieldRendererTest::TestListWithUndefOptions()
{
	// set up the configuration
	this->ConfigureListWithUndefOptions();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureValue()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Value"] 		= "the value of field";
}

void FieldRendererTest::TestFieldValue()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->ConfigureValue();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureUnreadable()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Unreadable"] 	= 1;
}

void FieldRendererTest::TestFieldUnreadable()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->ConfigureUnreadable();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureSize()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Size"] 		= 50;
}

void FieldRendererTest::TestFieldSize()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->ConfigureSize();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureMaxlength()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Maxlength"] 	= 40;
}

void FieldRendererTest::TestFieldMaxlength()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->ConfigureMaxlength();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureWidth()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Width"]		= 30;
}

void FieldRendererTest::TestFieldWidth()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->ConfigureWidth();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureHeight()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Height"]		= 20;
}

void FieldRendererTest::TestFieldHeight()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->ConfigureHeight();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureWrap()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Wrap"]			= 10;
}

void FieldRendererTest::TestFieldWrap()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->ConfigureWrap();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::Configure4Opt()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = "the option nr. 2 of field";
	fConfig["Options"]["3"] = "the option nr. 3 of field";
}

void FieldRendererTest::TestField4Opt()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->Configure4Opt();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::Configure3Opt()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
	fConfig["Options"]["2"] = "the option nr. 2 of field";
}

void FieldRendererTest::TestField3Opt()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->Configure3Opt();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::Configure2Opt()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Options"]["0"] = "the option nr. 0 of field";
	fConfig["Options"]["1"] = "the option nr. 1 of field";
}

void FieldRendererTest::TestField2Opt()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->Configure2Opt();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::Configure1Opt()
{
	fConfig["Name"] 		= "the name of field";
	fConfig["Options"]["0"] = "the option nr. 0 of field";
}

void FieldRendererTest::TestField1Opt()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->Configure1Opt();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::Configure2OptNoSlotName()
{
	Anything OptAny;
	fConfig["Name"] 		= "the name of field";
	OptAny.Append( "the option nr. 0 of field" );
	OptAny.Append( "the option nr. 1 of field" );
	fConfig["Options"] = OptAny;
}
void FieldRendererTest::TestField2OptNoSlotName()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->Configure2OptNoSlotName();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::Configure1OptNoSlotName()
{
	Anything OptAny;
	fConfig["Name"] 		= "the name of field";
	OptAny.Append( "the option nr. 0 of field" );
	fConfig["Options"] = OptAny;
}

void FieldRendererTest::TestField1OptNoSlotName()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->Configure1OptNoSlotName();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::Configure3OptMixed()
{
	Anything OptAny;
	fConfig["Name"] 		= "the name of field";
	OptAny.Append( "the option nr. 0 of field" );
	OptAny["1"] = "the option nr. 1 of field";
	OptAny.Append( "the option nr. 2 of field" );
	fConfig["Options"] = OptAny;
}

void FieldRendererTest::TestField3OptMixed()
{
	// fConfig["Options"] is not mandatory
	// set up the configuration
	this->Configure3OptMixed();
	// render the configuration
	this->RenderConf();
}

void FieldRendererTest::ConfigureOptEmpty()
{
	Anything AnyOpt;
	fConfig["Name"] = "the name of field";
	fConfig["Options"] = AnyOpt;
}

void FieldRendererTest::TestFieldOptEmpty()
{
	// Can the reply be interpreted by HTML?
	// set up the configuration
	this->ConfigureOptEmpty();
	// render the configuration
	this->RenderConf();
}
