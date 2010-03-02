import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
    packagename : {
        'linkDependencies' : ['loki', 'boost'],
        'sourceFiles'      : SConsider.listFiles(['*.cpp']),
        'targetType'       : 'LibraryShared',
        'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
        'public' : {
            'includes'     : SConsider.listFiles(['*.h']),
#            'appendUnique' : { 'CPPDEFINES' : 'fooX' },
       }
    }
}

SConsider.createTargets(packagename, buildSettings)
