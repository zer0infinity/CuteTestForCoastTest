"""SConstruct

Main build driver for coast build. It specifies additional tools used to
build the coast framework.

"""
# vim: set et ai ts=4 sw=4:
# -------------------------------------------------------------------------
# Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software
# at HSR Rapperswil, Switzerland
# All rights reserved.
#
# This library/application is free software; you can redistribute and/or
# modify it under the terms of the license that is included with this
# library/application in the file license.txt.
# -------------------------------------------------------------------------
import pkg_resources
pkg_resources.require(["SConsider"])

import SCons
import os

# enable using our own tools from the following directory if it exists
_project_tool_dir = os.path.join(os.getcwd(),'sconsider_extensions')
if os.path.exists(_project_tool_dir):
    SCons.Tool.DefaultToolpath.append(_project_tool_dir)

_project_tools = ["coast_options",
                  "ThirdParty",
                  "DoxygenBuilder",
                  "Package",]

_sconsider_dist = pkg_resources.get_distribution('SConsider').parsed_version
if _sconsider_dist >= pkg_resources.parse_version("0.5.dev"):
    # FIXME: CertificateBuilders tool should potentially be added where we
    #        really need it (SSL, StdDataAccess)
    _project_tools.extend(["CertificateBuilders"])

# extend SConsider default tools by this list
# -> in effect for SConsider versions > 0.3.5
DefaultEnvironment(_SCONSIDER_TOOLS_=_project_tools)

# initialize the SConsider extension
import SConsider

