/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ANYUTILS_H
#define _ANYUTILS_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "Anything.h"

//---- AnyUtils ----------------------------------------------------------------------
//! Utility functions to compare and merge anythings
/*!
<b>AnyUtils::AnyCompareEqual</b><br></br>
Compare two anythings along a complete hierarchy.
To compare two anythings, the master anything contains only the slots to be checked for, the input any may
contain more slots, but these are ignored. Any leaves are compared as strings.
Slots without names are nevertheless checked for exact matches only
If you ARE interested in the presence of a slot but not in its contents (for example if you know the slot
contents will be different), use the string <b>ignore</b> as the slot value in the master any.
<br></br><b>Example configuration:</b><pre>
inputAny
{
	/Test2 { Hallo }
	/Test { Hallo }
	/Test3 "fda=dkveklrjtijijckd MON 10 Mar. 2000"
	/Test4 { { /B 4 } { /A 3 } }
}

masterAny
{
	{ Hallo }
	/Test2 { Hallo }
	/Test { Hallo }
	/Test3 ignore
	/Test4:1.A	3
}
</pre>
\note In slots without names, comparison are not shown at the leaf level but from the point at which the slotname free entries began. This is because we attempt to match slots without name by recursive comparison, and verbose is turned off during this matchmaking.

<b>AnyUtils::AnyMerge</b><br></br>
Recursively merge the content of two anythings.
There are four different merging strategies:
-# Named simple valued slot\n
If it does not exist in the master anything, the slot simply gets copied from the roaToMerge anything. Only if the slot exists and the overwrite flag is set,
the value will be overwritten.
-# Named array slot\n
If the named array does not exist it will be created. If it exists, only the non-existing parts will be inserted. You can <b>not</b> overwrite a named array from
top-level.

\note Slots get only replaced - when overwrite flag is set and value is unequal - for simple valued entries.

For unnamed slots it is a bit more sophisticated.
-# Unnamed simple valued slot\n
If the value does not yet exist it will be appended at the specified level.
-# Unnamed array slot\n
It will check all unnamed slots for the <b><i>best matching</i></b> slot which then gets merged.

<br></br><b>Example configuration:</b><pre>
anyMaster
{
	{
		Already
	}
	/Whatever {
		abc
	}
	{
		Here
	}
}

roaToMerge
{
	{					<-- unnamed array slot
		Here			<-- unnamed simple value
		Me
	}
	/Whatever {			<-- named array slot
		Jup
		/ASubSlot	345	<-- named simple value
	}
}

modified anyMaster
{
	{
		Already
	}
	/Whatever {
		abc
		Jup
		/ASubSlot	345
	}
	{
		Here
		Me
	}
}
</pre>
\note For more realistic examples see AnyMergeTest.any in Test/config directory
*/
class EXPORTDECL_FOUNDATION AnyUtils
{
public:
	//! Compare two anythings at content match level. Only compare the slots given in masterAny.
	//! \param inputAny the anything we want to be compared against a master
	//! \param masterAny the master template we want to compare against
	//! \param pathSoFar defines a key path that records the path taken up to a mismatch
	//! \param verbose defines a stream that records the differences in verbose form
	//! \param delimSlot slot delimiter for LookupPath
	//! \param delimIdx index delimiter for LookupPath
	//! \return overall match of all wanted slots, if a single slot fails, comparison fails, if verbose flag is set details of failure are output to cerr
	static bool AnyCompareEqual( const ROAnything &inputAny, const ROAnything &masterAny, String pathSoFar, ostream *verbose, char delimSlot = '.', char delimIdx = ':');

	//!render an anything to simple XML rep,
	//! user namespace any: for anonymous stuff
	//! e.g. <any:seq></any:seq> for {}
	//!  <any:seq><any:elt>foo</any:elt><bar>baz</bar></any:seq>
	//! for { foo /bar baz }
	static void PrintSimpleXML( ostream &os, ROAnything output);

	//! Merge the content of two anythings. The master anything gets modified using the specification from roaToMerge
	//! non-array entries can also be overwritten
	//! \param anyMaster master anything which gets modified by the content of roaToMerge
	//! \param roaToMerge parts to merge into anyMaster
	//! \param bOverwriteSlots if true, non-array-slots will be replaced by the content of the equivalent roaToMerge slot
	//! \param delimSlot slot delimiter for LookupPath
	//! \param delimIdx index delimiter for LookupPath
	static void AnyMerge(Anything &anyMaster, const ROAnything &roaToMerge, bool bOverwriteSlots = false, const char &delimSlot = '.', const char &delimIdx = ':');

protected:
	//!helper method to dump cerr output with additional message and information
	static void Dump(const char *message, const ROAnything &inputAny, const ROAnything &masterAny, const String &masterSlotName, const String &pathSoFar, ostream *verbose);

	//!helper method to spot differences in long anythings more easily; internal use only
	//! \param masterString contents that should be
	//! \param inputString actual content
	//! \param verbose output stream used for comparison contains the result
	static void ShowDifferences( String &masterString, String &inputString, ostream *verbose );

	//!helper method to do the merging work; internal use only
	//! \param anyMaster master anything which gets modified by the content of roaToMerge
	//! \param roaToMerge parts to merge into anyMaster
	//! \param bOverwriteSlots if true, non-array-slots will be replaced by the content of the equivalent roaToMerge slot
	//! \param bTest if true, do only test for match count instead of modifying
	//! \param delimSlot slot delimiter for LookupPath
	//! \param delimIdx index delimiter for LookupPath
	static long DoAnyMerge(Anything &anyMaster, const ROAnything &roaToMerge, bool bOverwriteSlots, bool bTest, const char &delimSlot, const char &delimIdx);
};

//---- Recording ----------------------------------------------------------------------
//! read and write anythings as testrun input
/*!
Defines a recorder api to read in and write out an anything containing activities for testruns
*/
class EXPORTDECL_FOUNDATION Recording
{
public:
	//! used to read recordings of activities for use as input for subsequent test runs
	//! \param fileName The filename to write to
	//! \param recording The contents of the recording
	//! \return true if file can be opened else false
	static bool ReadinRecording(const String &fileName, Anything &recording );

	//! used to write recordings of activities for use as input for subsequent test runs
	//! \param fileName The filename to write to
	//! \param recording The contents of the recording
	//! \return true if file can be opened else false
	static bool WriteoutRecording(const String &fileName, Anything &recording );
};

#endif
