/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LocalizationUtils.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "StringStream.h"
#include "Threads.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

Mutex	LocalizationUtils::fgLanguageMapMutex("LocalizationUtils", Storage::Global());
bool	LocalizationUtils::fgLanguageMapIsInit = false;
Anything LocalizationUtils::fgLanguageMap(Storage::Global());
//---- LocalizationUtils ----------------------------------------------------------------
const char *LocalizationUtils::Eval(const char *lang, const ROAnything &config)
{
	const char *string = 0;

	ROAnything match;
	if ((String() != lang) && config.LookupPath(match, lang)) {
		string = match.AsCharPtr(0);
	}

	if (string == 0) {
		ROAnything dft;
		if (config.LookupPath(dft, "Default")) {
			string = dft.AsCharPtr(0);
		}
#ifdef DEPRECATED
		if (string == 0) {
			ROAnything value;
			if (config.LookupPath(value, "Value")) {
				string = value.AsCharPtr(0);
			}
		}
#endif
	}
	return string;
}

const ROAnything LocalizationUtils::EvalAny(const char *lang, const ROAnything &config)
{
	ROAnything match;
	if (lang && config.LookupPath(match, lang)) {
		return match;
	}

	config.LookupPath(match, "Default");
	return match;		// return default or NullAny
}

void LocalizationUtils::BuildLanguageMap(Context &context)
{
	StartTrace(LocalizationUtils.BuildLanguageMap);
	ROAnything langKeyMap = context.Lookup("LanguageKeyMap");
	for ( long i = 0, sz = langKeyMap.GetSize(); i < sz; ++i) {
		const char *lang = langKeyMap.SlotName(i);
		ROAnything languages = langKeyMap[i];
		for ( long j = 0, szl = languages.GetSize(); j < szl; ++j) {
			fgLanguageMap[ languages[j].AsCharPtr("") ] = lang;
		}
	}
}

const char *LocalizationUtils::FindLanguageKey(Context &c, const char *dftLang)
{
	StartTrace(LocalizationUtils.FindLanguageKey);
	// extract the supported language string from
	// the browsers accept language list
	bool	useBrowserPrefs = (c.Lookup("UseBrowserLang", 0L) != 0);
	Anything env = c.GetEnvStore();
	Anything header = env["header"];
	TraceAny(header, "header");
	long i = 0, sz = 0;

	ROAnything langKeyMap = c.Lookup("LanguageKeyMap");
	TraceAny(langKeyMap, "LanguageKeyMap");

	if ( useBrowserPrefs && header.IsDefined("ACCEPT-LANGUAGE") ) {
		// Do not check fgLanguageMap.GetSize() == 0 since there might be no data to intialize it!
		if ( fgLanguageMapIsInit == false ) {
			MutexEntry me(fgLanguageMapMutex);
			// Double checked locking pattern, see Doug Schmidt
			if ( fgLanguageMapIsInit == false ) {
				BuildLanguageMap(c);
				fgLanguageMapIsInit = true;
			}
		}
		// accesses to fgLanguageMap will always encounter the initialized fgLanguageMap
		Anything languages = header["ACCEPT-LANGUAGE"];
		for ( i = 0, sz = languages.GetSize(); i < sz; ++i) {
			if ( fgLanguageMap.IsDefined(languages[i].AsCharPtr("") ) ) {
				Trace("returning language [" << fgLanguageMap[languages[i].AsCharPtr("")].AsCharPtr("") << "]");
				return fgLanguageMap[languages[i].AsCharPtr("")].AsCharPtr("");
			}
		}
	}
	return dftLang;
}
// PS: should use cSeparator from System
// PS: this routine should be better factorized.
// PS: three loops should happen, i think
// PS: should be able to read gifs etc as well.

static istream *tryopen(String &absolutepath, const char *rdir, const char *tdir, const char *langdir, const char *filename, const char *ext = 0)
{
	absolutepath = rdir;
	absolutepath << tdir << (System::Sep());
	if (langdir) {
		absolutepath << langdir << (System::Sep());
	}
	absolutepath << filename;
	return System::OpenIStream(absolutepath, ext ? ext : "html");
}

istream *LocalizationUtils::OpenStream(Context &c, const char *filename, String &absoluteFileName)
{
	StartTrace(LocalizationUtils.OpenStream);
	istream *fp = 0;
	const char *lang = c.Language();
	Trace("using language [" << NotNull(lang) << "]");
	// default language has been set if no language set explicitly

	// search the configured directory tree
	// first the language specific

	const char *dft = "config/HTMLTemplates";
	StringTokenizer st(c.Lookup("TemplateDir").AsCharPtr(dft), ':');
	String templateDir;
	ROAnything languageDirMap(c.Lookup("LanguageDirMap"));
	String localizedPath;

	String rootpath(System::GetRootDir());
	rootpath << (System::Sep());

	if (lang) { // language is defined try getting a localized template
		localizedPath = languageDirMap[lang].AsCharPtr("Localized_D");
		Trace("using localized path [" << localizedPath << "]");
		while ( !fp && st.NextToken(templateDir) ) {
			// get localized template
			Trace("current template path [" << templateDir << "]");
			fp = tryopen(absoluteFileName, rootpath, templateDir, localizedPath, filename);
		}
	}

	// try non localized template
	st.Reset(); // try again
	while ( !fp &&  st.NextToken(templateDir)  ) {
		Trace("current template path [" << templateDir << "]");
		// get master template
		fp = tryopen(absoluteFileName, rootpath, templateDir, 0, filename);
	}
	Trace("[" << absoluteFileName << (fp ? "] " : "] could not be ") << "opened");
	return fp;
}

void LocalizationUtils::EmptyLanguageMap()
{
	// Only used for test purposes, IS NOT THREAD SAVE!
	Anything null;
	fgLanguageMap = null;
} // EmptyLanguageMap

