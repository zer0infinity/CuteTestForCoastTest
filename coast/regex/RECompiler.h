/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RECompiler_H
#define _RECompiler_H

#include "Anything.h"

//!this class translates a regular expression into a NFA program
//!the syntax accepted is described in the NFA engine class RE
//!major entry point is the compile() method. Other methods are
//!only useful for testing or error reporting
//!relies on Coast's foundation Anything and String.
class RECompiler
{
	//! (intermediate) result of compilation
	Anything	fInstruction;
	//! keep syntax or other error messages here
	String		fErrorMsg;

	// Input state for compiling regular expression
	//! Input string
	String fPattern;
	//!Current input index into fPattern
	long fPatternIdx;
	//! Total number of paren pairs
	int fNofParentheses;
	//!bitmask of currently closed parenthized subexpressions
	//!to validate backreferences \1 - \9
	unsigned long fMaskOfClosedParentheses;

public:
	//!empty ctor
	RECompiler() {}
	//!main entry point, compile the pattern returns
	//!the resulting program within an Anything
	//!an empty Anything denotes an error in pattern
	Anything compile(const String &pattern);
	//!access the resulting error message, if any
	String	GetErrorMsg() {
		return fErrorMsg;
	}
protected:

	friend class RECompilerTest;
	//!auxiliary for testing
	Anything GetInstruction() {
		return fInstruction;
	}
	//!add an new NFA node in the middle at index insertAt
	void InsertNode(long opcode, long opdata, long insertAt);
	//!adjoin node with index pointTo to all open branches
	//!after index node
	void setNextOfEnd(long node, long pointTo);
	//!add a new NFA node at the end of the program
	//!returns its index
	long AppendNode(long opcode, long opdata);

	//! returns position of copied program
	Anything CopyProgram(long start, long len);
	//!append a deepcloned copy of prog
	long AppendProgram(Anything prog);
	//!collect s into fErrorMsg
	void syntaxError(String s) ;
	//!auxiliary for bracket()
	long ReadNumber(long def = 0);

	//!generate code for optional part of closure operator {n,m}
	void GenerateQuestions(Anything terminalprog, long terminalNode, long repetitions);
	//!generate code for optional REs used for ? operator and GeneraeQuestions
	void GenerateQuestion(long terminalNode);
	//!generate code for kleene's closure *
	void GenerateStar(long terminalNode);
	//!generate code for + closure operator
	void GeneratePlus(long terminalNode);
	//!generate code for required part of closure operator {n.m}
	long RepeatTerminal(Anything terminalprog, long terminalNode, long repetitions);
	//! treat pathological case of 0 repetition of closure operator {n,m}
	void RemoveTerminal(long terminalNode, long terminalEnd);

	//!parse {n,m} closure operator, return n and m-n in minimum and optional
	bool bracket(long &minimum, long &optional);
	//!parse an escape sequence starting with backslash '\\'
	//!can do octals and hex escapes, as well as some specials derived from perl
	//! \\s \\S \\b \\B \\d \\D \\w \\W see RE.h
	long escape() ;
	//!parse and generate code for POSIX style character classes,i.e. [:alnum:]
	long PosixCharacterClass() ;
	//!parse and generate code for character classes/sets i.e. [a-z0-9]
	long characterClass() ;
	//!parse and generate code for atoms which are sequences of literals
	long atom();
	//!parse and generate code for terminals, which are atoms, characterClass
	//!any operator . beginning of line ^ or end of line $ or subexpressions
	//!embedded in parentheses or an escape-special
	//!isnullable is used by closure to decide if a closure operator is valid
	long terminal(bool &isnullable);
	//!parse and generate code for a closure, which is a not-nullable terminal
	//! and a closure operator ? * + or {n,m}
	long closure(bool &isnullable) ;
	//!parse and generate code for a branch, implements concatenation
	//!of subexpressions within a branch
	long Branch(bool &isnullable) ;
	//!parse a (sub-)expression which consists of optionally a list
	//!of branches separated by |
	long expr(bool &isnullable) ;
	//!do not allow copying
	RECompiler(const RECompiler &); // no copy ctor
};
#endif
