/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URLFilter_H
#define _URLFilter_H

#include "IFAConfObject.h"

class Context;
class Anything;
class ROAnything;

//! Policy object to filter URL content and unscramble URL Variables
class URLFilter : public NotCloned
{
public:
	//! constructor with filter name
	URLFilter(const char *filterName)	: NotCloned(filterName)		{ }

	//! destructor does nothing
	virtual ~URLFilter()	{ }

	//! Copies elements from a cookie into the query
	/*! \param query the query anything relevant for this request
	    \param env environment anything containing the cookie, since cookie is a headerfield
	    \param filterCookieConf configuration anything containing the slotnames to be copied
	    \param ctx Context to work with */
	virtual bool HandleCookie(Anything &query, Anything &env, const ROAnything &filterCookieConf, Context &ctx);

	//! Filters public elements and unscrambles scrambled state
	/*! @section URLFilterHandleQueryConfiguration HandleQuery Configuration
		@par \c Tags2Suppress
		Anything defining query tags that have to be removed if they appear unscrambled
		@par \c Tags2Unscramble
		Anything defining query tags that have to be unscrambled
		@subsection URLFilterHandleQueryConfigurationExample filterQueryConf example
		@code
		{
			/Tags2Suppress {
				sessionId role page
				/action { rolechangeaction1 rolechangeaction2 }
			}
			/Tags2Unscramble { X X1 X2 }
		}
		@endcode
		@par When Tags2Suppress is an array the meaning is as follows
		Remove query[slotname] if the value of query[slotname] is contained in the array, eg. supress query[action] if query action is either rolechangeaction1 ore rolechangeaction2

	 * @param query Query anything relevant for this request
	 * @param filterQueryConf a filter specification consisting of the tags "Tags2Suppress" and "Tags2Unscramble"
	 * @param ctx Context to work with */
	virtual bool HandleQuery(Anything &query, const ROAnything &filterQueryConf, Context &ctx);

	//---- registry api
	RegCacheDef(URLFilter);	// FindURLFilter()

	//! filter the state of the query based on the filter Tags spec
	//! \param query the query anything to be modified
	//! \param filterTags anything containing of tag specifications see above
	//! \param ctx reduced request context, containing server, env and query
	//! if you override this method you can accept additional filter specs but you have
	//! to handle the defined ones
	virtual bool FilterState(Anything &query, const ROAnything &filterTags, Context &ctx);

	//! unscramble elements defined in filterTags of query this normally results in an updated query
	//!: caution if the same tags exist in different scrambled tags, they are set overriden by the first one
	virtual bool UnscrambleState(Anything &query, const ROAnything &unscramblers, Context &ctx);

	//! filters a single slot by removing it from the query
	//! \return returns true if slotname is not null (ignoring whether slotName is in query or not)
	virtual bool DoFilterState(Anything &query, const char *slotName, Context &ctx);

	//! unscrambles a single slot by removing it from the query and inserting the unscrambled content into it
	//! \return returns true if the slot is there and successfully unscrambled or the slot is not there, returns false in case of unscrambling error or null
	virtual bool DoUnscrambleState(Anything &query, const char *slotName, Context &ctx);

	//! Render a filter value
	void RenderFilterVal(String &filterVal, const ROAnything &filterCookieConf, Context &ctx);

private:
	//! empty constructor is forbidden
	URLFilter();

	//! copy constructor is forbidden
	URLFilter(const URLFilter &);

	//! assignement is forbidden
	URLFilter &operator=(const URLFilter &);
};

#define RegisterURLFilter(name) RegisterObject(name, URLFilter)

#endif
