/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TABLERENDERER_H
#define _TABLERENDERER_H

#include "config_renderer.h"
#include "Renderer.h"

//---- TableRenderer -------------------------------------------------------------------
//! <B>Preconfigured Table renderer, some things are hardcoded</B>
/*!
<B>Configuration:</B><PRE>
{
	/DataSource		String		mandatory, name of slot in TempStore which contains the table data
	/Columns {		Anything
		{			Anything	optional, table columns
			/Title {			optional, columnheader
				/Options {		optional, columnheader options for TH tag
					...
				}
				/Render		Rendererspec	mandatory, how to render the title name
			}
			/Body {
				/Options {		optional, column options for TD tag
					...
				}
				/Render		Rendererspec	mandatory, how to render the columndata, columndata gets stored per row in slot RowData of TempStore and RowIndex contains the rows index beginning from 0
			}
		}
		...
	}
	/RowColors {	Anything	optional, background color for rows, all listed colors will be used in cycling order
		color1		String		optional, bg color in hexadecimal notation
		color2		String		optional, bg color in hexadecimal notation
		...
	}
	/Rows			Long		optional, default 1000, number of lines per page
	/Options {		Anything	optional, options for the inner table
		/Key		Value
		...
	}
}
global config from Context:
{
	/TitleBGColor	String		optional, hexadecimal value of background color, eg. FFFFFF for white
	/TitleFGColor	String		optional, hexadecimal value of foreground color, eg. 000000 for black
	/ListNavTable {	Anything	optional, specify if you want to have Next / Prev buttons for table-data navigation
		/Next		Rendererspec	optional, spec how to render the next button, the action of the surrounding mini-form is Next
		/Prev		Rendererspec	optional, spec how to render the previous button, the action of the surrounding mini-form is Prev
	}
}
config from Query:
{
	/StartPos		Long		optional, default 0, index from where to start in DataSource, passed from mini-form when using ListNavTable
}
</PRE>
\note Following things of the outer table are hardcoded: BORDER=1 ALIGN=CENTER CELLPADDING=2 BGCOLOR=#BEBEBE
*/
class EXPORTDECL_RENDERER TableRenderer : public Renderer
{
public:
	TableRenderer( const char *name );

	void RenderAll(ostream &reply, Context &c, const ROAnything &config);

private:
	void PrintRow(ostream &reply, Context &c, const ROAnything &row);
	const char *GetColor(long index, const ROAnything &config);
	void PrintControls(ostream &reply, Context &c, long start, long rowsperpage, long size);
	void PrintHeader(ostream &reply, Context &c, const ROAnything &tbColHeader);
};

//---- ItemRenderer ---------------------------------------------------------------
//! <B>ItemRenderer for use with Table renderer</B>
/*!
<B>Configuration:</B><PRE>
{
	/Attr		ColumnName	String	mandatory, name of the column to get from TmpStore.RowData, unnamed slots not possible, special 'name' @Index returns the row index
}
</PRE>
*/
class EXPORTDECL_RENDERER ItemRenderer : public Renderer
{
public:
	ItemRenderer(const char *name);

	void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
