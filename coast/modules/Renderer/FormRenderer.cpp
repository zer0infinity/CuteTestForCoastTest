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
#include "Session.h"
#include "TemplateParser.h"
#include "Dbg.h"

//---- FormRenderer -------------------------------------------------------------------
RegisterRenderer(FormRenderer);
FormRenderer::FormRenderer(const char *name)
	: HTMLTemplateRenderer(name)
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
	static Renderer *r = Renderer::FindRenderer("URLRenderer");
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
	reply << " size='1\'";
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
	reply << " size=\"";
	// length of list (default 1 see HTML-Spec.)
	String str;
	if (config.IsDefined("Size")) {
		RenderOnString(str, context, config["Size"]);
	}
	reply << str.AsLong(1L);
	reply << "\"";
	if (IsMultipleSelect(context, config)) {
		reply << " multiple";
	}
	FieldRenderer::RenderOptions(reply, context, config);
}

void SelectBoxRenderer::RenderAll(ostream &reply, Context &context, const ROAnything &config)
{
	StartTrace(SelectBoxRenderer.Render);
	TraceAny(config, "config");
	ROAnything n = config["Name"];
	if ( !n.IsNull() ) {
		// streams name, size and (if defined) multiple into reply
		reply << "<select name=\"";

		RenderName(reply, context, config);
		reply << "\"";

		RenderOptions(reply, context, config);

		reply << ">\n";

		ROAnything roaListName, roaListData;
		// select list to process
		if ( config.LookupPath(roaListName, "ListName") || config.LookupPath(roaListData, "ListData") ) {
			String strListName, strPrependListName, strAppendListName;
			RenderOnString(strListName, context, roaListName);
			RenderOnString(strPrependListName, context, config["PrependListName"]);
			RenderOnString(strAppendListName, context, config["AppendListName"]);

			ROAnything roaTempList;
			if ( strPrependListName.Length() ) {
				RenderOptionList(reply, context, config, strPrependListName);
			} else if ( context.Lookup("PrependListData", roaTempList) ) {
				strPrependListName = "SelectBoxData";
				Context::PushPopEntry<ROAnything> aEntryData(context, strPrependListName, roaTempList, strPrependListName);
				RenderOptionList(reply, context, config, strPrependListName);
			}

			if ( strListName.Length() ) {
				RenderOptionList(reply, context, config, strListName);
			} else {
				strListName = "SelectBoxData";
				Context::PushPopEntry<ROAnything> aEntryData(context, strListName, roaListData, strListName);
				RenderOptionList(reply, context, config, strListName);
			}

			if ( strAppendListName.Length() ) {
				RenderOptionList(reply, context, config, strAppendListName);
			} else if ( context.Lookup("AppendListData", roaTempList) ) {
				strAppendListName = "SelectBoxData";
				Context::PushPopEntry<ROAnything> aEntryData(context, strAppendListName, roaTempList, strAppendListName);
				RenderOptionList(reply, context, config, strAppendListName);
			}
		}
		reply << "</select>\n";
	} else {
		SystemLog::Warning("SelectBoxRenderer::RenderAll: mandatory 'Name' slot is missing in configuration!");
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
		rendererConfig["ListName"] = listname;
		rendererConfig["EntryStore"] = RenderToStringWithDefault(context, config["EntryStore"], "SelectBoxOption");
		rendererConfig["IndexSlot"] = config["IndexSlot"].AsString("SelectBoxOptionIndex");
		rendererConfig["SlotnameSlot"] = config["SlotnameSlot"].AsString("SelectBoxOptionSlotname");
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
		if ( config.LookupPath(value, "OptionGroup") ) {
			rendererConfig["OptionGroup"] = value.DeepClone();
		}
		if ( config.LookupPath(value, "Options.class") ) {
			rendererConfig["Options"]["class"] = value.DeepClone();
		}
		TraceAny(rendererConfig, "OptionListRenderer Config");
		pRenderer->RenderAll(reply, context, rendererConfig);
	}
}

RegisterRenderer(OptionListRenderer);

void OptionListRenderer::RenderEntry(ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(OptionListRenderer.RenderEntry);
	ROAnything value;
	TraceAny(listItem, "listItem");
	reply << "<option";
	{
		PrintOptions3(reply, c, config);
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
	reply << "</option>\n";
}

long OptionListRenderer::EntryHeaderNrToBeRendered(Context &ctx, const ROAnything &config, Anything &anyRenderState)
{
	StartTrace(OptionListRenderer.EntryHeaderNrToBeRendered);
	if ( config.IsDefined("OptionGroup") ) {
		return 0;
	}
	return -1;
}

bool OptionListRenderer::EntryFooterHasToBeRendered(Context &ctx, const ROAnything &config, Anything &anyRenderState)
{
	StartTrace(OptionListRenderer.EntryFooterHasToBeRendered);
	bool render(false);
	if ( config.IsDefined("OptionGroup") ) {
		long lListSize = anyRenderState["ListSize"].AsLong();
		// these indexes are 1-based, not zero!
		long entryIndex = anyRenderState["RenderIndex"].AsLong() + 1L;
		// need only to do this after we rendered the last item
		render = ( entryIndex == lListSize );
	}
	return render;
}

bool OptionListRenderer::GetEntryHeader(const ROAnything &config, long nr, ROAnything &roaEntryHeader, Anything &anyRenderState)
{
	StartTrace(OptionListRenderer.GetEntryHeader);
	if ( config.LookupPath(roaEntryHeader, "OptionGroup") ) {
		TraceAny(roaEntryHeader, "using following optiongroup config");
	}
	return !roaEntryHeader.IsNull();
}

bool OptionListRenderer::GetEntryFooter(const ROAnything &config, ROAnything &roaEntryFooter, Anything &anyRenderState)
{
	StartTrace(OptionListRenderer.GetEntryFooter);
	if ( config.LookupPath(roaEntryFooter, "OptionGroup") ) {
		TraceAny(roaEntryFooter, "using following optiongroup config");
	}
	return !roaEntryFooter.IsNull();
}

void OptionListRenderer::RenderEntryHeader(ostream &reply, Context &ctx, const ROAnything &entryHeader, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(OptionListRenderer.RenderEntryHeader);
	if ( !entryHeader.IsNull() ) {
		String strCurGroupKey, strEntryKey;
		// check if we are already inside an option group and get its key
		if ( anyRenderState.IsDefined("GroupKey") ) {
			strCurGroupKey = anyRenderState["GroupKey"].AsString();
			Trace("current key in state [" << strCurGroupKey << "]");
		}
		// render current key
		RenderOnStringWithDefault(strEntryKey, ctx, entryHeader["KeyRenderer"], strCurGroupKey);
		Trace("group key of entry [" << strEntryKey << "]");

		// check if the key changed and render end tag if so
		if ( strCurGroupKey.Length() && strEntryKey.Length() && !strCurGroupKey.IsEqual(strEntryKey) ) {
			Trace("key changed, rendering end tag");
			// render end tag
			reply << "</optgroup>\n";
			strCurGroupKey.Trim(0);
			anyRenderState.Remove("GroupKey");
		}

		// key changed
		if ( !strCurGroupKey.Length() && strEntryKey.Length() ) {
			String strLabel;
			RenderOnString(strLabel, ctx, entryHeader["LabelRenderer"]);
			// render start tag
			reply << "<optgroup label='" << strLabel << "'>\n";
			Trace("key changed, rendering start tag with label [" << strLabel << "]");
			anyRenderState["GroupKey"] = strEntryKey;
		}
	}
}

void OptionListRenderer::RenderEntryFooter(ostream &reply, Context &ctx, const ROAnything &entryFooter, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(OptionListRenderer.RenderEntryFooter);
	if ( !entryFooter.IsNull() ) {
		// check if we are already inside an option group and get its key
		if ( anyRenderState.IsDefined("GroupKey") ) {
			String strCurGroupKey;
			strCurGroupKey = anyRenderState["GroupKey"].AsString();
			Trace("current key in state [" << strCurGroupKey << "]");
			long lListSize = anyRenderState["ListSize"].AsLong();
			// these indexes are 1-based, not zero!
			long entryIndex = anyRenderState["RenderIndex"].AsLong() + 1L;
			// need only to do this after we rendered the last item
			if ( entryIndex == lListSize ) {
				// render end tag
				reply << "</optgroup>\n";
				anyRenderState.Remove("GroupKey");
			}
		}
	}
}

void OptionListRenderer::RenderValue(ostream &reply, Context &c, const ROAnything &config, const ROAnything &valueConfig, const ROAnything &listItem)
{
	StartTrace(OptionListRenderer.RenderValue);
	reply << " value=\"";
	Render(reply, c, valueConfig);
	reply << "\"";
}

void OptionListRenderer::RenderSelected(ostream &reply, Context &c, const ROAnything &config, const ROAnything &selectedConfig, const ROAnything &listItem)
{
	StartTrace(OptionListRenderer.RenderSelected);
	String strSel;
	RenderOnString(strSel, c, selectedConfig);
	if ( strSel.Length() ) {
		ROAnything result = c.Lookup(strSel);
		if ( result.GetType() == AnyLongType && result.AsBool() ) {
			reply << " selected";
		}
	}
}

void OptionListRenderer::RenderText(ostream &reply, Context &c, const ROAnything &config, const ROAnything &textConfig, const ROAnything &listItem)
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
		SystemLog::Error("FieldRenderer::RenderAll: mandatory 'Name' slot is missing in configuration!");
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
		SystemLog::Error("TextAreaRenderer::RenderAll() mandatory 'Name' slot is missing in configuration!");
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
