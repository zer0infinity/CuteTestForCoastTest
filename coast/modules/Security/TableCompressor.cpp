/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TableCompressor.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "StringStream.h"
#include "Server.h"
#include "Dbg.h"

// ------------------- TableCompressor ---------------------------------------------
RegisterCompressor(TableCompressor);
RegisterAliasSecurityItem(tc, TableCompressor);

TableCompressor::TableCompressor(const char *name) : Compressor(name), fMap(Storage::Global())
{
	StartTrace1(TableCompressor.Ctor, "Name: <" << NotNull(name) << ">");
}

TableCompressor::~TableCompressor()
{
	StartTrace1(TableCompressor.Dtor, "Name: <" << fName << ">");
}

bool TableCompressor::Init(ROAnything config)
{
	StartTrace(TableCompressor.Init);
	TraceAny(config, "Config: ");

	ROAnything tableCompressorConfig;
	if ( config.LookupPath(tableCompressorConfig, "SecurityModule.TableCompressor") ) {
		MakeTable(tableCompressorConfig["Key2UriMap"], "Key2UriMap", config);
		MakeReverseTable(fMap["Key2UriMap"], "Key2UriMap", "Uri2KeyMap");
		MakeTable(tableCompressorConfig["Val2UriMap"], "Val2UriMap", config);
		MakeReverseTable(fMap["Val2UriMap"], "Val2UriMap", "Uri2ValMap");
		fMap["ValMapTags"] = tableCompressorConfig["ValMapTags"].DeepClone(Storage::Global());
		return true;
	}
	return false;
}

void TableCompressor::DoCompress(String &scrambledText, const Anything &dataIn)
{
	StartTrace(TableCompressor.DoCompress);

	ROAnything keyTable(GetKey2UriMap());
	ROAnything valTable(GetVal2UriMap());
	ROAnything valMapTags(GetValMapTags());
	Anything dataOut;

//	 compress it by short keys in table
	long dataSz = dataIn.GetSize();
	for (long i = 0; i < dataSz; i++) {
		const char *slotname = dataIn.SlotName(i);
		if (slotname) {
			if ( keyTable.IsDefined(slotname) ) {
				const char *extSlotname = keyTable[slotname].AsCharPtr();
				String intVal = dataIn[i].AsCharPtr(); // in cases we are interested in it is always a string
				// map some well defined entities e.g. role, page, action
				// to shorter names
				if (intVal.Length() > 0) {
					if ( valMapTags.Contains(slotname) ) {
						if ( valTable.IsDefined(intVal) ) {
							dataOut[extSlotname] = valTable[intVal].AsCharPtr();
						} else {
							dataOut[extSlotname] = dataIn[i];
						}
					} else {
						dataOut[extSlotname] = dataIn[i];
					}
				}
			} else {
				dataOut[slotname] = dataIn[i];
			}
		} else {
			dataOut.Append(dataIn[i]);
		}
	}
	OStringStream os(&scrambledText);
	dataOut.PrintOn(os, false);

}

bool TableCompressor::DoExpand(Anything &dataOut, const String &scrambledText)
{
	StartTrace(TableCompressor.DoExpand);
	IStringStream is((String *)&scrambledText);
	ROAnything keyTable(GetUri2KeyMap());
	ROAnything valTable(GetUri2ValMap());
	ROAnything valMapTags(GetValMapTags());
	Anything dataIn;
	Trace(scrambledText);
	TraceAny(keyTable, "KeyTable: ");

	if ( dataIn.Import(is) ) {
		// expand it to long internal keys
		long dataSz = dataIn.GetSize();
		for (long i = 0; i < dataSz; i++) {
			const char *slotname = dataIn.SlotName(i);
			if (slotname) {
				if ( keyTable.IsDefined(slotname) ) {
					const char *intSlotname = keyTable[slotname].AsCharPtr();
					const char *compVal = dataIn[i].AsCharPtr();

					// expand some well known values also
					// e.g. role, page action names
					if ( valMapTags.Contains(intSlotname) ) {
						if ( valTable.IsDefined(compVal) ) { // is the value really compressed
							dataOut[intSlotname] = valTable[compVal].AsCharPtr();
						} else {
							dataOut[intSlotname] = dataIn[i];
						}
					} else {
						dataOut[intSlotname] = dataIn[i];
					}
				} else {
					dataOut[slotname] = dataIn[i];
				}
			} else {
				dataOut.Append(dataIn[i]);
			}
		}
	}
	return true;
}

void TableCompressor::MakeTable(ROAnything baseState, const char *tag, ROAnything config)
{
	StartTrace1(TableCompressor.MakeTable, "tag: " << tag);
	Anything state = baseState.DeepClone();
	TraceAny(state, "State: ");
	if (tag) {
		long sz = state.GetSize(), i;
		Anything map;
		Anything aSlot;
		const char *slotname = 0;

		for (i = sz - 1; i >= 0; i--) {
			aSlot = state[i];
			if ( aSlot.GetType() == Anything::eArray ) {
				ExpandConfig(i, state, config);
			}
		}

		for (i = 0; i < sz; i++) {
			aSlot = state[i];
			if ( aSlot.GetType() == Anything::eCharPtr ) {
				slotname = aSlot.AsCharPtr("");
			}
			if ( slotname ) {
				map[slotname] = CalcKey(i);
			}
			slotname = 0;
		}
		fMap[tag] = map;
	}
}

void TableCompressor::MakeReverseTable(ROAnything state, const char *tag, const char *reverseTag)
{
	StartTrace1(TableCompressor.MakeTable, "tag: <" << tag << "> reverseTag: <" << reverseTag << ">");
	TraceAny(state, "State: ");

	if (tag && reverseTag) {
		long sz = state.GetSize();
		Anything revKeyTable;
		const char *slotname;

		for (long i = 0; i < sz; i++) {
			slotname = state.SlotName(i);
			revKeyTable[state[i].AsCharPtr("")] = slotname;
		}

		fMap[reverseTag] = revKeyTable;
	}
}

void TableCompressor::ExpandConfig(long index, Anything &state, ROAnything config)
{
	StartTrace(TableCompressor.ExpandConfig);
	Trace("index: " << index);
	TraceAny(state, "State: ");
	TraceAny(config, "Config: ");

	String tag(state[index].SlotName(0L));
	ROAnything toExpand;

	Trace("Tag: <" << tag << ">");
	if (tag == "Expand" && config.LookupPath(toExpand, state[index]["Expand"].AsCharPtr(""))) {
		state.Remove(index);
		InstallConfig(index, state, toExpand);
	}
}

void TableCompressor::InstallConfig(long index, Anything &state, ROAnything part)
{
	StartTrace(TableCompressor.InstallConfig);
	Trace("index: " << index);
	TraceAny(state, "State: ");
	TraceAny(part, "Part: ");
	long sz = part.GetSize();
	const char *slotname = 0;
	for (long i = 0; i < sz; i++) {
		slotname = part.SlotName(i);
		if ( slotname ) {
			state[index++] = slotname;
			InstallConfig(index, state, part[i]);
		} else if (part[i].GetType() == Anything::eCharPtr) {
			state[index++] = part[i].AsCharPtr();
		}
		slotname = 0;
	}
}

String TableCompressor::CalcKey(long index)
{
	StartTrace1(TableCompressor.CalcKey, "index: " << index);
	String key;
	if ( (index / 52) > 0 ) {
		key << CalcKey((index / 52) - 1);
	}
	key << KeyAt(index % 52);
	Trace("key: <" << key << ">");
	return key;
}

char TableCompressor::KeyAt(int index)
{
	StartTrace1(TableCompressor.KeyAt, "index: " << (long)index);
	Assert((0 <= index) && (index < 52));
	if ( index < 0 ) {
		index = 0;
	}
	if ( index >= 52 ) {
		index = 51;
	}
	if ( index < 26 ) {
		return (index + 97);
	}
	return (index + 65 - 26);
}

ROAnything TableCompressor::GetMap(const char *tag) const
{
	ROAnything a;
	ROAnything(fMap).LookupPath(a, tag);
	return a;
}

ROAnything TableCompressor::GetKey2UriMap() const
{
	return GetMap("Key2UriMap");
}

ROAnything TableCompressor::GetVal2UriMap() const
{
	return GetMap("Val2UriMap");
}

ROAnything TableCompressor::GetUri2KeyMap() const
{
	return GetMap("Uri2KeyMap");
}

ROAnything TableCompressor::GetUri2ValMap() const
{
	return GetMap("Uri2ValMap");
}

ROAnything TableCompressor::GetValMapTags() const
{
	return GetMap("ValMapTags");
}
