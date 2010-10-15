/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RE_H
#define _RE_H

#include "config_regex.h"
#include "Anything.h"

//!implement a simple regular expression engine for Coast
/*! it uses WD's memory management mechanisms with Anythings and Strings
	to make it useful for multi-threaded programs (which the standard recomp(),reexec()
	aren't). Derived from Jakarta's regular expression subproject java source
	and heavily refactored.
	the compiled regular expression is stored in an Anything
	each slot represents a node in the non-deterministic (backtracking)
	automaton. each node is a 3 slot anything with opcode, opdata (=parameter),
	and opnext, the relative offset to the next node to consider if
	the current node matches. */
class EXPORTDECL_REGEX RE
{
public:
	/*! with this RE impl. the matching engine handles the following flags, not the
		RE compiler as it is the case with regcomp/regexec fuctions */
	//! flag combinations for further convenience
	enum eMatchFlags {
		MATCH_NORMAL          = 0x0000, //!< Specifies normal, case-sensitive matching behaviour.
		MATCH_ICASE           = 0x0001, //!< Flag to indicate that matching should be case-independent (folded)
		MATCH_MULTILINE       = 0x0002, //!< Newlines should match as BOL/EOL (^ and $)
		ICASE_MULTILINE		  = 0x0003, //!< convenience
		DOT_MATCHES_NEWLINE   = 0x0004, //!< Consider all input a single body of text - newlines are matched by .
		ICASE_NEWLINE		  = 0x0005, //!< convenience
		MULTI_NEWLINE    	  = 0x0006, //!< not a very sensible combination
		ICASE_MULTI_NEWLINE	  = 0x0007  //!< convenience
	};

	//! the opcodes of our RE engine
	enum eOpCodes {
		//   Opcode              Char       Opdata/Operand  Meaning
		//   ----------          ---------- --------------- --------------------------------------------------
		OP_END              = 'E',  //!<                 end of program
		OP_BOL              = '^',  //!<                 match only if at beginning of line
		OP_EOL              = '$',  //!<                 match only if at end of line
		OP_ANY              = '.',  //!<                 match any single character except newline
		OP_ANYOF            = '[',  //!< count/ranges    match any char in the list of ranges
		OP_BRANCH           = '|',  //!< node            match this alternative or the next one
		OP_ATOM             = 'A',  //!< length/string   length of string followed by string itself
		OP_OPEN             = '(',  //!< number          nth opening paren
		OP_CLOSE            = ')',  //!< number          nth closing paren
		OP_BACKREF          = '#',  //!< number          reference nth already matched parenthesized string
		OP_GOTO             = 'G',  //!<                 nothing but a (back-)pointer
		OP_NOTHING          = 'N',  //!<                 match null string such as in '(a|)'
		OP_RELUCTANTSTAR    = '8',  //!< none/expr       reluctant '*' (mnemonic for char is unshifted '*')
		OP_RELUCTANTPLUS    = '=',  //!< none/expr       reluctant '+' (mnemonic for char is unshifted '+')
		OP_RELUCTANTMAYBE   = '/',  //!< none/expr       reluctant '?' (mnemonic for char is unshifted '?')
		OP_BOW		        = 'b'   //!< bool	           match at word boundary or not at word boundary
	};

	//! special Escape codes supported within atoms/literals and character classes [...]
	enum eEscapeCodes {
		E_ALNUM             = 'w',  //!< Alphanumeric
		E_NALNUM            = 'W',  //!< Non-alphanumeric
		E_BOUND             = 'b',  //!< Word boundary - not in character class []
		E_NBOUND            = 'B',  //!< Non-word boundary - not in character class []
		E_SPACE             = 's',  //!< Whitespace
		E_NSPACE            = 'S',  //!< Non-whitespace
		E_DIGIT             = 'd',  //!< Digit
		E_NDIGIT            = 'D'   //!< Non-digit
	};

	//! Node layout constants, we use a three slot Anything per node
	enum eLayoutConstants {
		offsetOpcode = 0,            //!< Opcode offset
		offsetOpdata = 1,            //!< Opdata offset
		offsetNext   = 2,            //!< Next index offset
		nodeSize     = 1             //!< Node size (in Anything offsets)
	};

public:
	// before matching
	//!provide convenience ctor to avoid ambiguity between String and Anything
	RE(const char *pattern, eMatchFlags matchFlags = MATCH_NORMAL);

	//!compile pattern
	RE(const String &pattern, eMatchFlags matchFlags = MATCH_NORMAL);

	//!take a precompiled program produced by RECompiler::compile()
	RE(Anything program, eMatchFlags matchFlags = MATCH_NORMAL);
	//!Constructs a regular expression matcher with no initial program.
	//!This is likely to be an uncommon practice, but is still supported.
	RE();
	//!convenience function substituting * with .* re and masking
	//!other re special characters
	static String SimplePatternToFullRegularExpression(const String &pattern);
	//!modify matching behavior
	void SetMatchFlags(eMatchFlags matchFlags) {
		this->fMatchFlags = matchFlags;
	}
	//!make it a new re by setting a new program.
	void setProgram(Anything program) {
		fProgram = program["program"];
		fPrefix = program["prefix"].AsCharPtr("");
		fRegisters = Anything(Anything::ArrayMarker());
		fBackRefs = Anything(Anything::ArrayMarker());
	}
	//!was the given pattern compilable
	bool IsValid() const {
		return fProgram.GetSize() > 0 ;
	}
	// do the matching
	//! does the pattern match from position i within search string
	bool ContainedIn(const String &search, long fromPosition = 0);
	//!same as ContainedIn, but delivers Register 0 (=whole match) directly
	//!initial value of start gives the starting point in s
	bool Match(const String &s, long &start, long &len);
	//!split the string between matches like a very flexible StringTokenizer
	//!i.e. Split with an RE of [^A-Za-z0-9_]+ should return all words from a text
	Anything Split(const String &s);
	//!replace occurences of RE in substitureIn by string substitution
	//!take care with REs that match the empty string!
	String Subst(const String &substituteIn, const String &substitution, bool replaceall = true);
	//!retrieve collection of slots where the value is matching from search
	//!should we keep the slotnames?
	Anything Grep(Anything search);
	//!retrieve collection of slots with matching slotnames from search
	Anything GrepSlotNames(Anything search);

	//after the match use the following methods to access the
	//matched substrings
	//!how many parentheses exist and matched last time in the re
	long GetRegisterCount() const {
		return fRegisters.GetSize();
	}
	//!get the matched string, 0 means complete match, >0 take register
	String GetMatch(long const subMatch = 0L) const;
	//! return the start and end of a RE subexpression
	//!!param - which: 0 means position of whole mach, > 0 register of match
	long GetStartRegister(long const which) const;
	long GetEndRegister(long const which) const;

protected:
	//! auxiliaries used for matching
	void SetStartRegister(long const which, long const posInSearch);

	void SetEndRegister(long const which, long const posInSearch);
	//!implement the recursive (backtracking) NFA engine
	//! matches fSearch from position idxStart with the
	//! (sub-)NFA from firstNode to lastNode
	long MatchNodes(long firstNode, long lastNode, long idxStart);
	bool MatchAt(long i);
public:
	//!auxiliary for debugging purposes
//!	friend
//!	ostream &operator<<(ostream &os,const RE& re){ os << "prog:\n" << re.fProgram << "\n:prog end" << endl; return os;}

protected:
	// State of current program
	//!Compiled regular expression
	Anything fProgram;
	//!string to search in
	//! we backtrack, therefore we cannot easily use an istream
	String fSearch;
	//!literal prefix if any, to speed up matching
	String fPrefix;
	//!control the behavior of the matching engine, see eMatchFlags
	enum eMatchFlags fMatchFlags;
	//!keep track of matching positions within fSearch, cf. to regmatch_t of regexec
	//!access with GetStartRegister and GetEndRegister
	Anything fRegisters;
	//!keep a stack of potentially matched subexpressions
	//! ready for backreferences.
	//! only the leftmost one will be matched by the corresponding backref
	Anything fBackRefs;
private:
	friend class RegexTest;
};
#endif
