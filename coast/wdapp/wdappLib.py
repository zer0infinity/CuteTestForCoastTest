import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
                   packagename : {
                       'includeSubdir'    : '',
                       'linkDependencies' : ['CoastWDBase'],
                       'sourceFiles'      : ['Main.cpp'],
                       'targetType'       : 'Program',
                   }
                 }

SConsider.createTargets(packagename, buildSettings)
