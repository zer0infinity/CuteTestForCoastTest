/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SimpleAccessControllers_H
#define _SimpleAccessControllers_H

//---- baseclass include -------------------------------------------------
#include "config_AccessControl.h"
#include "AccessController.h"
#include "Threads.h"

//---- FileUDAC ----------------------------------------------------------
//! <B>FileUserDataAccessController (FileUDAC): Controls access to file db
//! containing userdata.</B>
//! Requires two defined DataAccesses: "ReadUserData" and "WriteUserData".
//! The first should store the contents of the user file under /FileContent
//! in tmp store the latter should read from /FileContent and store it in
//! user file.
class EXPORTDECL_ACCESSCONTROL FileUDAC : public UserDataAccessController
{
public:
	// constructors
	FileUDAC(const char *name) : UserDataAccessController(name), fUserDataMutex("fUserdataMutex") {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		FileUDAC *fudac = new (a) FileUDAC(fName);
		// prevent deletion by terminators
		fudac->MarkStatic();
		return fudac;
	}

	// access queries via userid (all modifications are locked)
	bool Exists(String userid);

	String GetTokenId(String userid);

	String GetPassword(String userid);
	bool SetPassword(String userid, String newpasswd);
	// sets password equal to username
	bool ResetPassword(String userid);

	bool GetGroups(String userid, Anything &groups);
	bool SetGroups(String userid, ROAnything groups);

protected:
	virtual bool LoadUserData();
	virtual bool SaveUserData() ;

	Anything fUserData;
	Mutex fUserDataMutex;
};

//---- FileTDAC ----------------------------------------------------------
//! <B>FileTokenDataAccessController (FileTDAC): Controls access to file DB
//! containing token data.</B>
//! Requires two defined DataAccesses: "ReadTokenData" and "WriteTokenData".
//! The first should store the contents of the token file (which is an Anything)
//! under tmp.FileContent, the latter should read from tmp.FileContent and store
// it in user file.
class EXPORTDECL_ACCESSCONTROL FileTDAC : public TokenDataAccessController
{
public:
	// constructors
	FileTDAC(const char *name) : TokenDataAccessController(name), fTokenDataMutex("fTokenDataMutex") {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		FileTDAC *ftdac = new (a) FileTDAC(fName);
		// prevent deletion by terminators
		ftdac->MarkStatic();
		return ftdac;
	}

	// access queries via userid (all modifications are locked)
	String GetSeed(String tokenid);
	unsigned long GetCount(String tokenid);	// return 0 if not defined (>0 == ok)
	bool IncCount(String userid, long by);

protected:
	virtual bool LoadTokenData();
	virtual bool SaveTokenData() ;

	Anything fTokenData;
	Mutex fTokenDataMutex;
};

//---- FileEDAC ----------------------------------------------------------
//! <B>FileEntityDataAccessController (FileEDAC): Controls access to
//! file DB containing access rights to permissible entities.</B>
//! Requires two defined DataAccesses: "ReadEntityData" and "WriteEntityData".
//! The first should store the contents of the entity file (which is an Anything)
//! under tmp.FileContent, the latter should read from tmp.FileContent and store
//! it in entity file.
class EXPORTDECL_ACCESSCONTROL FileEDAC : public EntityDataAccessController
{
public:
	// constructors
	FileEDAC(const char *name) : EntityDataAccessController(name), fEntityDataMutex("fEntityDataMutex") {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		FileEDAC *fedac = new (a) FileEDAC(fName);
		// prevent deletion by terminators
		fedac->MarkStatic();
		return fedac;
	}

	// access queries via userid (all modifications are locked)
	bool GetAllowedEntitiesForGroup(String group, Anything &allowed);
	bool GetAllowedEntitiesForGroups(Anything groups, Anything &allowed);
	bool IsAllowed(String group, String entity);

protected:
	virtual bool LoadEntityData();
	virtual bool SaveEntityData() ;

	Anything fEntityData;
	Mutex fEntityDataMutex;		// not really needed yet, since no modifications are performed
};

//---- MockUDAC -----------------------------------------------------------
//! Mock UserDataAccessController for easy testing.
/*!
Configuration:<PRE>
{
	/User {
		/UserId		uid
		/Password	pwd
		/TokenId	tid
		/Groups {
			group1
			group2
		}
	}
	... etc.

	# Example:
	/tester {
		/UserId		"tester"
		/Password	"secret"
		/TokenId	"000100002"
		/Groups {
			"ultraUsers"
			"tvJunkies"
		}
	}
	... etc.
}
</PRE>
Initial test config is modified in memory only.
*/
class EXPORTDECL_ACCESSCONTROL MockUDAC : public FileUDAC
{
public:
	MockUDAC(const char *name) : FileUDAC(name) {};
	virtual ~MockUDAC() {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MockUDAC(fName);
	}

protected:
	virtual bool LoadUserData() {
		if (fUserData.IsNull()) {
			fUserData = fConfig.DeepClone();
		}
		return true;
	}
	virtual bool SaveUserData() {
		return true;
	}

	friend class MockAccessControllerTests;
	virtual Anything GetMockedState() {
		return fUserData.DeepClone();
	}
};

//---- MockTDAC -----------------------------------------------------------
//! Mock TokenDataAccessController for easy testing.
/*!
Configuration:<PRE>
{
	/TokenId {
		/Seed	seed
		/Count	count
	}
	... etc.

	# Example:
	/"000100002" {
		/Seed 	"e5a4103be54395b743c1c70365d47603"
		/Count 	612314215
	}
	... etc.
}
</PRE>
Initial test config is modified in memory only.
*/
class EXPORTDECL_ACCESSCONTROL MockTDAC : public FileTDAC
{
public:
	MockTDAC(const char *name) : FileTDAC(name) {};
	virtual ~MockTDAC() {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MockTDAC(fName);
	}

protected:
	virtual bool LoadTokenData() {
		if (fTokenData.IsNull()) {
			fTokenData = fConfig.DeepClone();
		}
		return true;
	}
	virtual bool SaveTokenData() {
		return true;
	}

	friend class MockAccessControllerTests;
	virtual Anything GetMockedState() {
		return fTokenData.DeepClone();
	}
};

//---- MockEDAC -----------------------------------------------------------
//! Mock EntityDataAccessController for easy testing.
/*!
Configuration:<PRE>
{
	/Group {
		right1
		right2
	}
	... etc.

	# Example:
	/tvJunkies {
		"remoteControl"
		"tvGuide"
	}
	/ultraUsers {
		"format"
	}
	... etc.
}
</PRE>
Initial test config is modified in memory only.
*/
class EXPORTDECL_ACCESSCONTROL MockEDAC : public FileEDAC
{
public:
	MockEDAC(const char *name) : FileEDAC(name) {};
	virtual ~MockEDAC() {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MockEDAC(fName);
	}

protected:
	virtual bool LoadEntityData() {
		if (fEntityData.IsNull()) {
			fEntityData = fConfig.DeepClone();
		}
		return true;
	}
	virtual bool SaveEntityData() {
		return true;
	}

	friend class MockAccessControllerTests;
	virtual Anything GetMockedState() {
		return fEntityData.DeepClone();
	}
};

#endif
