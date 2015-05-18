/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RE.h"
#include "Tracer.h"
#include "REBitSet.h"
#include "RECompiler.h"
#include <cstring>

RE::RE(const char *pattern, eMatchFlags matchFlags)
	: fMatchFlags(matchFlags)
{
	StartTrace(RE.RE);
	Trace(pattern);
	setProgram(RECompiler().compile(pattern));
}
RE::RE(const String &pattern, eMatchFlags matchFlags)
	: fMatchFlags(matchFlags)
{
	StartTrace(RE.RE);
	Trace(pattern);
	setProgram(RECompiler().compile(pattern));
}
RE::RE(Anything program, eMatchFlags matchFlags)
	: fMatchFlags(matchFlags)
{
	StartTrace(RE.RE);
	TraceAny(program, "program");
	setProgram(program);
}
RE::RE()
{
	StartTrace(RE.RE);
}
String RE::SimplePatternToFullRegularExpression(const String &pattern)
{
	StartTrace(RE.SimplePatternToFullRegularExpression);
	String buf;
	for (long i = 0; i < pattern.Length(); ++i) {
		char c = pattern[i];
		switch (c) {
			case '*':
				buf.Append(".*");
				break;

			case '.':
			case '[':
			case ']':
			case '\\':
			case '+':
			case '?':
			case '{':
			case '}':
			case '$':
			case '^':
			case '|':
			case '(':
			case ')':
				buf.Append('\\');
			default:
				buf.Append(c);
				break;
		}
	}
	return buf;
}
String RE::GetMatch(long const which) const
{
	StartTrace1(RE.GetMatch, "match#" << which);
	long start;
	if (which < fRegisters.GetSize() && (start = GetStartRegister(which)) >= 0) {
		return fSearch.SubString(start, GetEndRegister(which)-start);
	}
	return String();
}

long RE::GetStartRegister(long const which) const
{
	StatTrace(RE.GetStartRegister, "match#" << which << " position: " << ROAnything(fRegisters)[which][0L].AsLong(-1), coast::storage::Current());
	return ROAnything(fRegisters)[which][0L].AsLong(-1);
}
long RE::GetEndRegister(long const which) const
{
	StatTrace(RE.GetEndRegister, "match#" << which << " position: " << ROAnything(fRegisters)[which][1L].AsLong(-1), coast::storage::Current());
	return ROAnything(fRegisters)[which][1L].AsLong(-1);
}
void RE::SetStartRegister(long const which, long const i)
{
	StatTrace(RE.SetStartRegister, "match#" << which << " pos:" << i, coast::storage::Current());
	fRegisters[which][0L] = i;
}
void RE::SetEndRegister(long const which, long const i)
{
	StatTrace(RE.SetEndRegister, "match#" << which << " pos:" << i, coast::storage::Current());
	fRegisters[which][1L] = i;
}
long RE::MatchNodes(long firstNode, long lastNode, long idxStart)
{
	StartTrace(RE.MatchNodes);
	Trace("firtsNode = " << firstNode);
	// Our current place in the string
	long idx = idxStart;
	if (idx < 0) {
		return -1;
	}

	long idxNew(-1);
	if (lastNode < 0) {
		lastNode = fProgram.GetSize(); // go to the end
	} else {
		Trace("lastnode: " << lastNode);
	}
	long next = lastNode;
	for (long node = firstNode; node < lastNode; node = next) {
		next   = node + (short)fProgram[node][offsetNext].AsLong(lastNode);

		long opcode = fProgram[node][offsetOpcode].AsLong(-1);
		long opdata = fProgram[node][offsetOpdata].AsLong(-1);

		TraceAny(fProgram[node], "match node : " << node);
		Trace("at: " << idx << " = char:" << fSearch[idx]);
		Assert(idx >= 0); // SOP: simplyfy conditionals below.

		switch (opcode) {
			case OP_RELUCTANTMAYBE: {
				long once = 0;
				do {
					// Try to match the rest without using the reluctant subexpr
					if ((idxNew = MatchNodes(next, -1, idx)) != -1) {
						return idxNew;
					}
				} while ((once++ == 0) && (idx = MatchNodes(node + nodeSize, next, idx)) != -1);
				return -1;
			}
			case OP_RELUCTANTPLUS:
				while ((idx = MatchNodes(node + nodeSize, next, idx)) != -1) {
					// Try to match the rest without using the reluctant subexpr
					if ((idxNew = MatchNodes(next, -1, idx)) != -1) {
						return idxNew;
					}
				}
				return -1;
			case OP_RELUCTANTSTAR:
				do {
					// Try to match the rest without using the reluctant subexpr
					if ((idxNew = MatchNodes(next, -1, idx)) != -1) {
						return idxNew;
					}
				} while ((idx = MatchNodes(node + nodeSize, next, idx)) != -1);
				return -1;
			case OP_OPEN:
				fBackRefs[opdata][0L].Append(idx); // push idx, might be wrong and doesn't match
				if ((idxNew = MatchNodes(next, -1, idx)) != -1) {
					if (GetStartRegister(opdata) == -1) {
						SetStartRegister(opdata, idx);
					}
				} else {
					long topindex = fBackRefs[opdata][0L].GetSize() - 1;
					Assert(topindex >= 0);
					fBackRefs[opdata][0L].Remove(topindex); // pop after fail to match
				}
				return idxNew;
			case OP_CLOSE:
				fBackRefs[opdata][1L].Append(idx); // push idx
				if ((idxNew = MatchNodes(next, -1, idx)) != -1) {
					if (GetEndRegister(opdata) == -1) {
						SetEndRegister(opdata, idx);
					}
				} else {
					long topindex = fBackRefs[opdata][1L].GetSize() - 1;
					Assert(topindex >= 0);
					fBackRefs[opdata][1L].Remove(topindex); // pop idx after failed match
				}
				return idxNew;
			case OP_BACKREF: {
				long s = ROAnything(fBackRefs)[opdata][0L][0L].AsLong(-1);
				long e = ROAnything(fBackRefs)[opdata][1L][0L].AsLong(-1);
				if (s == -1 || e == -1) {
					return -1;
				}
				if (s == e) {
					break;
				}
				long len = e - s;
				if (fSearch.Length() <= (idx + len - 1)) {
					return -1;
				}
				// SOP: might be optimized by a substring compare method of String
				if ((fMatchFlags & MATCH_ICASE) != 0) {
					if (String::CaselessCompare(fSearch.SubString(idx, len), fSearch.SubString(s, len))) {
						return -1;
					} else {
						idx += len; // we matched
					}
				} else {
					const char *base = fSearch;
					if (0 == strncmp(base + idx, base + s, len)) {
						idx += len;
					} else {
						return -1;
					}
				}
				break;
			}
			case OP_BOL:
				if ((idx != 0) &&
					((fMatchFlags & MATCH_MULTILINE) != MATCH_MULTILINE
					 || '\n' != fSearch.At(idx - 1))) {
					return -1;
				}
				break;
			case OP_EOL:
				if (fSearch.Length() > 0 && fSearch.Length() > idx
					&& ((fMatchFlags & MATCH_MULTILINE) != MATCH_MULTILINE
						|| '\n' != fSearch.At(idx))) {
					return -1;
				}
				break;
			case OP_BOW: {
				char cLast = ((idx == 0 /*GetStartRegister(0)*/) ? ' ' : fSearch.At(idx - 1));
				char cNext = ((fSearch.Length() <= (idx)) ? ' ' : fSearch.At(idx));
				if ((isalnum(cLast) == isalnum(cNext)) == (opdata != 0)) {
					return -1;
				}
				break;
			}
			case OP_ANY:
				if ((fSearch.Length() <= (idx)) ||
					(fSearch.At(idx) == '\n' &&
					 (fMatchFlags & DOT_MATCHES_NEWLINE) != DOT_MATCHES_NEWLINE)) {
					return -1;
				}
				++idx;
				break;
			case OP_ATOM: {
				String atom = fProgram[node][RE::offsetOpdata].AsCharPtr();
				Assert(atom.Length() > 0);
				if (fSearch.Length() <= (atom.Length() + idx - 1)) {
					return -1;
				}
				if ((fMatchFlags & MATCH_ICASE) != 0) {
					if (0 != String::CaselessCompare(atom, fSearch.SubString(idx, atom.Length()))) {
						return -1;
					}
				} else {
					if (0 != fSearch.CompareN(atom, atom.Length(), idx)) {
						return -1;
					}
				}
				idx += atom.Length();
				break;
			}
			case OP_ANYOF: {
				if (fSearch.Length() <= (idx)) {
					return -1;
				}
				char c = fSearch.At(idx);
				if (fMatchFlags & MATCH_ICASE) {
					c = tolower(c);
				}
				REBitSet *cset = (REBitSet *)fProgram[node][RE::offsetOpdata].AsCharPtr(0);
				if (!cset || !cset->IsMember((unsigned char)c)) {
					Assert(cset); // internal error
					return -1;
				}
				++idx;
				break;
			}
			case OP_BRANCH: {
				if (fProgram[next][offsetOpcode].AsLong(0L) != OP_BRANCH) {
					// If there aren't any other choices, just evaluate this branch.
					next = node + nodeSize;
					continue;
				}
				// Try all available branches
				long nextBranch;
				do {
					// Try matching the branch against the string
					if ((idxNew = MatchNodes(node + nodeSize, -1, idx)) != -1) {
						return idxNew;
					}
					// Go to next branch (if any)
					nextBranch = fProgram[node][offsetNext].AsLong(0);
					node += nextBranch;
				} while (nextBranch != 0 && (fProgram[node][offsetOpcode].AsLong(0) == OP_BRANCH));
				// Failed to match any branch!
				return -1;
			}
			case OP_NOTHING:
			case OP_GOTO:
//SOP: we should detect endless loops not advancing input i.e. epsilon closures
// this is what isnullable is used for in the compiler.... otherwise we might need
// to "optimize" the automaton i.e. make it a DFA before we run it.
				// Just advance to the next node without doing anything
				break;
			case OP_END:
				// Match has succeeded!
				return idx;
			default:
				Trace("invalid opcode :" << opcode);
				Assert(false);
				return -1;
		}
	}
	Trace("corrupt program");
	Assert(false);
	return -1;
}

bool RE::MatchAt(long i)
{
	StartTrace(RE.MatchAt);
	Trace("position = " << i);
	Assert(i >= 0);
	if (i >= 0) {
		// Initialize start pointer, paren cache and paren count
		fRegisters = Anything(Anything::ArrayMarker());
		SetStartRegister(0, i);

		// Allocate backref arrays (unless optimizations indicate otherwise)
		fBackRefs = Anything(Anything::ArrayMarker());

		// Match against string
		long idx;
		if ((idx = MatchNodes(0, -1, i)) != -1) {
			SetEndRegister(0, idx);
			return true;
		}
	}
	// Didn't match
	fRegisters = Anything(Anything::ArrayMarker());
	return false;
}

bool RE::ContainedIn(const String &search, long i)
{
	StartTrace(RE.ContainedIn);
	TraceAny(fProgram, "program");
	Trace("prefix : " << fPrefix);
	Trace("search : " << search);
	Trace("start  : " << i);

	fRegisters = Anything(Anything::ArrayMarker());
	if (fProgram.GetSize() <= 0) {
		Trace("No RE program to run!");
		Assert(fProgram.GetSize() > 0);
		return false;
	}
	fSearch = search;

	// Can we optimize the search by looking for a prefix string?
	if (fPrefix.Length() <= 0) {
		// Unprefixed matching must try for a match at each character
		for ( ; fSearch.Length() > (i - 1); ++i) {
			// Try a match at index i
			if (MatchAt(i)) {
				return true;
			}
		}
		return false;
	} else {
		bool caseIndependent = (fMatchFlags & MATCH_ICASE) != 0;
		for ( ; fSearch.Length() > (i + fPrefix.Length() - 1); i++) {
			bool match = false;
			if (caseIndependent) {
				match = 0 == String::CaselessCompare(fPrefix, fSearch.SubString(i, fPrefix.Length()));
			} else {
				match = fPrefix == fSearch.SubString(i, fPrefix.Length());
			}

			if (match) {
				if (MatchAt(i)) {
					return true;
				}
			}
		}
		return false;
	}
}

Anything RE::Split(const String &s)
{
	StartTrace(RE.Split);
	Trace("input:" << s);
	Anything res;

	int pos = 0;
	long len = s.Length();
	while (pos < len && ContainedIn(s, pos)) {
		long start = GetStartRegister(0);
		long newpos = GetEndRegister(0);
		if (newpos == pos) {
			res.Append(s.SubString(pos, start + 1 - pos));
			++newpos;
		} else {
			res.Append(s.SubString(pos, start - pos));
		}
		pos = newpos;
	}
	String remainder = s.SubString(pos);
	if (remainder.Length() != 0) {
		res.Append(remainder);
	}
	return res;
}
String RE::Subst(const String &substituteIn, const String &substitution, bool replaceall) {
	StartTrace1(RE.Subst, "replaceall?" << (replaceall?"true":"false") << " input [" << substituteIn << "] Subst [" << substitution << "]");
	long len = substituteIn.Length();
	String ret(len);
	int pos = 0;

	while (pos < len && ContainedIn(substituteIn, pos)) {
		ret.Append(substituteIn.SubString(pos, GetStartRegister(0) - pos));
		ret.Append(substitution);
		long newpos = GetEndRegister(0);
		if (newpos == pos) {
			++newpos;
		}
		pos = newpos;
		if (!replaceall) {
			break;
		}
	}
	if (pos < len) {
		ret.Append(substituteIn.SubString(pos));
	}
	Trace("result:" << ret);
	return ret;
}
Anything RE::Grep(Anything search)
{
	StartTrace(RE.Grep);
	TraceAny(search, "input");
	Anything res;
	for (long i = 0; i < search.GetSize(); ++i) {
		String s = search[i].AsString();
		if (ContainedIn(s)) {
			const char *sname = search.SlotName(i);
			if (sname && ! res.IsDefined(sname)) {
				res[sname] = search[i];
			} else {
				res.Append(search[i]);
			}
		}
	}
	TraceAny(res, "result");
	return res;
}
Anything RE::GrepSlotNames(Anything search)
{
	StartTrace(RE.GrepSlotNames);
	TraceAny(search, "input");
	Anything res;
	for (long i = 0; i < search.GetSize(); ++i) {
		String s = search.SlotName(i);
		if (ContainedIn(s)) {
			if (s.Length() > 0) {
				res[s] = search[i];    // caution for reference semantics of Anythings
			} else {
				res.Append(search[i]);
			}
		}
	}
	TraceAny(res, "result");
	return res;
}
bool RE::Match(const String &s, long &start, long &len)
{
	StartTrace(RE.Match);
	if (ContainedIn(s, start)) {
		start = GetStartRegister(0);
		len = GetEndRegister(0) - start;
		Trace("matched");
		return true;
	}
	len = 0L;
	Trace("didn't match");
	return false;
}
