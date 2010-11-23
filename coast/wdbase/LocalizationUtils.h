/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LocalizationUtils_H
#define _LocalizationUtils_H

#include "Context.h"
#include "Threads.h"

class LocalizationUtils
{
public:
	static const char *Eval(const char *lang, const ROAnything &config);
	static const ROAnything EvalAny(const char *lang, const ROAnything &config);
	static std::istream *OpenStream(Context &c, const char *filename, String &relPath);
	static const char *FindLanguageKey(Context &c, const char *dft);

protected:
	static void BuildLanguageMap(Context &context);

private:
	// only for use in LocalizationUtilsTest!
	static void EmptyLanguageMap();

	static Mutex fgLanguageMapMutex;
	static bool  fgLanguageMapIsInit;
	static Anything fgLanguageMap;
	friend class LocalizationUtilsTest;
};

#endif
