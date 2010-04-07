import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
    packagename : {
        'includeSubdir'    : '',
        'linkDependencies' : ['CoastSecurity', 'openssl'],
        'sourceFiles'      : SConsider.listFiles(['*.cpp']),
        'targetType'       : 'LibraryShared',
        'appendUnique'     : { 'CPPDEFINES' : ['SSL_IMPL'] },
        'public' : {
            'includes'     : SConsider.listFiles(['*.h']),
            'includeSubdir': '',
        }
    }
}

SConsider.createTargets(packagename, buildSettings)
