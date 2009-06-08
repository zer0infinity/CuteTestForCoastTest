import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 'CoastLDAPDataAccess' : {
                     'includeSubdir'    : '',
                     'libDepends'       : ['iplanetLDAP', 'openssl', 'CoastDataAccess', 'CoastSecurity'],
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
