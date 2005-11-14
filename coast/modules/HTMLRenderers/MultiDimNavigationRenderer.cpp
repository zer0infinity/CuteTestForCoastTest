/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface ------------
#include "MultiDimNavigationRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- MultiDimNavigationRenderer ---------------------------------------------------------------
RegisterRenderer(MultiDimNavigationRenderer);

MultiDimNavigationRenderer::MultiDimNavigationRenderer(const char *name) : Renderer(name) { }

MultiDimNavigationRenderer::~MultiDimNavigationRenderer() { }

void MultiDimNavigationRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(MultiDimNavigationRenderer.RenderAll);

	String tableWidth(""), borderWidth(""), cellpadding(""), cellspacing(""), eltsProLine(""), nrOfLines(""), nameOfSpecEltsList(""), tdBGColor(""),
		   optionalTagBefore(""), optionalTagAfter(""), defaultElt(""), overrideDefaultElt("");
	ROAnything roaSlotConfig, roaUndefEltConfig, roaSpecEltsConfig;

	// Default value of tableWidth is 20%
	if (config.LookupPath(roaSlotConfig, "TableWidth")) {
		RenderOnString(tableWidth, c, roaSlotConfig);
	}
	if ( tableWidth.AsLong(-1) < 0 ) {
		tableWidth = "20";
	}

	// Default value of borderWidth is 0
	if (config.LookupPath(roaSlotConfig, "BorderWidth")) {
		RenderOnString(borderWidth, c, roaSlotConfig);
	}
	if ( borderWidth.AsLong(-1) < 0 ) {
		borderWidth = "0";
	}

	// Default value of cellpadding is 0
	if (config.LookupPath(roaSlotConfig, "Cellpadding")) {
		RenderOnString(cellpadding, c, roaSlotConfig);
	}
	if ( cellpadding.AsLong(-1) < 0 ) {
		cellpadding = "0";
	}

	// Default value of cellspacing is 0
	if (config.LookupPath(roaSlotConfig, "Cellspacing")) {
		RenderOnString(cellspacing, c, roaSlotConfig);
	}
	if ( cellspacing.AsLong(-1) < 0 ) {
		cellspacing = "0";
	}

	// Default value of eltsProLine is 1
	if (config.LookupPath(roaSlotConfig, "EltsProLine")) {
		RenderOnString(eltsProLine, c, roaSlotConfig);
	}
	if ( eltsProLine.AsLong(-1) < 0 ) {
		eltsProLine = "1";
	}

	// Default value of nrOfLines is 1
	if (config.LookupPath(roaSlotConfig, "NrOfLines")) {
		RenderOnString(nrOfLines, c, roaSlotConfig);
	}
	if ( nrOfLines.AsLong(-1) < 0 ) {
		nrOfLines = "1";
	}

	// tdBGColor has no default
	if (config.LookupPath(roaSlotConfig, "tdBGColor")) {
		RenderOnString(tdBGColor, c, roaSlotConfig);
	}

	// optionalTagBefore has no default
	if (config.LookupPath(roaSlotConfig, "optionalTagBefore")) {
		RenderOnString(optionalTagBefore, c, roaSlotConfig);
	}

	// optionalTagAfter has no default
	if (config.LookupPath(roaSlotConfig, "optionalTagAfter")) {
		RenderOnString(optionalTagAfter, c, roaSlotConfig);
	}

	if (config.LookupPath(roaSlotConfig, "Elt_Default")) {
		RenderOnString(defaultElt, c, roaSlotConfig);
	}

	long lineNrMax = nrOfLines.AsLong(1);
	long columnNrMax = eltsProLine.AsLong(1);

	reply << "<table border=" << borderWidth << " cellpadding=" << cellpadding << " cellspacing=" << cellspacing <<  " width=\"" << tableWidth << "%\">";
	for ( long lineNr = 0; lineNr < lineNrMax; ++lineNr ) {
		reply << "<tr>";
		for ( long columnNr = 0; columnNr < columnNrMax; ++columnNr ) {
			reply << "<td align=center ";
			if (tdBGColor.Length()) {
				reply << "BGColor=\"" <<  tdBGColor << "\" ";
			}

			reply << " >" << optionalTagBefore;
			String elementSlotName;
			elementSlotName = String( "" ) << "Elt_L" << lineNr << "_C" << columnNr;
			String elementCondSlotName;
			elementCondSlotName << elementSlotName << "_Cond";
			String elementDefaultSlotName;
			elementDefaultSlotName = String( "" ) << "Elt_L" << lineNr << "_Default";

			bool renderButton = true;

			if (config.LookupPath(roaSlotConfig, elementCondSlotName)) {
				String condSlot;

				Renderer::RenderOnString(condSlot, c, roaSlotConfig);

				if (condSlot == "1") {
					renderButton = true;
				} else if (condSlot == "0") {
					renderButton = false;
				} else if (condSlot.Length() > 0) {
					renderButton = c.Lookup(condSlot).AsBool(false);
				}
			}

			String buttonRendered("");
			if (renderButton && config.LookupPath(roaSlotConfig, elementSlotName)) {
				Renderer::RenderOnString(buttonRendered, c, roaSlotConfig);
			}

			if (buttonRendered.Length() > 0) {
				reply << buttonRendered;
			} else {
				if (config.LookupPath(roaSlotConfig, elementDefaultSlotName)) {
					overrideDefaultElt = "";
					RenderOnString(overrideDefaultElt, c, roaSlotConfig);
					if (overrideDefaultElt.Length() > 0) {
						reply << overrideDefaultElt;
					} else {
						reply << defaultElt;
					}
				} else {
					reply << defaultElt;
				}
			}
			reply << optionalTagAfter << "</td>";
		}
		reply << "</tr>";
	}
	reply << "</table>";
}
