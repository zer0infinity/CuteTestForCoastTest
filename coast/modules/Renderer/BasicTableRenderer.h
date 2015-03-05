/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BasicTableRenderer_H
#define _BasicTableRenderer_H

#include "Renderer.h"

class RowAccessor;

//---- BasicTableRenderer -------------------------------------------------------------------
//! BasicTableRenderer is a fairly generic renderer used to generate simple HTML-tables.
/*!
\par Configuration
\code
{
	/DataSource		String			mandatory, slot - in the context - that contains the data to be used for the table
	/TableFormats	{...}			optional, an associative array of settings used for the main table settings
	/RowFormats		{...}			optional, an associative array of settings used for table rows
	/CellFormats	{...}			optional, an associative array of settings used for table cells

	/GlobalTableFormats		String	optional, global settings for the main table
	/GlobalTableRowFormats	String	optional, global settings for table rows
	/GlobalTableCellFormats	String	optional, global settings for table cells

	/FormatTable 	String			optional, reference to an entry of /TableFormats or /GlobalTableFormats, options get rendered into TABLE tag

	/Outline {						mandatory, definition of the table structure (i.e. where does the data come from and what renderer is used to display it)
		/Header {					mandatory, definition of the table header section
			{...}					optional, first column
			{...}					optional, second columns
			...						optional, further columns
		}
		/Body {						mandatory, definition of the different sections of the table body
			{						first section
				/DataSource String	mandatory, source of the data to be used for this section
				/FormatRow String	mandatory, format used for this row (background color, etc) is looked up in: /RowFormats or /GlobalTableRowFormats
				/Items {			mandatory, definitions of the renderers used for the cells of this section (exactly same structure as the content of /Header)
					{...}			optional, first column
					{...}			optional, second columns
					...				optional, further columns
				}
			}
			{...}					optional, further sections
		}
	}
}
\endcode

The renderer makes the following assumptions regarding the table tobe generated (which may not be suitable for all purposes..):
- The table consists of a fixed number of columns and a variable number of
rows containing data.
- The data used for the table is obtained from one structured Anything that
is retrieved from the context.
- The table is devided into a header and a body:
- A table starts with one (or several) rows that serve as a header. This header
is followed by the rows containing the actual data. (Notice: The header
defines how many columns there are in the table.)
- Not all the rows of a table need to be of the same structure. However, rows
of the same structure are grouped into blocks, i.e. the body may be divided into
several subsections - each of which contains rows of exactly one type.
(The data for one of the subsections is retrieved from exactly one data source,
i.e. an element of the Anything mentioned in 2). That element - an Anything
array - usually contains one entry for each row to be generated.)
- It is the responsibility of the developer to make sure that the different
rows containing the data each sum up to the same number of columns defined
by the header. (see:r formatting options)

Features:

- Arbitrary renderer specifications may be used to define the content of
individual table cells. (In addition some syntactic sugar allows
special renderers to be used - with less key strokes being involved while
setting up the configuration.)
- Formatting of the table is possible on the table, row and cell level.
- Use of optional background color depending on the browser the client is using.
(i.e. Netscape2.0 does not support background color)

Example configuration:
----------------------
<PRE>
/BasicTableRenderer {
		# specification of the slot - in the context - that contains the data to be
		# used for the table
		/DataSource 	"myData"

		# definitions of different formattings used by the table.. (optional)
		/TableFormats { } # an associative array of settings used for the main table settings
		/RowFormats { } # an associative array of settings used for table rows
		/CellFormats { } # an associative array of settings used for table cells

		# shared settings may also be defined once somewhere in the context - using
		# the special slots (/GlobalTableFormats, /GlobalTableRowFormats and
		# /GlobalTableCellFormats)

		# the main format used for the table (reference to an entry of /TableFormats or
		# /GlobalTableFormats..)
		/FormatTable 		fmtExample

		# definition of the table structure (i.e. where does the data come from
		# and what renderer is used to display it)
		/Outline {
			# definition of the table header section (mandatory)
			/Header {
				{
					# here goes the definition for the first column (see below for options)
				}
				{
					# here goes the definition for the next column
				}
				{
					# here goes the definition for the next column, etc
				}
			}

			# definition of the different sections of the table body (mandatory).
			# the /Body slot must contain an Anything array with one entry
			# for each section of the table
			/Body {
				{
					# row definition used for the first section of the body.. there
					# must be three slots:

					# 1) source of the data to be used for this section
					/DataSource "Detail"

					# 2) format used for this row (background color, etc)
					# is looked up in: /RowFormats or /GlobalTableRowFormats
					/FormatRow fmtExampleRow

					# 3) definitions of the renderers used for the cells of
					# this section (exactly same structure as the content of /Header)
					/Items {
						{
							# here goes the definition for the first column
						}
						{
							# here goes the definition for the next column, etc
						}
					}
				}
				{
					# here might go a different definition for the second section
					# of the body, etc. (whole block is optional)
				}
			}
		}
}
</PRE>
\note Each cell definition (entries in /Items) results in an HTML <TD></TD> pair
being generated. A new row is implicitly started always with the first cell
definition.
Using formatting options, however these basic layout rules may be altered: One
cell may span multiple columns by using the HTML COLSPAN option. Additional
linefeeds may be added by using the /Newline feature (see below).

Example cell format:
---------------------
(these might be defined eiher in /GlobalTableCellFormats - anywhere
in the context - or inside the table specification: /CellFormats )
<PRE>
/CellFormats { # an associative array of settings used for table cells
		/fmtExampleCell {
			/Options 	"COLSPAN=3 ALIGN=LEFT NOWRAP"   # native \<TD\> options
			/Font		"SIZE=-1"						# native \<FONT\> options
			/Invert 	1
			/Bold 		1
		}
}
</PRE>
The string at slot /Options may contain valid options for the HTML \<TD\> tag.
Following the slot /Font valid HTML \<FONT\> tag options - to be used for the
output - may be specified.
If slot /Invert is set to 1 the text color is set to white (only if the clients
browser supports background color). Setting slot /Bold to 1 allows texts to be
displayed using bold face. (the different slots are optional)

Cell format settings may conflict with the output generated by the renderer
that is used to create the content of the cell. Unfortunately - due to the present
specification of HTML - there is no way to always prevent such conflicts.

Hint: Always check the HTML source code in the browser if the layout does not meet
your expectations!

Example cell definition (as may be used in the /Header or /Items sections):
-----------------------
<PRE>
{
		/StoreLookup	"RowData.xyz"
		/FormatCell		fmtExampleCell
}
</PRE>
A cell definition basically may consist of three parts:

- Specification of the renderer to be used for creating the cell content: Using
the slot /RendererSpecification an arbitrary renderer specification may be supplied.
\note For users of LookupRenderers: The current input data is always accessible at slot /RowData in the TmpStore!

Alternatively an abbreviated syntax may be used for selected renderers: e.g. with
the keyword /UseFastLookup currently a PageRelatedLookupRenderer may be used directly
<PRE>
{
		/UseFastLookup       	"inclDate_L"
		/FormatCell 			fmtExampleCell
}
</PRE>
By subclassing BasicTableRenderer, syntactic sugar for other frequently used renderers
might be added too...
- /FormatCell references some cell format that is used for formatting of the cell (see above)
- The special slot /Newline allows to proceed to the next row of the
table at any time: When encountering a cell specification with a defined /Newline
slot, the BasicTableRenderer ends the current line and resumes output on the next
line. This feature is usefull if more than one line of output needs to be produced
using one element of the input data
<PRE>
{
		/Newline True
		/UseFastLookup       	"inclDate_L"
		/FormatCell 			fmtExampleCell
}
</PRE>

Two settings are accepted in a row-format configuration:
- /Style bold: the entries of this row are enclosed into <TH> instead
of <TD> tags.. the entries usually appear in bold face
- /RowColors: the strings contained in this Anything array define the
background colors used for the different rows in this section of the table body.
For each new row of the table the - at that time - next color is retrieved
from /RowFormats. The pattern defined in /RowColors is repeatedly used.
<PRE>
/RowFormats {
	/fmtExampleRow {
		/Style bold
		/RowColors {
			"D3D3D3"
			"FFFFFF"
		}
	}
}
</PRE>

The BasicTableRenderer uses two nested HTML tables. There are three settings
that may be used for a table-format configuration:
- /OuterTableOptions: this string is used as table options for the outer
HTML table. Since the outer table basically contains only a single element -
i.e. the inner table - the most importanht use of these options is to draw
a border arround the whole table.

- /Options: this string is used as table options for the inner table

- /HdrBackgrdColor: this color is used for the background of the table header
(only if the browser of the client supports background colors)
<PRE>
/fmtExampleTable {
	/OuterTableOptions "BORDER=1 CELLPADDING=0 CELLSPACING=0"
	/Options "BORDER=0 CELLPADDING=1 WIDTH=300 CELLSPACING=0"
	/HdrBackgrdColor "0000C0"
}
</PRE>
\note By defining the boolean tag "/UseBgColors 0" somewhere in the context
use of background color may be suppressed.

Example with data & respective renderer configuration:
------------------------------------------------------

data to be rendered by BasicTableRenderer (e.g. stored in TmpStore):
<PRE>
/myData {				# all the data used in some table section
	/firstSection {		# array of data used for one table section
		{
			/accountNumber "111111.0"
			/accountName   "EXAMPLE ACCOUNT 1"
		}
		{
			/accountNumber "111111.1"
			/accountName   "EXAMPLE ACCOUNT 2"
		}
	}
	# there might be more data for other table sections
}
</PRE>
FEATURE: if there is always exactly one data item for a section,
instead of storing that one item in the section array, alternatively
the idividual slots of the item may be put directly into the section array..

example renderer configuration:
<PRE>
/BasicTableRenderer {
		/DataSource 	"myData"

		# definitions of formats used in the table (clearly if several
		# tables are to share the same formats, these should be stored
		# once using the "/Global*" settings.. the following settings
		# could then be ommitted)
		/TableFormats {
			/fmtTable {
				/OuterTableOptions "BORDER=1 CELLPADDING=0 CELLSPACING=0"
				/Options "BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=650"
				/HdrBackgrdColor "0000C0"	# blue background
			}
		}
		/RowFormats {
			/fmtRow {
				/RowColors {	# alternating white & grey background color
					"D3D3D3"
					"FFFFFF"
				}
			}
		}
		/CellFormats {
			/fmtHeaderCell {
				/Options "ALIGN=LEFT NOWRAP"
				/Font "SIZE=-1"
				/Invert 1
			}
			/fmtCell {
				/Options "ALIGN=LEFT"
				/Font	"SIZE=-1"
			}
		}

		# layout definition
		/FormatTable 		"fmtTable"

		/Outline {
			/Header {
				{
					"account #"
					/FormatCell		fmtHeaderCell
				}
				{
					"account name"
					/FormatCell		fmtHeaderCell
				}
			}
			/Body {
				{
					/DataSource "firstSection"
					/FormatRow "fmtRow"
					/Items {
						{
							/StoreLookup	"RowData.accountNumber"
							/FormatCell		fmtCell
						}
						{
							/StoreLookup	"RowData.accountName"
							/FormatCell		fmtCell
						}
					}
				}
			}
		}
}
</PRE>
*/
class BasicTableRenderer : public Renderer
{
public:
	BasicTableRenderer(const char *name);
	void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

protected:
	//! the renderer allows special renderers to be addressed using an
	//! abbreviated syntax in the configuration data (syntactic sugar)..
	//! subclasses may add their own specially handled renderers by overwriting:
	virtual bool DoHandleAdditionalRenderers(ROAnything &elmt, ROAnything &fmt, Renderer **resultRenderer, Anything &resultConfig);

	//! subclasses may implement a specific caching strategy for row data by overwriting
	virtual void DoSetRowDataCache(const Anything &) {}

	//! building blocks for the rendering process
	void PrintBodySection(const ROAnything &config, int &row, std::ostream &reply, Context &c, const ROAnything &repBody, const Anything &repData);
	void PrintRow(int &row, std::ostream &reply, Context &c, RowAccessor *accessors, long rowSize, ROAnything &rowColors, bool invertHeaders = true);
	void PrintNewLine(int &row, std::ostream &reply, ROAnything &rowColors, bool invertHeaders);
	void PrintElement(std::ostream &reply, Context &c, RowAccessor &accessor);

	//! hook method to do something special if section is empty
	virtual void DoPrintEmptySection(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &sectionConfig,  int &row);

	//! within a body section always the same renderers are used for the
	//! creation of the different columns.. the following methods perform some caching
	RowAccessor *SetupRowAccessors(const ROAnything &conf, Context &c, const ROAnything &rowMeta, long *rowSize, bool isBold, bool invertHeaders);
	void CleanupRowAccessors(RowAccessor *accessors, long size);

	// utility methods
	//! special treatment of old browsers
	bool CanUseInvertedHeaders(Context &c);
	ROAnything GetFormat(const ROAnything &config, Context &c, const char *fmtName, const char *cfgSlot, const char *dftSlot);
	const char *GetColor(long index, const ROAnything &config, bool invertHeaders);

	friend class BasicTableRendererTest;
};

//---- SequenceRenderer --------------------------------------------------------------
//! Just renders what is given in the config, similar to the renderers default behavior
/*!
\par Configuration
\code
{
	...		Rendererspec	optional, will be rendered using Renderer::Render
}
\endcode

*/
class SequenceRenderer : public Renderer
{
public:
	SequenceRenderer(const char *name);
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif		//not defined _BasicTableRenderer_H
