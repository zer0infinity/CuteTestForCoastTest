import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 'testfwWDBase' : {
                     'includeSubdir'    : '',
                     'linkDependencies'       : ['testfwFoundation', 'CoastWDBase'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'lazylinking'      : True,
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                 'includes'     : StanfordUtils.listFiles(['*.h']),
#                                'appendUnique' : { 'CPPDEFINES' : 'fooX' },
                    }
                 }
                }

StanfordUtils.createTargets(packagename, buildSettings)
