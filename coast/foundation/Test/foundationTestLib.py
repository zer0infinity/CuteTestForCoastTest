import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 packagename : {
                     'includeSubdir'    : '',
                     'linkDependencies'       : ['CoastFoundation', 'testfwFoundation'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'ProgramTest',
#                     'appendUnique'     : { 'CPPDEFINES' : 'fooX' },
                     'configFiles'      : StanfordUtils.listFiles(['config/*.any',
                                                                   'config/*.txt',
                                                                   'config/*.tst',
                                                                   'config/testscript.sh',
                                                                   'config/prjconfig.sh',
                                                                   'tmp/*.any',
                                                                   '*.any']),
                     }
                 }

StanfordUtils.createTargets(packagename, buildSettings)
