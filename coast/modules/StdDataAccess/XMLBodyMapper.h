/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _XMLBodyMapper_H
#define _XMLBodyMapper_H

#include "config_stddataaccess.h"
#include "Mapper.h"

class Registry;
class Context;

//!Maps from and to XML Document bodies
//!The Get Part:<BR>
//!to be defined<P>
//!The Put Part:<BR>
//!Reads the Body part of an XML Document from a stream and transforms it to an Anything.
//!Very basic implementation, that just maps XML Tags to Anything slotnames, with
//!the following limitations.<UL>
//!<LI>Does ignore XML Tag attributes.</LI>
//!<LI>Maps all same Tags of one Level into one Anything slot - therefore preservation
//!of sequence is not garanteed.</LI>
//!<LI>Mixed type content
//!<PRE><aTAG>SomeText<anotherTAG>Text</anotherTAG>MoreText</aTAG></PRE>
//!is not supported.</LI></UL>
//!<PRE>Example:
//!<Team>
//!	<Programmer>
//!		<Name>Andy Birrer</Name>
//!		<Skill>C++</Skill>
//!		<Skill>Java</Skill>
//!	</Programmer>
//!	<Architect>
//!		<Name>Peter Sommerlad</Name>
//!		<Skill>OO-Design</Skill>
//!		<Skill>C++</Skill>
//!		<Skill>Design Patterns</Skill>
//!	</Architect>
//!	<Programmer>
//!		<Name>Marcel Rueedi</Name>
//!		<Skill>C++</Skill>
//!		<Skill>Java</Skill>
//!		<Skill>PL1</Skill>
//!	</Programmer>
//!</Team>
//!Maps to
//!{
//!	/Team	{
//!		/Programmer {
//!			{
//!				/Name	"Andy Birrer"
//!				/Skill {
//!					"C++"
//!					"Java"
//!				}
//!			}
//!			{
//!				/Name	"Marcel Rueedi"
//!				/Skill {
//!					"C++"
//!					"Java"
//!					"PL1"
//!				}
//!			}
//!		}
//!		/Architect {
//!			/Name	"Peter Sommerlad"
//!			/Skill {
//!				"OO-Design"
//!				"C++"
//!				"Design Patterns"
//!			}
//!		}
//!	}
//!}
//!</PRE>

class EXPORTDECL_STDDATAACCESS XMLBodyMapper : public ResultMapper
{
public:
	XMLBodyMapper(const char *name) : ResultMapper(name) { }
	~XMLBodyMapper()	{ }

	IFAObject *Clone() const {
		return new XMLBodyMapper(fName);
	}

	bool DoFinalPutStream(const char *key, istream &is, Context &ctx);
private:
	XMLBodyMapper();
	XMLBodyMapper(const XMLBodyMapper &);
	XMLBodyMapper &operator=(const XMLBodyMapper &);
};

#endif
