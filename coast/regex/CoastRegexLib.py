import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
     packagename : {
                     'includeSubdir'    : '',
                     'linkDependencies'       : ['CoastFoundation'],
                     'sourceFiles'      : SConsider.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL','REGEX_IMPL'] },
                     'public' : {
                                 'includes'     : SConsider.listFiles(['*.h']),
#                                'appendUnique' : { 'CPPDEFINES' : 'fooX' },
                    }
                 }
                }

SConsider.createTargets(packagename, buildSettings)
