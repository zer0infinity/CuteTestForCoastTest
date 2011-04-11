/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MIMEHeaderTest.h"
#include "MIMEHeader.h"
#include "TestSuite.h"
#include "AnyUtils.h"

void MIMEHeaderTest::SimpleHeaderTest() {
	StartTrace(MIMEHeaderTest.SimpleHeaderTest);

	Anything result, result1;

	// some bad data tests
	String testinput = "nonsense";
	{
		MIMEHeader mh;
		StringStream is(testinput);

		// basic checks of success
		try {
			t_assertm(!mh.ParseHeaders(is), "expected header parsing to fail");
		} catch (MIMEHeader::InvalidLineException &e) {
			t_assertm(true, "expected invalid line exception to happen");
		} catch (...) {
			t_assertm(false,"did not expect other exceptions to be thrown");
		}

		// sanity checks
		t_assertm(mh.IsMultiPart() == false, "expected no multipart");
		t_assertm(mh.GetBoundary().Length() == 0, "expected no multipart seperator");
		t_assertm(mh.GetContentLength() == -1, "expected -1, since field is not set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");
		assertAnyEqual(result, mh.GetInfo());
		TraceAny(mh.GetInfo(), "Header: ");
	}
}

void MIMEHeaderTest::ConfiguredTests() {
	StartTrace(MIMEHeaderTest.ConfiguredTests);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		TString caseName;
		if (!aEntryIterator.SlotName(caseName)) {
			caseName << "At index: " << aEntryIterator.Index();
		}
		Coast::URLUtils::NormalizeTag normalizeKey = (Coast::URLUtils::NormalizeTag) (caseConfig["NormalizeTag"].AsLong(
				(long) Coast::URLUtils::eUpshift));
		MIMEHeader mh(normalizeKey);

		String testinput = caseConfig["Input"].AsString();
		Trace("TestInput: <" << testinput << ">");
		StringStream is(testinput);
		// basic checks of success
		t_assertm(mh.ParseHeaders(is), TString("expected header parsing to succeed at ") << caseName);

		// sanity checks
		assertEqualm(caseConfig["IsMultiPart"].AsBool(false), mh.IsMultiPart(), caseName);
		assertComparem(caseConfig["BoundaryLength"].AsLong(0), equal_to, mh.GetBoundary().Length(), caseName);
		assertCharPtrEqualm(caseConfig["Boundary"].AsString(), mh.GetBoundary(), caseName);
		assertComparem(caseConfig["ContentLength"].AsLong(-1), equal_to, mh.GetContentLength(), caseName);
		assertEqualm("", mh.Lookup("NotThere", ""), caseName);
		assertAnyCompareEqual(caseConfig["Expected"], mh.GetInfo(), caseName, '.', ':');
	}
}

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>

namespace client {
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	typedef String stringtype;

	template <typename Iterator>
	struct rfc_skipper : public qi::grammar<Iterator>
	{
		rfc_skipper() : rfc_skipper::base_type(start, "rfc_skipper")
		{
			using qi::lexeme;
			using qi::string;
			using ascii::char_;
			using qi::lit;
			qpair   =  char_('\\') >> char_;
			ctext   =  char_ - char_("()\\\r");
			rCR     =  char_('\r');
			rLF     =  char_('\n');
			rCRLF   =  rCR >> rLF;
			rSP     =  char_(' ');
			rHT     =  char_('\t');
			comment =  char_('(')
					   >>  *(   lwsp
							|   ctext
							|   qpair
							|   comment
							)
					   >> ')';
			lwsp    = +(    -lit(rCRLF)
							>> ( rSP | rHT )
					   );
			junk    = lwsp | comment;
			start = lexeme[junk];
		}
		qi::rule<Iterator> rCR, rLF, rSP, rHT, rCRLF;
		qi::rule<Iterator> qpair, ctext, comment, lwsp, junk, start;
	};

    typedef std::pair<stringtype, boost::optional<stringtype> > string_pair;
    typedef std::vector<string_pair> pairs_type;

    template <typename Iterator>
	struct rfc2616_headers : public qi::grammar<Iterator, pairs_type()>
	{
		rfc2616_headers() : rfc2616_headers::base_type(start, "rfc2616_headers")
		{
			using qi::lexeme;
			using qi::string;
			using ascii::char_;
			using qi::lit;
			rOctet  =  char_('\x00','\xff');
			rChar   =  char_('\x00','\x7f');
			rUpAlpha=  ascii::upper;
			rLoAlpha=  ascii::lower;
			rAlpha  =  rUpAlpha | rLoAlpha;
			rDigit  =  ascii::digit;
			rCtl    =  char_('\x00', '\x1f') | char_('\x7f');
			rCR     =  char_('\r');
			rLF     =  char_('\n');
			rCRLF   =  rCR >> rLF;
			rSP     =  char_(' ');
			rHT     =  char_('\t');
			lwsp    = +(    -lit(rCRLF)
							>> ( rSP | rHT )
					   );
			rDQuote =  char_('"');
			rText   =  ((rOctet - rCtl) | lwsp); // ??
			rHex    =  ascii::xdigit;
			rSeparators =  char_("()<>@,;:\\/[]?={}") | rDQuote | rSP | rHT;
			rToken  =  +(char_ - rCtl - rSeparators);
			qpair   =  char_('\\') >> rChar;
			rQuotedString = rDQuote >> *(rQDText | qpair) >> rDQuote;
			ctext   =  rText - char_("()");
			rQDText   =  rText - rDQuote;
			comment =  char_('(')
					   >>  *(   ctext
							|   qpair
							|   comment
							)
					   >> ')';

			rMessageHeaders = *(rHeader >> rCRLF) >> rCRLF;
			rHeader = rFieldName >> ":" >> -rFieldValue;
			rFieldName = rToken;
			rFieldValue = *(rFieldContent | lwsp);
			rFieldContent = *rText | *(rToken | rSeparators | rQuotedString);
//			junk    = lwsp | comment;
//			start = lexeme[junk];
			start = lexeme[rMessageHeaders];
		}
		qi::rule<Iterator> rOctet, rChar, rUpAlpha, rLoAlpha, rAlpha, rDigit, rCtl, rCR, rLF, rSP, rHT, rDQuote, rCRLF, rText, rHex, rSeparators, rQuotedString, rQDText;
		qi::rule<Iterator, pairs_type()> rMessageHeaders, start;
		qi::rule<Iterator, string_pair()> rHeader;
		qi::rule<Iterator, stringtype()> rFieldName, rFieldValue, rFieldContent, rToken, lwsp;
		qi::rule<Iterator> qpair, ctext, comment, junk;
	};

	template<typename Iterator, typename SKIP = qi::unused_type>
	struct string_parser: qi::grammar<Iterator, stringtype(), SKIP> {
		string_parser() :
			string_parser::base_type(quoted_string) {
			using qi::int_;
			using qi::lit;
			using qi::double_;
			using qi::lexeme;
			using ascii::char_;

			quoted_string %= lexeme['"' >> +(char_ - '"') >> '"'];

			//            start %=
			//                lit("employee")
			//                >> '{'
			//                >>  int_ >> ','
			//                >>  quoted_string >> ','
			//                >>  quoted_string >> ','
			//                >>  double_
			//                >>  '}'
			//                ;
		}

		qi::rule<Iterator, client::stringtype(), ascii::space_type> quoted_string;
		//        qi::rule<Iterator, employee(), ascii::space_type> start;
	};
}

void MIMEHeaderTest::spiritTests() {
	StartTrace(MIMEHeaderTest.spiritTests);
	namespace qi = boost::spirit::qi;
	using boost::spirit::ascii::char_;
	using boost::spirit::ascii::string;
	using boost::spirit::ascii::space;

	typedef String::const_iterator iter_type;
	{
		String kstr, vstr, instr;
		instr = "key: value";
		iter_type sbeg = instr.begin(), send = instr.end();
		qi::phrase_parse(sbeg, send, +(qi::char_("a-zA-Z") - ':') >> ':' >> +qi::char_, space, kstr, vstr);
		Trace("key [" << kstr << "] value [" << vstr << "]");
		assertCharPtrEqual("key", kstr);
		assertCharPtrEqual("value", vstr);
	}
	qi::rule<iter_type> keyvalrule = +( qi::char_("a-zA-Z") - ':' ) >> ':' >> +qi::char_;
	{
		String kstr, vstr, instr;
		instr = "key: val\r\n ue";
		client::rfc_skipper<iter_type> rfcskipper;
		iter_type sbeg = instr.begin(), send = instr.end();
//		qi::phrase_parse(sbeg, send, +(qi::char_("a-zA-Z") - ':') >> qi::lexeme[':' >> +qi::char_], rfcskipper, kstr, vstr);
		qi::parse(sbeg, send, qi::skip(rfcskipper.start.alias())[+qi::char_], kstr);
		Trace("key [" << kstr << "] value [" << vstr << "]");
		assertCharPtrEqual("key:value", kstr);
	}
	{
		String kstr, vstr, instr;
		instr = "12\r\n (te\\((HEU12)st)(\r\n )\tJUN\t82";
		client::rfc_skipper<iter_type> rfcskipper;
		iter_type sbeg = instr.begin(), send = instr.end();
		qi::parse(sbeg, send, qi::skip(rfcskipper.start.alias())[+qi::char_], kstr);
		Trace("key [" << kstr << "] value [" << vstr << "]");
		assertCharPtrEqual("12JUN82", kstr);
	}
	{
		String kstr, vstr, instr;
		instr = "key: val\r\n ue";
		client::rfc_skipper<iter_type> rfcskipper;
		iter_type sbeg = instr.begin(), send = instr.end();
		qi::phrase_parse(sbeg, send, +(qi::char_("a-zA-Z") - ':') >> ':' >> +qi::char_, rfcskipper, kstr, vstr);
		Trace("key [" << kstr << "] value [" << vstr << "]");
		assertCharPtrEqual("key", kstr);
		assertCharPtrEqual("value", vstr);
	}
}

void MIMEHeaderTest::testrfcheaders() {
	StartTrace(MIMEHeaderTest.testrfcheaders);
//	namespace qi = boost::spirit::qi;
//    String input("key1: value1\r\nkey2:\r\nkey3:val\r\n ue3");
//    typedef String::const_iterator stringIter;
//    stringIter begin = input.begin();
//    stringIter  end = input.end();
//
//    client::rfc2616_headers<stringIter> p;
//    client::pairs_type m;
//
//    if (!qi::parse(begin, end, p, m))
//    {
//        std::cout << "-------------------------------- \n";
//        std::cout << "Parsing failed\n";
//        std::cout << "-------------------------------- \n";
//    }
//    else
//    {
//        std::cout << "-------------------------------- \n";
//        std::cout << "Parsing succeeded, found entries:\n";
//        client::pairs_type::iterator end = m.end();
//        for (client::pairs_type::iterator it = m.begin(); it != end; ++it)
//        {
//            std::cout << (*it).first;
//            if ((*it).second)
//                std::cout << "=" << boost::get<client::stringtype>((*it).second);
//            std::cout << std::endl;
//        }
//        std::cout << "---------------------------------\n";
//    }
}

// builds up a suite of testcases, add a line for each testmethod
Test *MIMEHeaderTest::suite() {
	StartTrace(MIMEHeaderTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MIMEHeaderTest, SimpleHeaderTest);
	ADD_CASE(testSuite, MIMEHeaderTest, ConfiguredTests);
	ADD_CASE(testSuite, MIMEHeaderTest, spiritTests);
	ADD_CASE(testSuite, MIMEHeaderTest, testrfcheaders);
	return testSuite;
}

