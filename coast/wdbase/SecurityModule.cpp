/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SecurityModule.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Registry.h"
#include "SysLog.h"
#include "Dbg.h"
#include "System.h"
#include "DiffTimer.h" /* for GetHRTIME() */

#include <stdlib.h>

// ------------------- SecurityModule ---------------------------------------------
RegisterModule(SecurityModule);

const char *SecurityItem::fgcLegacyMasterKey = ".wsoäö3$n4.ert?";

#define FindSecurityItemWithDefault(var,name,Type)\
	const Type *var = SafeCast(FindSecurityItem(name),Type);\
	if (!var){ Trace("using standard item: " _QUOTE_(Type));\
		var=SafeCast(FindSecurityItem(_QUOTE_(Type)),Type);\
	} Assert(var);

//----- SecurityItem --- factored commonality

long SecurityItem::GetNamePrefixFromEncodedText(String &scramblername, const String &encodedText)
{
	StartTrace(SecurityItem.GetNamePrefixFromEncodedText);
	Trace("Scrambled Text: " << encodedText);
	scramblername.Trim(0L);
	for (long i = 0, sz = encodedText.Length(); i <= sz && ':' != encodedText[i] && '-' != encodedText[i]; ++i) {
		scramblername.Append(encodedText[i]);
	}

	Trace("scramblername: " << scramblername);
	return scramblername.Length();
}

void SecurityItem::Encode(const char *itemtouse, String &encodedText, const String &cleartext)
{
	if (String() == itemtouse) {
		itemtouse = "Encoder";
	}
	StartTrace1(SecurityItem.Encode, "using SecurityItem <" << itemtouse << ">");
	FindSecurityItemWithDefault(encoder, itemtouse, SecurityItem);

	String name(itemtouse);
	if (encoder) {
		encoder->GetName(name);
	}
	String encoded;
	if (encoder) {
		encoder->DoEncode(encoded, cleartext);
	}
	Trace("adding item name for later decoding <" << name << "> wanted to use <" << itemtouse << ">\n");
	encodedText << name << '-' << encoded;
	Trace("result :<" << encodedText << ">\n");
}

bool SecurityItem::Decode(String &cleartext, const String &encodedText)
{
	StartTrace(SecurityItem.Decode);
	String encodername;
	long delimiterposition = 0;
	if (!(delimiterposition = GetNamePrefixFromEncodedText(encodername, encodedText))) {
		Trace("encodername not in encodedText! : [" << encodedText << "]");
		return false;
	}
	Trace("using encoder <" << encodername << ">");
	FindSecurityItemWithDefault(encoder, encodername, SecurityItem);
	return encoder && encoder->DoDecode(cleartext, encodedText.SubString(delimiterposition + 1, encodedText.Length()));
}

bool SecurityItem::DoGetConfigName(const char *category, const char *objName, String &configFileName)
{
	configFileName = "SecurityItems";
	return true;
}

bool SecurityItem::DoLoadConfig(const char *category)
{
	StartTrace1(SecurityItem.DoLoadConfig, fName);
	SysLog::WriteToStderr(".");
	if ( HierarchConfNamed::DoLoadConfig(category)) {
		if ( fConfig.IsDefined(fName) ) {
			// trx impls use only a subset of the whole configuration file
			fConfig = fConfig[fName];
			TraceAny(fConfig, "Config:");
			Assert(!fConfig.IsNull());

			return Init(fConfig);
		}
		Trace("nothing to configure");
		return true; // nothing to configure
	}
	Trace("DoLoadConfig of HierarchConfNamed failed");
	return false; // faile to load config
}

bool SecurityItem::Init(ROAnything config)
{
	StartTrace1(SecurityItem.Init, fName);
	bool bRetCode = false;
	if (config.IsDefined("Key")) {
		Trace("using key from config");
		InitKey(config["Key"].AsString());
		bRetCode = true;
	} else if (config.IsDefined("KeyLoc")) {
		Trace("looking for key file");
		String key;
		if (DoLoadKeyFile(config["KeyLoc"].AsCharPtr(), key)) {
			Trace("Initializing key: <" << key << ">");
			InitKey(key);
			bRetCode = true;
		} else {
			String msg;
			msg << "didn't find key file " << config["KeyLoc"].AsCharPtr("undefined name");
			Trace(msg);
			SYSERROR(msg);
		}
	}
	return bRetCode;
}

// the following code is from keyscrambler in Frontdoor which might be obsoleted by that
bool SecurityItem::DoLoadKeyFile(const char *name, String &key)
{
	StartTrace1(SecurityItem.DoLoadKeyFile, name);
	// search file with path
	// open file with relative path
	String resolvedFileName =  System::GetFilePath(name, (const char *)0);
	iostream *Ios = System::OpenIStream(resolvedFileName, "");
	if ( Ios ) {
		String sBuf(4096);
		char *buf = (char *)(const char *) sBuf;
		while ( !(Ios->eof()) ) {
			Ios->getline(buf, 4096);
			key.Append(buf);
		}
		delete Ios;
		return (key.Length() > 0L);

	}
	Trace("Unable to open file: [" << name << " resolved: [" << resolvedFileName << "]");
	return false;
}

String SecurityItem::GenerateRandomString(long length)
{
	String result(length);
	for (; length > 0; --length) {
#if !defined(WIN32)
		result.Append((char)lrand48());
#else
		result.Append((char)rand());
#endif
	}
	return result;
}

//----
String	SecurityModule::fgScrambler(Storage::Global());
String	SecurityModule::fgEncoder(Storage::Global());
String	SecurityModule::fgSigner(Storage::Global());
String	SecurityModule::fgCompressor(Storage::Global());

SecurityModule::SecurityModule(const char *name) : WDModule(name)
{
}

SecurityModule::~SecurityModule()
{
}

bool SecurityModule::Init(const ROAnything config)
/* in: config: Server configuration
  ret: true if successful, false otherwise.
 what: Only writes a message and installs itself
 note: Derived classes must call this method at the end of their Init to install
	   themselves! (InstallHandler is private)
	   Usually derived classes do not need to override Init if they do not want
	   to do anything by themselves. Installation happens automatically.
*/
{
	StartTrace( SecurityModule.Init );
	bool result = true;
	TraceAny(config, "Config");
	SysLog::WriteToStderr(String("\t") << fName << ". ");
#if !defined(WIN32)
	srand48((long)GetHRTIME());
#else
	srand((long)GetHRTIME());
#endif
	ROAnything moduleConfig(config[fName]);
	TraceAny(moduleConfig, "ModuleConfig");

	fgScrambler = moduleConfig["Scrambler"].AsCharPtr("Scrambler");
	fgEncoder = moduleConfig["Encoder"].AsCharPtr("Encoder");
	fgSigner = moduleConfig["Signer"].AsCharPtr("Signer");
	fgCompressor = moduleConfig["Compressor"].AsCharPtr("Compressor");

	HierarchyInstaller hi("SecurityItem");
	result =  RegisterableObject::Install(moduleConfig["SecurityItems"], "SecurityItem", &hi);

	// TableCompressor is special, it needs the complete Config.any for /Expand Roles /Expand Pages /Expand Actions
	// can be removed if this is never used. Frontdoor doesn't rely on this feature
	Compressor *compressor = Compressor::FindCompressor(fgCompressor);
	if ( compressor ) {
		compressor->Init(config);
	}
	SysLog::WriteToStderr(result ? "done\n" : "failed\n");
	Trace("result :" << (result ? "done\n" : "failed\n"));
	return result;
}

bool SecurityModule::ResetInit(const ROAnything config)
{
	StartTrace(SecurityModule.ResetInit);
	ROAnything moduleConfig(config[fName]);
	if ( moduleConfig["DoNotReset"].AsBool(0) == 1 ) {
		String msg;
		msg << "\t" << fName << "  Configured not to call Init() on reset\n";
		SysLog::WriteToStderr(msg);
		return true;
	}
	return SecurityModule::Init(config);
}

bool SecurityModule::ResetFinis(const ROAnything config)
{
	StartTrace(SecurityModule.ResetFinis);
	ROAnything moduleConfig(config[fName]);
	if ( moduleConfig["DoNotReset"].AsBool(0) == 1 ) {
		String msg;
		msg << "\t" << fName << "  Configured not to call Finis() on reset\n";
		SysLog::WriteToStderr(msg);
		return true;
	}
	AliasTerminator at("SecurityItems");
	return RegisterableObject::ResetTerminate("SecurityItem", &at);
}

bool SecurityModule::Finis()
/* ret: true if successful, false otherwise.
*/
{
	StartTrace(SecurityModule.Finis);
	return StdFinis("SecurityItem", "SecurityItems");
}

//#define URLSTAT_TRACING
// URL encryption an decryption
void SecurityModule::ScrambleState(String &result, const Anything &state)
/* in: state: some data, usually the state information for a Coast link
  out: result: The state data appended in linearized form.
 what: Standard URLUtils, no real encryption.
*/
{
	StartTrace(SecurityModule.ScrambleState);

	TraceAny(state, "input state");

	String compressedText;
	String signedText;
	String cryptText;
	String encodedText;

	Compressor::Compress(fgCompressor, compressedText, state);

	if (compressedText.Length() > 0) {
		Trace("compressed text:<" << compressedText << ">");

		Signer::Sign(fgSigner, signedText, compressedText);
		Trace("signed text:<" << signedText << ">");
		// scramble cleartext url
		Scrambler::Scramble(fgScrambler, cryptText, signedText);
		Trace("scrambled text:<" << cryptText << ">");
		// encode scrambled url
		Encoder::Encode(fgEncoder, encodedText, cryptText);

		// Append it to URL
		result.Append(encodedText);

#ifdef URLSTAT_TRACING
		SysLog::WriteToStderr(String("URL comp<") << compressedText << ">\n");
		SysLog::WriteToStderr("URL: (c, e, c, p) ");
		// do some length tracking
		long rawLen =	compressedText.Length();
		long resLen =	encodedText.Length();
		long cryptLen =	cryptText.Length();
		SysLog::WriteToStderr(String() << rawLen << ", ");
		SysLog::WriteToStderr(String() << cryptLen << ", ");
		SysLog::WriteToStderr(String() << resLen << ", ");
		SysLog::WriteToStderr(String() << ((resLen * 100) / (rawLen)) << "%\n");
#endif
	}
	Trace("resulting text <" << result << ">");

} // DoScrambleState

bool SecurityModule::UnscrambleState(Anything &state, const String &s)
/* in: s: A pointer to a null terminated array of char containing scrambled data
		  This contains exactly the characters appended by the previous method.
  out: state: The unscrambled data
 rets: true if unscrambling was possible, false if somebody tried to unscramble garbage
 what: Standard URLUtils decryption, does not decrypt serious URL's
*/
{
	StartTrace(SecurityModule.UnscrambleState);
	Trace("StateString: " << s);
	String crypt(s.Length());
	if ( Encoder::Decode( crypt, s ) ) {
		Trace("Decoded ok: <" << crypt << ">");
		String signedText(crypt.Length());
		if ( Scrambler::Unscramble(signedText, crypt) ) {
			Trace("Unscrambled ok: <" << signedText << ">");
			String compressedText(signedText.Length());
			if ( Signer::Check(compressedText, signedText) ) {
				Trace("Checked ok: <" << compressedText << ">");
#ifdef URLSTAT_TRACING
				SysLog::WriteToStderr(String("URL2exp<") << compressedText << ">\n");
#endif
				if ( Compressor::Expand(state, compressedText) ) {
					TraceAny(state, "state: ");
					return true;
				}
			}
		}
	}
	if (crypt == "AnyArrayImpl") {
		SysLog::Error("Expected one token to unscramble, got > 1. Maybe duplicate cookies, /Domain wrong.");
	}
	Trace("Unscrambling failed");
	return false;
} // DoUnscrambleState

//--- SecurityItem --------------------------------------------------------------
RegCacheImpl(SecurityItem);		// FindSecurityItem()
//--- Scrambler -----------------------------------------------------------------

RegisterScrambler(Scrambler);
RegisterAliasSecurityItem(ascs, Scrambler);
// register legacy keymanager generated names with a 0 appended
RegisterAliasSecurityItem(ascs0, Scrambler);
RegisterAliasSecurityItem(Scrambler0, Scrambler);

Scrambler::Scrambler(const char *name) : SecurityItem(name)
{

}

Scrambler::~Scrambler()
{

}

void Scrambler::Scramble(const char *encodername, String &encodedText, const String &cleartext)
{
	if (String() == encodername) {
		encodername = "Scrambler";
	}
	StartTrace(Scrambler.Scramble);
	Trace("Scrambler: " << encodername);
	SecurityItem::Encode(encodername, encodedText, cleartext);
}

bool Scrambler::Unscramble(String &cleartext, const String &encodedText)
{
	StartTrace(Scrambler.Unscramble);
	Trace("Scrambled Text: " << encodedText);
	return SecurityItem::Decode(cleartext, encodedText);
}
//--- Encoder -----------------------------------------------------------------
RegisterEncoder(Encoder);
RegisterAliasSecurityItem(asce, Encoder);

Encoder::Encoder(const char *name) : SecurityItem(name)
{
}

Encoder::~Encoder()
{
}

//--- Compressor -----------------------------------------------------------------
RegisterCompressor(Compressor);
RegisterAliasSecurityItem(comp, Compressor);

Compressor::Compressor(const char *name) : SecurityItem(name)
{
}

Compressor::~Compressor()
{
}

void Compressor::Compress(const char *theName, String &encodedText, const Anything &cleartext)
{
	if (String() == theName) {
		theName = "Compressor";
	}
	Compressor *compressor = Compressor::FindCompressor(theName);

	if ( !compressor ) {
		// do nothing
		return;
	}
	String name;
	compressor->GetName(name);
	String result;
	compressor->DoCompress(result, cleartext);
	encodedText << name << '-' << result;
}

bool Compressor::Expand(Anything &result, const String &encodedText)
{
	StartTrace(Compressor.Decode);
	String compressorname;
	long delimiterposition = 0;
	if (!(delimiterposition = GetNamePrefixFromEncodedText(compressorname, encodedText))) {
		return false;
	}
	Compressor *compressor = Compressor::FindCompressor(compressorname);
	// FIXME: inconsistency!
	// no problem if we don't find the compressor but inconsistent!
	return compressor && compressor->DoExpand(result, encodedText.SubString(delimiterposition + 1, encodedText.Length()));
}

void Compressor::DoCompress(String &encodedText, const Anything &dataIn)
{
	// default implementation:
	// no compression at all
	// just stream out anything

	OStringStream os(&encodedText);
	dataIn.PrintOn(os, false);
}

bool Compressor::Init(ROAnything config)
{
	// do nothing
	return true;
}

bool Compressor::DoExpand(Anything &dataOut, const String &scrambledText)
{
	// default implementation:
	// no expansion at all
	// just stream in anything

	IStringStream is(scrambledText);
	dataOut.Import(is);

	return true;
}

//--- Signer -----------------------------------------------------------------

RegisterSigner(Signer);
RegisterAliasSecurityItem(nos, Signer);
Signer::Signer() : SecurityItem("nos")
{
}

Signer::Signer(const char *name) : SecurityItem(name)
{
}

Signer::~Signer()
{
}

void Signer::Sign(const char *signername, String &encodedText, const String &cleartext)
{
	if (String() == signername) {
		signername = "Signer";
	}
	StartTrace(Signer.Sign);
	Trace("Signer: " << signername);
	SecurityItem::Encode(signername, encodedText, cleartext);
}

bool Signer::Check(String &cleartext, const String &scrambledText)
{
	StartTrace(Signer.Check);
	return SecurityItem::Decode(cleartext, scrambledText);
}
