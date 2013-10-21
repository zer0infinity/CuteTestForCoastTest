/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPHeaderFieldParameterMapper_H
#define _HTTPHeaderFieldParameterMapper_H

#include "Mapper.h"

namespace coast {
	namespace http {
		//! HTTP header field parameter mapping
		/*!
		 * This mapper will use DoGetAny(key,...) to get header field values and to either
		 * place them onto the stream or into the given value Anything. The name of the header
		 * field must be the last part of the given key. ParameterMapper::getDelim() will be used
		 * to cut away the prefix part before creating a valid <b><tt>headerkey: value\\r\\n</tt></b>
		 * pair.
		 * @section httphfmexample Example
		 * @subsection httphfmexampleconfig Configuration (InputMapperMeta.any)
		 * \code
		/MyMapperAlias {
			/header {
				/USER-AGENT {
					/RendererMapper {
						/SubStringRenderer {
							/String	{ /ContextLookupRenderer header.USER-AGENT }
							/Length 7
						}
					}
				}
			}
		}
		 * \endcode
		 * @subsection httphfmexampletmpstore Prepared content of TmpStore
		 * Assume the configured DataAccess will issue a ParameterMapper::Get("Header", ...)
		 * \code
		/TmpStore {
			/Header {
				/HOST "my.host.dom"
				/USER-AGENT "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.6) Gecko/20100629 Ubuntu/10.10 (maverick) Firefox/3.6.6 GTB7.0"
				/ACCEPT {
				  "text/html"
				  "application/xhtml+xml"
				  "application/xml;q=0.9"
				}
			}
		}
		 * \endcode
		 * @subsection httphfmexampleoutput Returned content on stream
		 * \code
		USER-AGENT: Mozilla\r\n
		 * \endcode
		 * \see HTTPHeaderFieldParameterMapperTest.any for more examples
		 */
		class HTTPHeaderFieldParameterMapper: public ParameterMapper {
			HTTPHeaderFieldParameterMapper();
			HTTPHeaderFieldParameterMapper(const HTTPHeaderFieldParameterMapper &);
			HTTPHeaderFieldParameterMapper &operator=(const HTTPHeaderFieldParameterMapper &);
		public:
			HTTPHeaderFieldParameterMapper(const char *name) :
					ParameterMapper(name) {
			}

			/*! @copydoc IFAObject::Clone(Allocator *) */
			IFAObject *Clone(Allocator *a) const {
				return new (a) HTTPHeaderFieldParameterMapper(fName);
			}
		protected:
			//! Render http header field, look'd up using Get(key,...) and rendered onto os
			/*! @copydoc ParameterMapper::DoGetStream(const char *, std::ostream &, Context &, ROAnything) */
			virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything conf);

			//! Render http header field, look'd up using Get(key,...) and placed into value
			/*! @copydoc ParameterMapper::DoGetAny(const char *, Anything &, Context &, ROAnything) */
			virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything info);
		};
	}
}

#endif
