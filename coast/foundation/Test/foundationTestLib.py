import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)
target = None

buildSettings = {
                 packagename : {
#                     'appendUnique'     : { 'CPPDEFINES' : 'fooX' },
                     'includeSubdir'    : '',
                     'libDepends'       : ['CoastFoundation', 'testfwFoundation'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'publicIncludes'   : [],
                     'configFiles'      : StanfordUtils.listFiles(['config/*.any',
                                                                   'config/*.txt',
                                                                   'config/*.tst',
                                                                   'config/testscript.sh',
                                                                   'config/prjconfig.sh',
                                                                   'tmp/*.any',
                                                                   '*.any']),
                     }
                 }

targetEnv, target = StanfordUtils.createProgram(packagename, buildSettings)

# either create a new environment to register this package with or use targetEnv when no real target gets created
registerEnv = targetEnv
registerEnv.Tool('registerObjects',
             package=packagename,
             testApps=[target],
             config=buildSettings[packagename].get('configFiles',[])
             )
