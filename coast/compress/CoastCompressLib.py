import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
     packagename : {
                     'includeSubdir'    : '',
                     'linkDependencies'       : ['CoastFoundation', 'zlib'],
                     'sourceFiles'      : SConsider.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL','COMPRESS_IMPL'] },
                     'public' : {
                                 'includes'     : SConsider.listFiles(['*.h']),
#                                 'appendUnique' : { 'CPPDEFINES' : [], 'LIBS' : [] },
                    }
                 }
                }

SConsider.createTargets(packagename, buildSettings)
