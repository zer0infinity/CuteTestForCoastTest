/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SecurityModule_H
#define _SecurityModule_H

#include "WDModule.h"
#include "Threads.h"

class Anything;
class Context;
class String;

namespace coast {
	namespace security {
		unsigned long nextRandomNumber();
		//! create a string of length random bytes
		String generateRandomString(long length);
	}
}

class SecurityItem: public HierarchConfNamed {
	SecurityItem();
	SecurityItem(const SecurityItem &);
	SecurityItem &operator=(const SecurityItem &);
public:
	SecurityItem(const char *name) :
		HierarchConfNamed(name) {
	}

	virtual void InitKey(const String &key) {
	} // deliberately non-const! useful for Scrambler and Signer
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

	// factor common code to retrieve the name of a security item from the encoded text
	// returns position in encodedText to continue from. if 0 this means no name was found
	static long GetNamePrefixFromEncodedText(String &theName, const String &encodedText);RegCacheDef(SecurityItem); // FindSecurityItem()

protected:
	// generate the config file name (without extension, which is assumed to be any)
	// out of category and objName
	// the default implementation just takes the objName
	virtual bool DoGetConfigName(const char *category, const char *objName, String &configFileName) const;

	// load an anything and make it available by storing a reference in fConfig
	// the default implementation uses the cache handler
	// if you provide your own implementation be careful about the lifetime of
	// the loaded anything otherwise fConfig points to invalid data
	virtual bool DoLoadConfig(const char *category);
	// read a key from the file named name
	virtual bool DoLoadKeyFile(const char *name, String &key);
	virtual bool Init(ROAnything config);

	static const char *fgcLegacyMasterKey;
};

class Scrambler: public SecurityItem {
	Scrambler();
	Scrambler(const Scrambler &);
	Scrambler &operator=(const Scrambler &);
public:
	Scrambler(const char *name) :
		SecurityItem(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) Scrambler(fName);
	}

	static void Scramble(const char *scramblername, String &scrambledText, const String &cleartext);
	static bool Unscramble(String &cleartext, const String &encodedText);

	static Scrambler *FindScrambler(const char *name) {
		return SafeCast(FindSecurityItem(name), Scrambler);
	}
};

class Signer: public SecurityItem {
	Signer(const Signer &);
	Signer &operator=(const Signer &);
public:
	Signer() :
		SecurityItem("nos") {
	}
	Signer(const char *name) :
		SecurityItem(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) Signer(fName);
	}

	static void Sign(const char *encodername, String &encodedText, const String &cleartext);
	static bool Check(String &cleartext, const String &scrambledText);
	static Signer *FindSigner(const char *name) {
		return SafeCast(FindSecurityItem(name), Signer);
	}
};

class Encoder: public SecurityItem {
	Encoder();
	Encoder(const Encoder &);
	Encoder &operator=(const Encoder &);
public:
	Encoder(const char *name) :
		SecurityItem(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) Encoder(fName);
	}
	static bool GetEncoderName(String &encodername, const String &encodedText);
	static Encoder *FindEncoder(const char *name) {
		return SafeCast(FindSecurityItem(name), Encoder);
	}
};

class Compressor: public SecurityItem {
	Compressor();
	Compressor(const Compressor &);
	Compressor &operator=(const Compressor &);
public:
	Compressor(const char *name) :
		SecurityItem(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) Compressor(fName);
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
};

class SecurityModule: public WDModule {
public:
	// Module methods
	SecurityModule(const char *name) :
		WDModule(name) {
	}
	/* in: config: Server configuration
	 ret: true if successful, false otherwise.
	 what: Only writes a message and installs itself
	 note: Derived classes must call this method at the end of their Init to install
	 themselves! (InstallHandler is private)
	 Usually derived classes do not need to override Init if they do not want
	 to do anything by themselves. Installation happens automatically.
	 */
	virtual bool Init(const ROAnything config);
	virtual bool ResetInit(const ROAnything config);
	virtual bool Finis();
	virtual bool ResetFinis(const ROAnything);

	/* in: state: some data, usually the state information for a Coast link
	 out: result: The state data appended in linearized form.
	 what: Standard URLUtils, no real encryption.
	 */
	static void ScrambleState(String &result, const Anything &state);
	/* in: s: A pointer to a null terminated array of char containing scrambled data
	 This contains exactly the characters appended by the previous method.
	 out: state: The unscrambled data
	 rets: true if unscrambling was possible, false if somebody tried to unscramble garbage
	 what: Standard URLUtils decryption, does not decrypt serious URL's
	 */
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
