/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TableRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "FormRenderer.h"
#include "Dbg.h"

//--- c modules used ----------------------------------------------------
#include <cstring>

//---- TableRenderer -------------------------------------------------------------------
RegisterRenderer(TableRenderer);

TableRenderer::TableRenderer(const char *name) : Renderer(name)
{
}

void TableRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &any)
{
	//--- tracing
	StartTrace1(TableRenderer.RenderAll, "");
	TraceAny(any, "Config");

	ROAnything tbColHeader = any["Columns"];
	ROAnything rowColors = any["RowColors"];

	long rowsPerPage = any["Rows"].AsLong(1000);

	long startRow = c.GetQuery()["StartPos"].AsLong(0);

	reply << ("<TABLE BORDER=1 ALIGN=CENTER CELLPADDING=2 BGCOLOR=\"#BE BE BE\">");
	reply << ("<TR><TD>");

	PrintOptions(reply, "TABLE", any);

	PrintHeader(reply, c, tbColHeader);

	Anything eData;
	Anything appTmpStore = c.GetTmpStore();

	if (any.IsDefined("DataSource")) {
		const char *name = any["DataSource"].AsCharPtr(0);
		if (name) {
			// assumption: data of DataSource is always in the TmpStore
			// and never directly built into the config files!
			appTmpStore.LookupPath(eData, name);
		}
	}

	for (long i = startRow, sz = eData.GetSize(); (i < sz) && (i < startRow + rowsPerPage); ++i) {
		const char *rowColor = GetColor(i, rowColors);
		if (rowColor) {
			reply << "<TR BGCOLOR=\"#" << rowColor << "\">";
		} else {
			reply << ("<TR>");
		}
		// these statements wouldn't work if we had a ROAnything for eData
		appTmpStore["RowData"] = eData[i];
		appTmpStore["RowIndex"] = i;

		PrintRow(reply, c, tbColHeader);
		reply << ("</TR>");
	}

	reply << ("</TABLE>");
	reply << ("</TD></TR></TABLE>");

	PrintControls(reply, c, startRow, rowsPerPage, eData.GetSize());
}

void TableRenderer::PrintControls(ostream &reply, Context &c, long start, long rowsperpage, long size)
{
	//--- list navigation elements
	bool hasNextElements = start + rowsperpage < size;
	bool hasPrevElements = start > 0;

	StartTrace1(TableRenderer.PrintControls, "hasNextElements: " << hasNextElements << "hasPrevElements: " << hasPrevElements);

	ROAnything navTable = c.Lookup("ListNavTable");

	PrintOptions(reply, "TABLE", navTable);

	reply << ("<TR>");
	if (hasNextElements || hasPrevElements) {
		if (hasNextElements) {
			reply << ("<TD>");

			Anything a;
			a["Action"] = "Next";
			a["StartPos"] = start + rowsperpage;
			FormRenderer::PrintFormBegin(reply, c, a);
			Render(reply, c, navTable["Next"]);
			FormRenderer::PrintFormEnd(reply, c);

			reply << ("</TD>");
		}
		if (hasPrevElements) {
			reply << ("<TD>");

			Anything a;
			a["Action"] = "Prev";
			a["StartPos"] = start - rowsperpage;
			FormRenderer::PrintFormBegin(reply, c, a);
			Render(reply, c, navTable["Prev"]);
			FormRenderer::PrintFormEnd(reply, c);

			reply << ("</TD>");
		}
	}
	reply << ("</TR>");
	reply << ("</TABLE>");
}

void TableRenderer::PrintHeader(ostream &reply, Context &c, const ROAnything &tbColHeader)
{
	const char *titleBGColor = c.Lookup("TitleBGColor", (char *)0);
	const char *titleFGColor = c.Lookup("TitleFGColor", (char *)0);

	if (titleBGColor) {
		reply << "<TR BGCOLOR=\"#" << titleBGColor << "\">";
	} else {
		reply << ("<TR>");
	}
	for (long r = 0, sz = tbColHeader.GetSize(); r < sz; ++r) {
		ROAnything header = tbColHeader[r]["Title"];
		PrintOptions(reply, "TH", header);
		if (titleFGColor) {
			reply << "<FONT COLOR=\"#" << titleFGColor << "\">";
		}
		Render(reply, c, header["Render"]);
		reply << ("</TH>");
	}

	reply << ("</TR>");
}

const char *TableRenderer::GetColor(long index, const ROAnything &config)
{
	return config[index % config.GetSize()].AsCharPtr(0);
}

void TableRenderer::PrintRow(ostream &reply, Context &c, const ROAnything &row)
{
	StartTrace1(TableRenderer.PrintRow, "");
	TraceAny(row, "Table Row");

	for (long i = 0, sz = row.GetSize(); i < sz; ++i) {
		ROAnything body = row[i]["Body"];
		PrintOptions(reply, "TD", body);
		Render(reply, c, body["Render"]);
		reply << ("</TD>");
	}
}

//---- ItemRenderer ----------------------------------------------------------------

RegisterRenderer(ItemRenderer);

ItemRenderer::ItemRenderer(const char *name) : Renderer(name)
{
}

void ItemRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(ItemRenderer.RenderAll);
	TraceAny(config, "Item Config");

	if (config.IsDefined("Attr")) {
		const char *attr = config["Attr"].AsCharPtr(0);
		if (attr) {
			if ( strcmp("@Index", attr) == 0 ) {
				Anything tmpStore(c.GetTmpStore());
				long ix( tmpStore["RowIndex"].AsLong(-1) );
				if (ix >= 0) {
					reply << (ix);
				}
			} else {
				Anything row( c.GetTmpStore()["RowData"] );
				if (row.IsDefined(attr)) {
					Anything data = row[attr];
					OStringStream form;
					form.precision(2);
					if (data.GetType() == AnyDoubleType) {
						form << data.AsDouble(0.0);
					} else if (data.GetType() == AnyLongType) {
						form << (double) data.AsLong(0);
					} else {
						form << data.AsCharPtr("");
					}
					reply << (form.str());
				}
			}
		}
	}
}
