/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnyUtils.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SysLog.h"
#include "Dbg.h"

//---- Any utilities
void AnyUtils::ShowDifferences( String &masterString, String &inputString, ostream *verbose )
{
	StartTrace(AnyUtils.ShowDifferences);
	// show first difference
	if (verbose) {
		*verbose << "- Show First Difference: -" << endl;
		*verbose << "Size Master:" << masterString.Length() << " Size Input:" << inputString.Length() << endl;

		for ( long j = 0, sz = masterString.Length(); j < sz; ++j ) {
			if ( masterString[j] != inputString[j] ) {
				*verbose << masterString[j] << "<-[";
				if ( inputString[j] != '\0' ) {
					*verbose << inputString[j];
				}
				*verbose << "]" << endl;
				break;
			} else {
				*verbose << masterString[j];
			}
		}
		*verbose << "- End Differences: -" << endl;
	}
}

void AnyUtils::Dump(const char *message, const ROAnything &inputAny, const ROAnything &masterAny, const String &masterSlotName, const String &pathSoFar, ostream *verbose)
{
	StartTrace(AnyUtils.Dump);
	Trace(String(message) << " [" << masterSlotName << "] at [" << pathSoFar << "]");
	if (verbose) {
		TraceAny(inputAny, "Input");
		TraceAny(masterAny, "Master");
		*verbose << "_________________________________________________" << endl;
		*verbose << message << " at [" << pathSoFar << "]" << endl;
		*verbose << "Master Slot: " << endl;
		if (masterSlotName.Length()) {
			*verbose << "/" << masterSlotName << " ";
		}
		*verbose << masterAny << endl;
		*verbose << "Input: " << endl ;
		*verbose << inputAny << endl;
	}
}

bool AnyUtils::AnyCompareEqual( const ROAnything &inputAny, const ROAnything &masterAny, String pathSoFar, ostream *verbose, char delimSlot, char delimIdx )
{
	StartTrace1(AnyUtils.AnyCompareEqual, "Delim[" << delimSlot << "] IndexDelim[" << delimIdx << "]");
	//-------------------------------
	// author - Mike egoless
	// only slots actually appearing in the "master" are tested, others are ignored..
	//-------------------------------

	if ( masterAny.GetType() == AnyArrayType ) {
		bool result = true; // match is presupposed

		// recursive comparison of anything, on
		for ( long i = 0, szm = masterAny.GetSize(); i < szm; ++i ) {
			ROAnything anySlotCfg;
			String mySlotname = masterAny.SlotName(i);
			if ( !mySlotname.Length() ) {
				String newPath(pathSoFar);
				newPath.Append(delimIdx);
				newPath.Append(i);
				Trace("MasterSlotname [" << newPath << "]");
				// try to find null as slotname match in input any, must iterate through all "null" slotname members for this
				bool found = false;
				for ( long j = 0, szi = inputAny.GetSize(); j < szi; ++j ) {
					String myInSlotname = inputAny.SlotName(j);
					if ( !myInSlotname.Length() ) {
						myInSlotname.Append(delimIdx).Append(j);
						Trace("InputSlotname [" << myInSlotname << "]");
						found = AnyCompareEqual( inputAny[j], masterAny[i], newPath, NULL, delimSlot, delimIdx );

						if ( found ) {
							// break out of loop as soon as match found
							break;
						}
					} else {
						Trace("InputSlotname [" << myInSlotname << "]");
					}
				}

				if ( !found ) {
					result = false;
					Dump("Master slot not matched", inputAny, masterAny[i], "", pathSoFar, verbose);
				}
			} else if ( inputAny.LookupPath(anySlotCfg, mySlotname, delimSlot, delimIdx) ) {
				// ! we do not get here when the slot was a Null-Anything !
				String newPath(pathSoFar);
				if ( mySlotname[0L] != delimIdx ) {
					newPath.Append(delimSlot);
				}
				newPath.Append(mySlotname);
				Trace("MasterSlotname [" << newPath << "]");
				if ( !AnyCompareEqual( anySlotCfg, masterAny[i], newPath, verbose, delimSlot, delimIdx ) ) {
					result = false;
				}
			} else {
				result = false;
				Dump("Master slot not matched", inputAny, masterAny[i], mySlotname, pathSoFar, verbose);
			}
		}
		return result;
	} else {
		// do a direct comparison
		String masterString = masterAny.AsString();
		String inputString = inputAny.AsString();
		if ( ( !masterAny.IsEqual("ignore") ) && ( inputString != masterString ) ) {
			long lSlotnameIdx = ((pathSoFar.StrRChr(delimSlot)) > (pathSoFar.StrRChr(delimIdx)) ? (pathSoFar.StrRChr(delimSlot)) : (pathSoFar.StrRChr(delimIdx)) );
			String strSlot = ( (pathSoFar.At(lSlotnameIdx) == delimSlot) ? pathSoFar.SubString(lSlotnameIdx + 1L) : pathSoFar.SubString(lSlotnameIdx) );
			pathSoFar.Trim(lSlotnameIdx);
			Trace("path so far [" << pathSoFar << "] slot [" << strSlot << "]");
			Dump("Slots not equal", inputAny, masterAny, pathSoFar, strSlot, verbose);
			ShowDifferences( masterString, inputString, verbose );
			return false;
		}
		return true;
	}
}

void AnyUtils::AnyMerge(Anything &anyMaster, const ROAnything &roaToMerge, bool bOverwriteSlots, const char &delimSlot, const char &delimIdx)
{
	StartTrace1(AnyUtils.AnyMerge, "Delim[" << delimSlot << "] IndexDelim[" << delimIdx << "]");
	DoAnyMerge(anyMaster, roaToMerge, bOverwriteSlots, false, delimSlot, delimIdx);
	SubTraceAny(TraceInputAny, anyMaster, "Result");
}

long AnyUtils::DoAnyMerge(Anything &anyMaster, const ROAnything &roaToMerge, bool bOverwriteSlots, bool bTest, const char &delimSlot, const char &delimIdx)
{
	StartTrace1(AnyUtils.DoAnyMerge, (bTest ? "Test" : ""));
	SubTraceAny(TraceInputAny, anyMaster, "current master content:");
	SubTraceAny(TraceInputAny, roaToMerge, "input content:");
	long lHitCount = 0L;
	// roaToMerge serves as master defining the slots to add/modify
	if ( roaToMerge.GetType() == AnyArrayType ) {
		if ( roaToMerge.GetSize() ) {
			// prepare temporary index any of master
			Anything anyMasterIdx;
			for ( long m = 0, szm = anyMaster.GetSize(); m < szm; ++m ) {
				if ( ( anyMaster.SlotName(m) == NULL ) && ( anyMaster[m].GetType() == AnyArrayType ) ) {
					Trace("is index:" << m);
					anyMasterIdx[anyMasterIdx.GetSize()] = m;
				}
			}
			TraceAny(anyMasterIdx, "MasterIndexes");
			// recursive comparison of anything, on
			for ( long i = 0, sz = roaToMerge.GetSize(); i < sz; ++i ) {
				String strInputSlotname = roaToMerge.SlotName(i);
				Trace("current input slotname [" << strInputSlotname << "], anyType:" << (long)roaToMerge[i].GetType() );
				// first check for leaf
				if ( roaToMerge[i].GetType() != AnyArrayType ) {
					if ( strInputSlotname.Length() ) {
						Trace("noArray:named: simple entry, value [" << roaToMerge[i].AsString() << "]");
						if ( !anyMaster.IsDefined(strInputSlotname) || bOverwriteSlots ) {
							Trace("noArray:named: " << (bOverwriteSlots ? "overwriting" : "adding") << " named simple slot");
							if ( !bTest ) {
								anyMaster[strInputSlotname] = roaToMerge[i].DeepClone();
							}
						}
					} else {
						Trace("noArray:unnamed: simple leaf check of input [" << roaToMerge[i].AsString() << "]");
						// just need to check if leaf is already defined in master or not
						if ( !anyMaster.Contains(roaToMerge[i].AsString()) ) {
							Trace("noArray:unnamed: appending non-existing leaf");
							if ( !bTest ) {
								anyMaster.Append(roaToMerge[i].DeepClone());
							}
						} else {
							Trace("noArray:unnamed: leaf already defined, incrementing match count");
							++lHitCount;
						}
					}
				} else {
					Trace("checking input array at index:" << i);
					if ( !strInputSlotname.Length() ) {
						Trace("Array:unnamed: input slot at:" << i);
						// work on copy of indexed master because of slot deletion
						Anything anyMWork = anyMasterIdx.DeepClone();
						// first try slot with same index
						if ( anyMWork.GetSize() ) {
							Anything anyMatches;
							while ( anyMWork.GetSize() ) {
								long lMasterIdx = anyMWork[0L].AsLong(-1L);
								anyMWork.Remove(0L);
								Trace("Array:unnamed: testing against unnamed master at:" << lMasterIdx);
								long lMatch = DoAnyMerge(anyMaster[lMasterIdx], roaToMerge[i], bOverwriteSlots, true, delimSlot, delimIdx);
								Trace("Array:unnamed: match was:" << lMatch);
								anyMatches[lMasterIdx] = lMatch;
							}
							TraceAny(anyMatches, "all matches");
							// 1. favorize entry with highest matchcount
							// 2. for equal match counts, check if one of them has the same array-index as the ToMerge-any
							long lMaxIdx = -1, lMaxVal = -1, lSameCount = 1;
							for (long lIdx = anyMatches.GetSize() - 1L; lIdx >= 0L; --lIdx) {
								if ( anyMatches[lIdx].AsLong(-1L) > lMaxVal ) {
									lMaxVal = anyMatches[lIdx].AsLong(-1L);
									lMaxIdx = lIdx;
									lSameCount = 1;
									Trace("new maxcount:" << lMaxVal);
								} else if ( anyMatches[lIdx].AsLong(-1L) == lMaxVal ) {
									++lSameCount;
									lMaxIdx = lIdx;
									Trace("lower equal match found at:" << lIdx);
								} else {
									Trace("index:" << lIdx << " had lower count");
									anyMatches[lIdx] = Anything();
								}
							}
							// now we have either a winner or the lowest index of multiple equally matched entries
							if ( lSameCount > 1L ) {
								Trace("test for same array index in ToMerge and Master");
								if ( anyMatches[i].AsLong(-1L) == lMaxVal ) {
									Trace("setting index to same as in ToMerge:" << lMaxIdx);
									lMaxIdx = i;
								}
							}
							Trace("the winner is:" << lMaxIdx);
							if ( !bTest ) {
								Trace("Array::unnamed: merging data");
								long lMatch = DoAnyMerge(anyMaster[lMaxIdx], roaToMerge[i], bOverwriteSlots, bTest, delimSlot, delimIdx);
								Trace("Array:unnamed: match was:" << lMatch);
							}
						} else {
							Trace("Array:unnamed: master has no unnamed array, appending content");
							if ( !bTest ) {
								anyMaster.Append(roaToMerge[i].DeepClone());
							}
						}
					} else {
						Trace("Array:named: slot [" << strInputSlotname << "]");
						if ( anyMaster.IsDefined(strInputSlotname) ) {
							Trace("Array:named:recurse: check contents of slot [" << strInputSlotname << "]");
							long lMatch = DoAnyMerge(anyMaster[strInputSlotname], roaToMerge[i], bOverwriteSlots, bTest, delimSlot, delimIdx);
							Trace("match was:" << lMatch);
							lHitCount += lMatch;
						} else {
							Trace("Array:named: slot in master undefined, setting content");
							if ( !bTest ) {
								anyMaster[strInputSlotname] = roaToMerge[i].DeepClone();
							}
						}
					}
				}
			}
		} else {
			Trace("empty input array, nothing to do");
		}
	} else {
		// do a direct comparison
		if ( ( anyMaster.IsNull() && roaToMerge.IsNull() ) || ( anyMaster.AsString().IsEqual(roaToMerge.AsString()) ) ) {
			Trace("content equal");
			++lHitCount;
		} else {
			Trace("content unequal, overwrite: " << (bOverwriteSlots ? "yes" : "no"));
			if ( !bTest && bOverwriteSlots ) {
				anyMaster = roaToMerge.DeepClone();
			}
		}
	}
	return lHitCount;
}

bool Recording::ReadinRecording( const String &fileName, Anything &recording )
{
	StartTrace(Recording.ReadinRecording);
	//	hardcoded:
	String recFilename = System::GetRootDir();
	recFilename << System::Sep() << "config" << System::Sep();
	recFilename << fileName;

	iostream *ifp = System::OpenStream( recFilename, "any");

	if (ifp == 0) {
		String eMsg = "Can't open recording file ";
		eMsg << recFilename << ".any";
		SysLog::Error( eMsg );
		return false;
	} else {
		recording.Import(*ifp);
	} // if
	delete ifp; // close file

	return true;
}

bool Recording::WriteoutRecording( const String &fileName, Anything &recording )
{
	StartTrace(Recording.WriteoutRecording);

	String recFilename = System::GetRootDir();
	recFilename << System::Sep() << "config" << System::Sep();
	recFilename << fileName;

	iostream *ifp = System::OpenStream( recFilename, "any", ios::out );

	if (ifp == 0) {
		String eMsg = "Can't open config file for writing ";
		eMsg << recFilename << ".any";
		SysLog::Error( eMsg );
		return false;
	} else {
		// write back new any to file....
		recording.Export(*ifp);
	}
	// close file
	delete ifp;

	return true;
}

static void DoPrintSimpleXML( ostream &os, ROAnything output);

static void DoPrintEltXML( ostream &os, ROAnything output)
{
	os << "<any:elt>" ;
	DoPrintSimpleXML(os, output);
	os << "</any:elt>";
}

static void DoPrintSimpleXML( ostream &os, ROAnything output)
{

	if (output.IsNull()) {
		return;
	}
	if (AnyArrayType == output.GetType()) {
		// iterate and recurse...
		os << "<any:seq>";
		for (long i = 0, sz = output.GetSize(); i < sz; ++i) {
			const char *slotname = output.SlotName(i);
			if (slotname) {
				os << '<' << slotname << '>';
				DoPrintSimpleXML(os, output[i]);
				os << "</" << slotname << '>';
			} else {
				DoPrintEltXML(os, output[i]);
			}
		}
		os << "</any:seq>";

	} else {
		os << output.AsCharPtr("") ;
	}

}

void AnyUtils::PrintSimpleXML( ostream &os, ROAnything output)
{
	if (! output.IsNull() && AnyArrayType != output.GetType()) {
		DoPrintEltXML(os, output);
	} else {
		DoPrintSimpleXML(os, output);
	}
}

