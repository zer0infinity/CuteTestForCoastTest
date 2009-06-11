import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 'testfw' : {
                     'includeSubdir'    : '',
                     'linkDependencies'       : [],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'lazylinking'      : True,
                     'appendUnique'     : {},
                     'public' : {
                                 'includes'     : StanfordUtils.listFiles(['*.h']),
#                                'appendUnique' : { 'CPPDEFINES' : 'fooX' },
                    }
                 }
                }

StanfordUtils.createTargets(packagename, buildSettings)
