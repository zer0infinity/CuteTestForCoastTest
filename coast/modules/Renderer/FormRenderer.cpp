/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "FormRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Session.h"
#include "TemplateParser.h"
#include "Dbg.h"

//---- FormRenderer -------------------------------------------------------------------
RegisterRenderer(FormRenderer);
FormRenderer::FormRenderer(const char *name) : HTMLTemplateRenderer(name)
{
}

TemplateParser *FormRenderer::GetParser()
{
	return new FormTemplateParser;
}

void FormRenderer::RenderAll(ostream &reply, Context &context, const ROAnything &config)
{
	StartTrace(FormRenderer.Render);
	TraceAny(config, "config");
	PrintFormBegin(reply, context, config);
	ROAnything layout = config["Layout"]; // PS: most efficient because ROAnything doesn't expand!
	if (!layout.IsNull()) {
		// pass layout to Renderer::Render
		Render(reply, context, layout);
	} else { // pass this config to the super class
		HTMLTemplateRenderer::RenderAll(reply, context, config);
	}
	PrintFormEnd(reply, context);
}

void FormRenderer::PrintFormBegin(ostream &reply, Context &context, const ROAnything &config)
{
	reply << "<FORM ACTION=\"";
	Renderer *r = Renderer::FindRenderer("URLRenderer");
	String actionURL; // Save the URL for Later use
	{
		OStringStream os(&actionURL);
		r->RenderAll(os, context, config);
	}
	reply << actionURL;

	String method;
	ROAnything methodConfig;
	if ( config.LookupPath(methodConfig, "Method")) {
		RenderOnString(method, context, methodConfig);
	} else {
		method = "POST";
	}
	reply << "\" METHOD=\"" << method << "\"";

	if (config.IsDefined("EncType")) { // optimize with FindIndex if required
		reply << " ENCTYPE=\"" << config["EncType"].AsCharPtr("???") << "\"";
	}

	if (config.IsDefined("Target")) {
		reply << " TARGET=\"" << config["Target"].AsCharPtr("_self") << "\"";
	}

	PrintOptions3(reply, context, config);

	reply << ">";

	bool useBaseURL = (context.Lookup("UseBaseURL", 0L) != 0);

	if ( !useBaseURL && (method == "GET") ) {
		long pos = actionURL.StrChr('=');
		reply << "<INPUT TYPE=HIDDEN NAME=\"X\" VALUE=\"" << actionURL.SubString(pos + 1) << "\">";
	}
}

void FormRenderer::PrintFormEnd(ostream &reply, Context &)
{
	reply << ("</FORM>");
}

//---- PulldownMenuRenderer -----------------------------------------------------------
RegisterRenderer(PulldownMenuRenderer);

PulldownMenuRenderer::PulldownMenuRenderer(const char *name) : SelectBoxRenderer(name) {}
void PulldownMenuRenderer::RenderOptions(ostream &reply, Context &context, const ROAnything &config)
{
	reply << " SIZE='1\'";
	FieldRenderer::RenderOptions(reply, context, config);
}

//---- HiddenFieldRenderer ------------------------------------------------------------
RegisterRenderer(HiddenFieldRenderer);

HiddenFieldRenderer::HiddenFieldRenderer(const char *name) : FieldRenderer(name) {}
void HiddenFieldRenderer::RenderOptions(ostream &reply, Context &context, const ROAnything &config)
{
	ROAnything value = config["Value"];
	if (!value.IsNull()) {
		reply << " VALUE=\"" ;
		Render(reply, context, value);
		reply << "\"";
	}
	FieldRenderer::RenderOptions(reply, context, config);
}

//---- ButtonRenderer -----------------------------------------------------------------
RegisterRenderer(ButtonRenderer);

ButtonRenderer::ButtonRenderer(const char *name) : FieldRenderer(name) {}
void ButtonRenderer::RenderOptions(ostream &reply, Context &context, const ROAnything &config)
{
	ROAnything label = config["Label"];
	if (!label.IsNull()) {
		reply << " VALUE=\"" ;
		Render(reply, context, label);
		reply << "\"";
	}
	FieldRenderer::RenderOptions(reply, context, config);
}

void ButtonRenderer::RenderName(ostream &reply, Context &context, const ROAnything &config)
{
	ROAnything name = config["Name"];
	if (! name.IsNull()) {
		reply << Session::FR_BUTTONPREFIX;
		Render(reply, context, name);
	}
}

//---- ImageButtonRenderer -----------------------------------------------------------------
RegisterRenderer(ImageButtonRenderer);

ImageButtonRenderer::ImageButtonRenderer(const char *name) : FieldRenderer(name) {}

void ImageButtonRenderer::RenderOptions(ostream &reply, Context &context, const ROAnything &config)
{
	ROAnything src = config["Src"];
	if (!src.IsNull()) {
		reply << " SRC=\"" ;
		Render(reply, context, src);
		reply << "\"";
	}
	FieldRenderer::RenderOptions(reply, context, config);
}
void ImageButtonRenderer::RenderName(ostream &reply, Context &context, const ROAnything &config)
{
	ROAnything name = config["Name"];
	if (!name.IsNull()) {
		reply << Session::FR_IMAGEPREFIX;
		Render(reply, context, name);
	}
}

//---- ResetButtonRenderer -----------------------------------------------------------------
RegisterRenderer(ResetButtonRenderer);

ResetButtonRenderer::ResetButtonRenderer(const char *name) : ButtonRenderer(name) {}

//---- SelectBoxRenderer --------------------------------------------------------------
RegisterRenderer(SelectBoxRenderer);

SelectBoxRenderer::SelectBoxRenderer(const char *name) : FieldRenderer(name)  {}

bool SelectBoxRenderer::IsMultipleSelect(Context &context, const ROAnything &config)
{
	StartTrace(SelectBoxRenderer.IsMultipleSelect);
	String str;
	if (config.IsDefined("Multiple")) {
		RenderOnString(str, context, config["Multiple"]);
	}
	return (str.AsLong(0L) == 1L);
}

void SelectBoxRenderer::RenderOptions(ostream &reply, Context &context, const ROAnything &config)
{
	// render SIZE argument
	reply << " SIZE=\"";
	// length of list (default 1 see HTML-Spec.)
	String str;
	if (config.IsDefined("Size")) {
		RenderOnString(str, context, config["Size"]);
	}
	reply << str.AsLong(1L);
	reply << "\"";
	if (IsMultipleSelect(context, config)) {
		reply << " MULTIPLE";
	}
	FieldRenderer::RenderOptions(reply, context, config);
}

void SelectBoxRenderer::RenderAll(ostream &reply, Context &context, const ROAnything &config)
{
	StartTrace(SelectBoxRenderer.Render);
	TraceAny(config, "config");
	ROAnything n = config["Name"];
	if ( ! n.IsNull() ) {

		// streams name, size and (if defined) MULTIPLE into reply
		reply << "<SELECT NAME=\"";

		RenderName(reply, context, config);
		reply << "\"";

		RenderOptions(reply, context, config);

		reply << ">\n";

		if (config.IsDefined("PlainList")) {
			if (config.IsDefined("PrependList")) {
				RenderOptionList(reply, context, config, "PrependList");
			}
			RenderOptionList(reply, context, config, "PlainList");
			if (config.IsDefined("AppendList")) {
				RenderOptionList(reply, context, config, "AppendList");
			}
		} else {
			// retrieve data
			ROAnything list = config["List"];
			ROAnything llist = config["LookupList"];
			// "LookupList" overrides "List" settings!
			if (! llist.IsNull()) {
				String lookupname;
				RenderOnString(lookupname, context, llist);
				if (lookupname.Length() > 0) {
					list = context.Lookup(lookupname);
				}
			}

			long listEntries = list.GetSize();
			// puts the list entries to <OPTION>-tags
			for (long i = 0; i < listEntries; i++) {
				reply << "<OPTION";
				ROAnything opt = list[i]["Options"];
				if ( !opt.IsNull() ) {
					ROAnything value = opt["Value"];
					if ( !value.IsNull() ) {
						reply << " VALUE=\"" ;
						Render(reply, context, value);
						reply << "\"";
					}
					if ( opt.IsDefined("Selected") ) {
						String selectValue;
						Renderer::RenderOnString( selectValue, context, opt["Selected"] );
						if ( ! selectValue.IsEqual("0") ) {
							reply << " SELECTED";
						}
					}
				}
				reply << ">";
				ROAnything text = list[i]["Text"];
				if ( !text.IsNull() ) {
					Render(reply, context, text);
				}
				reply << "\n";
			}
		}
		reply << ("</SELECT>\n");
	} else {
		SysLog::Warning("SelectBoxRenderer::RenderAll: mandatory 'Name' slot is missing in configuration!");
	}
}

void SelectBoxRenderer::RenderOptionList(ostream &reply, Context &context, const ROAnything &config, String listname)
{
	StartTrace(SelectBoxRenderer.RenderOptionList);
	TraceAny(config, "config");
	// as the OptionListRenderer can be exchanged, check for another Renderer
	String strOptionRenderer("OptionListRenderer");
	if (config.IsDefined("OptionListRenderer")) {
		strOptionRenderer = "";
		RenderOnString(strOptionRenderer, context, config["OptionListRenderer"]);
	}
	Trace("OptionListRenderer used:[" << strOptionRenderer << "]");
	Renderer *pRenderer = Renderer::FindRenderer(strOptionRenderer);
	if (pRenderer) {
		Anything rendererConfig;
		rendererConfig["ListData"] = config[listname].DeepClone();
		rendererConfig["EntryStore"] = "SelectBoxOption";
		rendererConfig["IndexSlot"] = "SelectBoxOptionIndex";
		rendererConfig["SlotnameSlot"] = "SelectBoxOptionSlotname";
		rendererConfig["EntryRenderer"] = "dummy";
		ROAnything value;
		if ( config.LookupPath(value, "ValueRenderer") ) {
			rendererConfig["ValueRenderer"] = value.DeepClone();
		}
		if ( config.LookupPath(value, "SelectedRenderer") ) {
			rendererConfig["SelectedRenderer"] = value.DeepClone();
		}
		if ( config.LookupPath(value, "TextRenderer") ) {
			rendererConfig["TextRenderer"] = value.DeepClone();
		}
		TraceAny(rendererConfig, "OptionListRenderer Config");
		pRenderer->RenderAll(reply, context, rendererConfig);
	}
}

RegisterRenderer(OptionListRenderer);

void OptionListRenderer::RenderEntry(ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, Anything &listItem)
{
	StartTrace(OptionListRenderer.RenderEntry);
	ROAnything value;
	TraceAny(listItem, "listItem");
	reply << "<OPTION";
	{
		if ( config.LookupPath(value, "ValueRenderer") ) {
			RenderValue(reply, c, config, value, listItem);
		}
		if ( config.LookupPath(value, "SelectedRenderer") ) {
			RenderSelected(reply, c, config, value, listItem);
		}
	}
	reply << ">";
	if ( config.LookupPath(value, "TextRenderer") ) {
		RenderText(reply, c, config, value, listItem);
	}
	reply << "\n";
}
void OptionListRenderer::RenderValue(ostream &reply, Context &c, const ROAnything &config, const ROAnything &valueConfig, Anything &listItem)
{
	StartTrace(OptionListRenderer.RenderValue);
	reply << " VALUE=\"";
	Render(reply, c, valueConfig);
	reply << "\"";
}
void OptionListRenderer::RenderSelected(ostream &reply, Context &c, const ROAnything &config, const ROAnything &selectedConfig, Anything &listItem)
{
	StartTrace(OptionListRenderer.RenderSelected);
	String strSel;
	RenderOnString(strSel, c, selectedConfig);
	if (strSel.Length()) {
		ROAnything result = c.Lookup(strSel);
		if (result.GetType() == Anything::eLong && result.AsBool()) {
			reply << " SELECTED";
		}
	}
}

void OptionListRenderer::RenderText(ostream &reply, Context &c, const ROAnything &config, const ROAnything &textConfig, Anything &listItem)
{
	StartTrace(OptionListRenderer.RenderText);
	Render(reply, c, textConfig);
}

//---- CheckBoxRenderer --------------------------------------------------------------
RegisterRenderer(CheckBoxRenderer);

CheckBoxRenderer::CheckBoxRenderer(const char *name) : FieldRenderer(name) {}

void CheckBoxRenderer::RenderOptions(ostream &reply, Context &context, const ROAnything &config)
{
	// CAUTION: semantics changed as compared to the FieldRenderer version:
	// In old version "Value" was always looked up in the context if a char* was
	// specified. Now a renderer specification has to be used for that effect!

	ROAnything val = config[ "Value"];
	String value;
	if (!val.IsNull()) {
		RenderOnString(value, context, val);
	}

	if (value == "on") {
		reply << (" CHECKED");
	}
	FieldRenderer::RenderOptions(reply, context, config);
}

//---- RadioButtonRenderer --------------------------------------------------------------
RegisterRenderer(RadioButtonRenderer);

RadioButtonRenderer::RadioButtonRenderer(const char *name) : FieldRenderer(name) {}
void RadioButtonRenderer::RenderOptions(ostream &reply, Context &context, const ROAnything &config)
{
	// CAUTION: semantics changed as compared to the FieldRenderer version:
	// In old version "Value" was always looked up in the context if a char* was
	// specified. Now a renderer specification has to be used for that effect!

	ROAnything value = config["Value"];
	if (!value.IsNull()) {
		reply << " VALUE=\"";
		Render(reply, context, value);
		reply << "\"";
	}
	if (config.IsDefined("Checked")) {
		reply << (" CHECKED");
	} else if (config.IsDefined("EvaluateChecked")) {
		String checkedValue;
		RenderOnString(checkedValue, context, config["EvaluateChecked"]);
		if (Anything(checkedValue).AsLong(0)) {
			reply << (" CHECKED");
		}
	}
	FieldRenderer::RenderOptions(reply, context, config);
}

//---- FieldRenderer ----------------------------------------------------------------
FieldRenderer::FieldRenderer(const char *name) : Renderer(name)
{
}
void FieldRenderer::RenderAll(ostream &reply, Context &context, const ROAnything &config)
{
	StartTrace(FieldRenderer.Render);
	TraceAny(config, "config");
	if (config.IsDefined("Name")) {
		reply << "<INPUT TYPE=\"" ;
		RenderType(reply, context, config);
		reply << "\" NAME=\"";
		RenderName(reply, context, config);
		reply << "\"";
		RenderOptions(reply, context, config);
		reply << ">";
	} else {
		SysLog::Error("FieldRenderer::RenderAll: mandatory 'Name' slot is missing in configuration!");
	}
}

void FieldRenderer::RenderName(ostream &reply, Context &context, const ROAnything &config)
{
	ROAnything name = config["Name"];
	if (!name.IsNull()) {
		reply << Session::FR_FIELDPREFIX;
		Render(reply, context, name);
	}
}

void FieldRenderer::RenderOptions(ostream &reply, Context &context, const ROAnything &config)
{
	// Renders any Key/Value pairs given in configs /Options Slot
	PrintOptions3(reply, context, config);
}

//---- TextFieldRenderer --------------------------------------------------------------
RegisterRenderer(TextFieldRenderer);

void TextFieldRenderer::RenderType(ostream &reply, Context &, const ROAnything &config)
{
	const char *type = "TEXT";
	if (config.IsDefined("Unreadable")) {
		type = "PASSWORD";
	}
	reply << type;
}
void TextFieldRenderer::RenderOptions(ostream &reply, Context &context, const ROAnything &config)
{
	ROAnything value = config["Value"];
	if (!value.IsNull()) {
		String str;
		RenderOnString(str, context, value);
		reply << " VALUE=\"" << str << "\"";
	}

	ROAnything sz = config["Size"];
	if (!sz.IsNull()) {
		String str;
		RenderOnString(str, context, sz);
		reply << " SIZE=\"" << str  << "\"";
	}
	ROAnything len = config["Maxlength"];
	if (!len.IsNull()) {
		String str;
		RenderOnString(str, context, len);
		reply << " MAXLENGTH=\"" << str << "\"";
	}
	FieldRenderer::RenderOptions(reply, context, config);
}

//---- TextAreaRenderer --------------------------------------------------------------
RegisterRenderer(TextAreaRenderer);
TextAreaRenderer::TextAreaRenderer(const char *name) : FieldRenderer(name)
{
}

void TextAreaRenderer::RenderAll(ostream &reply, Context &context,
								 const ROAnything &config)
{
	StartTrace(TextAreaRenderer.Render);
	TraceAny(config, "config");

	ROAnything name = config["Name"];
	if (!name.IsNull()) {
		reply << "<TEXTAREA NAME=\"";

		// render name
		RenderName(reply, context, config);
		reply << "\"";
		// render options
		ROAnything option = config["Width"];
		if (!option.IsNull()) {
			reply << " COLS=" << option.AsString();
		}
		option = config["Height"];
		if (!option.IsNull()) {
			reply << " ROWS=" << option.AsString();
		}
		option = config["Wrap"];
		if (!option.IsNull()) {
			reply << " WRAP=" << option.AsString();
		}

		RenderOptions(reply, context, config);

		reply << (">");

		// render default text
		ROAnything dft = config["Value"];
		if (!dft.IsNull()) {
			Render(reply, context, dft);
		}

		reply << "</TEXTAREA>";
	} else {
		SysLog::Error("TextAreaRenderer::RenderAll() mandatory 'Name' slot is missing in configuration!");
	}
}

//---- FieldNameRenderer ----------------------------------------------------------------
RegisterRenderer(FieldNameRenderer);

FieldNameRenderer::FieldNameRenderer(const char *name) : Renderer(name)
{
}
void FieldNameRenderer::RenderAll(ostream &reply, Context &context,
								  const ROAnything &config)
{
	StartTrace(FieldNameRenderer.Render);
	TraceAny(config, "config");
	const char *name = "";
	if (config.IsDefined("Default")) {
		name = config["Default"].AsCharPtr(name);
	}

	if (config.IsDefined("LookupName")) {
		const char *lookupname = config["LookupName"].AsCharPtr(0);
		if (lookupname) {
			name = context.Lookup(lookupname, name);
		}
	}
	reply << Session::FR_FIELDPREFIX << name;
}

//---- FileBrowseRenderer -----------------------------------------------------------------
//-- Renders a filebrowse inputbox used in down- or upload forms  -------------------------
RegisterRenderer(FileBrowseRenderer);

FileBrowseRenderer::FileBrowseRenderer(const char *name) : TextFieldRenderer(name)
{
}
