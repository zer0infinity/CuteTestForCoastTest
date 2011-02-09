/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "BasicTableRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "Timers.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- RowAccessor -------------------------------------------------------------------
// utility class used by BasicTableRenderer to cache the renderers/configuration data
// used for the creation of table rows

class RowAccessor
{
	// CAUTION: objects of this class are used as a better struct..
	// they are not initialized properly (using a constructor),
public:
	void Init(Allocator *a);

	// accessors
	void SetConfig(Anything &config) {
		fConfig = config;
	}
	const ROAnything GetConfig() const {
		return ROAnything(fConfig);
	}

	void SetFormat(Anything &format) {
		fFormat = format;
	}
	const ROAnything GetFormat() const {
		return fFormat;
	}

	void SetNewlineOn() {
		fInsertNewline = true;
	}
	bool GetNewline() {
		return fInsertNewline;
	}

	void SetBoldOn() {
		fIsBold = true;
	}
	bool IsBold() {
		return fIsBold;
	}

	void SetTeletypeOn() {
		fIsTeletype = true;
	}
	bool IsTeletype() {
		return fIsTeletype;
	}

	void SetFontOptions(String &options) {
		fFontOptions = options;
	}
	const String &GetFontOptions() const {
		return fFontOptions;
	}

	void SetRenderer(Renderer *r) {
		fRenderer = r;
	}
	Renderer *GetRenderer() const {
		return fRenderer;
	}

	RowAccessor &operator=(const RowAccessor &aka);

	static void *operator new[](size_t size, Allocator *a) throw ();
	static void operator delete[](void *ptr);

protected:
	const Anything GetIntConfig() const {
		return fConfig;
	}
	const Anything GetIntFormat() const {
		return fFormat;
	}
private:
	Anything fConfig;
	Anything fFormat;
	bool fResetFont;
	String fFontOptions;
	bool fInsertNewline;
	bool fIsBold;
	bool fIsTeletype;
	Renderer *fRenderer;
};

void RowAccessor::Init(Allocator *a)
{
	fConfig.SetAllocator(a);
	fFormat.SetAllocator(a);
	fFontOptions.SetAllocator(a);
}

RowAccessor &RowAccessor::operator=(const RowAccessor &ra)
{
	fConfig = ra.GetIntConfig();
	fFormat = ra.GetIntFormat();
	fFontOptions = ra.GetFontOptions();
	return *this;
}

void *RowAccessor::operator new[](size_t size, Allocator *a) throw()
{
	if (a) {
		void *mem = a->Calloc(1, size + sizeof(Allocator *));
		((Allocator **)mem)[0L] = a;				// remember address of responsible Allocator
		return (char *)mem + sizeof(Allocator *);
	} else {
		return 0;
	}
}

void RowAccessor::operator delete[](void *ptr)
{
	if (ptr) {
		void *realPtr = (char *)ptr - sizeof(Allocator *);
		Allocator *a = ((Allocator **)realPtr)[0L];	// retrieve Allocator
		a->Free(realPtr);
	}
	return;
}

//---- BasicTableRenderer -------------------------------------------------------------------

// REMINDER: HTML does not support to set the font size in tables
//           on a per table or even per row basis... it is therefore
//           necessary to set the font for each element of a table... (which
//           of course is completely braindead..

RegisterRenderer(BasicTableRenderer);

BasicTableRenderer::BasicTableRenderer(const char *name) : Renderer(name)
{
}

bool BasicTableRenderer::DoHandleAdditionalRenderers(ROAnything &, ROAnything &,
		Renderer **, Anything &)
{
	return false;
}

bool BasicTableRenderer::CanUseInvertedHeaders(Context &ctx)
{
	// old Netscape 2 browsers do not support background colors in tables
	// ...inverted headers are then invisible!
	bool invertHeaders = (ctx.Lookup("UseBgColors", 1L) != 0);	// default is 'on'

	String clientBrowser = ctx.Lookup("header.USER-AGENT", "");
	if ( clientBrowser.Contains("Mozilla/2") >= 0) {
		invertHeaders = false;
	}
	return invertHeaders;
}

void BasicTableRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	MethodTimer(BasicTableRenderer.RenderAll, "", ctx); 		// support for timing
	StartTrace(BasicTableRenderer.Render);
	TraceAny(config, "config");

	bool invertHeaders = CanUseInvertedHeaders(ctx);

	Anything reportData;

	long idx = config.FindIndex("DataSource");
	if (idx >= 0) {
		Anything tmpStore = ctx.GetTmpStore();
		const char *slot = config[idx].AsCharPtr("foo");

		if (!tmpStore.LookupPath(reportData, slot)) {
			String msg("BasicTableRenderer::RenderAll()...invalid data source <");
			msg << slot << ">";
			SystemLog::Error(msg);
		}
	}

	// put the table in an enclosing table.. so that a border
	// may be drawn around the whole thing...

	reply << "<TABLE";
	ROAnything format, f;
	if (config.LookupPath(f, "FormatTable")) {
		format = GetFormat(config, ctx, f.AsCharPtr(""), "TableFormats", "GlobalTableFormats");
		ROAnything outerOptions;
		if (format.LookupPath(outerOptions, "OuterTableOptions")) {
			reply << " " << outerOptions.AsString();
		}
	}
	reply << ">\n<TR><TD>\n";

	// create an inner table.. (which will contain the data)

	reply << "<TABLE";

	if (format.IsDefined("Options")) {	// options for inner table
		PrintOptions3(reply, ctx, format);
	}

	if (invertHeaders) {
		// only if the browser supports background colors in tables
		// the table headers are printed in white on a colored background

		ROAnything bgrdColor;
		if (format.LookupPath(bgrdColor, "HdrBackgrdColor")) {
			reply << " BGCOLOR=#" << bgrdColor.AsString();
		}
	}
	reply << ">\n";

	int row = 0;

	// print the header of the table

	ROAnything outline;
	if (config.LookupPath(outline, "Outline")) {

		ROAnything repHeader;
		if (outline.LookupPath(repHeader, "Header") && !repHeader.IsNull()) {
			ROAnything dummy;		// colors

			long rowSize = 0;
			RowAccessor *accessors = SetupRowAccessors(config, ctx, repHeader, &rowSize, true, invertHeaders);
			PrintRow(row, reply, ctx, accessors, rowSize, dummy, invertHeaders);

			CleanupRowAccessors(accessors, rowSize);
		}

		// print the body of the table

		if (!reportData.IsNull()) {
			ROAnything sectionConfigs;
			if (outline.LookupPath(sectionConfigs, "Body")) {
				for (long i = 0, szs = sectionConfigs.GetSize(); i < szs; ++i) {
					ROAnything sectionConfig = sectionConfigs[i];
					ROAnything key;
					if (sectionConfig.LookupPath(key, "DataSource")) {
						Anything data;
						if (reportData.LookupPath(data, key.AsCharPtr())) {
							PrintBodySection(config, row, reply, ctx, sectionConfig, data);

						} else {
							String msg("BasicTableRenderer: no data was found for table section");
							msg << " (" << key.AsCharPtr() << ")";
							SystemLog::Warning(msg);
						}
					} else {
						SystemLog::Error("BasicTableRenderer: no DataSource was specified for this section");
					}
				}
			}
		}
	} else {
		SystemLog::Error("BasicTableRenderer.. misconfiguration: no Outline section was defined");
	}
	reply << "</TABLE>\n</TD></TR>\n</TABLE>\n";
}

void BasicTableRenderer::DoPrintEmptySection(std::ostream &reply, Context &ctx,
		const ROAnything &config, const ROAnything &sectionConfig,  int &row)
{
}

void BasicTableRenderer::PrintBodySection(const ROAnything &config, int &row, std::ostream &reply,
		Context &ctx, const ROAnything &sectionConfig, const Anything &sectionData)
{
	// a table may consist of several sections: these individual sections
	// are composed of structurally similar rows

	Anything tmpStore = ctx.GetTmpStore();

	// determine format for this row (bg color & use bold font?)
	ROAnything rowFmt, f;
	if (sectionConfig.LookupPath(f, "FormatRow")) {
		rowFmt = GetFormat(config, ctx, f.AsCharPtr(""), "RowFormats", "GlobalTableRowFormats");
	}

	ROAnything rowColors;
	rowFmt.LookupPath(rowColors, "RowColors");

	bool isBold = false;
	ROAnything style;
	if (rowFmt.LookupPath(style, "Style") && (style.AsString() == "bold")) {
		isBold = true;
	}

	long     dataSize = 1;

	//check if record-count == 1 or n
	if ((sectionData.GetType() == AnyArrayType) &&
		(sectionData[0L].GetType() == AnyArrayType)) {

		dataSize = sectionData.GetSize();	// total number of rows
	}

	if (!dataSize) {
		DoPrintEmptySection(reply, ctx, config, sectionConfig, row);
	} else {
		long rowSize = 0;		// number of columns

		ROAnything itemConfigs;

		if (sectionConfig.LookupPath(itemConfigs, "Items")) {

			RowAccessor *accessors =
				SetupRowAccessors(config, ctx, itemConfigs, &rowSize, isBold, true);

			for (long i = 0; i < dataSize; ++i) {
				// 'sectionData' is expected to either contain an array
				// of structurally equivalent accociative Anything arrays
				// (each of which is used to create one row of the table)
				// or exactly one associative array of data which
				// is used to create one row

				Anything item;
				if (sectionData[0L].GetType() == AnyArrayType) {
					item = sectionData[i];
				} else {
					item = sectionData;
				}

				tmpStore["RowData"] = item;
				DoSetRowDataCache(item);

				tmpStore["RowIndex"] = i;

				PrintRow(row, reply, ctx, accessors, rowSize, rowColors);
			}
			CleanupRowAccessors(accessors, rowSize);
		}
	}
}

RowAccessor *BasicTableRenderer::SetupRowAccessors(const ROAnything &conf, Context &ctx,
		const ROAnything &rowMeta, long *rowSize, bool isBold, bool invertHeaders)
{
	// build cache for faster access to structurally identical rows

	(*rowSize) = rowMeta.GetSize();

	Allocator *wdallocator = Coast::Storage::Global();
	RowAccessor *accessors = new (wdallocator) RowAccessor[(*rowSize)];

	for (long i = 0; i < (*rowSize); ++i) {
		ROAnything elmt = rowMeta[i];				// meta for one element
		ROAnything fmt, f;
		if (elmt.LookupPath(f, "FormatCell")) {
			fmt = GetFormat(conf, ctx, f.AsCharPtr(""), "CellFormats", "GlobalTableCellFormats");
		}

		RowAccessor &accessor = accessors[i];
		accessor.Init(wdallocator);

		// ******** determine settings *********

		if (elmt.IsDefined("Newline")) {
			accessor.SetNewlineOn();
		}

		if (isBold || fmt.IsDefined("Bold")) {
			accessor.SetBoldOn();
		}

		Anything mutableFmt = fmt.DeepClone();
		accessor.SetFormat(mutableFmt);

		if (fmt.IsDefined("Teletype")) {
			accessor.SetTeletypeOn();
		}

		long fmtIdx = fmt.FindIndex("Font");
		if (fmtIdx >= 0 || fmt.IsDefined("Invert")) {

			String options("<FONT ");

			if (fmtIdx >= 0) {
				options.Append(fmt[fmtIdx].AsCharPtr());
			}

			// use white text only if browser supports background color...
			if (invertHeaders && fmt.IsDefined("Invert")) {
				options.Append(" COLOR=#FFFFFF");
			}

			options.Append(">");

			accessor.SetFontOptions(options);

		}

		// ********* determine Renderer / config *********
		// special keywords are accepted for hardwired use of renderers
		// "UseColumbiaItem", "RendererSpecification" and "UseFastLookup"

		ROAnything a;
		if (elmt.LookupPath(a, "RendererSpecification")) {
			// arbitrary render is used / a simple string is always used
			// for a context lookup!

			if (a.GetType() == AnyCharPtrType) {
				// no renderer set.. -> lookup
				Anything mutableA = a.DeepClone();
				accessor.SetConfig(mutableA);
			} else {

				long idx = a.FindIndex("Type");
				if (idx >= 0) {
					const char *type = a[idx].AsCharPtr(0);
					if (type) {
						Renderer *r = FindRenderer(type);
						accessor.SetRenderer(r);
						if (r) {
							idx = a.FindIndex("Data");
							if (idx >= 0) {
								Anything mutableData = a[idx].DeepClone();
								accessor.SetConfig(mutableData);
							} else {
								Anything mutableA = a.DeepClone();
								accessor.SetConfig(mutableA);
							}
						} else {
							String msg("BasicTableRenderer: Type: ");
							msg << type << "not found";
							SystemLog::Error(msg);
						}
					}
				}
			}

		} else  if (elmt.LookupPath(a, "UseFastLookup")) {

			// page specific lookup renderer
			if (a.GetType() == AnyCharPtrType) {
				Anything config;
				//	config["LookupName"]= a.AsCharPtr();
				config.Append(a.AsCharPtr());

				accessor.SetRenderer(FindRenderer("FastLookup"));
				accessor.SetConfig(config);
			}

		} else {
			// subclasses may know about additional renderers
			Renderer *r = 0;
			Anything config;
			if (DoHandleAdditionalRenderers( elmt, fmt, &r, config)) {
				accessor.SetRenderer(r);
				accessor.SetConfig(config);

			} else {
				SystemLog::Error("BasicTableRenderer::DoHandleAdditionalRenderers() unsupported item !");
			}
		}
	}
	return accessors;
}

void BasicTableRenderer::CleanupRowAccessors(RowAccessor *accessors, long size)
{
	if ( accessors ) {
		delete [] accessors;
	}
}

void BasicTableRenderer::PrintRow(int &row, std::ostream &reply, Context &ctx,
								  RowAccessor *accessors, long rowSize, ROAnything &rowColors, bool invertHeaders)
{
	PrintNewLine(row, reply, rowColors, invertHeaders);

	for (long i = 0; i < rowSize; ++i) {
		RowAccessor &accessor = accessors[i];

		if (accessor.GetNewline()) {							// create new line
			PrintNewLine(row, reply, rowColors, invertHeaders);
		}

		// set cell attributes
		if (accessor.IsBold()) {								// start tag
			PrintOptions(reply, "TH", accessor.GetFormat());
		} else {
			PrintOptions(reply, "TD", accessor.GetFormat());
		}

		if (accessor.IsTeletype()) {							// start teletype tag
			reply << ("<TT>");
		}

		String fontOptions = accessor.GetFontOptions();
		if (fontOptions.Length()) {						// start font tag
			reply << fontOptions;
		}

		PrintElement(reply, ctx, accessors[i]);						// render element

		if (fontOptions.Length()) {							// end font tag
			reply << ("</FONT>");
		}

		if (accessor.IsTeletype()) {							// end teletype tag
			reply << ("</TT>");
		}

		if (accessor.IsBold()) {								// end tag
			reply << ("</TH>\n");
		} else {
			reply << ("</TD>\n");
		}
	}
	reply << "</TR>\n";
}

void BasicTableRenderer::PrintElement(std::ostream &reply, Context &ctx, RowAccessor &accessor)
{
	Renderer *r = accessor.GetRenderer();

	if (r) {
		r->RenderAll(reply, ctx, accessor.GetConfig());
	} else {
		// e.g. simple strings
		Render(reply, ctx, accessor.GetConfig());	// lookup
	}
}

void BasicTableRenderer::PrintNewLine(int &row, std::ostream &reply, ROAnything &rowColors, bool invertHeaders)
{
	if (!rowColors.IsNull()) {
		const char *rowColor = GetColor(row++, rowColors, invertHeaders);
		if (rowColor) {
			reply << "<TR BGCOLOR=\"#" << rowColor << "\">\n";
		} else {
			reply << ("<TR>\n");
		}
	} else {
		reply << ("<TR>\n");
	}
}

ROAnything BasicTableRenderer::GetFormat(const ROAnything &config, Context &ctx,
		const char *fmtName, const char *cfgSlot, const char *dftSlot)
{
	// determine format to be used for rows (bg colors & bold)
	ROAnything fmt, rowFmt;
	if (config.LookupPath(rowFmt, cfgSlot)) {
		if (rowFmt.LookupPath(fmt, fmtName) && !fmt.IsNull()) {
			return fmt;
		}
	}

	// check global settings
	rowFmt = ctx.Lookup(dftSlot);
	if (rowFmt.LookupPath(fmt, fmtName))
		;

	return fmt;
}

const char *BasicTableRenderer::GetColor(long index, const ROAnything &rowcolors, bool invertHeaders)
{
	if (invertHeaders) {
		return rowcolors[index % rowcolors.GetSize()].AsCharPtr(0);
	}
	return 0;
}

//---- SequenceRenderer --------------------------------------------------------------

RegisterRenderer(SequenceRenderer);

SequenceRenderer::SequenceRenderer(const char *name) : Renderer(name) {}

void SequenceRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	Renderer::Render(reply, ctx, config);
}
