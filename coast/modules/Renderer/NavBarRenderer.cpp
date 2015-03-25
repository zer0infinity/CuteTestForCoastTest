/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "NavBarRenderer.h"
#include "Tracer.h"

static String ENRTY_STORE_NAME_DEFAULT("EntryData");
//---- NavBarRenderer ---------------------------------------------------------
RegisterRenderer(NavBarRenderer);

NavBarRenderer::NavBarRenderer(const char *name) : Renderer(name)
{
}

NavBarRenderer::~NavBarRenderer()
{
}

void NavBarRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{

	StartTrace(NavBarRenderer.RenderAll);
	TraceAny(config, "config");
	Anything info;

	// Check for mandatory configuration and presence of ListName
	if (config.IsDefined("Info")	  &&
		GetInfo(c, config, info) ) {
		// retrieve the Slotname where the entryData have to be stored
		String entryStoreName;
		ROAnything es;
		if ( config.LookupPath(es, "EntryStore") ) {
			RenderOnString(	entryStoreName, c, es);
		} // if config is defined EntryStore
		else {
			entryStoreName = ENRTY_STORE_NAME_DEFAULT;
		} // else config is defined EntryStore

		RenderLevel(reply, c, config, info, entryStoreName);
	}	// if initial checks are ok
}

void NavBarRenderer::RenderLevel(std::ostream &reply, Context &c, const ROAnything &config, Anything &info, String &entryStoreName)
{
	StartTrace(NavBarRenderer.RenderLevel);
	Anything tempStore = c.GetTmpStore();
	// retrieve config data

	ROAnything selectedRenderer = config["Selected"];
	ROAnything unselectedRenderer = config["Unselected"];
	ROAnything disabledRenderer = config["Disabled"];
	ROAnything levelHeader = config["LevelHeader"];
	ROAnything levelFooter = config["LevelFooter"];
	Anything childInfo;

	String actual;
	Anything actStore;
	if ( info.LookupPath(actStore, "ActualStore") ) {
		actual = c.Lookup(actStore.AsString("")).AsString("");
	}

	// Start of the rendering process ---------------------------------------------------

	// Header for the whole level
	if ( ! levelHeader.IsNull() ) {
		Render(reply, c, levelHeader);
	}

	// Entries
	Anything list = info["Entries"];
	long size(list.GetSize());
	for (int i = 0; (i < size); ++i) {
		// prepare data for rendering
		Anything entry = list[i];
		tempStore[entryStoreName] = entry;

		// Check if entry is disabled
		if ( entry.IsDefined("Disabled") ) {
			//Check if Entry has its own Renderer
			Anything ownRenderer;
			if ( entry.LookupPath(ownRenderer, "Renderer.Disabled") ) {
				Render(reply, c, ownRenderer);
			}
			// use standard Renderer
			else if ( ! disabledRenderer.IsNull() ) {
				Render(reply, c, disabledRenderer);
			}
		} else {
			// Check if entry's Name matches ActualName
			Trace("Actual : " << actual << "  Entry : " << entry["Name"].AsString("") );
			if ( actual.IsEqual(entry["Name"].AsString("")) ) {
				// entry is selected
				//Check if Entry has its own Renderer
				Anything ownRenderer;
				if ( entry.LookupPath(ownRenderer, "Renderer.Selected") ) {
					Render(reply, c, ownRenderer);
				}
				// use standard Renderer
				else if ( ! selectedRenderer.IsNull() ) {
					Render(reply, c, selectedRenderer);
				}

				if (entry.IsDefined("Childs")) {
					childInfo = entry["Childs"];
				}

			} else {
				// entry is not selected
				//Check if Entry has its own Renderer
				Anything ownRenderer;
				if ( entry.LookupPath(ownRenderer, "Renderer.Unselected") ) {
					Render(reply, c, ownRenderer);
				}
				// use standard Renderer
				else if ( ! unselectedRenderer.IsNull() ) {
					Render(reply, c, unselectedRenderer);
				}
			}
		}
	}

	// Footer for the whole level
	if ( ! levelFooter.IsNull() ) {
		Render(reply, c, levelFooter);
	}

	// Render next level if necessary
	if ( ! childInfo.IsNull() ) {
		RenderLevel(reply, c, config, childInfo, entryStoreName);
	}
}

bool NavBarRenderer::GetInfo(Context &c, const ROAnything &config, Anything &info)
{
	bool found(false);
	String infoDataName;
	RenderOnString(infoDataName, c, config["Info"]);

	// Check first within TempStore - prevents us from DeepCloning
	Anything tempStore = c.GetTmpStore();
	if ( tempStore.IsDefined(infoDataName) ) {
		info = tempStore[infoDataName];
		found = true;
	} // if ListName is in tempStore
	else {
		ROAnything ROinfo = c.Lookup(infoDataName);
		if ( ! ROinfo.IsNull() ) {
			info = ROinfo.DeepClone();
			found = true;
		}	// if info data are found in Context
	}	// else

	return found;
}
