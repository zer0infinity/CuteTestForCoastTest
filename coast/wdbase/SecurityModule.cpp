/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "SecurityModule.h"
#include "StringStream.h"
#include "Registry.h"
#include "SystemLog.h"
#include "SystemFile.h"
#include "DiffTimer.h"
#include "Policy.h"

RegisterModule(SecurityModule);

const char *SecurityItem::fgcLegacyMasterKey = ".u8&ey%2lv$skb?";

#include "InitFinisManager.h"
#include "singleton.hpp"
namespace {
	class RandomNumberInitializer {
	public:
		RandomNumberInitializer() {
#if !defined(WIN32)
			srand48(DiffTimer::getCurrentRawTime());
#else
			srand(DiffTimer.getCurrentRawTime());
#endif
			InitFinisManager::IFMTrace("RandomNumber::Initialized\n");
		}
		~RandomNumberInitializer() {
			InitFinisManager::IFMTrace("RandomNumber::Finalized\n");
		}
		unsigned long nextRandomNumber() const {
#if !defined(WIN32)
			return lrand48();
#else
			return rand();
#endif
		}
	};
    typedef coast::utility::singleton_default<RandomNumberInitializer> RandomNumberInitializerSingleton;
}

namespace coast {
	namespace security {
		unsigned long nextRandomNumber() {
			return RandomNumberInitializerSingleton::instance().nextRandomNumber();
		}
		String generateRandomString(long length) {
			StartTrace(Coast.Security.generateRandomString);
			String result(length);
			typedef unsigned long randNumberType;
			std::size_t const randomBytes=sizeof(randNumberType);
			while (result.Length() < length) {
				randNumberType randNumber=nextRandomNumber();
				result.AppendAsHex(reinterpret_cast<const unsigned char*>(&randNumber), randomBytes);
			}
			result.Trim(length);
			return result;
		}
	}
}

#define FindSecurityItemWithDefault(var,name,Type)\
	const Type *var = SafeCast(FindSecurityItem(name),Type);\
	if (!var){ Trace("using standard item: " _QUOTE_(Type));\
		var=SafeCast(FindSecurityItem(_QUOTE_(Type)),Type);\
	} Assert(var);

long SecurityItem::GetNamePrefixFromEncodedText(String &scramblername, const String &encodedText) {
	StartTrace(SecurityItem.GetNamePrefixFromEncodedText);
	Trace("Scrambled Text: " << encodedText);
	scramblername.Trim(0L);
	for (long i = 0, sz = encodedText.Length(); i <= sz && ':' != encodedText[i] && '-' != encodedText[i]; ++i) {
		scramblername.Append(encodedText[i]);
	}
	Trace("scramblername: " << scramblername);
	return scramblername.Length();
}

void SecurityItem::Encode(const char *itemtouse, String &encodedText, const String &cleartext) {
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

bool SecurityItem::Decode(String &cleartext, const String &encodedText) {
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

bool SecurityItem::DoGetConfigName(const char *category, const char *objName, String &configFileName) const {
	configFileName = "SecurityItems";
	return true;
}

bool SecurityItem::DoLoadConfig(const char *category) {
	StartTrace1(SecurityItem.DoLoadConfig, fName);
	SystemLog::WriteToStderr(".");
	if (HierarchConfNamed::DoLoadConfig(category)) {
		if (fConfig.IsDefined(fName)) {
			fConfig = fConfig[fName];
			TraceAny(fConfig, "Config:");Assert(!fConfig.IsNull());
			return Init(fConfig);
		}
		Trace("nothing to configure");
		return true;
	}
	Trace("DoLoadConfig of HierarchConfNamed failed");
	return false;
}

bool SecurityItem::Init(ROAnything config) {
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

bool SecurityItem::DoLoadKeyFile(const char *name, String &key) {
	StartTrace1(SecurityItem.DoLoadKeyFile, name);
	String resolvedFileName = coast::system::GetFilePath(name, (const char *) 0);
	std::iostream *Ios = coast::system::OpenIStream(resolvedFileName, "");
	if (Ios) {
		String sBuf(4096);
		char *buf = (char *) (const char *) sBuf;
		while (!(Ios->eof())) {
			Ios->getline(buf, 4096);
			key.Append(buf);
		}
		delete Ios;
		return (key.Length() > 0L);

	}

	Trace("Unable to open file: [" << name << " resolved: [" << resolvedFileName << "]");
	return false;
}

String SecurityModule::fgScrambler(coast::storage::Global());
String SecurityModule::fgEncoder(coast::storage::Global());
String SecurityModule::fgSigner(coast::storage::Global());
String SecurityModule::fgCompressor(coast::storage::Global());

bool SecurityModule::Init(const ROAnything config) {
	StartTrace( SecurityModule.Init );
	bool result = true;
	TraceAny(config, "Config");
	SystemLog::WriteToStderr(String("\t") << fName << ". ");
	ROAnything moduleConfig(config[fName]);
	TraceAny(moduleConfig, "ModuleConfig");

	fgScrambler = moduleConfig["Scrambler"].AsCharPtr("Scrambler");
	fgEncoder = moduleConfig["Encoder"].AsCharPtr("Encoder");
	fgSigner = moduleConfig["Signer"].AsCharPtr("Signer");
	fgCompressor = moduleConfig["Compressor"].AsCharPtr("Compressor");

	HierarchyInstaller hi("SecurityItem");
	result = RegisterableObject::Install(moduleConfig["SecurityItems"], "SecurityItem", &hi);

	// TableCompressor is special, it needs the complete Config.any for /Expand Roles /Expand Pages /Expand Actions
	// can be removed if this is never used. Frontdoor doesn't rely on this feature
	Compressor *compressor = Compressor::FindCompressor(fgCompressor);
	if (compressor) {
		compressor->Init(config);
	}
	SystemLog::WriteToStderr(result ? "done\n" : "failed\n");
	Trace("result :" << (result ? "done\n" : "failed\n"));
	return result;
}

bool SecurityModule::ResetInit(const ROAnything config) {
	StartTrace(SecurityModule.ResetInit);
	ROAnything moduleConfig(config[fName]);
	if (moduleConfig["DoNotReset"].AsBool(0) == 1) {
		String msg;
		msg << "\t" << fName << "  Configured not to call Init() on reset\n";
		SystemLog::WriteToStderr(msg);
		return true;
	}
	return SecurityModule::Init(config);
}

bool SecurityModule::ResetFinis(const ROAnything config) {
	StartTrace(SecurityModule.ResetFinis);
	ROAnything moduleConfig(config[fName]);
	if (moduleConfig["DoNotReset"].AsBool(0) == 1) {
		String msg;
		msg << "\t" << fName << "  Configured not to call Finis() on reset\n";
		SystemLog::WriteToStderr(msg);
		return true;
	}
	AliasTerminator at("SecurityItems");
	return RegisterableObject::ResetTerminate("SecurityItem", &at);
}

bool SecurityModule::Finis() {
	StartTrace(SecurityModule.Finis);
	return StdFinis("SecurityItem", "SecurityItems");
}

//#define URLSTAT_TRACING
void SecurityModule::ScrambleState(String &result, const Anything &state) {
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
		SystemLog::WriteToStderr(String("URL comp<") << compressedText << ">\n");
		SystemLog::WriteToStderr("URL: (c, e, c, p) ");
		// do some length tracking
		long rawLen = compressedText.Length();
		long resLen = encodedText.Length();
		long cryptLen = cryptText.Length();
		SystemLog::WriteToStderr(String() << rawLen << ", ");
		SystemLog::WriteToStderr(String() << cryptLen << ", ");
		SystemLog::WriteToStderr(String() << resLen << ", ");
		SystemLog::WriteToStderr(String() << ((resLen * 100) / (rawLen)) << "%\n");
#endif
	}
	Trace("resulting text <" << result << ">");
} // DoScrambleState

bool SecurityModule::UnscrambleState(Anything &state, const String &s) {
	StartTrace(SecurityModule.UnscrambleState);
	Trace("StateString: " << s);
	String crypt(s.Length());
	if (Encoder::Decode(crypt, s)) {
		Trace("Decoded ok: <" << crypt << ">");
		String signedText(crypt.Length());
		if (Scrambler::Unscramble(signedText, crypt)) {
			Trace("Unscrambled ok: <" << signedText << ">");
			String compressedText(signedText.Length());
			if (Signer::Check(compressedText, signedText)) {
				Trace("Checked ok: <" << compressedText << ">");
#ifdef URLSTAT_TRACING
				SystemLog::WriteToStderr(String("URL2exp<") << compressedText << ">\n");
#endif
				if (Compressor::Expand(state, compressedText)) {
					TraceAny(state, "state: ");
					return true;
				}
			}
		}
	}
	if (crypt == "AnyArrayImpl") {
		SystemLog::Error("Expected one token to unscramble, got > 1. Maybe duplicate cookies, /Domain wrong.");
	}
	Trace("Unscrambling failed");
	return false;
} // DoUnscrambleState

RegCacheImpl(SecurityItem)
; // FindSecurityItem()
RegisterScrambler(Scrambler);
RegisterAliasSecurityItem(ascs, Scrambler);
// register legacy keymanager generated names with a 0 appended
RegisterAliasSecurityItem(ascs0, Scrambler);
RegisterAliasSecurityItem(Scrambler0, Scrambler);

void Scrambler::Scramble(const char *encodername, String &encodedText, const String &cleartext) {
	StartTrace(Scrambler.Scramble);
	Trace("Scrambler: " << encodername);
	SecurityItem::Encode(encodername, encodedText, cleartext);
}

bool Scrambler::Unscramble(String &cleartext, const String &encodedText) {
	StartTrace(Scrambler.Unscramble);
	Trace("Scrambled Text: " << encodedText);
	return SecurityItem::Decode(cleartext, encodedText);
}
RegisterEncoder(Encoder);
RegisterAliasSecurityItem(asce, Encoder);
RegisterCompressor(Compressor);
RegisterAliasSecurityItem(comp, Compressor);

void Compressor::Compress(const char *theName, String &encodedText, const Anything &cleartext) {
	Compressor *compressor = Compressor::FindCompressor(theName);
	if (!compressor) {
		return;
	}
	String name;
	compressor->GetName(name);
	String result;
	compressor->DoCompress(result, cleartext);
	encodedText << name << '-' << result;
}

bool Compressor::Expand(Anything &result, const String &encodedText) {
	StartTrace(Compressor.Decode);
	String compressorname;
	long delimiterposition = 0;
	if (!(delimiterposition = GetNamePrefixFromEncodedText(compressorname, encodedText))) {
		return false;
	}
	Compressor *compressor = Compressor::FindCompressor(compressorname);
	//!@FIXME inconsistency!
	// no problem if we don't find the compressor but inconsistent!
	return compressor && compressor->DoExpand(result, encodedText.SubString(delimiterposition + 1, encodedText.Length()));
}

void Compressor::DoCompress(String &encodedText, const Anything &dataIn) {
	OStringStream os(&encodedText);
	dataIn.PrintOn(os, false);
}

bool Compressor::Init(ROAnything config) {
	return true;
}

bool Compressor::DoExpand(Anything &dataOut, const String &scrambledText) {
	IStringStream is(scrambledText);
	dataOut.Import(is);
	return true;
}
RegisterSigner(Signer);
RegisterAliasSecurityItem(nos, Signer);

void Signer::Sign(const char *signername, String &encodedText, const String &cleartext) {
	StartTrace(Signer.Sign);
	Trace("Signer: " << signername);
	SecurityItem::Encode(signername, encodedText, cleartext);
}

bool Signer::Check(String &cleartext, const String &scrambledText) {
	StartTrace(Signer.Check);
	return SecurityItem::Decode(cleartext, scrambledText);
}
