import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 packagename : {
                     'includeSubdir'    : '',
                     'libDepends'       : ['CoastFoundation', 'testfwFoundation'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'ProgramTest',
#                     'appendUnique'     : { 'CPPDEFINES' : 'fooX' },
                     'configFiles'      : StanfordUtils.listFiles(['*.any']),
                     }
                 }

StanfordUtils.createTargets(packagename, buildSettings)
