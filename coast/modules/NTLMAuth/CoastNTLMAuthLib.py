import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 packagename : {
                     'linkDependencies' : ['CoastSecurity', 'CoastWDBase', 'openssl'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                 'includes'     : StanfordUtils.listFiles(['*.h']),
                                 'includeSubdir'    : '',
                    }
                 }
                }

StanfordUtils.createTargets(packagename, buildSettings)
