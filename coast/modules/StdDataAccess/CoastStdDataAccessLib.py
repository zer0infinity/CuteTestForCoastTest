import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
     packagename : {
         'includeSubdir'    : '',
         'linkDependencies'       : ['CoastAppLog', 'CoastSSL', 'CoastHTTP', 'CoastAccessControl', 'CoastRegex'],
         'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
         'targetType'       : 'LibraryShared',
         'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
         'public' : {
                     'includes'     : StanfordUtils.listFiles(['*.h']),
        }
     }
    }

StanfordUtils.createTargets(packagename, buildSettings)
