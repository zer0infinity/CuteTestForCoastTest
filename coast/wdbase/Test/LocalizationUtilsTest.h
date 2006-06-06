/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LocalizationUtilsTest_h_
#define _LocalizationUtilsTest_h_

#include "TestCase.h"

class Context;

//---- LocalizationUtilsTest -----------------------------------------------------------
class LocalizationUtilsTest : public TestFramework::TestCase
{
public:
	LocalizationUtilsTest(TString tstrName);
	virtual ~LocalizationUtilsTest();

	static Test *suite ();
	void setUp ();
protected:
	void PrepareRootDir();
	void EvalNullLangEmptyConfigWODefault();
	void EvalEmptyLangEmptyConfigWODefault();
	void EvalEmptyConfigWODefault();
	void EvalNormalCaseWODefault();
	void EvalNullLangEmptyConfigWithDefault();
	void EvalEmptyLangEmptyConfigWithDefault();
	void EvalEmptyConfigWithDefault();
	void EvalNormalCaseWithDefault();
	void EvalAnyNullLangEmptyConfigWODefault();
	void EvalAnyEmptyLangEmptyConfigWODefault();
	void EvalAnyEmptyConfigWODefault();
	void EvalAnyNormalCaseWODefault();
	void EvalAnyNullLangEmptyConfigWithDefault();
	void EvalAnyEmptyLangEmptyConfigWithDefault();
	void EvalAnyEmptyConfigWithDefault();
	void EvalAnyNormalCaseWithDefault();
	void LangKeyKey0();
	void LangKeyKey1();
	void LangKeyKey2();
	void LangKeyKey3();
	void LangKeyKeyOneLang();
	void LangKeyBadAndGoodLanguages();
	void LangKeyEmptyLanguages();
	void LangKeyEmptyKeyMap();
	void LangKeyEmptyLanguagesEmptyKeyMap();
	void LangKeyMissingLanguages();
	void LangKeyMissingKeyMap();
	void LangKeyMissingLanguagesMissingKeyMap();
	void LangKeyDoNotUseBrowserLang();
	void OpenStreamLangD();
	void OpenStreamLangDAbsolut();
	void OpenStreamLangF();
	void OpenStreamLangF1stTemplateDirWrong();
	void OpenStreamLangDDefault();
	void OpenStreamLangDDefaultAbsolut();
	void OpenStreamLangDDefaultAbsolut1stTemplateDirWrong();
	void OpenStreamDefaultTemplateDir();

	Context fContext;
};

#endif
