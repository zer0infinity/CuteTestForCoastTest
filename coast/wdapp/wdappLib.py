import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)
target = None

buildSettings = {
                 packagename : {
                     'includeSubdir'    : '',
                     'libDepends'       : ['CoastWDBase'],
                     'sourceFiles'      : ['Main.cpp'],
                     'publicIncludes'   : [],
#                     'appendUnique'     : { 'CPPDEFINES' : 'fooX' },
                     }
                 }

targetEnv, target = StanfordUtils.createProgram(packagename, buildSettings)

# either create a new environment to register this package with or use targetEnv when no real target gets created
#registerEnv = targetEnv
#registerEnv.Tool('registerObjects',
#             package=packagename,
#             testApps=[target],
#             )
