/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "LocalizationUtils.h"

//--- interface include --------------------------------------------------------
#include "LocalizationUtilsTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"

//--- c-library modules used ---------------------------------------------------

LocalizationUtilsTest::LocalizationUtilsTest (TString tname) : TestCaseType(tname),
	fContext( MetaThing(), Anything(), 0, 0, 0, 0 ) {};
LocalizationUtilsTest::~LocalizationUtilsTest() {};

void LocalizationUtilsTest::setUp ()
{
	System::SetRootDir("."); // Root is usually local

	Anything Languages;
	Languages.Append("Deutsch");

	Anything env( fContext.GetEnvStore() );
	env["header"]["ACCEPT-LANGUAGE"] = Languages;

	Anything TmpStore(fContext.GetTmpStore());
	Anything LangKeyMap;
	LangKeyMap["Key0"] = "Deutsch";
	LangKeyMap["Key1"] = "Francais";
	LangKeyMap["Key2"] = "Italiano";
	LangKeyMap["Key3"] = "English";
	TmpStore["UseBrowserLang"] = 1;
	TmpStore["LanguageKeyMap"] = LangKeyMap;

	TmpStore["HTMLTemplateConfig"]["TemplateDir"] = "TestDir1:TestDir2:";

	Anything LangDirMap;
	LangDirMap["Key0"] = "Localized_D";
	LangDirMap["Key1"] = "Localized_F";
	LangDirMap["Key2"] = "Localized_I";
	LangDirMap["Key3"] = "Localized_E";
	TmpStore["HTMLTemplateConfig"]["LanguageDirMap"] = LangDirMap;
}

void LocalizationUtilsTest::EvalNullLangEmptyConfigWODefault()
// 'lang' is a NULL pointer 'config' is an empty anything, without default Language
{
	ROAnything ROconfig;

	String TestString( LocalizationUtils::Eval( 0, ROconfig) );

	assertEqual( "", TestString );

} // EvalNullLangEmptyConfigWODefault

void LocalizationUtilsTest::EvalEmptyLangEmptyConfigWODefault()
// 'config' is an empty anything, without default Language
{
	ROAnything ROconfig;

	const char Lang = 0;

	String TestString( LocalizationUtils::Eval( &Lang, ROconfig) );

	assertEqual( "", TestString );

} // EvalEmptyLangEmptyConfigWODefault

void LocalizationUtilsTest::EvalEmptyConfigWODefault()
// 'lang' exists but 'config' is an empty anything, without default Language
{
	ROAnything ROconfig;

	const char *Lang = "TestLang";

	String TestString( LocalizationUtils::Eval( Lang, ROconfig) );

	assertEqual( "", TestString );

} // EvalEmptyConfigWODefault

void LocalizationUtilsTest::EvalNormalCaseWODefault()
// Usual case, without default Language
{
	Anything config;

	config["TestLang"] = "TestString";

	ROAnything ROconfig = config;

	const char *Lang = "TestLang";

	String TestString( LocalizationUtils::Eval( Lang, ROconfig) );

	assertEqual( "TestString", TestString );

} // EvalNormalCaseWODefault

void LocalizationUtilsTest::EvalNullLangEmptyConfigWithDefault()
// 'lang' is a NULL pointer 'config' is an empty anything, with default Language
{
	Anything config;

	config["Default"] = "TestDefault";

	ROAnything ROconfig = config;

	String TestString( LocalizationUtils::Eval( 0, ROconfig) );

	assertEqual( "TestDefault", TestString );

} // EvalNullLangEmptyConfigWithDefault

void LocalizationUtilsTest::EvalEmptyLangEmptyConfigWithDefault()
// 'config' is an empty anything, with default Language
{
	Anything config;

	config["Default"] = "TestDefault";

	ROAnything ROconfig = config;

	const char Lang = 0;

	String TestString( LocalizationUtils::Eval( &Lang, ROconfig) );

	assertEqual( "TestDefault", TestString );

} // EvalEmptyLangEmptyConfigWithDefault

void LocalizationUtilsTest::EvalEmptyConfigWithDefault()
// 'lang' exists but 'config' is an empty anything, with default Language
{
	Anything config;

	config["Default"] = "TestDefault";

	ROAnything ROconfig = config;

	const char *Lang = "TestLang";

	String TestString( LocalizationUtils::Eval( Lang, ROconfig) );

	assertEqual( "TestDefault", TestString );

} // EvalEmptyConfigWithDefault

void LocalizationUtilsTest::EvalNormalCaseWithDefault()
// Usual case, with default language
{
	Anything config;

	config["TestLang"] = "TestString";
	config["Default"] = "TestDefault";

	ROAnything ROconfig = config;

	const char *Lang = "TestLang";

	String TestString( LocalizationUtils::Eval( Lang, ROconfig) );

	assertEqual( "TestString", TestString );

} // EvalTest

void LocalizationUtilsTest::EvalAnyNullLangEmptyConfigWODefault()
// 'lang' is a NULL pointer 'config' is an empty anything, without default Language
{
	ROAnything ROconfig;

	ROAnything TestAny = LocalizationUtils::EvalAny( 0, ROconfig);

	String assertString = TestAny.AsCharPtr();
	assertEqual( "", assertString );

} // EvalAnyNullLangEmptyConfigWODefault

void LocalizationUtilsTest::EvalAnyEmptyLangEmptyConfigWODefault()
// 'config' is an empty anything, without default Language
{
	ROAnything ROconfig;

	const char Lang = 0;

	ROAnything TestAny = LocalizationUtils::EvalAny( &Lang, ROconfig);

	String assertString = TestAny.AsCharPtr();
	assertEqual( "", assertString );

} // EvalAnyEmptyLangEmptyConfigWODefault

void LocalizationUtilsTest::EvalAnyEmptyConfigWODefault()
// 'lang' exists but 'config' is an empty anything, without default Language
{
	ROAnything ROconfig;

	const char *Lang = "TestLang";

	ROAnything TestAny = LocalizationUtils::EvalAny( Lang, ROconfig);

	String assertString = TestAny.AsCharPtr();
	assertEqual( "", assertString );

} // EvalAnyEmptyConfigWODefault

void LocalizationUtilsTest::EvalAnyNormalCaseWODefault()
// Usual case, without default Language
{
	Anything config;

	config["TestLang"] = "TestString";

	ROAnything ROconfig = config;

	const char *Lang = "TestLang";

	ROAnything TestAny = LocalizationUtils::EvalAny( Lang, ROconfig);

	assertEqual( "TestString", TestAny.AsCharPtr() );

} // EvalAnyNormalCaseWODefault

void LocalizationUtilsTest::EvalAnyNullLangEmptyConfigWithDefault()
// 'lang' is a NULL pointer 'config' is an empty anything, with default Language
{
	Anything config;

	config["Default"] = "TestDefault";

	ROAnything ROconfig = config;

	ROAnything TestAny = LocalizationUtils::EvalAny( 0, ROconfig);

	assertEqual( "TestDefault", TestAny.AsCharPtr() );

} // EvalAnyNullLangEmptyConfigWithDefault

void LocalizationUtilsTest::EvalAnyEmptyLangEmptyConfigWithDefault()
// 'config' is an empty anything, with default Language
{
	Anything config;

	config["Default"] = "TestDefault";

	ROAnything ROconfig = config;

	const char Lang = 0;

	ROAnything TestAny = LocalizationUtils::EvalAny( &Lang, ROconfig);

	assertEqual( "TestDefault", TestAny.AsCharPtr() );

} // EvalAnyEmptyLangEmptyConfigWithDefault

void LocalizationUtilsTest::EvalAnyEmptyConfigWithDefault()
// 'lang' exists but 'config' is an empty anything, with default Language
{
	Anything config;

	config["Default"] = "TestDefault";

	ROAnything ROconfig = config;

	const char *Lang = "TestLang";

	ROAnything TestAny = LocalizationUtils::EvalAny( Lang, ROconfig);

	assertEqual( "TestDefault", TestAny.AsCharPtr() );

} // EvalAnyEmptyConfigWithDefault

void LocalizationUtilsTest::EvalAnyNormalCaseWithDefault()
// Usual case, with default language
{
	Anything config;

	config["TestLang"] = "TestString";
	config["Default"] = "TestDefault";

	ROAnything ROconfig = config;

	const char *Lang = "TestLang";

	ROAnything TestAny = LocalizationUtils::EvalAny( Lang, ROconfig);

	assertEqual( "TestString", TestAny.AsCharPtr() );

} // EvalAnyNormalCaseWithDefault

void LocalizationUtilsTest::LangKeyKey0()
// Find the 1st language 'Deutsch' ( key0 )
{
	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "Key0", TestString );
}

void LocalizationUtilsTest::LangKeyKey1()
// Find the 2nd language ( key1 )
{
	Anything Languages;
	Languages.Append("Francais");
	Languages.Append("Deutsch");
	Anything env( fContext.GetEnvStore() );
	env["header"]["ACCEPT-LANGUAGE"] = Languages;

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "Key1", TestString );
}

void LocalizationUtilsTest::LangKeyKey2()
// Find the 3rd language ( key2 )
{
	Anything Languages;
	Languages.Append("Italiano");
	Languages.Append("Deutsch");

	Anything env( fContext.GetEnvStore() );
	env["header"]["ACCEPT-LANGUAGE"] = Languages;

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "Key2", TestString );

} // LangKeyKey2

void LocalizationUtilsTest::LangKeyKey3()
// Find the 4th language ( key3 )
{
	Anything Languages;
	Languages.Append("English");
	Languages.Append("Deutsch");

	Anything env( fContext.GetEnvStore() );
	env["header"]["ACCEPT-LANGUAGE"] = Languages;

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "Key3", TestString );

} // LangKeyKey3

void LocalizationUtilsTest::LangKeyKeyOneLang()
// Only one language available
{
	Anything Languages;
	Languages.Append("Deutsch");

	Anything env( fContext.GetEnvStore() );
	env["header"]["ACCEPT-LANGUAGE"] = Languages;

	Anything TmpStore(fContext.GetTmpStore());
	Anything LangKeyMap;
	LangKeyMap["Key0"] = "Deutsch";
	TmpStore["UseBrowserLang"] = 1;
	TmpStore["LanguageKeyMap"] = LangKeyMap;

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "Key0", TestString );
} // LangKeyKeyOneLang

void LocalizationUtilsTest::LangKeyBadAndGoodLanguages()
// Not existing languages
{
	Anything Languages;
	Languages.Append("Chinesisch");
	Languages.Append("Hollaendisch");
	Languages.Append("Italiano");
	Languages.Append("English");
	Anything env( fContext.GetEnvStore() );
	env["header"]["ACCEPT-LANGUAGE"] = Languages;

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "Key2", TestString );

} // LangKeyBadAndGoodLanguages

void LocalizationUtilsTest::LangKeyEmptyLanguages()
// No languages ( empty anything ), take default language
{
	Anything Languages;

	Anything env( fContext.GetEnvStore() );
	env["header"]["ACCEPT-LANGUAGE"] = Languages;

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "DefKey", TestString );
} // LangKeyEmptyLanguages

void LocalizationUtilsTest::LangKeyEmptyKeyMap()
// Key map is empty anything, take default language
{
	LocalizationUtils::EmptyLanguageMap();

	Anything TmpStore(fContext.GetTmpStore());
	Anything LangKeyMap;

	TmpStore["UseBrowserLang"] = 1;
	TmpStore["LanguageKeyMap"] = LangKeyMap;

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "DefKey", TestString );
} // LangKeyEmptyKeyMap

void LocalizationUtilsTest::LangKeyEmptyLanguagesEmptyKeyMap()
// No languages, empty key map, take default language
{
	Anything Languages;

	Anything env( fContext.GetEnvStore() );
	env["header"]["ACCEPT-LANGUAGE"] = Languages;

	Anything TmpStore(fContext.GetTmpStore());
	Anything LangKeyMap;

	TmpStore["LanguageKeyMap"] = LangKeyMap;

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "DefKey", TestString );
} // LangKeyEmptyLanguagesEmptyKeyMap

void LocalizationUtilsTest::LangKeyMissingLanguages()
{
	Anything EnvStore(fContext.GetEnvStore());
	Anything header = EnvStore["header"];

	header.Remove("ACCEPT-LANGUAGE");

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "DefKey", TestString );

	fContext.SetLanguage("Key0");
	// Context language now ignored by FindLanguageKey
	TestString = LocalizationUtils::FindLanguageKey( fContext, "DefKey") ;
	// Language found is independent of language set
	assertEqual( "DefKey", TestString );
	assertEqual( "Key0", fContext.Language() );

} // LangKeyMissingLanguages

void LocalizationUtilsTest::LangKeyMissingKeyMap()
{
	Anything TmpStore(fContext.GetTmpStore());
	TmpStore.Remove("LanguageKeyMap");

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "DefKey", TestString );

}

void LocalizationUtilsTest::LangKeyMissingLanguagesMissingKeyMap()
{
	Anything TmpStore(fContext.GetTmpStore());
	TmpStore.Remove("LanguageKeyMap");

	Anything EnvStore(fContext.GetEnvStore());
	Anything header = EnvStore["header"];

	header.Remove("ACCEPT-LANGUAGE");

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "DefKey", TestString );

} // LangKeyMissingLanguagesMissingKeyMap

void LocalizationUtilsTest::LangKeyDoNotUseBrowserLang()
// Normal constellation, but "UseBrowserLang" not set, take default language key
{
	Anything TmpStore(fContext.GetTmpStore());
	TmpStore.Remove("UseBrowserLang");

	String TestString( LocalizationUtils::FindLanguageKey( fContext, "DefKey") );
	assertEqual( "DefKey", TestString );

} // LangKeyDoNotUseBrowserLang

void LocalizationUtilsTest::OpenStreamLangD()
// Language D, Root directory relativ
{
	assertEqual(".", System::GetRootDir());

	String absoluteFileName;
	istream *is = LocalizationUtils::OpenStream( fContext, "Test", absoluteFileName );

	Anything any;
	if ( t_assert( is != NULL ) ) {
		*is >> any;
		assertEqual( "TestDir1Localized_D", any[ 0L ].AsCharPtr() );
		delete is;
	}

	// search a file in the second dir
	is = LocalizationUtils::OpenStream( fContext, "Test1", absoluteFileName );

	if ( t_assert( is != NULL ) ) {
		*is >> any;
		assertEqual( "TestDir2NonLocalizedTest1", any[ 0L ].AsCharPtr() );
		delete is;
	}

} // OpenStreamLangD
void LocalizationUtilsTest::PrepareRootDir()
{
	String cwd;
	System::GetCWD(cwd);
	System::ResolvePath(cwd);
	System::SetRootDir(cwd);
	String grd(System::GetRootDir());
	System::ResolvePath(grd);
	assertEqual(cwd, grd);
}
void LocalizationUtilsTest::OpenStreamLangDAbsolut()
// Language D, Root directory absolut
{
	PrepareRootDir();

	String absoluteFileName;
	istream *is = LocalizationUtils::OpenStream( fContext, "Test", absoluteFileName );
	t_assert( is != 0 );
	// this will fail if daily build did not check out correctly

	Anything any;
	if ( is ) {
		*is >> any;
		assertEqual( any[ 0L ].AsCharPtr(), "TestDir1Localized_D" );
		delete is;
	}
} // OpenStreamLangDAbsolut

void LocalizationUtilsTest::OpenStreamLangF()
// Language F, Root directory relative
{
	fContext.SetLanguage("Key1");
	// we already know that the language can be determined correctly
	// using FindLanguage which is used during construction of the Context

	String absoluteFileName;
	istream *is = LocalizationUtils::OpenStream( fContext, "Test", absoluteFileName );
	t_assert( is != 0 );
	Anything any;
	if ( is ) {
		*is >> any;
		assertEqual( "TestDir1Localized_F", any[ 0L ].AsCharPtr() );
		delete is;
	}
} // OpenStreamLangF

void LocalizationUtilsTest::OpenStreamLangF1stTemplateDirWrong()
// Language F, File is at the 2nd Template Directory
{
	fContext.SetLanguage("Key1");

	Anything TmpStore(fContext.GetTmpStore());
	TmpStore["HTMLTemplateConfig"]["TemplateDir"] = "Schrott:TestDir1:TestDir2";

	String absoluteFileName;
	istream *is = LocalizationUtils::OpenStream( fContext, "Test", absoluteFileName );
	t_assert( is != 0 );
	Anything any;
	if ( is ) {
		*is >> any;
		assertEqual( "TestDir1Localized_F", any[ 0L ].AsCharPtr() );
		delete is;
	}
} // OpenStreamLangF1stTemplateDirWrong

void LocalizationUtilsTest::OpenStreamLangDDefault()
// Language I not found, take the default language
{
	fContext.SetLanguage("Key2");

	String absoluteFileName;
	istream *is = LocalizationUtils::OpenStream( fContext, "Test", absoluteFileName );
	t_assert( is != 0 );
	Anything any;
	if ( is ) {
		*is >> any;
		assertEqual( "TestDir1NonLocalized", any[ 0L ].AsCharPtr() );
		delete is;
	}
} // OpenStreamLangDDefault

void LocalizationUtilsTest::OpenStreamLangDDefaultAbsolut()
// Language I not found, Root dir absolut, take the default language
{
	PrepareRootDir();
	String expectedPath = System::GetRootDir();
	expectedPath.Append("/TestDir1/Test");

	// only works in automatic test environment

	fContext.SetLanguage("Key2");
	// language dir will not be found, use non localized version

	String absoluteFileName;
	istream *is = LocalizationUtils::OpenStream( fContext, "Test", absoluteFileName );
//	System::ResolvePath(absoluteFileName);
	assertEqual(expectedPath, absoluteFileName);
	t_assert( is != 0 );
	// these will fail if daily build did not check out correctly
	Anything any;
	if ( is ) {
		*is >> any;
		assertEqual( "TestDir1NonLocalized", any[ 0L ].AsCharPtr() );
		delete is;
	}
} // OpenStreamLangDDefaultAbsolut

void LocalizationUtilsTest::OpenStreamLangDDefaultAbsolut1stTemplateDirWrong()
// Language I not found, Root dir absolut, 1st template dir wrong,
// take the default language
{
	PrepareRootDir();

	fContext.SetLanguage("Key2");

	String absoluteFileName;
	istream *is = LocalizationUtils::OpenStream( fContext, "Test", absoluteFileName );
	t_assert( is != 0 );
	// this will fail if daily build did not check out correctly
	Anything any;
	if ( is ) {
		*is >> any;
		assertEqual( "TestDir1NonLocalized", any[ 0L ].AsCharPtr() );
		delete is;
	}
} // OpenStreamLangDDefaultAbsolut1stTemplateDirWrong

void LocalizationUtilsTest::OpenStreamDefaultTemplateDir()
// Language specific directory not found, try template dirctory
{
	fContext.SetLanguage("Key2");

	String absoluteFileName;
	istream *is = LocalizationUtils::OpenStream( fContext, "Test", absoluteFileName );
	t_assert( is != 0 );
	Anything any;
	if ( is ) {
		*is >> any;
		assertEqual( "TestDir1NonLocalized", any[ 0L ].AsCharPtr() );
		delete is;
	}
} // OpenStreamDefaultTemplateDir

Test *LocalizationUtilsTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, LocalizationUtilsTest, EvalNullLangEmptyConfigWODefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalEmptyLangEmptyConfigWODefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalEmptyConfigWODefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalNormalCaseWODefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalNullLangEmptyConfigWithDefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalEmptyLangEmptyConfigWithDefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalEmptyConfigWithDefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalNormalCaseWithDefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalAnyNullLangEmptyConfigWODefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalAnyEmptyLangEmptyConfigWODefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalAnyEmptyConfigWODefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalAnyNormalCaseWODefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalAnyNullLangEmptyConfigWithDefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalAnyEmptyLangEmptyConfigWithDefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalAnyEmptyConfigWithDefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, EvalAnyNormalCaseWithDefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyKey0);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyKey1);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyKey2);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyKey3);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyKeyOneLang);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyBadAndGoodLanguages);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyEmptyLanguages);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyEmptyKeyMap);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyEmptyLanguagesEmptyKeyMap);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyMissingLanguages);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyMissingKeyMap);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyMissingLanguagesMissingKeyMap);
	ADD_CASE(testSuite, LocalizationUtilsTest, LangKeyDoNotUseBrowserLang);
	ADD_CASE(testSuite, LocalizationUtilsTest, OpenStreamLangD);
	ADD_CASE(testSuite, LocalizationUtilsTest, OpenStreamLangDAbsolut);
	ADD_CASE(testSuite, LocalizationUtilsTest, OpenStreamLangF);
	ADD_CASE(testSuite, LocalizationUtilsTest, OpenStreamLangF1stTemplateDirWrong);
	ADD_CASE(testSuite, LocalizationUtilsTest, OpenStreamLangDDefault);
	ADD_CASE(testSuite, LocalizationUtilsTest, OpenStreamLangDDefaultAbsolut);
	ADD_CASE(testSuite, LocalizationUtilsTest, OpenStreamLangDDefaultAbsolut1stTemplateDirWrong);
	ADD_CASE(testSuite, LocalizationUtilsTest, OpenStreamDefaultTemplateDir);

	return testSuite;

}
