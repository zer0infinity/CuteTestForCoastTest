/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FORMRENDERER_H
#define _FORMRENDERER_H

#include "config_renderer.h"
#include "HTMLTemplateRenderer.h"
#include "ListRenderer.h"

//---- FormRenderer ---------------------------------------------------------------
//! <B>FormRenderer is an extension of HTMLTemplateRenderer which generates HTML FORM tags</B>
/*!
<B>Configuration:</B><PRE>
{
	/Method		String			optional, default POST [GET|POST], take care when using GET because the URL will be appended with all fields
	/Action		Rendererspec	optional, default "", action that is invoked when submitting the form with any button or method
	/EncType	String			optional, default is no ENCTYPE tag, ENCTYPE tag may be specified using this slot
	/Target		String			optional, default is no TARGET tag [_parent|_top|_self|...], name of target frame which gets the reply from the form request
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the FORM tag. Please refer to OptionsPrinter description for further explaining
	/Layout		{...}			optional, layout in the form of a list of renderer specifications. If not defined the config is just passed to the super-class HTMLTemplateRenderer
}
</PRE>
FormRenderer is an extension of HTMLTemplateRenderer - which means that
all parameters that are valid for an HTMLTemplateRenderer may also be used
with a FormRenderer.

\note When using the "GET" method for passing form parameters, form settings show up in the location URL
displayed by the Web browser! "GET" should therefore be avoided if confidential password information is to be entered on a page!

Example:
<PRE>
{
	/Method			"GET"				# parameters are passed using the "GET" method
	/Action			"ExampleAction"		# the action "ExampleAction" is used when any submit button is pressed
	/TemplateName	"MyTemplate"		# the file MyTemplate.html is used inside the form, eg. HTMLTemplateRenderer is used to render the content of the form
}
</PRE>
*/
class EXPORTDECL_RENDERER FormRenderer: public HTMLTemplateRenderer
{
public:
	FormRenderer(const char *name);

	static void PrintFormBegin(ostream &reply, Context &context, const ROAnything &config);
	static void PrintFormEnd(ostream &reply, Context &context);
	void RenderAll(ostream &reply, Context &context, const ROAnything &config);
protected:
	class TemplateParser *GetParser();
};

//---- FieldRenderer ---------------------------------------------------------------
//! <B>FieldRenderer is the base to render any type of FORM widgets</B>
/*!
If a form is submitted, the fields go into query["fields"] which has the following form
<PRE>
/query {
   ....                 # other slots
   /Button    "Ok"      # Name of the pressed button to submit form (either a button or imagebutton)
   /fields {
      /my1FieldName  "UserEnteredValue"
      /my2FieldName  "UserEnteredValue"
      ....
}
</PRE>
Breaking FieldRenderer into several individual renderers serves to avoid confusion
regarding the different parameters that may be applicable with the different
widgets. It also makes configuration files more readable.

\note Now FieldRenderer does not render anything anymore. Use the dedicated renderers for buttons, hidden fields etc.
*/
class EXPORTDECL_RENDERER FieldRenderer : public Renderer
{
public:
	FieldRenderer(const char *name);
	void RenderAll(ostream &reply, Context &context, const ROAnything &config);

protected:
	virtual void RenderName(ostream &reply, Context &context, const ROAnything &config);
	virtual void RenderType(ostream &reply, Context &context, const ROAnything &config) {}
	virtual void RenderOptions(ostream &reply, Context &context, const ROAnything &config);
};

//---- SelectBoxRenderer ---------------------------------------------------------------
//! <B>This renderer renders a list from which the user may select one or multiple entries</B>
/*!
<B>Configuration:</B><PRE>
{
	/Name				Rendererspec	mandatory, identifier used for the field, prefixed with fld_
	/Size 				Rendererspec	optional, default 1, defines how many elements of the list are simultaneously displayed. If set to 1 and Multiple is undefined it degenerates to a PulldownMenuRenderer
	/Multiple			Rendererspec	optional, default 0 [0|1], if this flag evaluates to 1, multiple entries may be selected, otherwise exactly one entry may be selected
	/List {								optional, May contain the items used for the SELECT field.
		{								optional, first INPUT entry of SELECT box
			/Text		Rendererspec
			/Options 	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
		}
		{...}							optional, second INPUT entry of SELECT box
		...								optional, further elements
	}
	/LookupList			Rendererspec	optional, takes precedence over the prameter /List, the List will be looked up in the context using the rendered value of this slot

<B>Alternativly the option list can be specified like this (works analogous to the ListRenderer)</B>
	/PlainList			Rendererspec 	mandatory, resulting in the lists lookup name
	/TextRenderer		Rendererspec	mandatory, Used to renderer the OPTION Content
	/ValueRenderer		Rendererspec    optional, Used to renderer the OPTION's VALUE attribute
	/SelectedRenderer 	Rendererspec    optional, Used to renderer the OPTION's SELECTED attribute / Should result in either 0 or 1
	/OptionListRenderer	Rendererspec	optional, Used to exchange the OPTION tag renderer with a specialized one, see also class OptionListRenderer
	/PrependList		Rendererspec	optional, May contain items which are inserted BEFORE items from PlainList get inserted
	/AppendList			Rendererspec	optional, May contain items which are inserted AFTER PlainList items are inserted

	/Options 			{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the SELECT tag. Refer to OptionsPrinter for further description
}
</PRE>
The renderer builds on the HTML SELECT tag.<BR>
\note If /Size 1 and /Multiple is undefined the SelectBoxRenderer degenerates to a PulldownMenuRenderer

The individual elements contained within /List or /LookupList, consist of two parts
<OL>
<LI>\b /Text - Rendererspec that produces the text in the list visible on the screen</LI>
<LI>\b /Options - Renders options with the OPTION tag
<OL><LI>\b /Value - when committing a form, the selected items of a SELECT field are
usually reported using the strings displayed in the field. A special return
value may be specified by setting the /Value slot (/Value may contain an arbitrary renderer specification).</LI>
<LI>\b /Selected - Marks an item as selected. (If /Multiple is undefined, only one element may be marked selected.</LI></OL>
</LI></OL>

<PRE>Example:
/SelectBoxRenderer {
	/Name		"exampleList"
	/Size		25
	/Multiple	1
	/List {
		{ # first entry
			/Text {
				/String {
					/Default "1st list entry"
				}
			}
			/Options {
				/Value		"1"
				/Selected	1
			}
		}
		{ # second entry
			/Text	"2nd list entry"
			/Options {
				/Value		"2"
				/Selected	1
			}
		}
		{ # third entry
			/Text {
				/String {
					/Default "3rd list entry"
				}
			}
			/Options {
				/Value		"3"
				/Selected 	0
			}
		}
	}
}
</PRE>
The actual List item data can be accessed under SelectBoxOption, the Index SelectBoxOptionIndex and the slotname SelectBoxOptionSlotname
Use this if you dont want to build up an extra list for the selectbox renderer
*/
class EXPORTDECL_RENDERER SelectBoxRenderer : public FieldRenderer
{
public:
	SelectBoxRenderer(const char *name);
	virtual void RenderAll(ostream &reply, Context &context, const ROAnything &config);
protected:
	virtual void RenderType(ostream &, Context &, const ROAnything &) { }
	// Renderers SELECT Tag attributes
	virtual void RenderOptions(ostream &reply, Context &context, const ROAnything &config);
	// Renderes the <OPTION> list using a ListRenderer
	virtual void RenderOptionList(ostream &reply, Context &context, const ROAnything &config, String listname);
	virtual bool IsMultipleSelect(Context &context, const ROAnything &config);
};

//---- OptionListRenderer -----------------------------------------------------------
//! <B>This renderer renders the OPTION tags of the SelectBoxRenderer</B>
/*!
<B>Configuration:</B><PRE>
{
	/Name				Rendererspec	mandatory, identifier used for the field, prefixed with fld_
	/TextRenderer		Rendererspec	mandatory, Used to renderer the OPTION Content
	/ValueRenderer		Rendererspec    optional, Used to renderer the OPTION's VALUE attribute
	/SelectedRenderer 	Rendererspec    optional, Used to renderer the OPTION's SELECTED attribute / Should result in either 0 or 1
}
</PRE>
*/
class EXPORTDECL_RENDERER OptionListRenderer : public ListRenderer
{
public:
	OptionListRenderer(const char *name) : ListRenderer(name) {};
	~OptionListRenderer() {};

protected:
	virtual void RenderEntry(ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, Anything &listItem);
	virtual void RenderValue(ostream &reply, Context &c, const ROAnything &config, const ROAnything &valueConfig, Anything &listItem);
	virtual void RenderSelected(ostream &reply, Context &c, const ROAnything &config, const ROAnything &selectedConfig, Anything &listItem);
	virtual void RenderText(ostream &reply, Context &c, const ROAnything &config, const ROAnything &textConfig, Anything &listItem);
};

//---- PulldownMenuRenderer ---------------------------------------------------------------
//! <B>This renderer renders a pulldown menu - a list from which the user has to select exactly one entry</B>
/*!
Except for the \b Size and \b Multiple slots - which are meaningless here -
PulldownMenuRenderer uses exactly the same Configuration as a SelectBoxRenderer.
If no item contained in a pulldown menu is marked as selected, the first item
is choosen as the default.
*/
class EXPORTDECL_RENDERER PulldownMenuRenderer : public SelectBoxRenderer
{
public:
	PulldownMenuRenderer(const char *name);
protected:
	virtual void RenderOptions(ostream &reply, Context &context, const ROAnything &config);
};

//---- CheckboxRenderer ---------------------------------------------------------------
//! <B>The CheckBoxRenderer is used to create a checkbox.</B>
/*!
<B>Configuration:</B><PRE>
{
	/Name 		Rendererspec	mandatory, identifier used for the checkbox.
	/Value		Rendererspec	optional, default off [on|off], initial setting of the checkbox, if set to on, CHECKED gets rendered
	/Options 	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
</PRE>
\remarks Keep in mind that Web browsers only send back informations about the
checkboxes that where turned "on"! Checkboxes that do not show up in the next
query generated by the browser are therefore implicitly turned "off"!

Example:
<PRE>
/ExampleToggle {
	/CheckboxRenderer {
		/Name   "someToggle"
		/Value	"on"			# toggle is initally turned on
	}
}
</PRE>
*/
class EXPORTDECL_RENDERER CheckBoxRenderer : public FieldRenderer
{
public:
	CheckBoxRenderer(const char *name);
protected:
	virtual void RenderType(ostream &reply, Context &, const ROAnything &) {
		reply << "CHECKBOX";
	}
	virtual void RenderOptions(ostream &reply, Context &context, const ROAnything &config);
};

//---- RadioButtonRenderer ---------------------------------------------------------------
//! <B>The RadioButtonRenderer is used to create a radio button</B>
/*!
<B>Configuration:</B><PRE>
{
	/Name				Rendererspec	mandatory, identifier used for the field
	/Value				Rendererspec	mandatory, specifies the value stored in the field
	/Checked			-				optional, if defined the radiobutton is initially selected
	/EvaluateChecked	Rendererspec 	optional, is evaluated and converted to boolean, if true the radio gets checked
	/Options			{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
</PRE>
Related radio buttons are identified by the \b /Name slot. If there are multiple radio buttons using
the same \b /Name, only one may be selected at any time. The return value for the finally selected
button is specified using the \b /Value slot. The initially selected button may be specified
using a \b /Checked slot.

Example: (three mutually exclusive radio buttons)
<PRE>
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
</PRE>
The value returned in the query if button Radio1 had been finally selected would be

<PRE>
/fields {
	/x1	"pos1"
}
</PRE>
*/
class EXPORTDECL_RENDERER RadioButtonRenderer : public FieldRenderer
{
public:
	RadioButtonRenderer(const char *name);
protected:
	virtual void RenderType(ostream &reply, Context &, const ROAnything &) {
		reply << "RADIO";
	}
	virtual void RenderOptions(ostream &reply, Context &context, const ROAnything &config);
};

//---- HiddenFieldRenderer ---------------------------------------------------------------
//! <B>HiddenFieldRenderer renders an arbitrary hidden field</B>
/*!
<B>Configuration:</B><PRE>
{
	/Name		Rendererspec	mandatory, identifier used for the field
	/Value		Rendererspec	mandatory, value stored in the hidden field
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
</PRE>
<B>Example:</B>
<PRE>
/ExampleHidden {
	/HiddenFieldRenderer {
		/Name		"myKey"
		/Value		"myValue"
	}
}
</PRE>
*/
class EXPORTDECL_RENDERER HiddenFieldRenderer : public FieldRenderer
{
public:
	HiddenFieldRenderer(const char *name);
protected:
	virtual void RenderType(ostream &reply, Context &, const ROAnything &) {
		reply << "HIDDEN";
	}
	virtual void RenderOptions(ostream &reply, Context &context, const ROAnything &config);
};

//---- ButtonRenderer ---------------------------------------------------------------
//! <B>ButtonRenderer renders an HTML submit button</B>
/*!
<B>Configuration:</B><PRE>
{
	/Name		Rendererspec	mandatory, identifier used for the button
	/Label		Rendererspec	mandatory, label of the button
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
</PRE>
If a form is submitted by pressing the button, its name goes into query["Button"]

Example:
<PRE>
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
</PRE>
*/
class EXPORTDECL_RENDERER ButtonRenderer : public FieldRenderer
{
public:
	ButtonRenderer(const char *name);
protected:
	virtual void RenderName(ostream &reply, Context &context, const ROAnything &config);
	virtual void RenderType(ostream &reply, Context &, const ROAnything &) {
		reply << "SUBMIT";
	}
	virtual void RenderOptions(ostream &reply, Context &context, const ROAnything &config);
};

//---- ImageButtonRenderer ---------------------------------------------------------------
//! <B>ImageButtonRenderer renders an HTML INPUT with Type Image, that can be used to submit a form</B>
/*!
<B>Configuration:</B><PRE>
{
	/Name		Rendererspec	mandatory, identifier used for the button
	/Src		Rendererspec	mandatory, local filename/path to the image
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
</PRE>
If a form is submitted by clicking on the imagebutton, its name goes into query["Button"]
additionally the query contains a slot named Click which contains the x and y position
where the mouseclick has occured
<PRE>
query {
	...
	/Button    ImageButtonName
	/Click {
		/x     11
		/y     25
	}
}
</PRE>
Example:
<PRE>
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
</PRE>
*/
class EXPORTDECL_RENDERER ImageButtonRenderer : public FieldRenderer
{
public:
	ImageButtonRenderer(const char *name);
protected:
	virtual void RenderName(ostream &reply, Context &context, const ROAnything &config);
	virtual void RenderType(ostream &reply, Context &, const ROAnything &) {
		reply << "IMAGE";
	}
	virtual void RenderOptions(ostream &reply, Context &context, const ROAnything &config);
};

//---- ResetButtonRenderer ---------------------------------------------------------------
//! <B>ResetButtonRenderer renders an HTML reset button</B>
/*!
<B>Configuration:</B><PRE>
{
	/Label		Rendererspec	mandatory, label of the button
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
</PRE>
<B>Example:</B>
<PRE>
/ExampleReset {
	/ResetButtonRenderer {
		/Label		"Reset"
	}
}
</PRE>
*/
class EXPORTDECL_RENDERER ResetButtonRenderer : public ButtonRenderer
{
public:
	ResetButtonRenderer(const char *name);
protected:
	virtual void RenderType(ostream &reply, Context &, const ROAnything &) {
		reply << "RESET";
	}
};

//---- TextFieldRenderer ---------------------------------------------------------------
//! <B>TextFieldRenderer is supposed to be used for text- and password input fields</B>
/*!
<B>Configuration:</B><PRE>
{
	/Name			Rendererspec	mandatory, identifier used for the field.
	/Value			Rendererspec	optional, initial setting of the field.
	/Unreadable		-				optional, used for password input fields, if defined the users input inside the field is rendered unreadable
	/Size			Rendererspec	optional, The width of the field.
	/Maxlength		Rendererspec	optional, Maximum number of characters that may be entered.
	/Options		{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
</PRE>
<B>Example:</B>
<PRE>
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
</PRE>
*/
class EXPORTDECL_RENDERER TextFieldRenderer : public FieldRenderer
{
public:
	TextFieldRenderer(const char *name) : FieldRenderer(name) {}
protected:
	virtual void RenderType(ostream &reply, Context &context, const ROAnything &config);
	virtual void RenderOptions(ostream &reply, Context &context, const ROAnything &config);
};

//---- TextAreaRenderer --------------------------------------------------------------
//! <B>TextAreaRenderer serves to create a multi-line text input area</B>
/*!
<B>Configuration:</B><PRE>
{
	/Name		Rendererspec	mandatory, identifer used for returning the result
	/Value		Rendererspec	optional, initial content of the field
	/Height		String			optional, height of the textfield in chars
	/Width		String			optional, width of the textfield in chars
	/Wrap		String			optional, wrapping behaviour .. either: "HARD", "SOFT" or "NONE"
	/Options	{...}			optional, An array of Key-Value pairs that allows to render any HTML option within the INPUT tag. Refer to OptionsPrinter for further description
}
</PRE>
It builds on the HTML TEXTAREA tag.
<B>Example:</B>
<PRE>
/ExampleArea {
	/TextAreaRenderer {
		/Name		"myTextArea"
		/Value  	"This is the default text!"
		/Height		10
		/Width		20
		/Wrap		"HARD"
	}
}
</PRE>
*/
class EXPORTDECL_RENDERER TextAreaRenderer : public FieldRenderer
{
public:
	TextAreaRenderer(const char *name);
	virtual void RenderAll(ostream &reply, Context &context, const ROAnything &config);
protected:
	virtual void RenderType(ostream &, Context &, const ROAnything &) { }

};

//---- FieldNameRenderer ---------------------------------------------------------------
//! <B>Renders a fieldname including Coast prefix given a name</B>
/*!
<B>Configuration:</B><PRE>
{
	/Default	String	optional, name of the field
	/LookupName	String	optional, name will be looked up in the context with the given string
}
</PRE>
*/
class EXPORTDECL_RENDERER FieldNameRenderer : public Renderer
{
public:
	FieldNameRenderer(const char *name);
	void RenderAll(ostream &reply, Context &context, const ROAnything &config);
};

//---- FileBrowseRenderer --------------------------------------------------------------
//! <B>Creates a filebrowse input area</B>
//! The configuration is equal to the TextFieldRenderer but the type is fixed to \b FILE
class EXPORTDECL_RENDERER FileBrowseRenderer : public TextFieldRenderer
{
public:
	FileBrowseRenderer(const char *name);
protected:
	virtual void RenderType(ostream &reply, Context &, const ROAnything &) {
		reply << "FILE";
	}
};
#endif
