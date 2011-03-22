/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTMLTemplateRenderer.h"
#include "Timers.h"
#include "LocalizationUtils.h"
#include "HTMLTemplateCacheLoader.h"
#include "TemplateParser.h"
#include "StringStream.h"

//---- HTMLTemplateRenderer ----------------------------------------------------
RegisterRenderer(HTMLTemplateRenderer);
ROAnything HTMLTemplateRenderer::fgTemplates;
ROAnything HTMLTemplateRenderer::fgNameMap;

HTMLTemplateRenderer::HTMLTemplateRenderer(const char *name) : Renderer(name)
{
}
void HTMLTemplateRenderer::BuildCache(const ROAnything config)
{
	StartTrace(HTMLTemplateRenderer.BuildCache);

	HTMLTemplateCacheBuilder htcb;

	htcb.BuildCache(config);
	CacheHandler *cache = CacheHandler::Get();
	if (cache) {
		fgTemplates = cache->GetGroup("HTML");
		TraceAny(fgTemplates, "Cache Templates");
		fgNameMap = cache->Get("HTMLMappings", "HTMLTemplNameMap");
		TraceAny(fgNameMap, "Cache Map");
	}
}

TemplateParser *HTMLTemplateRenderer::GetParser()
{
	return new TemplateParser;
}

void HTMLTemplateRenderer::RenderAll(std::ostream &reply, Context &context, const ROAnything &args)
{
	StartTrace(HTMLTemplateRenderer.RenderAll);
	TraceAny(args, "args");
	ROAnything theRendererConfig, roaParserConfig;
	Anything rendererConfig;

	// check if we have special parser configuration to use
	if ( !args.LookupPath(roaParserConfig, "ParserConfig") ) {
		context.Lookup("HTMLTemplateConfig.ParserConfig", roaParserConfig);
	}

	if (args.IsDefined("TemplateName")) {
		String filename;
		Renderer::RenderOnString(filename, context, args["TemplateName"]);
		if (filename.Length() == 0) {
			SystemLog::Error("HTMLTemplateRenderer::RenderAll: cannot find template name");
		} else {
			const char *lang = context.Language();		// cache language flag for localized strings
			TraceAny(fgNameMap, "current cache map");
			String pathUsed = fgNameMap[filename][lang].AsCharPtr("");
			Trace("Template file :" << filename << ";" << pathUsed << ";" << lang );
			if ( pathUsed.Length() > 0 ) { // we got the filekey
				theRendererConfig = fgTemplates[pathUsed];
			} else {
				std::istream *fp = LocalizationUtils::OpenStream(context, filename, pathUsed);
				if (fp) {
					std::istream &reader = *fp;
					{
						MethodTimer(HTMLTemplateRenderer.RenderAll, filename, context);
						TemplateParser *tp = GetParser();
						rendererConfig = tp->Parse(reader, filename, 1L, rendererConfig.GetAllocator(), roaParserConfig);
						delete tp;
						theRendererConfig = rendererConfig;
					}
					delete fp;
				} else {
					String logMsg("HTMLTemplateRenderer::RenderAll: cannot open file ");
					logMsg << filename << ".html";
					SystemLog::Error(logMsg);
				}
			}
		}
	} else {
		// look for /Template
		MethodTimer(HTMLTemplateRenderer.RenderAll, "Template", context);
		ROAnything templ;
		if (args.IsDefined("Template")) {
			templ = args["Template"];
		} else {
			// bail out.
			OStringStream str;
			str << "HTMLTemplateRenderer::RenderAll: neither Template nor TemplateName is defined :";
			args.PrintOn(str); // log what we've got
			SystemLog::Error(str.str());
			return;
		}

		String buf;
		for (long i = 0, size = templ.GetSize(); i < size; ++i) {
			// PS: only append anonymous entries to allow for future parameters
			// PS: this will take care of things like /Options /Method in subclass FormRenderer
			if ( 0 == templ.SlotName(i) ) {
				buf.Append(templ[i].AsCharPtr());
			}
		}
		IStringStream reader(buf);
		TemplateParser *tp = GetParser();
		rendererConfig = tp->Parse(reader, "from config", 1L, rendererConfig.GetAllocator(), roaParserConfig);
		delete tp;
		theRendererConfig = rendererConfig;

	}
	Render(reply, context, theRendererConfig);
}
