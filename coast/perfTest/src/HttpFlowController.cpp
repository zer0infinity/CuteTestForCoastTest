/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HttpFlowController.h"
#include "Threads.h"
#include "URLUtils.h"
#include "Resolver.h"
#include "Renderer.h"
#include "Scheduler.h"
#include "SystemFile.h"
#include "StringStream.h"

RegisterFlowController(HttpFlowController);
//---- HttpFlowController -----------------------------------------------------------
HttpFlowController::HttpFlowController(const char *HttpFlowControllerName) : FlowController(HttpFlowControllerName), fJmpTableInit(false), fDoRelocate(false)
{
	StartTrace1(HttpFlowController.HttpFlowController, "<" << HttpFlowControllerName << ">");
}

void HttpFlowController::ProcessSetCookie(Context &ctx)
{
	StartTrace(HttpFlowController.ProcessSetCookie);

	String cookieInfo;
	Anything tmpStore = ctx.GetTmpStore();
	// set-cookie is a HTTP header
	Anything setCookie = tmpStore["Mapper"]["HTTPHeader"]["set-cookie"];

	// Process all cookies, loop if we're an Array
	if ( setCookie.GetType() == AnyArrayType ) {
		for (long i = 0; i < setCookie.GetSize(); i++ ) {
			cookieInfo = setCookie[i].AsString();
			DoProcessSetCookie(cookieInfo, ctx);
		}
	} else {
		cookieInfo = setCookie.AsString("");
		if ( cookieInfo != "" ) {
			DoProcessSetCookie(cookieInfo, ctx);
		}
	}
	TraceAny( tmpStore["Cookies"], "<-- latest cookie any" );
}

void HttpFlowController::DoProcessSetCookie(String &cookieInfo, Context &ctx)
{
	StartTrace(HttpFlowController.DoProcessSetCookie);

	long Pos = cookieInfo.StrChr( '=' );
	Trace( "pos = is->" << Pos << " in cookieString->" << cookieInfo );

	if (Pos > 0 ) {
		String cookieName = cookieInfo.SubString(0, Pos);
		String explicitDomainName = "";
		String explicitPortName = "";
		bool explicitDomainOrPortUsed = false;

		long Pos2 = cookieInfo.StrChr( ';' );
		Trace( "pos ; is->" << Pos2 << " cookie name is->" << cookieName );
		String cookieFull = cookieInfo.SubString(0, Pos2);
		Trace( "CookieFull is-> " << cookieFull << "<- length is " << cookieFull.Length() );

		if (Pos2 > 0 ) {
			String tempCookieInfo = cookieInfo;
			tempCookieInfo.ToUpper();
			long Pos3 = tempCookieInfo.Contains( "DOMAIN" );
			if (Pos3 > 0) {
				String domainInfo = cookieInfo.SubString(Pos3, cookieInfo.Length() - Pos3);
				Trace( "Domain found->" << domainInfo);

				long Pos4 = domainInfo.StrChr( '=' );
				Trace( "= found at " << Pos4 );

				if (Pos4 > 0) {
					Pos2 = domainInfo.StrChr( ';', Pos4 );
					Trace( "; found at " << Pos2);
					long posEndOfDomain = (Pos2 > Pos4) ? Pos2 : domainInfo.Length() + 1;
					long domainLen = posEndOfDomain - Pos4 - 1;
					explicitDomainName = domainInfo.SubString(Pos4 + 1, domainLen);
					Trace( "Explicit domain name set" << explicitDomainName );
					explicitDomainOrPortUsed = true;
				}
			}

			if (!explicitDomainOrPortUsed) {
				Trace( "No domain name set"  );
				ROAnything roaName;
				if ( ctx.Lookup("CurrentServer.ServerName", roaName) ) {
					explicitDomainName = roaName.AsString("");
					Trace( "Set Domain to->" << explicitDomainName);
					// If the cookie contains no domain, we assign all cookies independent for what port they are intended
					// to the same domain (localhost), because this is what we would expect to be done by the browser in this case.
				}
			}
			explicitDomainName.ToUpper();
			Anything tmpStore = ctx.GetTmpStore();
			tmpStore["Cookies"][explicitDomainName][cookieName] = cookieFull;
		}
	}
}

// resolve Jmp locations with a first pass, build a jmpTable, could be moved up into general flowcontrol functionality...
bool HttpFlowController::ResolveLabels(Anything &jmpTable, const ROAnything &runConfig, Anything &tmpStore)
{
	StartTrace(FlowController.DoPrepare);

	long sz = runConfig.GetSize(); // size of whole config for the run
	for (long i = 0; i < sz; i++) {
		ROAnything currentConfStep = runConfig[i];
		long resNr = currentConfStep.FindIndex("Label"); // only first Label in each step is checked!

		if ( resNr >= 0 ) {
			String labelName = currentConfStep[resNr].AsString("no label name");
			if ( jmpTable.IsDefined( labelName ) ) {
				// labels must be unique
				String eMsg = "";
				eMsg << "Label name <" << labelName << "> already used";
				HandleTheError( eMsg, tmpStore );
				return false;
			} else {
				jmpTable[labelName] = i;
			}
		}
	}
	return true;
}

long HttpFlowController::FindJumpNr(String &jmpLabel)
{
	StartTrace(HttpFlowController.FindJumpNr);
	TraceAny( fJmpTable, "the Jump Table" );
	Trace( "and this is the label to find:" << jmpLabel );
	for ( int i = 0; i < fJmpTable.GetSize(); i++ ) {
		Trace( "jmptable memberIndex is " << fJmpTable.SlotName(i) );
	}

	long res = fJmpTable.FindIndex( jmpLabel );

	if ( res >= 0 ) {
		return fJmpTable[res].AsLong(-1);
	}
	return -1;
}

void HttpFlowController::CheckFuzzyMatch(
	Anything &containerToCheck, // Input only
	String &strToLookFor  // Input and Output
)
{
	// checks slotnames of passed anything for the passed string,
	// down shift both strings...
	StartTrace(HttpFlowController.CheckFuzzyMatch);

	for (long j = 0; j < containerToCheck.GetSize(); j++ ) {
		String containerSlotname = String( containerToCheck.SlotName(j) );
		strToLookFor.ToLower();
		containerSlotname.ToLower();

		if ( containerSlotname.Contains( strToLookFor ) >= 0 ) {
			strToLookFor = containerToCheck[j].AsString("");
			j = containerToCheck.GetSize() + 2; // leave loop
			Trace( "CONTAINS INDIRECT MATCH FieldSlotName is->" << strToLookFor << " found in -" << containerSlotname );
		}
	} // for..
}

bool HttpFlowController::DoProcessLinksFormsAndFrames(Context &ctx)
{
	//--------------------------------------------------------------------------------
	// can only follow one link, so this routing is only called once.... if you define more than one form or link or frame bad luck!
	// Problem: Framework is modelled around HTTP possibilities. i.e. socket on which mappers and renderers
	// subsequently work is opened before mapping and rendering.
	// This means that rendering a link in a page can only influence the RequestURI and NOT the socket on which the
	// request is made. i.e. a GET command can not influence itself the socket on which it works, this decision
	// has already been made.
	// Fortunately with the Stresser we know in advance which Link or Form/Action will be used and can assemble required info
	// for socket creation in advance.
	// Other (better) solution would be to render to an internal buffer which is passed to the appropriate socket once rendering is
	// complete.
	//
	// if form or link specific in flow controller next step, then select the
	// Server and Protocol etc. from the link in question so that the correct sockect is opened prior to
	// mapping and rendering.
	// Set up tmpStore["CurrentServer"]["Server"] etc. from

	// place link or form info into CurrentServer anything
	// 1. for calls immeadiately after this method to ready socket
	// 2. to provide information for the rendering of the request made (via Mapper->Renderer) to this socket
	// init
	//--------------------------------------------------------------------------------

	StartTrace(HttpFlowController.DoProcessLinksFormsAndFrames);

	Anything resultAnything;
	Anything tmpStore = ctx.GetTmpStore();

	bool isMultipartContent = false;
	Anything boundary;

	if (tmpStore.LookupPath(resultAnything, "Form") ) {
		// FORM in config
		Anything formContentsConfig = resultAnything["formContents"];
		String formButtonPressed = resultAnything["buttonPressed"].AsString("");
		String formImageButtonPressed = resultAnything["imageButtonPressed"].AsString("");
		TraceAny(formContentsConfig, "<----Fields in CONFIG" );

		Anything formNameMapContents;
		if ( GetFormOrLinkInfo( "Form", "NameMap", ctx, formNameMapContents  ) && !formNameMapContents.IsNull()) {
			TraceAny(formNameMapContents, "<----NameMap in the incoming HTML Form" );
		}

		// Check if multipart content:
		//
		if ( tmpStore.LookupPath(boundary, "Form.boundarySeparated") ) {
			isMultipartContent = true;
		}

		Anything formContentsInitial;
		String multipartContents;
		// success is when retval is true AND given anything is NOT NULL
		// this is because I need to distinguish an error from a nonfound slot
		if ( GetFormOrLinkInfo( "Form", "Fields", ctx, formContentsInitial )) {
			if (!formContentsInitial.IsNull()) {
				TraceAny(formContentsInitial, "<----Fields in the incoming HTML Form" );
				Trace( "form contents config size is-" << formContentsConfig.GetSize() );
				// now add/merge in things from current Config anything
				for (long i = 0; i < formContentsConfig.GetSize(); i++ ) {
					// iterate through configuration
					String fieldSlotName = formContentsConfig.SlotName(i);
					String fieldContent;

					if ( !isMultipartContent ) {

						Renderer::RenderOnString(fieldContent, ctx, formContentsConfig[i]); // i.e. can contain nested lookup..

						Trace("field[" << i << "]:<" << fieldContent << ">");

						// check magic word
						if (fieldContent == "_DoNotSend_") {

							formContentsInitial.Remove(fieldSlotName);

						} else {
							//if label matches immediately then apply, if not then could be symbolic name, so check that, 1st absolutely and then just contains...
							if ( formContentsInitial.IsDefined( fieldSlotName ) ) {
								formContentsInitial[ fieldSlotName ] = fieldContent;
								Trace( "SIMPLE MATCH FieldSlotName is->" << fieldSlotName );
							} else {
								// not known, could be symbolic name - check the name map
								if ( formNameMapContents.IsDefined( fieldSlotName ) ) { //i.e. "Customer Number:"
									fieldSlotName = formNameMapContents[formNameMapContents.FindIndex(fieldSlotName)].AsString("");
									Trace( "SIMPLE INDIRECT MATCH FieldSlotName is->" << fieldSlotName );
								} else { // less strigent test, just tests whether slotname contains the string in
									String tmpResult1;
									// factor out later
									CheckFuzzyMatch( formNameMapContents, fieldSlotName );
								}

								if ( formContentsInitial.IsDefined( fieldSlotName ) ) {
									formContentsInitial[ fieldSlotName ] = fieldContent;
								} else {
									String eMsg = "";
									eMsg << "In configuration, field <" << fieldSlotName << "> unknown" ;
									HandleTheError(eMsg, tmpStore );
									return false;
								}
							}
						}
					} else {
						// Multipart for each field to generate:
						GenerateMultipartContent( fieldSlotName, formContentsConfig[i], boundary, multipartContents, ctx );
					}
				}
				if ( isMultipartContent ) {
					// Complete Multipart with last boundary:
					multipartContents << "--" << boundary.AsString("") << "--" << "\r\n";
				}
			}
		} else {
			// case of error
			return false;
		}

		Anything formButtonContents;
		// success is when retval is true AND given anything is NOT NULL
		// this is because I need to distinguish an error from a nonfound slot
		if ( GetFormOrLinkInfo( "Form", "Buttons", ctx, formButtonContents ) ) {
			if (!formButtonContents.IsNull()) {
				TraceAny(formButtonContents, "<----Buttons in the incoming HTML Form" );

				if ( ( formButtonPressed != "" ) && ( formButtonContents.IsDefined(formButtonPressed) ) ) {
					Trace("True ->formButtonContents.IsDefined(formButtonPressed)" );
					String tmpValue = formButtonContents[formButtonPressed].AsString("");

					if ( tmpValue.IsEqual("IMAGETYPE")) {
						String Xcoord = formButtonPressed;
						String Ycoord = formButtonPressed;
						Xcoord << ".x";
						Ycoord << ".y";
						formContentsInitial[Xcoord] = "10";
						formContentsInitial[Ycoord] = "10";
					} else {
						formContentsInitial[formButtonContents[formButtonPressed].AsString("")] = formButtonPressed;
					}
				} else if ( formButtonPressed == "" ) {
					// not an error...
				} else {
					Trace("False->formButtonContents.IsDefined(formButtonPressed)" );
					String eMsg = "";
					eMsg << "Button in configuration <" << formButtonPressed << "> not known in incoming page, first button taken as default";
					HandleTheError(eMsg, tmpStore );
////foo: should I return here ?
					if ( formButtonContents.GetSize() > 0 ) {
						// first button from input (only) added to request...
						formContentsInitial[formButtonContents[0L].AsString("")] = formButtonContents.SlotName(0L);
					} // else ... no buttons on this form!
				}
			}
		} else {
			// case of error
			return false;
		}

		Anything formImageButtonContents;
		// success is when retval is true AND given anything is NOT NULL
		// this is because I need to distinguish an error from a nonfound slot
		if ( GetFormOrLinkInfo( "Form", "ImageButtons", ctx, formImageButtonContents ) ) {
			if (!formImageButtonContents.IsNull()) {
				TraceAny(formImageButtonContents, "<----ImageButtons in the incoming HTML Form" );

				if ( ( formImageButtonPressed != "" ) && ( formImageButtonContents.IsDefined(formImageButtonPressed) ) ) {
					Trace("True ->formImageButtonContents.IsDefined(formImageButtonPressed)" );
					String Xcoord = formImageButtonPressed;
					String Ycoord = formImageButtonPressed;
					Xcoord << ".x";
					Ycoord << ".y";
					formContentsInitial[Xcoord] = "10";
					formContentsInitial[Ycoord] = "10";
				} else if ( formImageButtonPressed == "" ) {
					// not an error...
				} else {
					Trace("False->formImageButtonContents.IsDefined(formImageButtonPressed)" );
					String eMsg = "";
					eMsg << "ImageButton in configuration <" << formImageButtonPressed << "> not known in incoming page, no navigation possible";
					HandleTheError(eMsg, tmpStore );
				}
			}
		} else {
			// case of error
			return false;
		}

		TraceAny(formContentsInitial, "<----MERGED Fields from CONFIG and input FORM" );

		if ( !isMultipartContent ) {
			tmpStore["CurrentServer"]["formContents"] = Coast::URLUtils::EncodeFormContent( formContentsInitial );
		} else {
			tmpStore["CurrentServer"]["formContents"] = multipartContents;
		}

		// empty existing any
		resultAnything = Anything();
		// success is when retval is true AND given anything is NOT NULL
		// this is because I need to distinguish an error from a nonfound slot
		if ( GetFormOrLinkInfo( "Form", "method", ctx, resultAnything ) ) {
			if (!resultAnything.IsNull()) {
				Trace( "Form is requested" );
				tmpStore["CurrentServer"]["Method"] = resultAnything.AsString(""); // Method i.e. GET or POST etc.
				Trace( "Method found->" << resultAnything.AsString("") );
				// empty existing any
				resultAnything = Anything();
				// success is when retval is true AND given anything is NOT NULL
				// this is because I need to distinguish an error from a nonfound slot
				if ( GetFormOrLinkInfo( "Form", "action", ctx, resultAnything ) ) {
					if (!resultAnything.IsNull()) {
						String resultString = resultAnything.AsString(""); // action URI
						Trace ("Action URI is->" << resultString );
						Coast::URLUtils::HandleURI2( tmpStore["CurrentServer"], resultString, tmpStore["PreviousPage"]["BASE"].AsCharPtr("") );
						TraceAny( tmpStore["CurrentServer"], "<-Result of HandledURI" );
						// empty existing any
						resultAnything = Anything();
						// success is when retval is true AND given anything is NOT NULL
						// this is because I need to distinguish an error from a nonfound slot
						if ( GetFormOrLinkInfo( "Form", "enctype", ctx, resultAnything ) && !resultAnything.IsNull()) {
							// If boundary separation of form elements required for stresser
							// insert given content of slot "boundarySeparated":
							if (( resultAnything.AsString("").Contains("multipart/form-data") == 0 ) && tmpStore.LookupPath(boundary, "Form.boundarySeparated") ) {
								tmpStore["CurrentServer"]["Enctype"] = resultAnything.AsString("") << "; boundary=" << boundary.AsString("");
							}
							// Otherwise:
							else {
								tmpStore["CurrentServer"]["Enctype"] = resultAnything.AsString(""); // Method i.e. GET or POST etc.
							}
							Trace ("Encode type is ->" << resultAnything.AsString("") );
						} else {
							tmpStore["CurrentServer"]["Enctype"] = "application/x-www-form-urlencoded";
						}
					}
				} else {
					// case of error
					return false;
				}
			} else {
				// case of error
				return false;
			}
		} else {
			// case of error
			return false;
		}
		tmpStore.Remove("Form");
		return true;
	}

	// success is when retval is true AND given anything is NOT NULL
	// this is because I need to distinguish an error from a nonfound slot
	if ( GetFormOrLinkInfo( "Link", "href", ctx, resultAnything ) ) {
		if (!resultAnything.IsNull()) {
			// LINK
			Trace( "Link is requested" );
			tmpStore["CurrentServer"]["Method"] = "GET"; // Method i.e. GET or POST etc.
			Coast::URLUtils::HandleURI2( tmpStore["CurrentServer"], resultAnything.AsString(""), tmpStore["PreviousPage"]["BASE"].AsString("") );
			TraceAny( tmpStore["CurrentServer"], "<-Result of HandledURI" );
			tmpStore.Remove("Link");
			return true;
		}
	} else {
		// case of error
		return false;
	}

	// empty existing any
	resultAnything = Anything();
	// success is when retval is true AND given anything is NOT NULL
	// this is because I need to distinguish an error from a nonfound slot
	if ( GetFormOrLinkInfo( "Frame", "src", ctx, resultAnything ) ) {
		if (!resultAnything.IsNull()) {
			// FRAME
			Trace( "Frame is requested" );
			tmpStore["CurrentServer"]["Method"] = "GET"; // Method i.e. GET or POST etc.
			Coast::URLUtils::HandleURI2( tmpStore["CurrentServer"], resultAnything.AsString(""), tmpStore["PreviousPage"]["BASE"].AsString("") );
			TraceAny( tmpStore["CurrentServer"], "<-Result of HandledURI" );
			tmpStore.Remove("Frame");
			return true;
		}
	} else {
		// case of error
		return false;
	}
	return true;
}

// returns true if NO relocation of config step (jump) needed
bool HttpFlowController::DoProcessToken( Context &ctx, bool &boJump )
{
	StartTrace(HttpFlowController.DoProcessToken);
	bool res = true;
	boJump = false;
	bool noError = false;
	Anything tokenConfig;
	Anything tmpStore = ctx.GetTmpStore();
	TraceAny( tmpStore, "Current tmp store" );
	if (tmpStore.LookupPath(tokenConfig, "Token") ) { // Token config incoming
		//TOKEN - use same mechanism as for links etc. for now
		Trace( "Token is requested" );
		int mustBePresent = 0;
		String jumpName = "";
		long jumpNr = 0L;
		TraceAny( tokenConfig, "token config is" );

		// process incoming config step.
		for ( long i = 0; i < tokenConfig.GetSize(); i++ ) {
			SystemLog::WriteToStderr(".", 1);
			String stringToFind = "";
			jumpName = "";
			jumpNr = -1L; // init
			mustBePresent = 0;
			noError = false;

			Anything tokenDef = tokenConfig[i];
			TraceAny( tokenDef, "specific token def is" );

			for ( long j = 0; j < tokenDef.GetSize(); j++ ) {
				String theString = tokenDef.SlotName(j);
				theString.ToUpper();

				if ( theString == "MUST" ) {
					mustBePresent += 1;
					//stringToFind= tokenDef[j].AsString(); // string to scan for in page
					Renderer::RenderOnString(stringToFind, ctx, tokenDef[j]);
					stringToFind.ToLower();
				} else if ( theString == "MUSTNOT" ) {
					mustBePresent += 2;
					//stringToFind= tokenDef[j].AsString(); // string to scan for in page
					Renderer::RenderOnString(stringToFind, ctx, tokenDef[j]);
					stringToFind.ToLower();
				} else if ( theString == "NOERROR" ) {
					noError = true;
				} else if ( theString == "JUMP" ) {
					jumpName = tokenDef[j].AsString();
					Trace("Resolving next jump");
					if ((jumpNr = ResolveJumpNameToNr( jumpName, tmpStore )) < 0) {
						return false;
					}
					Trace("next jump to step:" << jumpNr);
				} else {
					String eMsg = "Unknown keyword <";
					eMsg << theString << "> in Token definition";
					HandleTheError(eMsg, tmpStore );
					return false;
				}
			}

			if ( mustBePresent < 0 || mustBePresent > 2 ) {
				String eMsg = "In Token definition, MUST or MUSTNOT must appear, but only one or the other";
				HandleTheError(eMsg, tmpStore );
				return false;
			}

			if ( ( tmpStore["Token"].IsDefined("String")  && tmpStore["Token"].IsDefined("slotName") ) ) {
				String eMsg = "Config error: Old Token definition, please update";
				HandleTheError(eMsg, tmpStore );
				return false;
			}

			Anything outputLookupResult;
			Trace("testing Mapper output for Tokens");
			if ( tmpStore.LookupPath(outputLookupResult, "Mapper.Output.Tokens" ) ) { // extract slot number from NameMap
				String stringToBeSearched = outputLookupResult.AsString("");
				stringToBeSearched.ToLower();
				Trace( "TOKEN " << stringToFind << " searched for in" << stringToBeSearched );
				if ( stringToBeSearched.Contains( stringToFind ) >= 0 ) {
					Trace( "\nTOKEN " << stringToFind << "found in " << stringToBeSearched );

					if ( mustBePresent != 1) {

						if ( !noError ) {
							String eMsg = "";
							eMsg << "Unwanted Token <" << stringToFind << "> DID occur in page:" << tmpStore["CurrentServer"]["ServerName"].AsString("Server/") << tmpStore["CurrentServer"]["Path"].AsString("Path") << "\r\r\nPage contained:" << stringToBeSearched;
							HandleTheError(eMsg, tmpStore );
							return false;
						}

						if (jumpNr >= 0 ) {
							long nextStep = tmpStore["FlowState"]["RequestNr"].AsLong(0);

							if (	( nextStep - 1 != jumpNr )		// jump to self not allowed
									&& ( nextStep != jumpNr   )	) {	// jump to next done anyway
								tmpStore["FlowState"]["RequestNr"] = jumpNr;	// effect the Jump
								boJump = true;
							}
						}
					}
				} else {
					Trace( "\nTOKEN " << stringToFind << " not found in " << stringToBeSearched );
					if ( mustBePresent == 1 ) {
						if ( !noError ) {
							String eMsg;
							eMsg << "Wanted Token <" << stringToFind << "> not found in page:" << tmpStore["CurrentServer"]["ServerName"].AsString("Server/") << tmpStore["CurrentServer"]["Path"].AsString("Path") << "\r\r\nPage contained:" << stringToBeSearched;;
							HandleTheError(eMsg, tmpStore );
							return false;
						}

						if (jumpNr >= 0 ) {
							long nextStep = tmpStore["FlowState"]["RequestNr"].AsLong(0);

							if (	( nextStep - 1 != jumpNr )		// jump to self not allowed
									&& ( nextStep != jumpNr   )	) { // jump to next done anyway
								tmpStore["FlowState"]["RequestNr"] = jumpNr;	// effect the Jump
								boJump = true;
							}
						}
					}
				}
			} else {
				String eMsg = "";
				eMsg << "No tokens present in page to be tested, server could be down?";
				HandleTheError(eMsg, tmpStore);
				return false;
			}
		} // end for
	} // if token check in config

	tmpStore.Remove("Token"); // from current Page config
	return res;
}

void HttpFlowController::HandleTheError(String &theMsg, Anything &tmpStore )
{
	String completedMsg = "";
	completedMsg << "In step:";
	completedMsg << tmpStore["FlowState"]["RequestNr"].AsLong(0);
	completedMsg << "<";

	String labelName = "";

	if (tmpStore.IsDefined("Label")) {
		labelName = tmpStore["Label"].AsString();
	} else {
		if (tmpStore["result"].IsDefined("ConfigStep")) {
			labelName = tmpStore["result"]["ConfigStep"].AsString("");
		} else {
			labelName = String("no label found");
		}
	}

	completedMsg << labelName;
	completedMsg << ">";

	completedMsg << theMsg;

	SystemLog::Error(completedMsg);
	tmpStore["result"]["ErrorMessage"].Append(completedMsg);
}

void HttpFlowController::DoCommand(Context &ctx, ROAnything commandConfig)
{
	StartTrace(HttpFlowController.DoCommand);

// Beware of security risk
	String cmd;
	Renderer::RenderOnString(cmd, ctx, commandConfig);

	long retVal =::system(cmd);
	if (retVal != 0) {
		String errorMsg = cmd;
		errorMsg << "; returned " << retVal;
		SystemLog::Error(errorMsg);
	}
}

bool HttpFlowController::PrepareRequest(Context &ctx)
{
	StartTrace(HttpFlowController.PrepareRequest);
	return FlowController::PrepareRequest(ctx);
}

// False returned when the last run is completed
bool HttpFlowController::PrepareRequest(Context &ctx, bool &bPrepareRequestSucceeded)
{
	StartTrace(HttpFlowController.PrepareRequest);
	Anything tmpStore = ctx.GetTmpStore();
	TraceAny(tmpStore, "tmp store" );
	TraceAny(fConfig, "fConfig" );

	ROAnything stepConfig = GetStepConfig(ctx);
////foo: testing with delay
	long lDelay = 0;

	// Establish a lookup hirarchy. TestConfig overwrites Config.
	if 	 ( stepConfig.IsDefined("DelayBetweenRequests") ) {
		lDelay = stepConfig["DelayBetweenRequests"].AsLong(0L);
	} else {
		if ( fConfig.IsDefined("DelayBetweenRequests") ) {
			lDelay = fConfig["DelayBetweenRequests"].AsLong(0L);
		}
	}

	if (lDelay) {
		{
			String strbuf;
			StringStream stream(strbuf);
			stream << "\nSuspending execution for " << lDelay / 1000L << " seconds before ";
			stepConfig["Label"].PrintOn(stream) << " gets executed.\n";
			stream.flush();
			SystemLog::WriteToStderr(strbuf);
		}
		SimpleMutex mtx("delay", Coast::Storage::Current());
		SimpleMutex::ConditionType cond;
		{
			LockUnlockEntry me(mtx);
			cond.TimedWait(mtx, lDelay / 1000L);
		}
		{
			String strbuf;
			StringStream stream(strbuf);
			stream << "\nResuming execution of    ";
			stepConfig["Label"].PrintOn(stream) << " now.\n";
			stream.flush();
			SystemLog::WriteToStderr(strbuf);
		}
	}
////foo:

	if ( stepConfig.IsDefined("DoCommand") ) {
		DoCommand(ctx, stepConfig["DoCommand"]);
	}

	if ( fJmpTableInit == false ) {
		fJmpTableInit = true;
		bPrepareRequestSucceeded = ResolveLabels( fJmpTable, fConfig["Run"], tmpStore );
		if (!bPrepareRequestSucceeded) {
			return true;
		}
	}

	if (tmpStore["Mapper"]["HTTPHeader"].IsDefined("set-cookie")) {
		// store cookie
		ProcessSetCookie( ctx );
	}
	// HandleErrors section---------------------------------------------------------------

	// the following could be set from HTTPDAImpl
	if (tmpStore["Mapper"].IsDefined("Error")) {
		String eMsg = "";
		eMsg  << tmpStore["Mapper"]["Error"].AsString("");
		HandleTheError(eMsg, tmpStore );
		bPrepareRequestSucceeded = false;
		return true;
	}

	// End HandleErrors section---------------------------------------------------------------

	// PreviousPage.Meta.refresh tested in following DoRelocate so must be set here
	// Here current page contents switched to PreviousPage. PreviousPage now new for testing and
	// Mapper is cleared ready for DataAccess to new page.
	tmpStore.Remove("PreviousPage");

	if (tmpStore["Mapper"].IsDefined("Output")) {
		tmpStore["PreviousPage" ] = tmpStore["Mapper"]["Output"];
	}
	tmpStore.Remove("Mapper");

	//__________________________________________________________________________________________________________________________
	// Get next config step: All the following steps can have a relocation of the "current" config step as a consequence.....
	// perhaps put in explicit method.
	//__________________________________________________________________________________________________________________________
	tmpStore["LastValidCurrentServer"] = tmpStore["CurrentServer"];
	TraceAny( tmpStore["LastValidCurrentServer"], "LastValidCurrentServer" );

	tmpStore.Remove("CurrentServer");
	// but save path across...
	tmpStore["CurrentServer"]["Path"] = tmpStore["LastValidCurrentServer"]["Path"];

	// get config for this "new" step, only after this step can you check for Token configs etc. in THIS config step
	if ( ! fDoRelocate ) {
		// call superclassto move to next step
		if (! FlowController::PrepareRequest(ctx) ) {
			return false;
		}

		// "current config now read in...
		TraceAny( tmpStore["CurrentServer"], "NEW ? ValidCurrentServer" );

		bool windowClosed = true;

		while ( windowClosed ) { // loop because step we move on to could also be outside window...

			if ( !SingleScheduling( ctx ) || !PeriodicalScheduling( ctx ) ) {
				// go to next config step....
				// clear out previously established config step configs
				tmpStore.Remove("Link");
				tmpStore.Remove("Frame");
				tmpStore.Remove("Form");
				tmpStore.Remove("Token");
				tmpStore.Remove("ClosePeriodically");
				tmpStore.Remove("CloseSingleTime");
				tmpStore.Remove("Label"); // actually ignored at run time...
				tmpStore["CurrentServer"] = tmpStore["LastValidCurrentServer"];

				// call superclassto move to jumped step
				if (! FlowController::PrepareRequest(ctx) ) {
					return false;
				}
			} else {
				windowClosed = false;
			}
		}
		TraceAny( tmpStore["CurrentServer"], "NEW - 2- ? ValidCurrentServer" );
	}
	//__________________________________________________________________________________________________________________________

	// config now present
	if (!tmpStore["CurrentServer"].IsDefined("Server")  && !tmpStore["CurrentServer"].IsDefined("ServerName") ) {
		// both missing, assume last server is to be used...
		tmpStore["CurrentServer"] = tmpStore["LastValidCurrentServer"];
		if ( !tmpStore["CurrentServer"].IsDefined("Server") ) {
			String eMsg = "Config error:No server or IP defined - exit";
			HandleTheError(eMsg, tmpStore );
			bPrepareRequestSucceeded = false;
			// return false here which causes the whole Run to finish
			return false;
		}
	}

	if (!tmpStore["CurrentServer"].IsDefined("Server") ) {
		if (tmpStore["CurrentServer"].IsDefined("ServerName") ) {
			tmpStore["CurrentServer"]["Server"] = Resolver::DNS2IPAddress( tmpStore["CurrentServer"]["ServerName"].AsString("") ) ;
		}
	} else if (!tmpStore["CurrentServer"].IsDefined("ServerName") ) {
		if (tmpStore["CurrentServer"].IsDefined("Server") ) {
			tmpStore["CurrentServer"]["ServerName"] = Resolver::IPAddress2DNS( tmpStore["CurrentServer"]["Server"].AsString("") ) ;
		}
	}
	Trace("Server: " << tmpStore["CurrentServer"]["Server"].AsString());
	Trace("ServerName: " << tmpStore["CurrentServer"]["ServerName"].AsString());
	// remove slots which are  only valid one time inside CurrentServer
	tmpStore["CurrentServer"].Remove("formContents");

	TraceAny(tmpStore, "<----tmp store is" );
	if ( ctx.Lookup("IsAbsPath", 0L) && !fDoRelocate ) {
		ROAnything rendererSpec;
		ctx.Lookup("AbsPath", rendererSpec);
		String absPath = Renderer::RenderToString(ctx, rendererSpec);
		if (absPath.Length() == 0L) {
			absPath.Append("/");
		}
		Trace("absPath: " << absPath);
		Coast::URLUtils::HandleURI2( tmpStore["CurrentServer"], absPath, tmpStore["PreviousPage"]["BASE"].AsCharPtr("") );
	} else {
		bPrepareRequestSucceeded = DoProcessLinksFormsAndFrames(ctx); // no relocations expected here
	}
	if ( tmpStore["CurrentServer"]["UseSSL"].AsLong(0) == 1L ) {
		PrepareConnector(ctx);
	}
	tmpStore.Remove("DataAccess");
	Trace("PrepareRequestSucceeded: " << (bPrepareRequestSucceeded ? "true" : "false"));
	return true;
}

void HttpFlowController::SetupSSLCtx(Anything &sslModuleCfg, Context &ctx)
{
	StartTrace(HttpFlowController.SetupSSLCtx);
	sslModuleCfg["SSLVerifyPeerCert"]			= 0L;
	sslModuleCfg["SSLVerifyFailIfNoPeerCert"]	= 0L;
	sslModuleCfg["SSLUseAppCallback"] 			= 0L;
	sslModuleCfg["SSLVerifyDepth"] 				= 0L;
	sslModuleCfg["SSLPeerCAFile"] 				= "";
	sslModuleCfg["SSLVerifyPath"] 				= "";
	sslModuleCfg["KeyFileClient"] 				= "";
	sslModuleCfg["CertFileClient"] 				= "";
	sslModuleCfg["NoCertAndPrivateKey"]			= 1L;
	sslModuleCfg["SSLClientSessionTimeout"]		= 86400L;	// one day
}

void HttpFlowController::PrepareConnector(Context &ctx)
{
	StartTrace(HttpFlowController.PrepareConnector);
	// clear content
	Anything toPush;
	toPush["CurrentServer"]["UseThreadLocalMemory"] = 1L;
	if ( ctx.Lookup("CurrentServer.UseSSL", 0L) == 1L ) {
		Anything sslModuleCfg;
		SetupSSLCtx(sslModuleCfg, ctx);
		toPush["SSLModuleCfg"] 				= sslModuleCfg;
		toPush["VerifyCertifiedEntity"] 	= 0L;
		toPush["CertVerifyString"]			= "";
		toPush["CertVerifyStringIsFilter"]  = 0L;
		toPush["SessionResumption"] 		= 1L;
	}
	String name("SSLData");
	// The information pushed may be needed in "follow up" requests triggered by redirects.
	// Because the TmpStore is lost after FlowController has handled one request, we must not
	// call PopStore.
	ctx.Push(name, toPush);
}

bool HttpFlowController::AnalyseReply(Context &ctx)
{
	StartTrace(HttpFlowController.AnalyseReply);
	// for debugging MIKE
	Anything tmpStore = ctx.GetTmpStore();
	bool boRet = true;
	TraceAny(tmpStore, "TempStore");

	// check if the request was OK
	long respCode = tmpStore["Mapper"]["HTTPResponse"]["Code"].AsLong(0);
	String respMsg = tmpStore["Mapper"]["HTTPResponse"]["Msg"].AsString("no message, possible timeout");

	if ((respCode != 200L) && (respCode != 302L)) {
		String eMsg = "";
		eMsg << "HTTP Response Code from Server is not 200 or 302 but <" << respCode <<
			 ">; Message = " << respMsg;

		if (tmpStore["Mapper"].IsDefined("RequestMade")) { // only happens in debug version
			eMsg << " In reply to request " << tmpStore["Mapper"]["RequestMade"].AsString("") << " NOTE:30x relocation requests currently not handled";
		}
		HandleTheError(eMsg, tmpStore );
		boRet = false;
	}
	tmpStore["Mapper"].Remove("RequestMade");

	fDoRelocate = DoRelocate(ctx);
	if ( boRet && !fDoRelocate ) {
		bool boJump;
		boRet = DoProcessToken(ctx, boJump);
		if ( boJump ) {
			; // possible config step jump as result of token test...
			tmpStore.Remove("Link");
			tmpStore.Remove("Frame");
			tmpStore.Remove("Form");
			tmpStore.Remove("Token");
			tmpStore.Remove("ClosePeriodically");
			tmpStore.Remove("CloseSingleTime");
			tmpStore.Remove("Label"); // actually ignored at run time...
			tmpStore["CurrentServer"] = tmpStore["LastValidCurrentServer"];
			tmpStore.Remove("IsAbsPath");
		}
	}
	Trace( "DoRelocate is: " << fDoRelocate);
	Trace( "AnalyseReply will return <" << boRet << "> - False is:" << false );

	tmpStore["CurrentServer"].Remove("MsgBody");
	tmpStore["CurrentServer"].Remove("Enctype");

	return boRet;
}

bool HttpFlowController::DoRelocate(Context &ctx)
{
	StartTrace(HttpFlowController.DoRelocate);
	return DoMetaRefreshRelocate(ctx)  || DoLocationRelocate(ctx);
}

bool HttpFlowController::DoMetaRefreshRelocate(Context &ctx)
{
	StartTrace(HttpFlowController.DoMetaRefreshRelocate);

	Anything tmpStore = ctx.GetTmpStore();
	Anything refresh;

	if (tmpStore.LookupPath(refresh, "Mapper.Output.Meta.refresh") ) {
		tmpStore["CurrentServer"]["Method"] = "GET";
		// refresh, relocation possible
		// If refresh meta then see if page different and relocate by ----> Not Checked yet !!!!
		// 1. setting /Server with  gethostbyname call on domain part
		// 2. setting /Port to port if explicitly given, otherwise 80 for HTTP and ?? for HTTPS
		// 3. and of course set path to navigate to on this server by
		//    1. setting LinkNr to <special value - negative perhaps?> <-- NO USE SPECIAL RENDERER
		//    2. rendering of Link for this value - array member <this value> IS the path.
		String myString1 = refresh.AsString("");
		String myString2 = "";
		String myServerStr = "";

		// input is
		//    /meta {
		//		  /refresh "0; URL=https://localhost:2023/fda?X=b64:YnMwOoNvmvJfRaTWrifcPagQcuY95OI9FlOA-$bIM77lo2Bpi2FI9b6u13T51CZSH$-kqY6n$uS8m4KDGd4yAcjrISlx8gNV8o7HyaK5$RoW8E2vUGcXdfKRkwZmyxQ5WPPBknlPONT5MQim"
		//	afterwards }
		//	/Server is IP of "localhost"
		//	/Port is 2023
		//	/"link34" {
		//		/href "fda?X=b64:YnMwOoNvmvJfRaTWrifcPagQcuY95OI9FlOA-$bIM77lo2Bpi2FI9b6u13T51CZSH$-kqY6n$uS8m4KDGd4yAcjrISlx8gNV8o7HyaK5$RoW8E2vUGcXdfKRkwZmyxQ5WPPBknlPONT5MQim"
		//	  }

		Trace( "contents are" << myString1 );
		long Pos = myString1.Contains( "URL" );
		Trace( "pos url is->" << Pos );

		if (Pos > 0 ) {
			//<META HTTP-EQUIV="Refresh" CONTENT="10; URL=sons/petitetoune.wav">
			// but Marcel/FT also deliver
			//<META HTTP-EQUIV="Refresh" CONTENT="10; URL='sons/petitetoune.wav'">

			myString2 = myString1.SubString(Pos + 4, myString1.Length() - Pos - 4); // 4 = "URL="
			Trace( "String 1 contents are" << myString2 );

			Coast::URLUtils::HandleURI2( tmpStore["CurrentServer"], myString2 );
			TraceAny( tmpStore["CurrentServer"], "<-Result of HandledURI" );
			return true;
		}
	}
	return false;
}

bool HttpFlowController::DoLocationRelocate(Context &ctx)
{
	StartTrace(HttpFlowController.DoLocationRelocate);

	Anything tmpStore = ctx.GetTmpStore();
	Anything refresh;

	// HTTP Response code must be 302
	// Location header contains the whole refresh path
	if ( (tmpStore["Mapper"]["HTTPResponse"]["Code"].AsLong(0L) == 302L ) &&
		 (tmpStore.LookupPath(refresh, "Mapper.HTTPHeader.location")) ) {
		{
			String anyRes = "";
			OStringStream os(&anyRes);
			os << tmpStore["CurrentServer"];
		}
		tmpStore["CurrentServer"]["Method"] = "GET";
		String refreshURL = refresh.AsString();
		Trace("Location header is: " << refreshURL);
		if (refreshURL.Length() > 0 ) {
			Coast::URLUtils::HandleURI2( tmpStore["CurrentServer"], refreshURL );
			TraceAny( tmpStore["CurrentServer"], "<-Result of HandledURI" );
			return true;
		}
	}
	return false;
}

// success is when retval is true AND result anything is NOT NULL
// this is because I need to distinguish an error from a nonfound slot
bool HttpFlowController::GetFormOrLinkInfo( const String &aTag, const String &aSlotname, Context &ctx, Anything &aResult )
{
	// called for example:
	// GetFormOrLinkInfo( "Form", "Buttons", ctx, formButtonContents )
	// looks for "Form" in input CONFIG, then checks to see whether one at least of "slotname" or "slotnr" are defined in this config.
	// It the finds this stipulated Form in the incoming HTML page.  This is done by physically building up are reference to the expected element
	// i.e. building the string "PreviousPage.Forms.<form name>" or "PreviousPage.Forms.<form number>" and once this object has been found
	// then specified parameter aSlotName is searched for and returned.
	// Thus the above call would extract the Buttons Anything from the Form[number specified in config] revealing which buttons the incoming form has.
	// This anything is delivered as aResult.

	StartTrace(HttpFlowController.GetFormOrLinkInfo);
	Trace( "Tag-> " << aTag << " Slotname->" << aSlotname );
	// brings together current form from server and config of current step or link of course
	bool tagFound = true;	// initialized to true because only errors should return false, nonfound slot is signaled by Null-Any
	Anything configForLinkFrameOrFormEtc;
	String SlotNrStr;
	String SlotNameStr;

	Anything myTmpStore = ctx.GetTmpStore();
	// from config slot, array known as Links or Forms etc.

	if (myTmpStore.LookupPath(configForLinkFrameOrFormEtc, aTag) ) {
		// Form, link or frame found, a Post has the form:
		// name=Gisle&email=gisle%40aas.no&gender=m&born=1964&trust=3%25
		// Post as multipart form data later!?
		TraceAny( configForLinkFrameOrFormEtc, "<-current Link/Form/Frame config is" );

		// from Page, array known as Links or Forms etc.
		SlotNrStr = "PreviousPage.";
		SlotNrStr << aTag << "s.";
		SlotNameStr = "PreviousPage.";
		SlotNameStr << aTag << "sNameMap.";

		String ActualSlotNrStr = "";
		bool legal = false;
		Anything outputLookupResult;

		// in incoming HTML which form/link/frame are we dealing with? find info in config...
		if (configForLinkFrameOrFormEtc.IsDefined( "slotName" ) ) {
			SlotNameStr << configForLinkFrameOrFormEtc["slotName"].AsString(""); //e.g."PreviousPage.LinksNameMap.FinXS"

			Trace("Looking up: " <<  SlotNameStr);
			Trace("Lookup was successful: " <<  (myTmpStore.LookupPath(outputLookupResult, SlotNameStr) ? "yes" : "no"));

			if ( myTmpStore.LookupPath(outputLookupResult, SlotNameStr ) ) { // extract slot number from NameMap
				ActualSlotNrStr = outputLookupResult.AsString(""); // should be single slot anything (?)
				Trace( "SLOT NUMBER " << ActualSlotNrStr << " found for " << SlotNameStr );
				legal = true;
			} else {
				String eMsg = "";
				eMsg << "SlotName " << SlotNameStr << " not present in incoming forms/links/frames";
				HandleTheError(eMsg, myTmpStore);
				return false;
			}
		}

		if ( ActualSlotNrStr != "" ) {
			SlotNrStr << ActualSlotNrStr ; //e.g. "PreviousPage.Links.0" -
		} else {
			if ( configForLinkFrameOrFormEtc.IsDefined( "slotNr" ) ) {
				SlotNrStr << configForLinkFrameOrFormEtc["slotNr"].AsString("0"); //e.g. "PreviousPage.Links.0" - Note 0 is default
				legal = true;
			}
		}

		if ( !legal ) {
			String eMsg = "SlotName or slotNr required in config of Frame/Form/Link but not found";
			HandleTheError( eMsg, myTmpStore );
			return false;
		}

		Trace( "Looking for named/numbered MATCHING form/link/frame number " << SlotNrStr );
		if (myTmpStore.LookupPath(outputLookupResult, SlotNrStr ) ) {
			TraceAny( outputLookupResult, "<- form/link found for formNr" );

			aResult = outputLookupResult[aSlotname]; // .AsString(""); // Method
			tagFound = true;
		} else {
			String eMsg = "";
			eMsg << "Link/Form/Frame with name " << SlotNameStr << " or with number " << SlotNrStr << " not found in incoming forms/links/frames";
			HandleTheError(eMsg, myTmpStore );
			return false;
		}
	}
	return tagFound;
}

bool HttpFlowController::GenericScheduling( Anything &tmpStore, Anything &scheduling, String &currentTimeDate, funcPtr conversionFunction )
{
	// interpret schedule statement...
	StartTrace(HttpFlowController.GenericScheduling);
	bool res = true;

	int mustBePresent = 0;
	String	from, tmpFrom, till, tmpTill;
	String jumpName = "";
	long jumpNr = 0L;

	// process incoming config step.
	for ( long i = 0; i < scheduling.GetSize(); i++ ) {
		jumpName = "";
		jumpNr = -1L; // init
		mustBePresent = 0;

		Anything schedulingDef = scheduling[i];
		TraceAny( schedulingDef, "specific token def is" );

		for ( long j = 0; j < schedulingDef.GetSize(); j++ ) {
			String theString = schedulingDef.SlotName(j);
			theString.ToUpper();

			if ( theString == "FROM" ) {
				mustBePresent += 1;
				tmpFrom = schedulingDef[j].AsString(); // string to scan for in page
				tmpFrom.ToUpper();
			} else if ( theString == "TILL" ) {
				mustBePresent += 2;
				tmpTill = schedulingDef[j].AsString(); // string to scan for in page
				tmpTill.ToUpper();
			} else if ( theString == "JUMP" ) {
				jumpName = schedulingDef[j].AsString();
			} else {
				String eMsg = "Unknown keyword <";
				eMsg << theString << "> in Schedule definition";
				HandleTheError(eMsg, tmpStore );
			}
		}

		if ( mustBePresent != 3) {
			String eMsg = "In Schedule definition, FROM and TILL must both appear";
			HandleTheError(eMsg, tmpStore );
			return res;
		}

		from = conversionFunction( tmpFrom );
		till = conversionFunction( tmpTill );

		if ((jumpNr = ResolveJumpNameToNr( jumpName, tmpStore )) < 0) {
			return false;
		}

		SystemLog::WriteToStderr(String("From:") << from << " Till:" << till << " currentTimeDate:" << currentTimeDate << "\n");
		SystemLog::WriteToStderr(String("currentTimeDate.Compare(from):") << (long)currentTimeDate.Compare(from) <<  "currentTimeDate.Compare(till):" << (long)currentTimeDate.Compare(till) << "\n");

		if (	( from.Length() > 0  )
				&&	( till.Length() > 0  )
				&&	( currentTimeDate.Length() > 0 )
				&&	( currentTimeDate.Compare(from) >= 0 )
				&&	( currentTimeDate.Compare(till) <= 0 ) ) {
			// Signal a period where no test is required
			CheckDoJump( jumpNr, tmpStore );
			return false;
		}
	}
	return res;
}

bool HttpFlowController::SingleScheduling( Context &ctx )
{
	StartTrace(HttpFlowController.SingleScheduling);

	Anything tmpStore(ctx.GetTmpStore());
	Anything scheduling;
	bool res = true;

	if ( !tmpStore.LookupPath( scheduling, "CloseSingleTime") ) {
		// no CloseSingleTime present
		return res;
	};

	String currTimeDate = Scheduler::CurrTimeDateInDecimal();
	return GenericScheduling( tmpStore, scheduling, currTimeDate, Scheduler::ConvertFromSingleTimeInDecimal  );
}
// SingleTimeScheduling

bool HttpFlowController::PeriodicalScheduling( Context &ctx )
{
	StartTrace(HttpFlowController.PeriodicalScheduling);
	Anything tmpStore(ctx.GetTmpStore());
	Anything scheduling;
	bool res = true;

	if ( !tmpStore.LookupPath( scheduling, "ClosePeriodically") ) {
		// no periodically present
		return res;
	};

	String	today;
	// prepare the current time/date in DayHourMin-Format
	today = Scheduler::currTimeInDayHourMin();
	// convert the current time/date from DayHourMin-Format into Decimal-Format
	today = Scheduler::convertFromDayHourMinInDecimal( today );

	return GenericScheduling( tmpStore, scheduling, today, Scheduler::convertFromDayHourMinInDecimal );
}
// Periodical scheduling

long HttpFlowController::ResolveJumpNameToNr(String &jumpName, Anything &tmpStore )
{
	long jumpNr = -1;

	if ( jumpName != "" ) { // jmp specified
		jumpNr = FindJumpNr(jumpName);

		if ( jumpNr < 0 ) {
			String eMsg = "Config error: In Token definition, the jump name <";
			eMsg << jumpName << "> is not known";
			HandleTheError(eMsg, tmpStore );
		}
	}
	return jumpNr;
}

void HttpFlowController::CheckDoJump(long jumpNr, Anything &tmpStore )
{

	if (jumpNr >= 0 ) {
		long nextStep = tmpStore["FlowState"]["RequestNr"].AsLong(0);

		if (	( nextStep - 1 != jumpNr )		// jump to self not allowed
				&& ( nextStep != jumpNr   )	) {	// jump to next done anyway
			tmpStore["FlowState"]["RequestNr"] = jumpNr;	// effect the Jump
		}
	}
}

void HttpFlowController::GenerateMultipartContent( String &fieldName, ROAnything fieldConfig, ROAnything boundary, String &multipartContents, Context &ctx )
{
	StartTrace(HttpFlowController.GenerateMultipartContent);

	//
	// Based on RFC 1806, RFC 1867:
	//
	String subHeaderString, fieldPartContent, fieldFilename, fieldContent;
	String slotForFile = "Filename";

	// Content-Disposition & name:
	{
		subHeaderString << "--" << boundary.AsString("") << "\r\n";
		subHeaderString << "Content-Disposition: form-data; name=\"" << fieldName << "\"";
	}

	// Filename:
	if ( fieldConfig.IsDefined( slotForFile ) ) {
		// Slot Filename:
		fieldFilename = fieldConfig[ slotForFile ].AsString("");
		subHeaderString << "; filename=" << fieldFilename;
	} else {
	}
	subHeaderString << "\r\n";

	// Content-Type default "text/plain" (other types can be "config'ed" later):
	{
		subHeaderString << "Content-Type: text/plain" << "\r\n\r\n";
	}

	// Content-Body:
	if ( fieldConfig.IsDefined( slotForFile ) ) {
		// Get File content with given Filename for this body:
		std::iostream *pS = Coast::System::OpenIStream( fieldFilename, "", std::ios::in, true ); // path will be resolved..
		if (pS) {
			int c;
			while ( (c = pS->get() ) != EOF ) {
				fieldPartContent.Append( (char)c );
			}
			delete pS;
		} else {
			Trace ( "Could'nt open file with :" << fieldFilename );
		}
	} else {
		Renderer::RenderOnString(fieldPartContent, ctx, fieldConfig);
	}

	// Ending (for this part/field):
	{
		fieldContent << subHeaderString << fieldPartContent << "\r\n\r\n";
	}

	multipartContents << fieldContent;
}

void HttpFlowController::DoCleanupAfterStep(Context &ctx, ROAnything roaStepConfig)
{
	StartTrace(FlowController.DoCleanupAfterStep);
	Anything tmpStore(ctx.GetTmpStore());
	SubTraceAny(TmpStore, ctx.GetTmpStore(), "TmpStore before");
	Trace("fDoRelocate: " << fDoRelocate);
	// If HTTPFlowController determines that a relocate is needed, we have to preserve
	// this information for the (inserted) relocate step. Since not all FlowControllers
	// CurrentServer must be kept in all cases, because we need to know a Server IP/Port to
	// go ahead with the tests after a relocate since we don't know to what Server IP/Port a server
	// redirects us.
	if ( !roaStepConfig.IsNull() && fDoRelocate == false ) {
		long sz = roaStepConfig.GetSize();
		for (long i = 0; i < sz; i++) {
			String slotName = roaStepConfig.SlotName(i);
			if ( slotName && slotName != "CurrentServer" ) {
				Trace("Slotname to remove : " << slotName);
				tmpStore.Remove(slotName);
			}
		}
	}
	SubTraceAny(TmpStore, tmpStore, "TmpStore after");
}
