/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ImageRenderers_H
#define _ImageRenderers_H

#include "Renderer.h"

//---- ImageLocationRenderer -------------------------------------------------------
//! Renders the full filename of an image
/*!
\par Configuration
\code
{
	/ImageName		Rendererspec	mandatory, filename of the image
	/ImagePath		Rendererspec	optional, default is to lookup ImagePath in the context and if not defined "./" will be taken
}
\endcode
ImageLocationRenderer renders the full filename of an image. The filename is devided in two sections:
-# a path - which designates a subdirectory of the HTTP servers document folder
-# the actual filename of the image
both parts are concatenated by the renderer to form the full filename
*/
class ImageLocationRenderer : public Renderer
{
public:
	ImageLocationRenderer(const char *name);

	void RenderAll(std::ostream &reply, Context &c, const ROAnything &data);

protected:
	bool RenderPathAndFilename(std::ostream &reply, Context &c, const ROAnything &config);
};

//---- ImageRenderer -------------------------------------------------------
//! Renders an HTML image tag
/*!
\par Configuration
\code
{
	/ImageName		Rendererspec	mandatory, filename of the image
	/ImagePath		Rendererspec	optional, default is to lookup ImagePath in the context and if not defined "./" will be taken
	/Options {		Anything		optional, HTML options
		/Key		Value
		...
	}
}
\endcode
or
\code
{
	"image name" 			creates: \<IMG SRC="ImagePath/image name"\>
}
\endcode

The ImageRenderer renders an HTML \<IMG\> tag into the reply (including different HTML options). (The ImageLocationRenderer is used to render the filename of the image.)
All HTML image options may be specified at slot /Options, e.g.:
-# /DYNSRC "..."\n
 Specifies the URL of a video clip or VRML world.\n
 An image can also be specified first using SRC= to cover for browsers that do not support videos. (IE)
-# /CONTROLS\n
 Adds a set of controls under the video clip. (IE)
-# /LOOP "n"\n
 For video clips, specifies the number of times to loop the clip.\n
 A value of "-1" or "INFINITE" makes it loop indefinitely. (IE)
-# /START "..."\n
 Specifies when the video clip should start playing.\n
 Possible values are "FILEOPEN" (default), "MOUSEOVER", or both.(IE)
-# /USEMAP "#map1"\n
 Tells the browser that the image is a client-side clickable image map defined under the name "map1".
-# /ISMAP\n
 Tells the browser that the image is a server-side clickable image map.
-# /ALT "..."\n
 Specifies a text string to be displayed on browsers that do not support inline images.
-# /BORDER "..."\n
 Specifies the width of the border drawn around the image.\n
 If BORDER is set to "0", there will be no border even around pictures that are links.
-# /LOWSRC "..."\n
 Specifies the URL of an image to be loaded first, before the image specified in SRC is loaded. LOWSRC usually reefers to a smaller image.
-# /ALIGN "..."\n
 Specifies the alignment of the image.\n
 Values:
	-# RIGHT or LEFT--Aligns the image to the specified side of the page, and all text is wrapped around the image.
	-# TOP, MIDDLE, BOTTOM, TEXTTOP, ABSMIDDLE, BASELINE, and ABSBOTTOM\n
	 Specifies the vertical alignment of the image with other items on the same line.
-# /VSPACE "..."\n
 Specifies the space left between the edge of the image and the items above or below it.
-# /HSPACE "..."\n
 Specifies the space left between the edge of the image and the items to the left or right of it.
-# /WIDTH "..."\n
 Specifies the width of the image. If the width is not the actual width, the image is scaled to fit.
-# /HEIGHT "..."\n
 Same as above ,except it specifies the height of the image.

PS: may be we should add more useful support for some of these kinds of IMG tag attributes (e.g. lowsrc, alignment, etc.)
PS: Language specific Images are needed. Learn from Applications and provide more versatile Image renderers (?)
*/
class ImageRenderer : public ImageLocationRenderer
{
public:
	ImageRenderer(const char *name);

	void RenderAll(std::ostream &reply, Context &c, const ROAnything &data);
};

//---- FigureRenderer ---------------------------------------------------------------
//! Renders an HTML image with a caption below using a surrounding table
/*!
\par Configuration
\code
{
	/ImageName		Rendererspec	mandatory, filename of the image
	/ImagePath		Rendererspec	optional, default is to lookup ImagePath in the context and if not defined "./" will be taken
	/Caption		Rendererspec	optional, text to be displayed below the image
	/Options {		Anything		optional, HTML options
		/Key		Value
		...
	}
}
\endcode

Renders an image with a caption (an HTML table is used instead of the regular HTML CAPTION feature which does not perform well on all browsers)
The renderer takes the same parameters as an ImageRenderer - i.e. 'ImagePath', 'ImageName' and 'Options' (eventhough all 'Options' might not have the desired results, since the image is enclosed by a table).
In addition the text to be displayed below the image is taken from the 'Caption' slot.
If 'Options' are not supplied explicitly, a border is drawn around the image as a default.
The caption-text will be used as ALT text for the image.
*/
class FigureRenderer : public ImageRenderer
{
public:
	FigureRenderer(const char *name);
	void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif		//not defined _ImageRenderers_H
