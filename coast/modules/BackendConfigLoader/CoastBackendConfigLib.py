import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
     packagename : {
         'includeSubdir'    : '',
         'linkDependencies' : ['CoastWDBase'],
         'sourceFiles'      : SConsider.listFiles(['*.cpp']),
         'targetType'       : 'LibraryShared',
         'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
         'public' : {
             'includes'     : SConsider.listFiles(['*.h']),
        }
     }
    }

SConsider.createTargets(packagename, buildSettings)
