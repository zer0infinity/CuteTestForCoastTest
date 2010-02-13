import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
     packagename : {
                     'includeSubdir'    : '',
                     'linkDependencies'       : ['CoastDataAccess'],
                     'sourceFiles'      : SConsider.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL','ACCESSCONTROL_IMPL'] },
                     'public' : {
                                 'includes'     : SConsider.listFiles(['*.h']),
#                                'appendUnique' : { 'CPPDEFINES' : 'fooX' },
                    }
                 }
                }

SConsider.createTargets(packagename, buildSettings)
