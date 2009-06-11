import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                   packagename : {
                       'includeSubdir'    : '',
                       'linkDependencies' : ['CoastWDBase'],
                       'sourceFiles'      : ['Main.cpp'],
                       'targetType'       : 'Program',
                   }
                 }

StanfordUtils.createTargets(packagename, buildSettings)
