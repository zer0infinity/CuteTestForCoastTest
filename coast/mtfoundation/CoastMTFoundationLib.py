import os, pdb
import SConsider

packagename = SConsider.getPackageName( __name__ )

buildSettings = {
    packagename : {
        'includeSubdir'    : '',
        'linkDependencies' : ['CoastFoundation'],
        'sourceFiles'      : SConsider.listFiles( ['*.cpp'] ),
        'targetType'       : 'LibraryShared',
        'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
        'public' : {
            'includes'     : SConsider.listFiles( ['*.h', '*.ipp'] ),
            'appendUnique' : { 'CPPDEFINES' : ['_POSIX_THREADS'], 'LIBS' : ['pthread'] },
        }
    }
}

SConsider.createTargets( packagename, buildSettings )
