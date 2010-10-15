/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SimpleAccessControllers.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "Context.h"
#include "DataAccess.h"

//--- c-modules used -----------------------------------------------------------

//---- FileUDAC ----------------------------------------------------------------
RegisterUserDataAccessController(FileUDAC);

bool FileUDAC::Exists(String userid)
{
	StartTrace(FileUDAC.Exists);

	LockUnlockEntry me(fUserDataMutex);
	LoadUserData();

	Trace("uid = " << userid);
	bool exists = fUserData.IsDefined(userid);
	Trace("exists? " << (exists ? "yes" : "no"));
	return exists;
}

String FileUDAC::GetTokenId(String userid)
{
	StartTrace(FileUDAC.GetTokenId);

	LockUnlockEntry me(fUserDataMutex);
	LoadUserData();

	Trace("uid = " << userid);
	String tid = ((ROAnything)fUserData)[userid]["TokenId"].AsString();
	Trace("token id = " << tid);
	return tid;
}

String FileUDAC::GetPassword(String userid)
{
	StartTrace(FileUDAC.GetPassword);

	LockUnlockEntry me(fUserDataMutex);
	LoadUserData();

	Trace("uid = " << userid);
	String pwd = ((ROAnything)fUserData)[userid]["Password"].AsString();
	Trace("password = " << pwd);
	return pwd;
}

bool FileUDAC::SetPassword(String userid, String newpasswd)
{
	StartTrace(FileUDAC.SetPassword);

	Trace("uid = " << userid);
	if ( Exists(userid) ) {
		LockUnlockEntry me(fUserDataMutex);
		LoadUserData();

		String oldpasswd = fUserData[userid]["Password"].AsString();
		fUserData[userid]["Password"] = newpasswd;

		Trace("old password = " << oldpasswd);
		Trace("new password = " << newpasswd);

		bool saveOk = SaveUserData();
		if ( !saveOk ) {
			Trace("SaveUserData failed. Password has not been set.");
			// undo password change to keep memory + file in sync
			fUserData[userid]["Password"] = oldpasswd;
		}
		return saveOk;
	} else {
		Trace("user '" << userid << "' doesn't exist - cannot set password");
		return false;
	}
}

bool FileUDAC::ResetPassword(String userid)
{
	StartTrace(FileUDAC.ResetPassword);
	return SetPassword(userid, userid);
}

bool FileUDAC::GetGroups(String userid, Anything &groups)
{
	StartTrace(FileUDAC.GetGroups);

	Trace("uid = " << userid);
	if ( !Exists(userid) ) {
		Trace("user '" << userid << "' doesn't exist - cannot retrieve groups");
		return false;
	}

	LockUnlockEntry me(fUserDataMutex);
	LoadUserData();

	groups = ((ROAnything)fUserData)[userid]["Groups"].DeepClone();
	TraceAny(groups, "groups = ");
	return true;
}

bool FileUDAC::SetGroups(String userid, ROAnything newgroups)
{
	StartTrace(FileUDAC.SetGroups);

	Trace("uid = " << userid);
	if ( !Exists(userid) ) {
		Trace("user '" << userid << "' doesn't exist - cannot set groups");
		return false;
	}

	Anything oldgroups = fUserData[userid]["Groups"];
	fUserData[userid]["Groups"] = newgroups.DeepClone();

	TraceAny(oldgroups, "old groups = ");
	TraceAny(newgroups, "new groups = ");

	bool saveOk = SaveUserData();
	if ( !saveOk ) {
		Trace("SaveUserData failed. Groups have not been set.");
		// undo groups change to keep memory + file in sync
		fUserData[userid]["Groups"] = oldgroups.DeepClone();
	}
	return saveOk;
}

bool FileUDAC::LoadUserData()
{
	StartTrace(FileUDAC.LoadUserData);

	// only load if not loaded yet
	if (fUserData.IsNull()) {
		Context c;
		if ( DataAccess("ReadUserData").StdExec(c) ) {
			Trace("Reading UserData succeded.");
			fUserData = c.Lookup("FileContent").DeepClone(fUserData.GetAllocator());
			TraceAny(fUserData, "fUserData:");
		} else {
			Trace("Reading UserData failed.");
			return false;
		}
	}
	return true;
}

bool FileUDAC::SaveUserData()
{
	StartTrace(FileUDAC.SaveUserData);

	Context c;
	c.GetTmpStore()["FileContent"] = fUserData;
	return DataAccess("WriteUserData").StdExec(c);
}

//---- FileTDAC ----------------------------------------------------------------

RegisterTokenDataAccessController(FileTDAC);

String FileTDAC::GetSeed(String tokenid)
{
	StartTrace(FileTDAC.GetSeed);

	LockUnlockEntry me(fTokenDataMutex);
	LoadTokenData();

	Trace("token id = " << tokenid);
	String seed = ((ROAnything)fTokenData)[tokenid]["Seed"].AsString();
	Trace("seed = " << seed);
	return seed;
}

unsigned long FileTDAC::GetCount(String tokenid)
{
	StartTrace(FileTDAC.GetCount);

	LockUnlockEntry me(fTokenDataMutex);
	LoadTokenData();

	Trace("token id = " << tokenid);
	if ( fTokenData.IsDefined(tokenid) && fTokenData[tokenid].IsDefined("Count") ) {
		unsigned long count = static_cast<unsigned long>(((ROAnything)fTokenData)[tokenid]["Count"].AsLong(0L));
		Trace("count = " << (long)count);
		return count;
	} else {
		Trace("unknown token id (" << tokenid << ") or count not defined");
		return 0L;
	}
}

bool FileTDAC::IncCount(String tokenid, long by)
{
	StartTrace(FileTDAC.IncCount);

	LockUnlockEntry me(fTokenDataMutex);
	LoadTokenData();

	Trace("token id = " << tokenid);
	if ( fTokenData.IsDefined(tokenid) && fTokenData[tokenid].IsDefined("Count") ) {
		unsigned long oldCount = GetCount(tokenid);
		Trace("old count = " << (long)oldCount);

		unsigned long newCount = oldCount + by;
		fTokenData[tokenid]["Count"] = static_cast<long>(newCount);
		Trace("new count = " << (long)newCount);

		bool saveOk = SaveTokenData();
		if ( !saveOk ) {
			Trace("SaveActerInfo failed. Count was not incremented.");
			// undo inc counter to keep memory & file in sync
			fTokenData[tokenid]["Count"] = static_cast<long>(oldCount);
		}
		return saveOk;
	} else {
		Trace("unknown token id (" << tokenid << ") or count not defined");
		return false;
	}
}

bool FileTDAC::LoadTokenData()
{
	StartTrace(FileTDAC.LoadTokenData);

	// only load if not loaded yet
	if (fTokenData.IsNull()) {
		Context c;
		if ( DataAccess("ReadTokenData").StdExec(c) ) {
			Trace("Reading TokenData succeded.");
			fTokenData = c.Lookup("FileContent").DeepClone(fTokenData.GetAllocator());
			TraceAny(fTokenData, "fTokenData:");
		} else {
			Trace("Reading TokenData failed.");
			return false;
		}
	}
	return true;
}

bool FileTDAC::SaveTokenData()
{
	StartTrace(FileTDAC.SaveTokenData);

	Context c;
	c.GetTmpStore()["FileContent"] = fTokenData;
	return DataAccess("WriteTokenData").StdExec(c);
}

//---- FileEDAC ----------------------------------------------------------------
RegisterEntityDataAccessController(FileEDAC);

bool FileEDAC::GetAllowedEntitiesForGroup(String group, Anything &allowed)
{
	StartTrace(FileEDAC.GetAllowedEntitiesForGroup);

	LockUnlockEntry me(fEntityDataMutex);
	LoadEntityData();

	Trace("group = " << group);
	if ( !fEntityData.IsDefined(group) ) {
		Trace("unknown group '" << group << "' - no permissible entities available");
		allowed = Anything(Anything::ArrayMarker());
		return false;
	}

	allowed = fEntityData[group].DeepClone();
	TraceAny(allowed, "allowed entities for " << group << " =");
	return true;
}

bool FileEDAC::GetAllowedEntitiesForGroups(Anything groups, Anything &allowed)
{
	StartTrace(FileEDAC.GetAllowedEntitiesForGroups);

	LockUnlockEntry me(fEntityDataMutex);
	LoadEntityData();

	TraceAny(groups, "groups =");

	Anything parts;
	allowed = Anything(Anything::ArrayMarker());
	String entity;

	for (long i = 0; i < groups.GetSize(); i++) {
		if ( !GetAllowedEntitiesForGroup(groups[i].AsString(), parts) ) {
			return false;
		}

		// aggregate
		for (long j = 0; j < parts.GetSize(); j++) {
			entity = parts[j].AsString();
			if ( !allowed.Contains(entity) ) {
				allowed.Append(entity);
			}
		}
	}

	TraceAny(allowed, "allowed entities for given groups =");
	return true;
}

bool FileEDAC::IsAllowed(String group, String entity)
{
	StartTrace(FileEDAC.IsAllowed);

	LockUnlockEntry me(fEntityDataMutex);
	LoadEntityData();

	Trace("group = " << group);
	Trace("entity = " << entity);

	Anything allowed;
	if ( !GetAllowedEntitiesForGroup(group, allowed) ) {
		return false;
	}
	return allowed.Contains(entity);
}

bool FileEDAC::LoadEntityData()
{
	StartTrace(FileEDAC.LoadEntityData);

	// only load if not loaded yet
	if (fEntityData.IsNull()) {
		Context c;
		if ( DataAccess("ReadEntityData").StdExec(c) ) {
			Trace("Reading EntityData succeded.");
			fEntityData = c.Lookup("FileContent").DeepClone(fEntityData.GetAllocator());
			TraceAny(fEntityData, "fEntityData:");
		} else {
			Trace("Reading EntityData failed.");
			return false;
		}
	}
	return true;
}

bool FileEDAC::SaveEntityData()
{
	StartTrace(FileEDAC.SaveEntityData);

	Context c;
	c.GetTmpStore()["FileContent"] = fEntityData;
	return DataAccess("WriteEntityData").StdExec(c);
}

//---- MockUDAC ----------------------------------------------------------------
RegisterUserDataAccessController(MockUDAC);

//---- MockTDAC ----------------------------------------------------------------
RegisterTokenDataAccessController(MockTDAC);

//---- MockEDAC ----------------------------------------------------------------
RegisterEntityDataAccessController(MockEDAC);
