/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ImageRenderers.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- ImageLocationRenderer ----------------------------------------------------------------
RegisterRenderer(ImageLocationRenderer);

ImageLocationRenderer::ImageLocationRenderer(const char *name) : Renderer(name)
{
}

bool ImageLocationRenderer::RenderPathAndFilename(ostream &reply, Context &c,
		const ROAnything &config)
{
	ROAnything name;
	if (!config.LookupPath(name, "ImageName")) {
		if (config.GetType() == Anything::eArray) {
			name = config[0L];
		}
	}

	if (!name.IsNull()) {
		ROAnything path;
		Anything dft("/");

		if (!config.LookupPath(path, "ImagePath")) {
			path = c.Lookup("ImagePath");
			if (path.IsNull()) {
				path = dft;
			}
		}
		Render(reply, c, path);
		Render(reply, c, name);

		return true;
	}
	return false;
}

void ImageLocationRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(ImageLocationRenderer.Render);
	TraceAny(config, "config");

	RenderPathAndFilename(reply, c, config);
}

//---- ImageRenderer ----------------------------------------------------------------

RegisterRenderer(ImageRenderer);

ImageRenderer::ImageRenderer(const char *name) : ImageLocationRenderer(name)
{
}

void ImageRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(ImageRenderer.Render);
	TraceAny(config, "config");

	reply << ("<img src=\"");

	RenderPathAndFilename(reply, c, config);

	reply << ("\"");
	PrintOptions3(reply, c, config);
	reply << ("/>");
}

//---- FigureRenderer --------------------------------------------------------------

RegisterRenderer(FigureRenderer);

FigureRenderer::FigureRenderer(const char *name) : ImageRenderer(name) {}

void FigureRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(FigureRenderer.Render);
	TraceAny(config, "config");

	Anything newConfig = config.DeepClone();	// modified for ImageRenderer
	String caption;

	ROAnything cap;
	if (config.LookupPath(cap, "Caption")) {
		RenderOnString(caption, c, cap);
	}

	if (!config.IsDefined("Options")) {
		// default: image has border, text that is displayed if no image is loaded
		// is the same as the image caption

		String options(" border=1 alt=\"");
		options << caption << "\"";
		newConfig["Options"] = options;
	}

	// CAPTION does not seem to work properly on all browsers.. workaround used
	reply << "<table cellspacing=0 cellpadding=0>\n";
	reply << "<tr><td>";
	ImageRenderer::RenderAll(reply, c, newConfig);
	reply << "</td></tr>\n";
	reply << "<tr><td><font size=\"-1\">" << caption << "</font></td></tr></table>\n";
}
