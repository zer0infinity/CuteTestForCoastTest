/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MultifunctionListBoxRenderer_H
#define _MultifunctionListBoxRenderer_H

#include "FormRenderer.h"

//---- MultifunctionListBoxRenderer ----------------------------------------------------------
//! comment Renderer
/*!	Structure of config:
<PRE>
{
	/Name						Name of the HTML element
	/Size						Size (rows) of the box
	/ColList					Rendererspec	slotname to list with definition of columns
	/ListName					Rendererspec	Slotname to lookup which contains the values of the fields
	/ValueRenderer				Rendererspec	 to set Value field of Option
	/SelectedRenderer			Rendererspec	, leaving a Slotname to lookup which evaluates to 1 when needed
	/OnChangeScript								optional, called everytime the listbox gets a Changed event
	/OnChangeEditFieldScript					optional, called when one of the added editfields gets a change event
	/ColSep						Rendererspec	optional, default "|&nbsp;"
	/ClientSideColSep			Rendererspec	, default "| ", specifies the column separator
												! specify when using ClientSideRendering or EditableList !
	/ClientSideValSep			Rendererspec	, default ";", specifies the value separator for the client-side javascripts
												if specified, don't forget to change the separator of the ValueRenderer too
	/FormName					Rendererspec	optional, Name of the form in which this list resides, used for OnLoad and other scripts
												! specify when using ClientSideRendering or EditableList !
	/ClientSideRendering						optional, 1 or 0, default 0, fill list contents on client using JavaScript
	/EditableList				Rendererspec	optional, 1 or 0, default 0:
												* each line which contains columns which are either editable or pulldown-menus
												  can be edited, the value in the list will be updated and the changes can be
												  saved or reset using the two additional buttons inserted
	/EditableMode								optional, "Line" or "Field", default "Field" mode
												* "Line" mode: changes made (add, delete or change) on a single line get stored in the edtChanged field
												  Added lines go into edtAdded and Deleted lines into edtDeleted field
												* "Field" mode: added fields go into edtAdded, deleted fields into edtDeleted and changed fields into edtChanged field
	/Multiple					Rendererspec, if this flag evaluates to 1, multiple entries may be selected, otherwise
								  exactly one entry may be selected
								! take care when the box is also set to editable !
	/BorderWidth				optional, borderwidth of the outermost frame, default 0L
	/BgColor					optional, background color of the box (grid), default transparent
	/Innertable {				optional
		/BorderWidth			borderwidth of the inner layout, default 0L
	}
	/Header {					optional
		/BgColor				background color of the header, default transparent
		/Options {				common options for all cells in the header, specify specific things in ColList
			/class	SelectBoxHeader
		}
	}
	/SelectBox {				optional
		/BgColor				background color of the selectbox, default transparent
		/Options {				option for the selectbox
			/class	SelectBoxSuchen
		}
	}
	/EditLine {					optional
		/BgColor				background color of the editline, default transparent
		/Options {				options for the editable fields
			/class	SelectBoxFooter
		}
	}
	/Navigation {				optional, if defined, enable navigation, default off
		/BorderWidth			borderwidth of the inner layout, default 0L
		/BgColor				background color of the editline, default transparent

	* The javascripts of the following buttons get stored in the TmpStore even if the button
	  is not displayed that the scripts can be Lookup'd from own Navigation.
	  The slots are named: JS_<BoxName>_<ButtonName>
	* The buttons are rendered in the sequence they are listed below

		/PrevButton				optional, when slot is defined show "<"
		/NextButton				optional, when slot is defined show ">"
		/SearchButton			optional, when slot is defined show "Suchen"
		/ClearButton			optional, when slot is defined show "Clear"
		/SaveButton				optional, when slot is defined show "Speichern"
		/ResetButton			optional, when slot is defined show "Änderungen zurücksetzen"
		/AddButton				optional, when slot is defined show "Hinzufügen"
		/DeleteButton			optional, when slot is defined show "Löschen"
		/PrintButton			optional, when slot is defined show "Drucken"
		/ExportButton			optional, when slot is defined show "Exportieren"

	* The buttons above can be configured using the following slots:
		/xxxButton {
			/Label				Rendererspec, replacing the default name
			/Options {			Rendererspec, replacing the default OnClick-Event
				/OnClick		own OnClick Event-Handler
			}
		}
		/SaveButton {			extends specification above with the following slot
			...
			/PreDoScript		optional, Rendererspec, specify an inline script which extends
								the default <boxname>_SaveButtonScript(buttonObj) before doing the real work
								-> this allows for instance to update or check things before 'saving' and if
								   something is wrong to stop the process of submitting the form
		}
		/DeleteButton {			extends specification above with the following slot
			...
			/PreDoScript	optional, Rendererspec, specify an inline script which extends
								the default <boxname>_DeleteButtonScript(buttonObj) before doing the real work
								-> this allows for instance to update or check things before 'deleting' and if
								   something is wrong to stop the process of submitting the form
		}

	}
	/StatusMessage				optional, message to display to the right of the navigation
}
</PRE>
 column list slots:
	/MyColList {
		{
			/Name			Rendererspec	mandatory, display name of the column
			/IntName		Rendererspec	optional, default is value of Name slot, internal name of the column, used for added/changed/deleted anything creation when box is editable
			/Width			Rendererspec	mandatory, width of column
			/Value			Rendererspec	mandatory, value for the column entry which is looked up each time the column gets rendererd
			/Align			Rendererspec	optional, alignment datacolumns, { left, center, right }, default left
			/HeaderAlign	Rendererspec	optional, alignment of header, { left, center, right }, default center
			/EditfieldAlign	Rendererspec	optional, alignment of editfield, { left, center, right }, default is value of slot /Align
			/EditfieldTitle Rendererspec	optional, display a title (context) for this editfield by mouseover. It's a <TD> option
			/Hide			Rendererspec	optional, long value, 0 or 1, default 0, hide from display
			/Changeable		Rendererspec	optional, long value, 0 or 1, default 1, do not update the list when a change is made
			/IsKey			long value		optional, 0 or 1, default 0, only used when in EditList mode, key-fields will be added to each slot of change
			/Filler			Rendererspec	optional, filler to align text in header, default &nbsp;
			/Editable		Rendererspec	optional, long or string, if it evaluates to 1 the column gets an input field at the end of the list
			/Sortable       String			optional, db attribute for sortorder (genau wie im SQL erforderlich aber ohne asc, desc)
			/SortIcon {
				/Action			action e.g. ProcessFormAction
				/Params {
					/Button		mandatory query name{ {/Lookup MultifunctionListBoxName} "_Search" }
				}
				/LinkStyle		String		optional, stylesheet definition for sorticons surrounding link, ex: "class=SortIconLinkStyle"
				/ImageAscending { mandatory, Renderespec for Ascending gif/Icon
					...
				}
				/ImageAscChosen { mandatory, Renderespec for Ascending chosen gif/Icon (toggled)
					...
				}
				/ImageDescending { mandatory, Renderespec for Descending gif/Icon (toggled)
					...
				}
				/ImageDescChosen { mandatory, Renderespec for Descending chosen gif/Icon (toggled)
					...
				}
			}
			/Pulldown {		optional, if defined the column gets a pulldown-menu for choosing values at the end of the list
				...			Rendererspec for PulldownMenuRenderer
			}
			/DataType {		optional
				/Type		datatype of Fieldvalue { "Integer", "Float" }
				/Validate	optional, JavaScript function to validate the field
			}
			/Options {		optional, add additional handlers to inputfield tag
				/OnXXX		DoWhatEver
			}
			/BgColor		optional, background color of the cell. It's advisable to specify it only if MultifunctionListBoxRenderer.EditLine.BgColor isn't set,
							otherwise we'll get <td bgcolor=...> repeated for each cell, when it could be simply once defined in MultifunctionListBoxRenderer.EditLine.BgColor

		}
		{
			...
		}
		...
	}
*/
class MultifunctionListBoxRenderer : public SelectBoxRenderer
{
public:
	//--- constructors
	MultifunctionListBoxRenderer(const char *name);
	~MultifunctionListBoxRenderer();

	//:Renders ?? on <I>reply </I>
	//!param: reply - out - the stream where the rendered output is written on.
	//!param: c - the context the renderer runs within.
	//!param: config - the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

	static bool IsHiddenField(Context &c, const ROAnything &toCheckConfig);
	static bool IsEditableField(Context &c, const ROAnything &toCheckConfig);
	static bool IsChangeableField(Context &c, const ROAnything &toCheckConfig);
	static bool IsPulldownField(Context &c, const ROAnything &toCheckConfig);
	static bool IsDataTypeDefined(Context &c, const ROAnything &toCheckConfig);
	static void RenderCellName(String &cellName, Context &c, const ROAnything &cellItem);
	static void GetBoxName(String &boxName, Context &c);
	static void GetFormName(String &formName, Context &c);
	static void GetTagStyle(String &tagStyle, Context &ctx);
	static void GetDataType(String &dataType, Context &c, const ROAnything &config);

private:
	void RenderStyleSheet(std::ostream &reply, Context &c, const ROAnything &config);
	void RenderHeader(std::ostream &reply, Context &c, const ROAnything &config, long &nColumns);
	void RenderBoxRow(std::ostream &reply, Context &c, const ROAnything &config, const long &nColumns);
	void RenderColumnInputFields(std::ostream &reply, Context &c, const ROAnything &config, const long &nColumns);
	void RenderAdditionalFilters(std::ostream &reply, Context &c, const ROAnything &config, const long &nColumns);
	void RenderStatusMessage(std::ostream &reply, Context &c, const ROAnything &config, const long &nColumns);
	void RenderNavigation(std::ostream &reply, Context &c, const ROAnything &config);
	bool DoRenderButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config, const String &strButtonName);
	void RenderScripts(std::ostream &reply, Context &c, const ROAnything &config);
	void RenderPrintScripts(std::ostream &reply, Context &c, const ROAnything &config);
	void RenderHiddenFieldsForEdit(std::ostream &reply, Context &c, const ROAnything &config);
	void RenderBoxName(String &boxName, Context &c, const ROAnything &config);
	void RenderFormName(String &formName, Context &c, const ROAnything &config);

	bool RenderPrevButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);
	bool RenderNextButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);
	bool RenderSearchButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);
	bool RenderClearButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);
	bool RenderPrintButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);
	bool RenderExportButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);
	bool RenderSaveButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);
	bool RenderResetButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);
	bool RenderAddButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);
	bool RenderDeleteButton(std::ostream &reply, Context &c, const ROAnything &navConfig, const ROAnything &config);

	bool Lookup(const ROAnything &nameConfig, Context &c, const ROAnything &config, Anything &result);

	friend class MultifunctionListBoxRendererTest;
};

//---- HeaderListRenderer -----------------------------------------------------------
class HeaderListRenderer : public ListRenderer
{
public:
	HeaderListRenderer(const char *name) : ListRenderer(name) {};
	~HeaderListRenderer() {};

protected:
	void RenderEntry(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState);
	void DoRenderSortIcons(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &listItem);
	void RenderSortIcon(std::ostream &reply, Context &c, const ROAnything &config, String strSortString, String strSortOrder, const ROAnything &imageConfig);
	bool IsSortableColumn(Context &c, const ROAnything &toCheckConfig);
};

#endif
