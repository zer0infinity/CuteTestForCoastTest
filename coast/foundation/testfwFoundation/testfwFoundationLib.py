import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
    packagename : {
        'linkDependencies' : ['testfw'],
        'sourceFiles'      : [],
        'targetType'       : 'IncludeOnly',
        'public' : {
            'includes'     : StanfordUtils.listFiles(['*.h']),
        }
    }
}

StanfordUtils.createTargets(packagename, buildSettings)
