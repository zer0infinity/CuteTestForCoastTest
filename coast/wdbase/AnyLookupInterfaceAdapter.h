/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyLookupInterfaceAdapter_H
#define _AnyLookupInterfaceAdapter_H

#include "IFAConfObject.h"

//---- AnyLookupInterfaceAdapter ----------------------------------------------------------
/*! <B>Wraps [RO]Anything in a LookupInterface (as needed eg. by Context::Push())</B> */
template < class ContainerType >
class AnyLookupInterfaceAdapter : public LookupInterface
{
public:
	/*! Constructor for LookupAdapter
		\param container [RO]Anything to use as underlying data container
		\param pcBaseKey optional param which specifies the segment name used to emulate nested content in a Lookup. If the lookup-key starts with this name we cut it away before doing a concrete lookup.*/
	AnyLookupInterfaceAdapter(ContainerType container, const char *pcBaseKey = NULL)
		: fContainer(container)
		, fstrBaseKey(pcBaseKey)
	{}
	~AnyLookupInterfaceAdapter() {}

protected:
	ContainerType fContainer;
	String	fstrBaseKey;

	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
		StartTrace(AnyLookupInterfaceAdapter.DoLookup);
		String strKey(key);
		// check if we need to adjust the key
		if ( fstrBaseKey.Length() ) {
			if ( strKey.StartsWith(fstrBaseKey) ) {
				Trace("given key [" << strKey << "] starts with [" << fstrBaseKey << "]");
				long lKeyLength = fstrBaseKey.Length();
				if ( ( strKey.Length() > lKeyLength ) && ( strKey[lKeyLength] == delim ) ) {
					++lKeyLength;
					Trace("deep non-index key detected, also cut away delim, length to trim:" << lKeyLength);
				}
				strKey.TrimFront(lKeyLength);
				Trace("key after trimming [" << strKey << "]");
				if ( !strKey.Length() ) {
					// here we return the whole internal store when fstrBaseKey was the only key
					result = (ROAnything)fContainer;
					return true;
				}
			} else {
				Trace("given key [" << strKey << "] does not start with [" << fstrBaseKey << "], content not found and returning false");
				return false;
			}
		}
		bool bRet = ((ROAnything)fContainer).LookupPath(result, strKey, delim, indexdelim);
		TraceAny(result, "Looking up [" << strKey << "] resulted in");
		return bRet;
	}
};

class ROAnyConfNamedObjectLookupAdapter : public AnyLookupInterfaceAdapter< ROAnything >
{
public:
	typedef AnyLookupInterfaceAdapter<ROAnything> BaseClassType;

	ROAnyConfNamedObjectLookupAdapter(ROAnything ro, ConfNamedObject *cfno)
		: BaseClassType(ro) {
		fCfno = cfno;
	}
	~ROAnyConfNamedObjectLookupAdapter() {}

protected:
	ConfNamedObject *fCfno;

	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
		if ( !BaseClassType::DoLookup(key, result, delim, indexdelim) ) {
			if ( fCfno != (ConfNamedObject *)NULL ) {
				return fCfno->Lookup(key, result, delim, indexdelim);
			}
			return false;
		}
		return true;
	}
};
#endif
