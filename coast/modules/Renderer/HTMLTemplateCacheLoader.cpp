/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTMLTemplateCacheLoader.h"

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "System.h"
#include "SysLog.h"
#include "TemplateParser.h"
#include "HTMLTemplateRenderer.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- TemplatesCacheModule -----------------------------------------------------------
RegisterModule(TemplatesCacheModule );

TemplatesCacheModule::TemplatesCacheModule(const char *name) : WDModule(name)
{
}

TemplatesCacheModule::~TemplatesCacheModule()
{
}

bool TemplatesCacheModule::Init(const Anything &config)
{
	HTMLTemplateRenderer::BuildCache(config);
	return true;
}

bool TemplatesCacheModule::Finis()
{
	// PS: here we should flush the cache, shouldn't we?
	// we do nothing here because CacheHandler flushes his cache
	// when reset
	return true;
}

//--- HTMLTemplateCacheLoader --------------------------------------------------
Anything HTMLTemplateCacheLoader::Load(const char *key)
{
	StartTrace1(HTMLTemplateCacheLoader.Load, "key: " << key);
	Anything cache(Storage::Global());
	istream *fp = System::OpenIStream(key, (const char *)"html");
	if (fp) {
		istream &reader = *fp;
		BuildCache(cache, reader, key);
		delete fp;
	} else {
		String logMsg("HTMLTemplateRenderer::RenderAll: cannot open file ");
		logMsg << key << ".html";
		SysLog::Error(logMsg);
	}
	SubTraceAny(TraceCache, cache, "Cache:");
	return cache;
}

void HTMLTemplateCacheLoader::BuildCache(Anything &cache, istream &reader, const char *filename)
{
	StartTrace(HTMLTemplateCacheLoader.BuildCache);
	if (fParser) {
		cache = fParser->Parse(reader, filename, 1L, cache.GetAllocator());
	} else {
		SysLog::Error("HTMLTemplateCacheLoader::BuildCache: OOPS Parser undefined");
	}
}

//--- HTMLTemplateCacheBuilder ------------------------------------------------------
void HTMLTemplateCacheBuilder::BuildCache(const Anything &config)
{
	StartTrace(HTMLTemplateCacheBuilder.BuildCache);
	SysLog::WriteToStderr("\tBuilding HTML Templates cache");

	Anything langDirMap(config["LanguageDirMap"]);

	StringTokenizer st(config["TemplateDir"].AsCharPtr("config/HTMLTemplates"), ':');
	String rootDir(System::GetRootDir());
	String filepath;
	String templateDir;
	Anything fileNameMap(Storage::Global());

	CacheHandler *cache = CacheHandler::Get();
	TemplateParser tp;
	HTMLTemplateCacheLoader htcl(&tp);

	while ( st.NextToken(templateDir) ) {
		// cache templates of template dir
		filepath = rootDir;
		filepath << System::Sep() << templateDir;
		System::ResolvePath(filepath);
		CacheDir(filepath, cache, &htcl, langDirMap, fileNameMap);

		// search over localized dirs
		for ( long j = 0; j < langDirMap.GetSize(); j++) {
			//reset filepath
			filepath = rootDir;
			filepath << System::Sep() << templateDir;

			// construct directory name
			filepath << System::Sep() << langDirMap[j].AsCharPtr("");
			System::ResolvePath(filepath);

			CacheDir(filepath, cache, &htcl, langDirMap.SlotName(j), fileNameMap);
		}
	}
	TraceAny(fileNameMap, "FileNameMap after caching pages");

	// install the mapping from file names to absolute pathnames in the cache
	HTMLTemplateNameMapLoader htnml(fileNameMap);
	cache->Load("HTMLMappings", "HTMLTemplNameMap", &htnml);

	SysLog::WriteToStderr(" done\n");
}

void HTMLTemplateCacheBuilder::CacheDir(const char *filepath, CacheHandler *cache, CacheLoadPolicy *htcl, const Anything &langDirMap, Anything &fileNameMap)
{
	StartTrace1(HTMLTemplateCacheBuilder.CacheDir, "cache-path [" << filepath << "]");
	// get all files of this directory
	Anything fileList = System::DirFileList(filepath, "html");
	String fileKey;

	// process all files
	for ( long i = 0; i < fileList.GetSize(); i++ ) {
		const char *file = fileList[i].AsCharPtr("");
		fileKey << filepath << System::Sep() << file;
		// smothen path not to load relative-path files more than once
		System::ResolvePath(fileKey);
		// ignore results, they are stored in the cachehandler anyway
		cache->Load("HTML", fileKey, htcl);
		// store away the name,langKey to fileKey mapping
		for (long j = 0; j < langDirMap.GetSize(); j++) {
			const char *langKey = langDirMap.SlotName(j);
			fileNameMap[file][langKey] = fileKey;
		}
		// reset the filekey
		fileKey = "";
		SysLog::WriteToStderr(".", 1);
	}
}

void HTMLTemplateCacheBuilder::CacheDir(const char *filepath, CacheHandler *cache, CacheLoadPolicy *htcl, const char *langKey, Anything &fileNameMap)
{
	StartTrace1(HTMLTemplateCacheBuilder.CacheDir, "cache-path [" << filepath << "]");
	// get all files of this directory
	Anything fileList = System::DirFileList(filepath, "html");
	String fileKey;

	// process all files
	for ( long i = 0; i < fileList.GetSize(); i++ ) {
		const char *file = fileList[i].AsCharPtr("");
		fileKey << filepath << System::Sep() << file;
		// smothen path not to load relative-path files more than once
		System::ResolvePath(fileKey);
		// ignore results, they are stored in the cachehandler anyway
		cache->Load("HTML", fileKey, htcl);
		// store away the name,langKey to fileKey mapping
		fileNameMap[file][langKey] = fileKey;
		// reset the filekey
		fileKey = "";
		SysLog::WriteToStderr(".", 1);
	}
}
