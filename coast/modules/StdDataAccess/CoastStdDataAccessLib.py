import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 'CoastStdDataAccess' : {
                     'includeSubdir'    : '',
                     'linkDependencies'       : ['CoastAppLog', 'CoastSSL', 'CoastHTTP', 'CoastAccessControl'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                 'includes'     : StanfordUtils.listFiles(['*.h']),
#                                'appendUnique' : { 'CPPDEFINES' : 'fooX' },
                    }
                 }
                }

StanfordUtils.createTargets(packagename, buildSettings)
