import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 'CoastCompress' : {
                     'includeSubdir'    : '',
                     'libDepends'       : ['CoastFoundation', 'zlib'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                 'includes'     : StanfordUtils.listFiles(['*.h']),
#                                 'appendUnique' : { 'CPPDEFINES' : [], 'LIBS' : [] },
                    }
                 }
                }

StanfordUtils.createTargets(packagename, buildSettings)
