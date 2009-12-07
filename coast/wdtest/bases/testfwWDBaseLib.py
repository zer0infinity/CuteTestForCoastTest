import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
	packagename : {
		'linkDependencies' : ['testfw', 'CoastWDBase'],
		'includeSubdir'    : '',
		'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
		'targetType'       : 'LibraryShared',
		'lazylinking'      : True,
		'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
		'public' : {
			'includeSubdir': '',
			'includes'     : StanfordUtils.listFiles(['*.h']),
		}
	}
}

StanfordUtils.createTargets(packagename, buildSettings)
