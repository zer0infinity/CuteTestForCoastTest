import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 'CoastMTFoundation' : {
                     'includeSubdir'    : '',
                     'libDepends'       : ['CoastFoundation'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                'includes'     : StanfordUtils.listFiles(['*.h', '*.ipp']),
                                'appendUnique' : { 'CPPDEFINES' : ['_POSIX_THREADS'], 'LIBS' : ['pthread'] },
                    }
                 }
                }

StanfordUtils.createTargets(packagename, buildSettings)
