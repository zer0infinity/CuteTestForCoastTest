import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)
target = None

buildSettings = {
                 packagename : {
                     'includeSubdir'    : '',
                     'libDepends'       : ['loki', 'boost'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'publicIncludes'   : StanfordUtils.listFiles([os.path.join('', '*.h')]),
#                     'publicIncludes'   : StanfordUtils.listFiles([os.path.join(buildSettings.get('includeSubdir',''), '*.h')]),
#                     'appendUnique'     : { 'CPPDEFINES' : 'fooX' },
                 }
                }

envVars = StanfordUtils.EnvVarDict(CPPDEFINES=[packagename.upper() + '_IMPL'])
targetEnv, target = StanfordUtils.createSharedLibrary(packagename, buildSettings, envVars=envVars)

# either create a new environment to register this package with or use targetEnv when no real target gets created
registerEnv = StanfordUtils.CloneBaseEnv()
registerEnv.Tool('registerObjects',
             package=packagename,
             libraries=[target],
             includes=buildSettings[packagename].get('publicIncludes',[])
             )
