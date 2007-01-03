/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SecurityModule_H
#define _SecurityModule_H

#include "config_wdbase.h"
#include "WDModule.h"
#include "Threads.h"

class Anything;
class Context;
class TRXContext;
class String;

//---- Scrambler -----------------------------------------------------------
class EXPORTDECL_WDBASE SecurityItem: public HierarchConfNamed
{
public:
	SecurityItem(const char *name) : HierarchConfNamed(name) { }
	virtual ~SecurityItem() {}

	//doesn't implement clone since this class is somehow abstract
	virtual IFAObject *Clone() const = 0;
	virtual void InitKey(const String &key) {} // deliberately non-const! useful for Scrambler and Signer
	virtual void DoEncode(String &scrambledText, const String &cleartext) const {
		scrambledText = cleartext;
	}
	//denote failure to decode by returning false
	virtual bool DoDecode(String &cleartext, const String &scrambledText) const {
		cleartext = scrambledText;
		return true;
	}

	// generic hooks for any item except Compressors
	static void Encode(const char *itemtouse, String &encodedText, const String &cleartext);
	static bool Decode(String &cleartext, const String &encodedText);
	//! create a string of length random bytes
	static String GenerateRandomString(long length);

	// factor common code to retrieve the name of a security item from the encoded text
	// returns position in encodedText to continue from. if 0 this means no name was found
	static long GetNamePrefixFromEncodedText(String &theName, const String &encodedText);
	RegCacheDef(SecurityItem);		// FindSecurityItem()

protected:
	// generate the config file name (without extension, which is assumed to be any)
	// out of category and objName
	// the default implementation just takes the objName
	virtual bool DoGetConfigName(const char *category, const char *objName, String &configFileName);

	// load an anything and make it available by storing a reference in fConfig
	// the default implementation uses the cache handler
	// if you provide your own implementation be careful about the lifetime of
	// the loaded anything otherwise fConfig points to invalid data
	virtual bool DoLoadConfig(const char *category);
	// read a key from the file named name
	virtual bool DoLoadKeyFile(const char *name, String &key);
	virtual bool Init(ROAnything config);

	static const char *fgcLegacyMasterKey;

private:
	SecurityItem();
	SecurityItem(const SecurityItem &);
	SecurityItem &operator=(const SecurityItem &);
};

class EXPORTDECL_WDBASE Scrambler :  public SecurityItem
{
public:
	Scrambler(const char *name);
	virtual ~Scrambler();
	IFAObject *Clone() const {
		return new Scrambler(fName);
	}

	static void Scramble(const char *scramblername, String &scrambledText, const String &cleartext);
	static bool Unscramble(String &cleartext, const String &encodedText);

	static Scrambler *FindScrambler(const char *name) {
		return SafeCast(FindSecurityItem(name), Scrambler);
	}
//	RegCacheDef(Scrambler);		// FindScrambler()

private:
	// block copy constructor and operator equal
	Scrambler();
	Scrambler(const Scrambler &);
	Scrambler &operator=(const Scrambler &);
};

class EXPORTDECL_WDBASE Signer :  public SecurityItem
{
public:
	Signer();
	Signer(const char *name);
	virtual ~Signer();
	IFAObject *Clone() const {
		return new Signer(fName);
	}

	static void Sign(const char *encodername, String &encodedText, const String &cleartext);
	static bool Check(String &cleartext, const String &scrambledText);
	static Signer *FindSigner(const char *name) {
		return SafeCast(FindSecurityItem(name), Signer);
	}

//	RegCacheDef(Signer);

private:
	// block copy constructor and operator equal
	Signer(const Signer &);
	Signer &operator=(const Signer &);
};

//---- Encoder -----------------------------------------------------------
class EXPORTDECL_WDBASE Encoder : public SecurityItem
{
public:
	Encoder(const char *name);
	virtual ~Encoder();
	IFAObject *Clone() const {
		return new Encoder(fName);
	}

	static bool GetEncoderName(String &encodername, const String &encodedText);
	static Encoder *FindEncoder(const char *name) {
		return SafeCast(FindSecurityItem(name), Encoder);
	}

//	RegCacheDef(Encoder);
private:
	// block copy constructor and operator equal
	Encoder();
	Encoder(const Encoder &);
	Encoder &operator=(const Encoder &);
};

//---- Compressor -----------------------------------------------------------
class EXPORTDECL_WDBASE Compressor :  public SecurityItem
{
public:
	Compressor(const char *name);
	virtual ~Compressor();
	IFAObject *Clone() const {
		return new Compressor(fName);
	}

	//!initialize compressor with config if necessary
	virtual bool Init(ROAnything config);

	virtual void DoCompress(String &encodedText, const Anything &state);
	virtual bool DoExpand(Anything &state, const String &clearText);

	static void Compress(const char *encodername, String &encodedText, const Anything &state);
	static bool Expand(Anything &state, const String &clearText);
	static bool GetCompressorName(String &encodername, const String &encodedText);
	static Compressor *FindCompressor(const char *name) {
		return SafeCast(FindSecurityItem(name), Compressor);
	}

//	RegCacheDef(Compressor);
private:
	// block copy constructor and operator equal
	Compressor();
	Compressor(const Compressor &);
	Compressor &operator=(const Compressor &);
};

//---- SecurityModule -----------------------------------------------------------
class EXPORTDECL_WDBASE SecurityModule : public WDModule
{
public:
	// Module methods
	SecurityModule(const char *name);
	virtual ~SecurityModule();
	virtual bool Init(const ROAnything config);
	virtual bool ResetInit(const ROAnything config);
	virtual bool Finis();
	virtual bool ResetFinis(const ROAnything );

	// URL encryption an decryption
	static void ScrambleState(String &result, const Anything &state);
	static bool UnscrambleState(Anything &state, const String &s);

protected:

	static String fgScrambler;
	static String fgEncoder;
	static String fgSigner;
	static String fgCompressor;

};

#define RegisterSecurityItem(name)		RegisterObject(name, SecurityItem)
#define RegisterAliasSecurityItem(name,theClass) RegisterShortName(name,theClass,SecurityItem)
#define RegisterScrambler(name)		RegisterSecurityItem(name)
#define RegisterEncoder(name) 		RegisterSecurityItem(name)
#define RegisterSigner(name) 		RegisterSecurityItem(name)
#define RegisterCompressor(name) 	RegisterSecurityItem(name)

#endif		//not defined _SecurityModule_H
