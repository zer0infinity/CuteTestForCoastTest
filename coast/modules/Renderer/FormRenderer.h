/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FORMRENDERER_H
#define _FORMRENDERER_H

#include "HTMLTemplateRenderer.h"
#include "ListRenderer.h"

//---- FormRenderer ---------------------------------------------------------------
//! FormRenderer is an extension of HTMLTemplateRenderer which generates HTML FORM tags
/*!
\par Configuration
\code
{
	/Method		String			optional, default POST [GET|POST], take care when using GET because the URL will be appended with all fields
	/Action		Rendererspec	optional, default "", action that is invoked when submitting the form with any button or method
	/EncType	String			optional, default is no ENCTYPE tag, ENCTYPE tag may be specified using this slot
	/Target		String			optional, default is no TARGET tag [_parent|_top|_self|...], name of target frame which gets the reply from the form request
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the FORM tag. Please refer to OptionsPrinter description for further explaining
	/Layout		{...}			optional, layout in the form of a list of renderer specifications. If not defined the config is just passed to the super-class HTMLTemplateRenderer
}
\endcode

FormRenderer is an extension of HTMLTemplateRenderer - which means that
all parameters that are valid for an HTMLTemplateRenderer may also be used
with a FormRenderer.

\note When using the "GET" method for passing form parameters, form settings show up in the location URL
displayed by the Web browser! "GET" should therefore be avoided if confidential password information is to be entered on a page!

\par Example:
\code
{
	/Method			"GET"				# parameters are passed using the "GET" method
	/Action			"ExampleAction"		# the action "ExampleAction" is used when any submit button is pressed
	/TemplateName	"MyTemplate"		# the file MyTemplate.html is used inside the form, eg. HTMLTemplateRenderer is used to render the content of the form
}
\endcode
*/
class FormRenderer: public HTMLTemplateRenderer
{
public:
	FormRenderer(const char *name);

	static void PrintFormBegin(std::ostream &reply, Context &context, const ROAnything &config);
	static void PrintFormEnd(std::ostream &reply, Context &context);
	void RenderAll(std::ostream &reply, Context &context, const ROAnything &config);
protected:
	class TemplateParser *GetParser();
};

//---- FieldRenderer ---------------------------------------------------------------
//! FieldRenderer is the base to render any type of FORM widgets
/*!
If a form is submitted, the fields go into query["fields"] which has the following form
@code
/query {
   ....                 # other slots
   /Button    "Ok"      # Name of the pressed button to submit form (either a button or imagebutton)
   /fields {
      /my1FieldName  "UserEnteredValue"
      /my2FieldName  "UserEnteredValue"
      ....
}
@endcode
Breaking FieldRenderer into several individual renderers serves to avoid confusion
regarding the different parameters that may be applicable with the different
widgets. It also makes configuration files more readable.

\note Now FieldRenderer does not render anything anymore. Use the dedicated renderers for buttons, hidden fields etc.
*/
class FieldRenderer : public Renderer
{
public:
	FieldRenderer(const char *name);
	void RenderAll(std::ostream &reply, Context &context, const ROAnything &config);

protected:
	virtual void RenderName(std::ostream &reply, Context &context, const ROAnything &config);
	virtual void RenderType(std::ostream &reply, Context &context, const ROAnything &config) {}
	virtual void RenderOptions(std::ostream &reply, Context &context, const ROAnything &config);
};

//---- SelectBoxRenderer ---------------------------------------------------------------
//! This renderer renders a list from which the user may select one or multiple entries
/*!
\par Configuration
\code
{
	/Name					Rendererspec	mandatory, identifier used for the field, prefixed with fld_
	/Size 					Rendererspec	optional, default 1, defines how many elements of the list are simultaneously displayed. If set to 1 and Multiple is undefined it degenerates to a PulldownMenuRenderer
	/Multiple				Rendererspec	optional, default 0 [0|1], if this flag evaluates to 1, multiple entries may be selected, otherwise exactly one entry may be selected
	/ListName				Rendererspec	mandatory (can be omitted if ListData slot is used instead), resulting a String which is used to Lookup the List in the TempStore and the context
	/ListData {				Anything		optional but mandatory if ListName is omitted, inline List to use for rendering, it is ignored if ListName is also specified and is not the empty list
		...
		...
	}
	/TextRenderer			Rendererspec	mandatory, Used to renderer the OPTION Content
	/ValueRenderer			Rendererspec    optional, Used to renderer the OPTION's VALUE attribute
	/SelectedRenderer 		Rendererspec    optional, Used to renderer the OPTION's SELECTED attribute / Should result in either 0 or 1
	/OptionListRenderer		Rendererspec	optional, Used to exchange the OPTION tag renderer with a specialized one, see also class OptionListRenderer
	/EntryStore				Rendererspec	optional, defaults to EntryData, resulting a String which identifies the temporary entry data while rendering
    /IndexSlot          	String			optional, denotes the slotname where the index (row number) of the actual entry is stored while rendering
    /SlotnameSlot       	String			optional, denotes the slotname where the slotname of the actual entry is stored while rendering
	/PrependListName		Rendererspec	optional, May contain items which are inserted BEFORE items from ListName get inserted
	/PrependListData {		Anything		optional, similar to ListData but for PrependList
		...
		...
	}
	/AppendListName			Rendererspec	optional, May contain items which are inserted AFTER ListName items are inserted
	/AppendListData {		Anything		optional, similar to ListData but for AppendList
		...
		...
	}
	/Options 			{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the SELECT tag. Refer to OptionsPrinter for further description
}
\endcode

The renderer builds on the HTML SELECT tag.\n
\note If /Size 1 and /Multiple is undefined the SelectBoxRenderer degenerates to a PulldownMenuRenderer

The individual elements contained within /ListData or /ListName, consist of two parts
- \b /Text - Rendererspec that produces the text in the list visible on the screen
- \b /Options - Renders options with the OPTION tag
 - \b /Value - when committing a form, the selected items of a SELECT field are
	usually reported using the strings displayed in the field. A special return
	value may be specified by setting the /Value slot (/Value may contain an arbitrary renderer specification).
 - \b /Selected - Marks an item as selected. (If /Multiple is undefined, only one element may be marked selected.

\par Example:
\code
/SelectBoxRenderer {
	/Name		"exampleList"
	/Size		25
	/Multiple	1
	/ListData {
		{ # first entry
			/Text {
				/String {
					/Default "1st list entry"
				}
			}
			/Value		"1"
			/Selected	1
		}
		{ # second entry
			/Text	"2nd list entry"
			/Value		"2"
			/Selected	1
		}
		{ # third entry
			/Text {
				/String {
					/Default "3rd list entry"
				}
			}
			/Value		"3"
			/Selected 	0
		}
	}
	/TextRenderer {
		/ContextLookupRenderer SelectBoxOption.Text
	}
	/ValueRenderer {
		/ContextLookupRenderer SelectBoxOption.Value
	}
	/SelectedRenderer {
		/ContextLookupRenderer SelectBoxOption.Selected
	}
}
\endcode

The actual List item data can be accessed under SelectBoxOption, the Index SelectBoxOptionIndex and the slotname SelectBoxOptionSlotname
Use this if you dont want to build up an extra list for the selectbox renderer
*/
class SelectBoxRenderer : public FieldRenderer
{
public:
	SelectBoxRenderer(const char *name);
	virtual void RenderAll(std::ostream &reply, Context &context, const ROAnything &config);
protected:
	virtual void RenderType(std::ostream &, Context &, const ROAnything &) { }
	// Renderers SELECT Tag attributes
	virtual void RenderOptions(std::ostream &reply, Context &context, const ROAnything &config);
	// Renderes the <OPTION> list using a ListRenderer
	virtual void RenderOptionList(std::ostream &reply, Context &context, const ROAnything &config, String listname);
	virtual bool IsMultipleSelect(Context &context, const ROAnything &config);
};

//---- OptionListRenderer -----------------------------------------------------------
//! This renderer renders the OPTION tags of the SelectBoxRenderer
/*!
\par Configuration
\code
{
	/Name				Rendererspec	mandatory, identifier used for the field, prefixed with fld_
	/TextRenderer		Rendererspec	mandatory, Used to renderer the OPTION Content
	/ValueRenderer		Rendererspec    optional, Used to renderer the OPTION's VALUE attribute
	/SelectedRenderer 	Rendererspec    optional, Used to renderer the OPTION's SELECTED attribute / Should result in either 0 or 1
	/EntryStore			Rendererspec	optional, defaults to EntryData, resulting a String which identifies the temporary entry data while rendering
    /IndexSlot          String			optional, denotes the slotname where the index (row number) of the actual entry is stored while rendering
    /SlotnameSlot       String			optional, denotes the slotname where the slotname of the actual entry is stored while rendering
}
\endcode

*/
class OptionListRenderer : public ListRenderer
{
public:
	OptionListRenderer(const char *name) : ListRenderer(name) {};
	~OptionListRenderer() {};

protected:
	/*! renders a list entry using the given configuration
		\param reply stream to generate output on
		\param ctx Context to be used for output generation
		\param config the configuration of the renderer
		\param entryRendererConfig renderer configuration for the entry
		\param listItem current list item data as shortcut (instead of using ctx.Lookup )
		\param anyRenderState anything to carry information between different calls */
	virtual void RenderEntry(std::ostream &reply, Context &ctx, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState);

	/*! Get the EntryHeader to be rendered
		\param config the configuration of the renderer
		\param nr index into headers list
		\param roaEntryHeader reference to header entry to be rendered
		\param anyRenderState anything to carry information between different calls
		\return true in case an entry was found and is not Null */
	virtual bool GetEntryHeader(const ROAnything &config, long nr, ROAnything &roaEntryHeader, Anything &anyRenderState);

	/*! Get the EntryFooter to be rendered
		\param config the configuration of the renderer
		\param roaEntryFooter reference to footer entry to be rendered
		\param anyRenderState anything to carry information between different calls
		\return true in case an entry was found and is not Null */
	virtual bool GetEntryFooter(const ROAnything &config, ROAnything &roaEntryFooter, Anything &anyRenderState);

	/*! Checks if the / which EntryHeader has to be rendered
		\param ctx the context the renderer runs within
		\param config the configuration of the renderer
		\param anyRenderState anything to carry information between different calls
		\return index into headers list if multiple, 0 if only one header entry, -1 if no header has to be rendered */
	virtual long EntryHeaderNrToBeRendered(Context &ctx, const ROAnything &config, Anything &anyRenderState);

	/*! Checks if the EntryFooter has to be rendered
		\param ctx the context the renderer runs within
		\param config the configuration of the renderer
		\param anyRenderState anything to carry information between different calls
		\return true if the entry footer has to be rendered */
	virtual bool EntryFooterHasToBeRendered(Context &ctx, const ROAnything &config, Anything &anyRenderState);

	/*! renders a entry header using the given configuration
		\param reply stream to generate output on
		\param ctx Context to be used for output generation
		\param entryHeader renderer configuration for the entry header
		\param listItem current list item data as shortcut (instead of using ctx.Lookup )
		\param anyRenderState anything to carry information between different calls */
	virtual void RenderEntryHeader(std::ostream &reply, Context &ctx, const ROAnything &entryHeader, const ROAnything &listItem, Anything &anyRenderState);

	/*! renders a entry footer using the given configuration
		\param reply stream to generate output on
		\param ctx Context to be used for output generation
		\param entryFooter renderer configuration for the entry footer
		\param listItem current list item data as shortcut (instead of using ctx.Lookup )
		\param anyRenderState anything to carry information between different calls */
	virtual void RenderEntryFooter(std::ostream &reply, Context &ctx, const ROAnything &entryFooter, const ROAnything &listItem, Anything &anyRenderState);

	virtual void RenderValue(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &valueConfig, const ROAnything &listItem);
	virtual void RenderSelected(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &selectedConfig, const ROAnything &listItem);
	virtual void RenderText(std::ostream &reply, Context &c, const ROAnything &config, const ROAnything &textConfig, const ROAnything &listItem);

};

//---- PulldownMenuRenderer ---------------------------------------------------------------
//! This renderer renders a pulldown menu - a list from which the user has to select exactly one entry
/*!
Except for the \b Size and \b Multiple slots - which are meaningless here -
PulldownMenuRenderer uses exactly the same Configuration as a SelectBoxRenderer.
If no item contained in a pulldown menu is marked as selected, the first item
is choosen as the default.
*/
class PulldownMenuRenderer : public SelectBoxRenderer
{
public:
	PulldownMenuRenderer(const char *name);
protected:
	virtual void RenderOptions(std::ostream &reply, Context &context, const ROAnything &config);
};

//---- CheckboxRenderer ---------------------------------------------------------------
//! The CheckBoxRenderer is used to create a checkbox.
/*!
\par Configuration
\code
{
	/Name 		Rendererspec	mandatory, identifier used for the checkbox.
	/Value		Rendererspec	optional, default off [on|off], initial setting of the checkbox, if set to on, CHECKED gets rendered
	/Options 	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
\endcode

\remarks Keep in mind that Web browsers only send back informations about the
checkboxes that where turned "on"! Checkboxes that do not show up in the next
query generated by the browser are therefore implicitly turned "off"!

\par Example:
\code
/ExampleToggle {
	/CheckboxRenderer {
		/Name   "someToggle"
		/Value	"on"			# toggle is initally turned on
	}
}
\endcode
*/
class CheckBoxRenderer : public FieldRenderer
{
public:
	CheckBoxRenderer(const char *name);
protected:
	virtual void RenderType(std::ostream &reply, Context &, const ROAnything &) {
		reply << "CHECKBOX";
	}
	virtual void RenderOptions(std::ostream &reply, Context &context, const ROAnything &config);
};

//---- RadioButtonRenderer ---------------------------------------------------------------
//! The RadioButtonRenderer is used to create a radio button
/*!
\par Configuration
\code
{
	/Name				Rendererspec	mandatory, identifier used for the field
	/Value				Rendererspec	mandatory, specifies the value stored in the field
	/Checked			-				optional, if defined the radiobutton is initially selected
	/EvaluateChecked	Rendererspec 	optional, is evaluated and converted to boolean, if true the radio gets checked
	/Options			{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
\endcode

Related radio buttons are identified by the \b /Name slot. If there are multiple radio buttons using
the same \b /Name, only one may be selected at any time. The return value for the finally selected
button is specified using the \b /Value slot. The initially selected button may be specified
using a \b /Checked slot.

\par Example: (three mutually exclusive radio buttons)
\code
/Radio1 {
	/RadioButtonRenderer {
		/Name   "x1"
		/Value	"pos1"
	}
}

/Radio2 {
	/RadioButtonRenderer {
		/Name   "x1"
		/Value	"pos2"
		/Checked 1    			# initially checked button
	}
}

/Radio3 {
	/RadioButtonRenderer {
		/Name   "x1"
		/Value	"pos2"
		/EvaluateChecked {
			/Condition {
				/ContextCondition  RadioOn 		# initially set button
				/Defined	1					# if RadioOn is defined in
				/Undefined	0					# the Context
			}
		}
	}
}
\endcode

The value returned in the query if button Radio1 had been finally selected would be

\code
/fields {
	/x1	"pos1"
}
\endcode
*/
class RadioButtonRenderer : public FieldRenderer
{
public:
	RadioButtonRenderer(const char *name);
protected:
	virtual void RenderType(std::ostream &reply, Context &, const ROAnything &) {
		reply << "RADIO";
	}
	virtual void RenderOptions(std::ostream &reply, Context &context, const ROAnything &config);
};

//---- HiddenFieldRenderer ---------------------------------------------------------------
//! HiddenFieldRenderer renders an arbitrary hidden field
/*!
\par Configuration
\code
{
	/Name		Rendererspec	mandatory, identifier used for the field
	/Value		Rendererspec	mandatory, value stored in the hidden field
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
\endcode
\par Example:
\code
/ExampleHidden {
	/HiddenFieldRenderer {
		/Name		"myKey"
		/Value		"myValue"
	}
}
\endcode
*/
class HiddenFieldRenderer : public FieldRenderer
{
public:
	HiddenFieldRenderer(const char *name);
protected:
	virtual void RenderType(std::ostream &reply, Context &, const ROAnything &) {
		reply << "HIDDEN";
	}
	virtual void RenderOptions(std::ostream &reply, Context &context, const ROAnything &config);
};

//---- ButtonRenderer ---------------------------------------------------------------
//! ButtonRenderer renders an HTML submit button
/*!
\par Configuration
\code
{
	/Name		Rendererspec	mandatory, identifier used for the button
	/Label		Rendererspec	mandatory, label of the button
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
\endcode

If a form is submitted by pressing the button, its name goes into query["Button"]

\par Example:
\code
/ExampleSubmit {
	/ButtonRenderer {
		/Name		"mySubmitButton"
		/Label {
			/String
				/Default    "Enter"
				/D          "Eingabe"
			}
		}
	}
}
\endcode
*/
class ButtonRenderer : public FieldRenderer
{
public:
	ButtonRenderer(const char *name);
protected:
	virtual void RenderName(std::ostream &reply, Context &context, const ROAnything &config);
	virtual void RenderType(std::ostream &reply, Context &, const ROAnything &) {
		reply << "SUBMIT";
	}
	virtual void RenderOptions(std::ostream &reply, Context &context, const ROAnything &config);
};

//---- ImageButtonRenderer ---------------------------------------------------------------
//! ImageButtonRenderer renders an HTML INPUT with Type Image, that can be used to submit a form
/*!
\par Configuration
\code
{
	/Name		Rendererspec	mandatory, identifier used for the button
	/Src		Rendererspec	mandatory, local filename/path to the image
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
\endcode

If a form is submitted by clicking on the imagebutton, its name goes into query["Button"]
additionally the query contains a slot named Click which contains the x and y position
where the mouseclick has occured

\code
query {
	...
	/Button    ImageButtonName
	/Click {
		/x     11
		/y     25
	}
}
\endcode
\par Example:
\code
/ExampleSubmit {
	/ImageButtonRenderer {
		/Name		"myImageButton"
		/Src {
			/ImageLocationRenderer {		# use the ImageLocationRenderer
				/ImageName {				# use StringRenderer to retrieve localized image
					/String {
						/Default    "myImage_e.gif"
						/D          "myImage_d.gif"
					}
				}
			}
		}
	}
}
\endcode
*/
class ImageButtonRenderer : public FieldRenderer
{
public:
	ImageButtonRenderer(const char *name);
protected:
	virtual void RenderName(std::ostream &reply, Context &context, const ROAnything &config);
	virtual void RenderType(std::ostream &reply, Context &, const ROAnything &) {
		reply << "IMAGE";
	}
	virtual void RenderOptions(std::ostream &reply, Context &context, const ROAnything &config);
};

//---- ResetButtonRenderer ---------------------------------------------------------------
//! ResetButtonRenderer renders an HTML reset button
/*!
\par Configuration
\code
{
	/Label		Rendererspec	mandatory, label of the button
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
\endcode
\par Example:
\code
/ExampleReset {
	/ResetButtonRenderer {
		/Label		"Reset"
	}
}
\endcode
*/
class ResetButtonRenderer : public ButtonRenderer
{
public:
	ResetButtonRenderer(const char *name);

protected:
	virtual void RenderType(std::ostream &reply, Context &, const ROAnything &) {
		reply << "RESET";
	}
};

//---- TextFieldRenderer ---------------------------------------------------------------
//! TextFieldRenderer is supposed to be used for text- and password input fields
/*!
\par Configuration
\code
{
	/Name			Rendererspec	mandatory, identifier used for the field.
	/Value			Rendererspec	optional, initial setting of the field.
	/Unreadable		-				optional, used for password input fields, if defined the users input inside the field is rendered unreadable
	/Size			Rendererspec	optional, The width of the field.
	/Maxlength		Rendererspec	optional, Maximum number of characters that may be entered.
	/Options		{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
\endcode
\par Example:
\code
/ExampleField {
	/TextFieldRenderer {
		/Name		"myField"
		/Value  	"Initial text inside the field"
		/Size		20
		/Options {
			"Readonly"
			/OnChange    "'SetStatus(\"Value changed in field myField\");'"   # Some java-script code
		}
	}
}
\endcode
*/
class TextFieldRenderer : public FieldRenderer
{
public:
	TextFieldRenderer(const char *name) : FieldRenderer(name) {}

protected:
	virtual void RenderType(std::ostream &reply, Context &context, const ROAnything &config);
	virtual void RenderOptions(std::ostream &reply, Context &context, const ROAnything &config);
};

//---- TextAreaRenderer --------------------------------------------------------------
//! TextAreaRenderer serves to create a multi-line text input area
/*!
\par Configuration
\code
{
	/Name		Rendererspec	mandatory, identifer used for returning the result
	/Value		Rendererspec	optional, initial content of the field
	/Height		String			optional, height of the textfield in chars
	/Width		String			optional, width of the textfield in chars
	/Wrap		String			optional, wrapping behaviour .. either: "HARD", "SOFT" or "NONE"
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
\endcode

It builds on the HTML TEXTAREA tag.
\par Example:
\code
/ExampleArea {
	/TextAreaRenderer {
		/Name		"myTextArea"
		/Value  	"This is the default text!"
		/Height		10
		/Width		20
		/Wrap		"HARD"
	}
}
\endcode
*/
class TextAreaRenderer : public FieldRenderer
{
public:
	TextAreaRenderer(const char *name);
	virtual void RenderAll(std::ostream &reply, Context &context, const ROAnything &config);

protected:
	virtual void RenderType(std::ostream &, Context &, const ROAnything &) { }

};

//---- FieldNameRenderer ---------------------------------------------------------------
//! Renders a fieldname including Coast prefix given a name
/*!
\par Configuration
\code
{
	/Default	String	optional, name of the field
	/LookupName	String	optional, name will be looked up in the context with the given string
}
\endcode
*/
class FieldNameRenderer : public Renderer
{
public:
	FieldNameRenderer(const char *name);

	void RenderAll(std::ostream &reply, Context &context, const ROAnything &config);
};

//---- FileBrowseRenderer --------------------------------------------------------------
//! Creates a filebrowse input area
/*! The configuration is equal to the TextFieldRenderer but the type is fixed to \b FILE */
class FileBrowseRenderer : public TextFieldRenderer
{
public:
	FileBrowseRenderer(const char *name);

protected:
	virtual void RenderType(std::ostream &reply, Context &, const ROAnything &) {
		reply << "FILE";
	}
};
#endif
