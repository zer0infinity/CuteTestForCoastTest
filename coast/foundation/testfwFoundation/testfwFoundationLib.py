import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
                 packagename : {
                     'linkDependencies' : ['testfw'],
                     'sourceFiles'      : [],
                     'targetType'       : 'IncludeOnly',
                     'public' : {
                         'includes'     : SConsider.listFiles(['*.h']),
                    }
                 }
                }

SConsider.createTargets(packagename, buildSettings)
