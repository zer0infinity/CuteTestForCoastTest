import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 packagename : {
                     'linkDependencies' : ['CoastFoundation', 'testfwFoundation'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'ProgramTest',
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
