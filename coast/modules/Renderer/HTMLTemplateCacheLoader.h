/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLTemplateCacheLoader_H
#define _HTMLTemplateCacheLoader_H

#include "config_renderer.h"

//---- WDModule include -------------------------------------------------
#include "WDModule.h"
#include "CacheHandler.h"

class TemplateParser;

//---- TemplatesCacheModule -----------------------------------------------------------
//! Used to load HTML-pages at startup and cache them for later access
//! <BR>Configuration: -> check HTMLTemplateCacheLoader for the configuration
class EXPORTDECL_RENDERER TemplatesCacheModule : public WDModule
{
public:
	TemplatesCacheModule(const char *name);
	~TemplatesCacheModule();

	virtual bool Init(const ROAnything config);
	virtual bool Finis();
};

//--- HTMLTemplateCacheLoader ------------------------------------------------------
//! Policy implementation to cache HTML files
/*!
<B>Configuration:</B>
<B>slots to define within Config.any</B><PRE>
/HTMLTemplateConfig {
	/TemplateDir		String		optional, default "config/HTMLTemplates", relative path to WD_ROOT in which to look for html-files
									use ":" to delimit multiple path segments
	/LanguageDirMap {				mandatory, specify the supported languages, if not specified the caching will not be active
		/D				String		optional, locale specific path to html-files, will be appended to TemplateDir
		/E				String		...
		/F				String		...
		/I				String		...
		...
	}
	/ParserConfig {					optional, global TemplateParser options, see TemplateParser for available options
		...
	}
}</PRE>
example configuration of Config.any
<PRE>
/HTMLTemplateConfig {
	/TemplateDir		"config/HTMLTemplates:config/SpecialTemplates"
	/LanguageDirMap {
		/D				"Localized_D"
		/E				"Localized_E"
		/F				"Localized_F"
		/I				"Localized_I"
	}
}</PRE>
With the example above every html-file in the directories:
<PRE>
config/HTMLTemplates/Localized_D
config/HTMLTemplates/Localized_E
config/HTMLTemplates/Localized_F
config/HTMLTemplates/Localized_I
config/SpecialTemplates/Localized_D
config/SpecialTemplates/Localized_E
config/SpecialTemplates/Localized_F
config/SpecialTemplates/Localized_I
</PRE>
will be loaded into the cache.
*/
class EXPORTDECL_RENDERER HTMLTemplateCacheLoader : public CacheLoadPolicy
{
public:
	HTMLTemplateCacheLoader(TemplateParser *parser, const ROAnything config = ROAnything())
		: fParser(parser)
		, froaConfig(config)
	{ }
	~HTMLTemplateCacheLoader() { }

	//! parse filename with extension .html for insertion into cache
	virtual Anything Load(const char *filename);
	virtual void BuildCache(Anything &cache, istream &reader, const char *filenamebody = "NO_FILE");

private:
	TemplateParser *fParser;
	const ROAnything froaConfig;
	friend class HTMLCacheLoaderTest;
};

//--- HTMLTemplateCacheBuilder ------------------------------------------------------
//! Worker class to load the html-files using the given CacheHandler and CacheLoadPolicy
class EXPORTDECL_RENDERER HTMLTemplateCacheBuilder
{
public:
	HTMLTemplateCacheBuilder() { }
	~HTMLTemplateCacheBuilder() { }

	void BuildCache(const ROAnything config);

private:
	void CacheDir(const char *filepath, CacheHandler *cache, CacheLoadPolicy *htcl, const ROAnything langDirMap, Anything &fileNameMap);
	void CacheDir(const char *filepath, CacheHandler *cache, CacheLoadPolicy *htcl, const char *langDir, Anything &fileNameMap);
};

//--- HTMLTemplateNameMapLoader ------------------------------------------------------
//! Dummy policy to cache the TemplateName to FilesystemFile map
class EXPORTDECL_RENDERER HTMLTemplateNameMapLoader : public CacheLoadPolicy
{
public:
	HTMLTemplateNameMapLoader(const Anything &nameMap) : fNameMap(nameMap, Storage::Global())  { }
	virtual ~HTMLTemplateNameMapLoader() { }
	virtual Anything Load(const char *)	{
		return fNameMap;
	}

private:
	Anything fNameMap;
};

#endif
