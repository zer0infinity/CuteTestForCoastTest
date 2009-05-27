import os, platform, pdb, traceback, string
import SCons
import StanfordUtils
from SCons.Script import Dir

_added = None
_packagename = None
_includeSubdir = ''

def generate(env, **kw):
    global _added, _packagename, _includeSubdir
    if not _added:
        _added = 1
        _packagename = StanfordUtils.getPackageName(__name__)

    # this libraries dependencies
    StanfordUtils.DependsOn(env, 'testfwWDBase')
    StanfordUtils.DependsOn(env, 'CoastPerfTest')
    # flags / settings used by this library and users of it
#    env.AppendUnique(CPPDEFINES =['ONLY_STD_IOSTREAM'])
    # export library dependency when we are not building ourselves
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library=[_packagename])
        #FIXME: unfortunately this is copy wasted from registerObjects.py because I was not able to
        installPath = env['BASEOUTDIR'].Dir(os.path.join(env['INCDIR'], _packagename))
        if not _includeSubdir == '':
            installPath = installPath.Dir(_includeSubdir)
        env.AppendUnique(CPPPATH=[installPath])
    else:
        # win32 specific define to export all symbols when creating a DLL
        env.AppendUnique(CPPDEFINES=[_packagename.upper()+'_IMPL'])
        # specify public headers here
        env.Tool('registerObjects', package=_packagename, includes=StanfordUtils.listFiles([os.path.join(_includeSubdir, '*.h')]))
        # maybe we need to add this libraries local include path when building it (if different from .)
        if not _includeSubdir == '':
            env.AppendUnique(CPPPATH=[Dir(_includeSubdir)])

def exists(env):
    return true
