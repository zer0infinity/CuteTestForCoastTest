"""coast_options.

Coast-SConsider-specific command line extensions to influence the build

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

from SCons.Script import AddOption, Dir, GetOption
from logging import getLogger
logger = getLogger(__name__)

added = None


def generate(env, **kw):
    global added
    if not added:
        added = 1
        AddOption(
            '--enable-Trace',
            dest='Trace',
            action='store_true',
            help='Enable trace support by defining COAST_TRACE, (StartTrace,\
 Trace,...), see Tracer.h for details')
        import socket
        envconfigdir_default = socket.gethostname()
        AddOption(
            '--env-cfg',
            dest='envconfigdir',
            action='store',
            nargs=1,
            type='string',
            default=envconfigdir_default,
            help='Define directory name to use for location dependent files,\
 default [' + envconfigdir_default + ']. When a config file gets copied and\
 a corresponding file exists below this directory, it will get used instead\
 of the original one. This allows to define configuration settings appropriate\
 for the current environment.')

    buildflags = []
    buildmode = GetOption('buildcfg')
    if buildmode == 'optimized':
        buildflags.append('OPT')
    elif buildmode == 'debug':
        buildflags.append('DBG')
    else:
        buildflags.append('PROFILE')

    if GetOption('Trace'):
        env.AppendUnique(VARIANT_SUFFIX=['_trace'])
        env.AppendUnique(CPPDEFINES=['COAST_TRACE'])
        buildflags.append('TRACE')

    env.AppendUnique(
        CPPDEFINES=[
            'COAST_BUILDFLAGS' +
            '="\\"' +
            '_'.join(buildflags) +
            '\\""'])
    compilerstring = [env.get('CXX', 'unknown')]
    if env.get('CXXVERSION', ''):
        compilerstring.append(env.get('CXXVERSION', 'unknown'))
    if env.get('CXXFLAVOUR', ''):
        compilerstring.append(env.get('CXXFLAVOUR', 'unknown'))
    env.AppendUnique(
        CPPDEFINES=[
            'COAST_COMPILER' +
            '="\\"' +
            '_'.join(compilerstring) +
            '\\""'])

    env['__envconfigdir__'] = Dir(GetOption('envconfigdir'))
    logger.info(
        "environment specific directory: %s",
        env['__envconfigdir__'].get_abspath())


def exists(env):
    return True
