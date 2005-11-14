/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "URLFilter.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "SecurityModule.h"
#include "Renderer.h"
#include "SysLog.h"
#include "Dbg.h"

//--- URLFilter ----------------------------------------------------------
bool URLFilter::HandleCookie(Anything &query, Anything &env, const ROAnything &filterCookieConf, Context &ctx)
{
	StartTrace(URLFilter.HandleCookie);
	SubTraceAny(query, query, "Query: ");
	SubTraceAny(env, env, "Env: ");
	SubTraceAny(filterCookieConf, filterCookieConf, "Filter configuration: ");
	// default implementation copies cookie tags
	// defined in the filter into the query
	// if they are not already there
	Anything cookies;
	bool ret = true;
	if ( env.LookupPath(cookies, "WDCookies") ) {
		long numOfFilters = filterCookieConf.GetSize();
		for (long i = 0; i < numOfFilters; ++i) {
			String filterVal;
			RenderFilterVal(filterVal, filterCookieConf[i], ctx);
			const char *slotName = filterVal;
			env["NrOfCookies"][slotName] = 0L;
			if ( cookies.IsDefined(slotName) &&
				 !query.IsDefined(slotName) ) {
				if ( cookies[slotName].GetType() != AnyArrayType ) {
					// don't override existing tags
					query[slotName] = cookies[slotName];
					env["NrOfCookies"][slotName] = 1L;
					//	cookies.Remove(slotName); // no duplicates
				} else {
					long size = cookies[slotName].GetSize();
					query[slotName] = "";
					String errMsg("Cookie: <");
					errMsg << slotName << "> was sent <" << size << "> times. " << slotName << " set to blank.";
					SysLog::Error(errMsg);
					env["NrOfCookies"][slotName] = size;
				}
				Trace("query[" << slotName << "]= " << cookies[slotName].AsCharPtr(""));
			}
		}
		SubTraceAny(query, query, "Query after: ");
	} else {
		long numOfFilters = filterCookieConf.GetSize();
		for (long i = 0; i < numOfFilters; ++i) {
			String filterVal;
			RenderFilterVal(filterVal, filterCookieConf[i], ctx);
			env["NrOfCookies"][filterVal] = 0L;
		}
		ret = false;
	}
	return ret;
}

void URLFilter::RenderFilterVal(String &filterVal, const ROAnything &filterCookieConf, Context &ctx)
{
	StartTrace(URLFilter.RenderFilterVal);
	ROAnything filter(filterCookieConf);
	if (filter.GetType() != AnyCharPtrType) {
		// assume renderer specification
		Renderer::RenderOnString(filterVal, ctx, filter);
	} else {
		filterVal = filter.AsCharPtr(0);
	}
	return;
}

bool URLFilter::HandleQuery(Anything &query, const ROAnything &filterQueryConf, Context &ctx)
{
	StartTrace(URLFilter.HandleQuery);
	SubTraceAny(query, query, "Query: ");
	SubTraceAny(filterQueryConf, filterQueryConf, "Filter configuration: ");

	ROAnything filters;
	ROAnything unscramblers;

	if ( filterQueryConf.LookupPath(filters, "Tags2Suppress") ) {
		if (!FilterState(query, filters, ctx)) {
			return false;
		}
	}
	if ( filterQueryConf.LookupPath(unscramblers, "Tags2Unscramble") ) {
		return UnscrambleState(query, unscramblers, ctx);
	}
	return true;

}

bool URLFilter::FilterState(Anything &query, const ROAnything &filterTags, Context &ctx)
{
	StartTrace(URLFilter.FilterState);

	SubTraceAny(query, query, "Query: ");
	SubTraceAny(filterTags, filterTags, "Filter: ");
	bool retCode = true;

	long numOfFilters = filterTags.GetSize();
	for (long i = 0; i < numOfFilters; ++i) {
		const char *slotName = filterTags.SlotName(i);
		ROAnything filter(filterTags[i]);
		if (!slotName) {
			String filterVal;
			if (filter.GetType() != AnyCharPtrType) {
				// assume renderer expression
				Renderer::RenderOnString(filterVal, ctx, filter);
			} else {
				filterVal = filter.AsCharPtr(0);
			}
			if ( filterVal.Length() == 0 ) {
				retCode = false;
			} else
				// simple filter just remove it from query
			{
				retCode = DoFilterState(query, filterVal, ctx) && retCode;
			}
		} else if (slotName) {
			// complex filter; remove from query if
			// filter[slotName].Contains(query[slotName]);
			if (	query.IsDefined(slotName) &&
					filter.Contains(query[slotName].AsCharPtr("")) ) {
				Trace("removing <" << slotName << ">=<" << query[slotName].AsCharPtr("") << ">");
				query.Remove(slotName);
				retCode = true && retCode;
			}
		} else {
			// we don't handle any other type
			retCode = false;
		}
	}
	return retCode;
}

bool URLFilter::UnscrambleState(Anything &query, const ROAnything &unscramblers, Context &ctx)
{
	StartTrace(URLFilter.UnscrambleState);

	SubTraceAny(query, query, "Query: ");
	SubTraceAny(unscramblers, unscramblers, "Filter: ");

	bool retCode = true;
	long numOfFilters = unscramblers.GetSize();
	for (long i = 0; i < numOfFilters; ++i) {
		ROAnything filter(unscramblers[i]);
		String tagToUnscramble;
		if (filter.GetType() != AnyCharPtrType) {
			// assume renderer specification
			Renderer::RenderOnString(tagToUnscramble, ctx, filter);
		} else {
			tagToUnscramble = filter.AsCharPtr(0);
		}
		// simple specification
		retCode = DoUnscrambleState(query, tagToUnscramble, ctx) && retCode;
	}
	return retCode;
}

bool URLFilter::DoFilterState(Anything &query, const char *slotName, Context &ctx)
{
	StartTrace(URLFilter.DoFilterState);
	Trace("Slot: " << NotNull(slotName));
	SubTraceAny(query, query, "Query: ");
	// security handling
	// filter out some sensitive keywords
	// that shouldn't appear in the public part
	// of an URL
	if ( slotName ) {
		if ( query.IsDefined(slotName) ) {
			Trace("removing <" << slotName << ">");
			query.Remove(slotName);
		}
		return true;
	}
	return false;
}

bool URLFilter::DoUnscrambleState(Anything &query, const char *slotName, Context &ctx)
{
	// Scrambled private information in slotName
	StartTrace1(URLFilter.DoUnscrambleState, "Slot: " << NotNull(slotName));

	if (slotName && query.IsDefined(slotName)) {
		// we need to decrypt some state

		Anything a;
		String s(query[slotName].AsCharPtr((const char *)0));
		if (s.Length() > 0) {
			if (SecurityModule::UnscrambleState(a, s)) {
				query.Remove(slotName);
				SubTraceAny(slot, a, "---- unscrambled [" << slotName << "] ----------");
				// PS unroll loop condition
				long sz = a.GetSize();
				for (long i = 0; i < sz; ++i) {
					const char *slot = a.SlotName(i);
					if (slot && !query.IsDefined(slot)) {
						// get the private state
						query[slot] = a[i];
					}
				}
				SubTraceAny(query, query,	"---- unscrambled query ---------------");
				return true;
			} else {
				String errMsg("Private state of URL is corrupt:<");
				errMsg << slotName << ">";
				SysLog::Error(errMsg);
				return false;
			}
		} else {
			String errMsg("Empty Slot for unscrambling:<");
			errMsg << slotName << ">";
			SysLog::Error(errMsg);
			return false;
		}
	}
	return (slotName) ? true : false;
}

//---- registry api
RegCacheImpl(URLFilter);	// FindURLFilter()
RegisterURLFilter(URLFilter);
