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
//! <B>Used to load HTML-pages at startup and cache them for later access</B>
//! <BR>Configuration: -> check HTMLTemplateCacheLoader for the configuration
class EXPORTDECL_RENDERER TemplatesCacheModule : public WDModule
{
public:
	TemplatesCacheModule(const char *name);
	~TemplatesCacheModule();

	virtual bool Init(const Anything &config);
	virtual bool Finis();
};

//--- HTMLTemplateCacheLoader ------------------------------------------------------
//! <B>Policy implementation to cache HTML files</B>
/*!
<B>Configuration:</B>
<B>slots to define within Config.any</B><PRE>
/TemplateDir		String		optional, default "config/HTMLTemplates", relative path to WD_ROOT in which to look for html-files
								use ":" to delimit multiple path segments
/LanguageDirMap {				mandatory, specify the supported languages, if not specified the caching will not be active
	/D				String		optional, locale specific path to html-files, will be appended to TemplateDir
	/E				String		...
	/F				String		...
	/I				String		...
	...
}
</PRE>
example configuration of Config.any
<PRE>
/TemplateDir		"config/HTMLTemplates:config/SpecialTemplates"
/LanguageDirMap {
	/D				"Localized_D"
	/E				"Localized_E"
	/F				"Localized_F"
	/I				"Localized_I"
}
</PRE>
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
	HTMLTemplateCacheLoader(TemplateParser *parser): fParser(parser) { }
	~HTMLTemplateCacheLoader() { }

	//! parse filename with extension .html for insertion into cache
	virtual Anything Load(const char *filename);
	virtual void BuildCache(Anything &cache, istream &reader, const char *filenamebody = "NO_FILE");

private:
	TemplateParser *fParser;
	friend class HTMLCacheLoaderTest;
};

//--- HTMLTemplateCacheBuilder ------------------------------------------------------
//! <B>Worker class to load the html-files using the given CacheHandler and CacheLoadPolicy</B>
class EXPORTDECL_RENDERER HTMLTemplateCacheBuilder
{
public:
	HTMLTemplateCacheBuilder() { }
	~HTMLTemplateCacheBuilder() { }

	void BuildCache(const Anything &config);

private:
	void CacheDir(const char *filepath, CacheHandler *cache, CacheLoadPolicy *htcl, const Anything &langDirMap, Anything &fileNameMap);
	void CacheDir(const char *filepath, CacheHandler *cache, CacheLoadPolicy *htcl, const char *langDir, Anything &fileNameMap);
};

//--- HTMLTemplateNameMapLoader ------------------------------------------------------
//! <B>Dummy policy to cache the TemplateName to FilesystemFile map</B>
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
