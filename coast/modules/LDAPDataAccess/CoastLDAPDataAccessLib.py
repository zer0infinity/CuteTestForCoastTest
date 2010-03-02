import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
                 'CoastLDAPDataAccess' : {
                     'includeSubdir'    : '',
                     'linkDependencies'       : ['iplanetLDAP', 'openssl', 'CoastDataAccess', 'CoastSecurity'],
                     'sourceFiles'      : SConsider.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                 'includes'     : SConsider.listFiles(['*.h']),
#                                'appendUnique' : { 'CPPDEFINES' : 'fooX' },
                    }
                 }
                }

SConsider.createTargets(packagename, buildSettings)
