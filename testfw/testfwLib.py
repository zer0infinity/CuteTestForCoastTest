import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 packagename : {
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'lazylinking'      : True,
                     'public' : {
                         'includes'     : StanfordUtils.listFiles(['*.h']),
                    }
                 }
                }

StanfordUtils.createTargets(packagename, buildSettings)
