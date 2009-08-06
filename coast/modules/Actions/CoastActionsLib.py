import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
     packagename : {
         'includeSubdir'    : '',
         'linkDependencies'       : ['CoastWDBase', 'CoastRegex'],
         'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
         'targetType'       : 'LibraryShared',
         'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
         'public' : {
                     'includes'     : StanfordUtils.listFiles(['*.h']),
        }
     }
    }

StanfordUtils.createTargets(packagename, buildSettings)
