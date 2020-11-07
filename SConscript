#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
import os
import SCons
import shutil
import subprocess
import time
import datetime
import glob
import uuid
from xml.dom import minidom
import SCons.Script as SCons

from build import util, depends

mixxx_version = util.get_mixxx_version()
branch_name = util.get_branch_name()
vcs_revision = util.get_revision()
vcs_name = util.get_current_vcs()
print("WE ARE IN:", os.getcwd())
print("Building ", branch_name, " - rev.", vcs_revision)

plugins = []

# Grab these from the SConstruct above us
Import('build')
Import('sources')

env = build.env
flags = build.flags

# Make a static library of all Mixxx's sources. This library will be linked into
# both mixxx and mixxx-test.
mixxx_lib = env.StaticLibrary('libmixxx',
                              [source for source in sources
                               if str(source) != 'src/main.cpp'])
# mixxx.qrc must not be bundled into libmixxx.a since the linker will not link
# it into the resulting binary unless it is on the link command-line explicitly
# (it has no link-time symbols that are needed by anything in Mixxx).
mixxx_qrc = env.StaticObject(env.Qrc5('res/mixxx.cc', 'res/mixxx.qrc'))
# libmixxx.a needs to precede all other libraries so that symbols it requires
# end up in the linker's list of unresolved symbols before other libraries are
# searched for symbols.
env.Prepend(LIBS=mixxx_lib)
mixxx_main = env.StaticObject('src/main.cpp')

#Tell SCons to build Mixxx
#=========================
if build.platform_is_windows:
        dist_dir = 'dist%s' % build.bitwidth
        # Populate the stuff that changes in the .rc file
        fo = open(File('src/mixxx.rc.include').abspath, "w")

        str_list = []
        str_list.append('#define VER_FILEVERSION             ')
        # Remove anything after ~ or - in the version number and replace the dots with commas
        str_list.append(mixxx_version.partition('~')[0].partition('-')[0].replace('.',','))
        if vcs_revision:
            str_list.append(','+str(vcs_revision))
        str_list.append('\n')

        str_list.append('#define VER_PRODUCTVERSION          ')
        str_list.append(mixxx_version.partition('~')[0].partition('-')[0].replace('.',','))
        if vcs_revision:
            str_list.append(','+str(vcs_revision))
        str_list.append('\n')

        import datetime
        now = datetime.datetime.now()
        str_list.append('#define CUR_YEAR                    "'+str(now.year)+'"\n\n')

        if build.build_is_debug:
            str_list.append('#define DEBUG                       1\n')
        if 'pre' in mixxx_version.lower():
            str_list.append('#define PRERELEASE                  1\n')

        fo.write(''.join(str_list))
        fo.close()

        mixxx_rc = env.RES('src/mixxx.rc')
        mixxx_bin = env.Program('mixxx',
                                [mixxx_main, mixxx_qrc, mixxx_rc],
                                LINKCOM = [env['LINKCOM'], 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;1'])
elif build.platform_is_osx:
        # Bug #1258435: executable name must match CFBundleExecutable in the
        # Info.plist. For codesigned bundles it seems the CFBundleExecutable
        # must match the bundle name or else we SIGILL at startup (not sure
        # why).
        mixxx_bin = env.Program('Mixxx', [mixxx_main, mixxx_qrc])
else:
        mixxx_bin = env.Program('mixxx', [mixxx_main, mixxx_qrc])

# For convenience, copy the Mixxx binary out of the build directory to the
# root. Don't do it on windows because the binary can't run on its own and needs
# the DLLs present with it.
if not build.platform_is_windows:
    copy_mixxx_bin = Command("../mixxx", mixxx_bin, Copy("$TARGET", "$SOURCE"))
    Default(copy_mixxx_bin)
else:
    Default(mixxx_bin)

test_bin = None
def define_test_targets(default=False):
        global test_bin
        test_files = Glob('src/test/*.cpp', strings=True)
        test_env = env.Clone()

        test_env.Append(CPPPATH="lib/googletest-1.8.x/googletest/include")
        test_env.Append(LIBPATH="lib/googletest-1.8.x/googletest")
        test_env.Append(LIBS=['gtest'])

        test_env.Append(CPPPATH="lib/googletest-1.8.x/googlemock/include")
        test_env.Append(LIBPATH="lib/googletest-1.8.x/googlemock")
        test_env.Append(LIBS=['gmock'])

        test_env.Append(CPPPATH="lib/benchmark/include")
        test_env.Append(LIBPATH="lib/benchmark")
        test_env.Append(LIBS=['benchmark'])

        test_files = [test_env.StaticObject(filename)
                      if filename !='src/test/main.cpp' else filename
                      for filename in test_files]

        if build.platform_is_windows:
                # For SHGetValueA in Google's benchmark library.
                test_env.Append(LIBS=['Shlwapi'])

                # We want a terminal for tests.
                if build.toolchain_is_msvs:
                    test_env['LINKFLAGS'].remove('/subsystem:windows,6.01')
                    test_env['LINKFLAGS'].append('/subsystem:console,6.01')
                elif build.toolchain_is_gnu:
                    test_env['LINKFLAGS'].remove('--subsystem,windows')
                    test_env['LINKFLAGS'].append('--subsystem,console')

                # Currently both executables are built with /subsystem:windows
                # and the console is attached manually
                test_bin = test_env.Program(
                        'mixxx-test', [test_files, mixxx_qrc, mixxx_rc],
                        LINKCOM = [env['LINKCOM'], 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;1'])
        else:
                test_bin = test_env.Program('mixxx-test', [test_files, mixxx_qrc])

        if not build.platform_is_windows:
                copy_test_bin = Command("../mixxx-test", test_bin, Copy("$TARGET", "$SOURCE"))
                env.Alias('mixxx-test', copy_test_bin)
                # Running mixxx-test via a Command is hacky because it expects a
                # target. Using the source '../mixxx-test' makes the Command
                # depend on the Copy.
                run_test = Command('mixxx-test-results', '../mixxx-test', './mixxx-test')
                env.Alias('test', run_test)

                if default:
                        Default(copy_test_bin)
        else:
                env.Alias('mixxx-test', test_bin)
                if default:
                        Default(test_bin)


# If the 'test' flag is 1, then build the mixxx-test target by default. If
# 'test' is in the target list then run mixxx-test.
build_tests_by_default = int(build.flags['test']) != 0
build_tests = 'mixxx-test' in COMMAND_LINE_TARGETS
run_tests = 'test' in COMMAND_LINE_TARGETS
if build_tests or run_tests or build_tests_by_default:
        define_test_targets(default=build_tests_by_default)

def construct_version(build, mixxx_version, branch_name, vcs_revision):
        if branch_name.startswith('release-'):
                branch_name = branch_name.replace('release-', '')

        # Include build type in the filename.
        build_type = 'release' if build.build_is_release else 'debug'

        # New, simpler logic: mixxx version, branch name, git revision,
        # release/build. Example: mixxx-1.12.0-master-gitXXXX-release
        return "%s-%s-%s%s-%s" % (mixxx_version, branch_name, vcs_name,
                                  vcs_revision, build_type)

def ubuntu_construct_version(build, mixxx_version, branch_name, vcs_revision,
                             ubuntu_version, distro_version):
        # The format of a Debian/Ubuntu version is:
        #
        #   [epoch:]upstream_version[-debian_revision]
        #
        # A detailed description of the valid characters and sorting order of
        # versions can be found here:
        # https://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-Version
        #
        # For package upgrades to work correctly, we want the following
        # orderings on package versions:
        #
        # nightly build < pre-alpha < alpha < beta < rc1 < rc2 < final release
        #
        # The sorting rules are complicated but the key detail is: "The lexical
        # comparison is a comparison of ASCII values modified so that all the
        # letters sort earlier than all the non-letters and so that a tilde
        # sorts before anything, even the end of a part."
        #
        # The Mixxx version stored in src/defs_version.h (the "mixxx_version"
        # parameter to this function) is formatted like:
        #
        # Pre Alpha: 2.0.0-alpha-pre
        # Alpha: 2.0.0-alpha
        # Beta: 2.0.0-beta
        # RC: 2.0.0-rc1
        # Final: 2.0.0
        #
        # Since hyphens are a separator character between the upstream version
        # and Debian version, we replace these with tildes.
        #
        # Other goals:
        # - We would like to know the branch and commit of a package.
        # - We would like the PPA to trump the official Debian package.
        #
        # The following versions are sorted from low to high order:
        # 1.9.9
        # 2.0.0~alpha~pre
        # 2.0.0~alpha
        # 2.0.0~beta~pre
        # 2.0.0~beta
        # 2.0.0~dfsg4 <- official Debian package version
        # 2.0.0~rc1
        # 2.0.0
        # 2.0.1~alpha~pre
        #
        # Our official Debian packages have a ~dfsg section, so in this case an
        # rc1 package in our PPA would trump an official Debian package
        # (probably not what we want but not too bad since we would probably
        # publish a "2.0.0" final to our PPA before the official Debian package
        # is even released.
        #
        # Note in the above sorted list that if the branch name were included
        # after the mixxx_version, 2.0.0~master would sort earlier than
        # 2.0.0~rc1~master!  To prevent branch and revision tags from
        # interfering with package ordering we include them in the
        # debian_revision portion of the version. This ensures they are only
        # used for sorting if the upstream version of two packages is identical.
        upstream_version = mixxx_version.replace('-', '~')
        assert '_' not in upstream_version

        # Strip underscores and dashes in the branch name.
        branch_name = branch_name.strip('_-')
        assert branch_name and branch_name != '(no branch)'

        return "%s-%s~%s~%s%s~%s" % (upstream_version, ubuntu_version, branch_name,
                                     vcs_name, vcs_revision, distro_version)

#Set up the install target
#=========================

#Mixxx binary
binary_files = [mixxx_bin];
if test_bin is not None:
        binary_files.append(test_bin)

if build.bundle_pdbs:
        binary_files.append(env.SideEffect('mixxx.pdb', mixxx_bin))

#Skins
skin_files = Glob('#res/skins/*')

#Controller mappings
controllermappings_files = Glob('#res/controllers/*')

# Translation files
# QT 5 translations have been separated into several files, and most of the qt_xx.qm files contain just shortcuts to load the qtbase, qtmultimedia etc files.
translation_files = Glob('#res/translations/*.qm') + Glob(os.path.join(build.env['QTDIR'], 'translations/qt_*.qm')) + Glob(os.path.join(build.env['QTDIR'], 'translations/qtbase_*.qm')) + Glob(os.path.join(build.env['QTDIR'], 'translations/qtmultimedia_*.qm')) + Glob(os.path.join(build.env['QTDIR'], 'translations/qtscript_*.qm')) + Glob(os.path.join(build.env['QTDIR'], 'translations/qtxmlpatterns_*.qm'))


# Font files
font_files = Glob('#res/fonts/*')

#Keyboard mapping(s)
keyboardmappings_files = Glob('#res/keyboard/*')

#Documentation
docs_files = Glob('#./LICENSE')
docs_files += Glob('#./README')
docs_files += Glob('#./Mixxx-Manual.pdf')

#.desktop file for KDE/GNOME menu
dotdesktop_files = Glob('#res/linux/mixxx.desktop')

#.appdata.xml file for KDE/GNOME AppStream iniative
dotappstream_files = Glob('#res/linux/mixxx.appdata.xml')

#udev rule file for USB HID and Bulk controllers
hidudev_files = Glob('#res/linux/mixxx-usb-uaccess.rules')

#Icon file for menu entry
icon_files = Glob('#res/images/mixxx_icon.svg')

#Images for preferences dialog
image_files = Glob('#res/images/preferences/*')  # These are compiled in to the "mixxx" binary through mixxx.qrc

#Windows DLLs

dll_files = []
if build.toolchain_is_msvs and not build.static_dependencies:
        # skip the MSVC DLLs in case they're in there too
        dll_files.extend(Glob('%s/*.dll' % build.winlib_path))
        dll_files.extend(Glob('%s/lib/*.dll' % build.winlib_path))

        if build.bundle_pdbs:
                dll_files.extend(Glob('%s/*.pdb' % build.winlib_path))
                dll_files.extend(Glob('%s/lib/*.pdb' % build.winlib_path))
elif build.crosscompile and build.platform_is_windows and build.toolchain_is_gnu and not build.static_dependencies:
        # We're cross-compiling, grab these from the crosscompile bin
        # folder. How should we be doing this?
        dll_files = Glob('#/../../mixxx-win%slib-crossmingw' % build.bitwidth)

qt_modules = depends.Qt.enabled_modules(build)

if build.platform_is_windows:
    suffix = 'd.dll' if build.build_is_debug else '.dll'
    if not build.static_qt:
        qt_modules = ['$QTDIR/lib/' + module.replace('Qt', 'Qt5') + suffix
                      for module in qt_modules]
        dll_files.extend(qt_modules)
    # https://doc.qt.io/qt-5/windows-deployment.html
    # "If dynamic OpenGL is used, you additionally need to include the
    # libraries required for ANGLE and software rendering. For ANGLE, both
    # libEGL.dll and libGLESv2.dll from Qt's lib directory are required as
    # well as the HLSL compiler from DirectX. The HLSL compiler library,
    # d3dcompiler_XX.dll, where XX is the version number that ANGLE
    # (libGLESv2) was linked against."
    dll_files.extend(['$QTDIR/bin/libEGL' + suffix,
                      '$QTDIR/bin/libGLESv2' + suffix])
    d3dcompiler_path = util.find_d3dcompiler_dll(build.env)
    if d3dcompiler_path:
        dll_files.append(d3dcompiler_path)

# Qt imageformats plugin
imgfmtdll_files = []
qt_imagesformats = depends.Qt.enabled_imageformats(build)

suffix = 'd.dll' if build.build_is_debug else '.dll'
if not build.static_qt:
    imgfmtdll_files.extend(['$QTDIR/plugins/imageformats/' + module + suffix for module in qt_imagesformats])
# We don't have Qt's dll pdb files in our release build environements, so only if build is debug
pdbSuffix = 'd.pdb' if (build.bundle_pdbs and build.build_is_debug) else ''
if pdbSuffix:
    imgfmtdll_files.extend(['$QTDIR/plugins/imageformats/' + module + pdbSuffix for module in qt_imagesformats])

sqldll_files = []
if int(flags.get('qt_sqlite_plugin', 0)):
    # TODO(rryan): Add the SQLite DLL For Qt5.
    pass

if build.platform_is_linux or build.platform_is_bsd:
        flags['prefix'] = ARGUMENTS.get('prefix', '/usr/local')
        if not os.path.exists(flags['prefix']):
                print("Error: Prefix path does not exist!")
                Exit(1)
        else:
                #install_root is used in Debian/Ubuntu packaging (check the debian/rules file in the Ubuntu package)
                #Basically, the flags['prefix'] is compiled into strings in Mixxx, whereas the install_root is not. When you're
                #building a Debian package, pbuilder wants to install Mixxx to a temporary directory, but you still need
                #the compiled-in strings using /usr as the prefix. That's why we have install_root and flags['prefix'].
                install_root = ARGUMENTS.get('install_root', flags['prefix'])
                print("Install root: " + install_root)
                unix_share_path = os.path.join(install_root,
                    env.get('SHAREDIR', default='share'))
                unix_bin_path = os.path.join(install_root,
                    env.get('BINDIR', default='bin'))

                binary = env.Install(unix_bin_path, binary_files)
                skins = env.Install(os.path.join(unix_share_path, 'mixxx', 'skins'), skin_files)
                fonts = env.Install(os.path.join(unix_share_path, 'mixxx', 'fonts'), font_files)
                controllermappings = env.Install(os.path.join(unix_share_path, 'mixxx', 'controllers'), controllermappings_files)
                translations = env.Install(os.path.join(unix_share_path, 'mixxx', 'translations'), translation_files)
                keyboardmappings = env.Install(os.path.join(unix_share_path, 'mixxx', 'keyboard'), keyboardmappings_files)
                dotdesktop = env.Install(os.path.join(unix_share_path, 'applications'), dotdesktop_files)
                dotappstream = env.Install(os.path.join(unix_share_path, 'appdata'), dotappstream_files)
                docs = env.Install(os.path.join(unix_share_path, 'doc', 'mixxx'), docs_files)
                icon = env.Install(os.path.join(unix_share_path, 'pixmaps'), icon_files)

                # NOTE(rryan): Hack to detect when we're Debian packaging.
                building_debian_package = 'debian/tmp/usr' in install_root
                udev_root = '/etc/udev/rules.d'
                hidudev = env.Install(udev_root, hidudev_files)

                #Makes each of those Install builders get fired off when you run "scons install" :)
                env.Alias('install', binary)
                env.Alias('install', skins)
                env.Alias('install', fonts)
                env.Alias('install', controllermappings)
                env.Alias('install', translations)
                env.Alias('install', keyboardmappings)
                env.Alias('install', docs)
                env.Alias('install', dotdesktop)
                env.Alias('install', dotappstream)
                env.Alias('install', icon)

                if not building_debian_package and os.access(udev_root, os.W_OK):
                        env.Alias('install', hidudev)


#Build the Mixxx.app bundle
if build.platform_is_osx and 'bundle' in COMMAND_LINE_TARGETS:
        #Mixxx build variables
        VOLNAME="Mixxx" #tmp tmp tmp, it's unclean to pass this into build_dmg this way. perhaps pass it in the env?
        ARCH = 'ppc' if build.machine in ['powerpc', 'powerpc64'] else 'macintel'
        ARCH += ("64" if build.machine_is_64bit else "32")

        DMG_ICON="#res/osx/VolumeIcon.icns"

        # In Qt 5, the SQLite driver was moved out of QtSql and into a plugin.
        sql_dylibs = ["libqsqlite.dylib"]

        qt_plugins = (
                [("iconengines", e) for e in ["libqsvgicon.dylib"]] +
                # Left out libqmng and libqtiff to save space.
                [("imageformats", e) for e in
                 ["libqgif.dylib", "libqjpeg.dylib", "libqsvg.dylib"]] +
                # Cocoa support moved to a plugin in Qt 5.
                [("platforms", "libqcocoa.dylib")] +
                [("sqldrivers", e) for e in sql_dylibs] +
                [("styles", "libqmacstyle.dylib")]
        )

        resource_map = {}
        for tfile in translation_files:
                resource_map[str(tfile)] = 'translations'

        qtdir = build.env['QTDIR']
        qt_frameworks = depends.Qt.find_framework_libdir(qtdir)
        if not qt_frameworks:
                raise Exception('Could not find frameworks in Qt directory: %s' % qtdir)
        #qt_menu.nib for Cocoa Qt 4.7+
        menu_nib = os.path.join(qt_frameworks, 'QtGui.framework',
                                'Resources', 'qt_menu.nib')
        otool_local_paths = [os.path.expanduser("~/Library/Frameworks"),
                             qt_frameworks,
                             "/Library/Frameworks",
                             "/Network/Library/Frameworks",
                             "/usr/local/lib",
                             "/opt/local/lib",
                             "/sw/local/lib"]
        otool_system_paths = ["/System/Library/Frameworks",
                              "/Network/Library/Frameworks",
                              "/usr/lib"]
        mixxx_osxlib_path = SCons.ARGUMENTS.get('osxlib', None)
        if mixxx_osxlib_path:
                otool_local_paths = [mixxx_osxlib_path,] + otool_local_paths

        qtplugindir = SCons.ARGUMENTS.get('qtplugindir', None)
        if not qtplugindir:
                #qtplugindir = '/Developer/Applications/Qt/'
                qtplugindir = qtdir
        sources = [mixxx_bin,
                   '#res/osx/application.icns',
                   Dir('#res/skins/'),
                   Dir('#res/controllers/'),
                   Dir('#res/fonts/'),
                   translation_files,
                   Dir('#res/keyboard/'),
                   Dir('#res/doc/'),
                   Dir(menu_nib),
                   File("#README"),
                   File("#LICENSE")]
        bundle = env.App(
                "Mixxx_bundle",
                sources,
                PLUGINS=plugins, ##XXX test what happens if we don't pass any plugins
                #Qt plugins ((Qt *NEEDS* its plugins in specific locations or it refuses to find them, however this is clearly awkward to write out like this.. maybe))
                QT_HACK = [(p_tgt_dir, os.path.join(qtplugindir, "plugins", p_tgt_dir, p)) for p_tgt_dir, p in qt_plugins], #sigh :(
                APP_RESOURCES_MAP=resource_map,
                IDENTIFIER="org.mixxx.mixxx",
                DISPLAY_NAME="Mixxx",
                VERSION=mixxx_version,
                SHORT_VERSION=mixxx_version,
                COPYRIGHT="Copyright © 2001-%s Mixxx Development Team" % datetime.datetime.now().year,
                MINIMUM_OSX_VERSION=util.get_osx_min_version(),
                CATEGORY="public.app-category.music",
                OTOOL_LOCAL_PATHS=otool_local_paths,
                OTOOL_SYSTEM_PATHS=otool_system_paths,
                FOR_APP_STORE=int(build.flags['macappstore']) > 0,
                )
        env.Alias('bundle', bundle)

        codesign_installer_identity = SCons.ARGUMENTS.get('osx_codesign_installer_identity', None)
        codesign_application_identity = SCons.ARGUMENTS.get('osx_codesign_application_identity', None)
        codesign_keychain = SCons.ARGUMENTS.get('osx_codesign_keychain', None)
        codesign_keychain_password = SCons.ARGUMENTS.get('osx_codesign_keychain_password', None)
        codesign_entitlements = SCons.ARGUMENTS.get('osx_codesign_entitlements', None)
        # CodeSign needs to take sources for it source so that there is an input
        # that changse. Otherwise SCons will think the CodeSign target is up to
        # date and not run it.
        codesign = env.CodeSign(
                'Mixxx_codesign',
                sources,
                CODESIGN_INSTALLER_IDENTITY=codesign_installer_identity,
                CODESIGN_APPLICATION_IDENTITY=codesign_application_identity,
                CODESIGN_KEYCHAIN=codesign_keychain,
                CODESIGN_KEYCHAIN_PASSWORD=codesign_keychain_password,
                CODESIGN_ENTITLEMENTS=codesign_entitlements)
        env.AlwaysBuild(codesign)
        env.Alias('sign', codesign)

        package_name = 'mixxx'
        package_version = construct_version(build, mixxx_version, branch_name,
                                            vcs_revision)
        dmg_name = '%s-%s-%s' % (package_name, package_version, ARCH)
        dmg = env.Dmg(dmg_name, [bundle, ] + docs_files, VOLNAME=VOLNAME, ICON = DMG_ICON)
        env.Alias('package', dmg)

if build.platform_is_windows:
        base_dist_dir = '#' + dist_dir
        skins = env.Install(os.path.join(base_dist_dir, "skins"), skin_files)
        controllermappings = env.Install(os.path.join(base_dist_dir, "controllers"), controllermappings_files)
        fonts = env.Install(os.path.join(base_dist_dir, "fonts"), font_files)
        translations = env.Install(os.path.join(base_dist_dir, "translations"), translation_files)
        keyboardmappings = env.Install(os.path.join(base_dist_dir, "keyboard"), keyboardmappings_files)
        docs = env.Install(os.path.join(base_dist_dir, "doc/"), docs_files)
        #icon = env.Install(base_dist_dir+"", icon_files)
        dlls = env.Install(base_dist_dir+"/", dll_files)
        binary = env.Install(base_dist_dir+"/", binary_files)

        #Always trigger these install builders when compiling on Windows
        env.Alias('mixxx', skins)
        env.Alias('mixxx', controllermappings)
        env.Alias('mixxx', fonts)
        env.Alias('mixxx', translations)
        env.Alias('mixxx', keyboardmappings)
        env.Alias('mixxx', docs)
        env.Alias('mixxx', dlls)
        #env.Alias('mixxx', icon)
        env.Alias('mixxx', binary)

        binaries_to_codesign = [binary, dlls]

        # imageformats DLL
        if imgfmtdll_files:
                imageformats_dll = env.Install(os.path.join(base_dist_dir, "imageformats"), imgfmtdll_files)
                binaries_to_codesign.append(imageformats_dll)
                env.Alias('mixxx', imageformats_dll)

        # QSQLite DLL
        if sqldll_files:
                sql_dlls = env.Install(os.path.join(base_dist_dir, "sqldrivers"), sqldll_files)
                binaries_to_codesign.append(sql_dlls)
                env.Alias('mixxx', sql_dlls)

        if 'sign' in COMMAND_LINE_TARGETS:
            codesign_subject_name = SCons.ARGUMENTS.get('windows_codesign_subject_name', '')
            if not codesign_subject_name:
                raise Exception('Code-signing was requested but windows_codesign_subject_name was not provided.')
            codesign = env.SignTool(
                'Mixxx_signtool',
                binaries_to_codesign,
                SUBJECT_NAME=codesign_subject_name)
            env.Alias('sign', codesign)

def BuildRelease(target, source, env):
    print("==== Mixxx Post-Build Checks ====")
    print("You have built version %s" % mixxx_version)
    if build.build_is_debug:
        print("YOU ARE ABOUT TO PACKAGE A DEBUG BUILD!!")
    print("Binary has size ", end='')
    if build.platform_is_windows:
        os.system('for %I in ('+dist_dir+'\mixxx.exe) do @echo %~zI')
    else:
        os.system('ls -lh '+dist_dir+'/mixxx.exe | cut -d \' \' -f 5')
    print("Installer file ", end='')
    package_name = 'mixxx'

    package_version = construct_version(build, mixxx_version, branch_name,
                                        vcs_revision)
    arch = "x64" if build.machine_is_64bit else "x86"
    msi_name = '%s-%s-%s.msi' % (package_name, package_version, arch)
    print(msi_name)
    print("Top line of README, check version:")
    if build.platform_is_windows:
        os.system('for /l %l in (1,1,1) do @for /f "tokens=1,2* delims=:" %a in (\'findstr /n /r "^" README ^| findstr /r "^%l:"\') do @echo %b')
    else:
        os.system('head -n 1 README')
    print("Top 2 lines of LICENSE, check version and copyright dates:")
    if build.platform_is_windows:
        os.system('for /l %l in (1,1,2) do @for /f "tokens=1,2* delims=:" %a in (\'findstr /n /r "^" LICENSE ^| findstr /r "^%l:"\') do @echo %b')
    else:
        os.system('head -n 2 LICENSE')

    #if (raw_input("Go ahead and build installer (yes/[no])? ") == "yes"):
    if True:
        # TODO(XXX): Installing a runtime isn't specific to MSVS?
        if build.toolchain_is_msvs:
            redist_file = 'vc_redist.%s.exe' % arch
            print("Searching for the Visual C++ DLL installer package" + redist_file)
            # Check for the runtime installer in the winlib root.
            redist_path = '%s' % os.path.join(build.winlib_path, redist_file)
            print("   ", redist_path,)
            if not os.path.isfile(redist_path):
                raise Exception('Could not find the MSVC++ runtime installer.')

        print("Now building installation package...")

        print("Looking for WIX Toolset...")
        wix_path = None
        if not build.crosscompile and build.platform_is_windows:
            wix_directory = os.getenv('WIX')
            wix_path = '%s' % os.path.join(wix_directory, "bin")
        elif build.crosscompile and build.platform_is_windows:
            # TODO(XXX) How to handle that ? what does this exactly means ?
            raise NotImplementedError

        if not wix_directory:
            raise Exception ('Cannot find WIX Toolkit. Do you have it installed?')
        else:
            print("    Found Wix Toolset in " + wix_path)

        WinSDK_path = 'build\\wix'

        if not os.path.isfile(os.path.join(WinSDK_path, 'wisubstg.vbs')):
            raise Exception ('can not find ' + WinSDK_path + '\wisubstg.vbs')

        if not os.path.isfile(os.path.join(WinSDK_path, 'WiLangId.vbs')):
            raise Exception ('can not find ' + WinSDK_path + '\WiLangId.vbs')

        # Generating random ProductID (should change on every run)
        # and put it in mixxx.wxs using the template
        ProductID = str(uuid.uuid1()).upper()
        with open("build/wix/ProductID.wxi.in", "rt") as fin:
            with open("build/wix/ProductID.wxi", "wt") as fout:
                for line in fin:
                    fout.write(line.replace('@PRODUCT_ID@', ProductID))
        fin.close()
        fout.close()

        # The default language
        defaultLanguage="en-us"
        # The langIds contained in the installer. starting with LangId of the default language
        langIds="1033"

        winArch = "x64" if build.machine_is_64bit else "x86"

        # Auto-create wxs file for each subdir and compile them
        print("*** Building intermediate files")
        for subdir in next(os.walk(dist_dir))[1]:
            print("    " + dist_dir + "\\" + subdir)
            # Exclude doc and imageformats helper DLLs, they are bundled elsewhere
            if subdir in ['doc', 'imageformats']:
                continue
            command = '"%(wix)s\\heat.exe" dir %(distdir)s\%(sub)s -nologo -sfrag -suid -ag -srd -cg %(sub)sComp -dr %(sub)sDir -out build\wix\subdirs\%(sub)s.wxs -sw5150 -var var.%(sub)sVar' % \
                {'wix': wix_path,
                 'distdir': dist_dir,
                 'sub': subdir}
            print("Using Command: " + command)
            subprocess.check_call(command)
            command = '"%(wix)s\\candle.exe" -nologo -dWINLIBPATH=%(winlibpath)s -dPlatform=%(arch)s -d%(sub)sVar=%(distdir)s\%(sub)s -arch %(arch)s -out build\wix\subdirs\%(sub)s.wixobj build\wix\subdirs\%(sub)s.wxs' % \
                {'wix': wix_path,
                 'winlibpath': build.winlib_path,
                 'arch': winArch,
                 'distdir': dist_dir,
                 'sub': subdir}
            print("Using Command: " + command)
            subprocess.check_call(command)

        # Handle QT's imageformats helper DLLs if dynamic QT
        imageformats = "no"
        if os.path.exists(os.path.join(dist_dir,"imageformats")) and not build.static_qt:
            imageformats = "yes"
            command = '"%(wix)s\\heat.exe" dir %(distdir)s\%(sub)s -nologo -sfrag -suid -ag -srd -cg %(sub)sComp -dr %(sub)sDir -out build\wix\subdirs\%(sub)s.wxs -sw5150 -var var.%(sub)sVar' % \
                {'wix': wix_path,
                 'distdir': dist_dir,
                 'sub': "imageformats"}
            print("Using Command: " + command)
            subprocess.check_call(command)

            command = '"%(wix)s\\candle.exe" -nologo -dWINLIBPATH=%(winlibpath)s -dPlatform=%(arch)s -d%(sub)sVar=%(distdir)s\%(sub)s -arch %(arch)s -out build\wix\subdirs\%(sub)s.wixobj build\wix\subdirs\%(sub)s.wxs' % \
                {'wix': wix_path,
                 'winlibpath': build.winlib_path,
                 'arch': winArch,
                 'distdir': dist_dir,
                 'sub': "imageformats"}
            print("Using Command: " + command)
            subprocess.check_call(command)

        # Harvest main DLL from install dir
        command = '"%(wix)s\\heat.exe" dir %(distdir)s -nologo -sfrag -suid -ag -srd -cg mainDLLCompGroup -dr INSTALLDIR -out build\wix\subdirs\mainDLL.wxs -sw5150 -var var.SourceDir -t build\wix\only-dll.xslt' % \
            {'wix': wix_path,
             'distdir': dist_dir}
        print("Using Command: " + command)
        subprocess.check_call(command)

        command = '"%(wix)s\\candle.exe" -nologo -dWINLIBPATH=%(winlibpath)s -dPlatform=%(arch)s -dSourceDir=%(distdir)s -arch %(arch)s -out build\wix\subdirs\mainDLL.wixobj build\wix\subdirs\mainDLL.wxs' % \
            {'wix': wix_path,
             'winlibpath': build.winlib_path,
             'arch': winArch,
             'distdir': dist_dir}
        print("Using Command: " + command)
        subprocess.check_call(command)

        # Harvest main PDB from install dir if they exist
        isPdb = "no"
        if build.bundle_pdbs and glob.glob(os.path.join(dist_dir, "*.pdb")):
            isPdb = "yes"
            command = '"%(wix)s\\heat.exe" dir %(distdir)s -nologo -sfrag -suid -ag -srd -cg mainPDBCompGroup -dr INSTALLDIR -out build\wix\subdirs\mainPDB.wxs -sw5150 -var var.SourceDir -t build\wix\only-pdb.xslt' % \
            {'wix': wix_path,
             'distdir': dist_dir}
            print("Using Command: " + command)
            subprocess.check_call(command)

            command = '"%(wix)s\\candle.exe" -nologo -dWINLIBPATH=%(winlibpath)s -dPlatform=%(arch)s -dSourceDir=%(distdir)s -arch %(arch)s -out build\wix\subdirs\mainPDB.wixobj build\wix\subdirs\mainPDB.wxs' % \
            {'wix': wix_path,
             'winlibpath': build.winlib_path,
             'arch': winArch,
             'distdir': dist_dir}
            print("Using Command: " + command)
            subprocess.check_call(command)

        # Compile main wix files
        command = '"%(wix)s\\candle.exe" -nologo -dWINLIBPATH=%(winlibpath)s -dPlatform=%(arch)s -dImageformats=%(isimageformats)s -dPDB=%(isPDB)s -arch %(arch)s -out build\wix\mixxx.wixobj build\wix\mixxx.wxs' % \
            {'wix': wix_path,
             'winlibpath': build.winlib_path,
             'isimageformats': imageformats,
             'isPDB': isPdb,
             'arch': winArch}
        print("Using Command: " + command)
        subprocess.check_call(command)

        # Build package for default language
        print("*** Building package for default language " + defaultLanguage)
        command = '"%(wix)s\\light.exe" -cc .\ -nologo -sw1076 -spdb -ext WixUIExtension -cultures:%(deflang)s -loc build\wix\Localization\mixxx_%(deflang)s.wxl -out %(package_name)s build\wix\*.wixobj build\wix\subdirs\*.wixobj' % \
            {'wix': wix_path,
             'deflang': defaultLanguage,
             'package_name': 'part.' + msi_name}
        print("Using Command: " + command)
        subprocess.check_call(command)

        bundlelocfile = open("build/wix/bundle/bundleloc.wxs", "w")
        bundlelocfile.write("<?xml version='1.0' encoding='windows-1252'?>\n")
        bundlelocfile.write("<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>\n")
        bundlelocfile.write("    <Fragment Id='FragmentBundleLoc'>\n")
        bundlelocfile.write("        <PayloadGroup Id='BundleLoc'>\n")

        for file in glob.glob('build\wix\Localization\mixxx_*.wxl'):
            doc = minidom.parse(file)
            wixloc = doc.getElementsByTagName("WixLocalization")[0]
            culture = wixloc.getAttribute("Culture")
            strings = doc.getElementsByTagName("String")
            LCID = None
            for string in strings:
                if string.getAttribute('Id') == "Language":
                    LCID = string.firstChild.data
                    break

            if not LCID:
                print("LCID not found, skipping file " + file)
                continue

            bundlelocfile.write("            <Payload Id=\"thm-%(culture)s\" Compressed=\"yes\" Name=\"%(LCID)s\\thm.wxl\" SourceFile=\"..\\Localization\\mixxx_%(culture)s.wxl\" />\n" %\
              {'culture': culture,
               'LCID': LCID}
            )

            # Do not build localized MSI if it's default language
            if culture == defaultLanguage:
                continue

            print("*** Building package transform for locale %(culture)s LangID %(LCID)s" % \
                {'culture': culture,
                 'LCID': LCID})

            command = '"%(wix)s\\light.exe" -cc .\ -reusecab -nologo -sw1076 -spdb -ext WixUIExtension -cultures:%(lang)s,%(deflang)s -loc %(wxl_file)s -out %(lang)s.msi build\wix\*.wixobj build\wix\subdirs\*.wixobj' % \
                {'wix': wix_path,
                 'lang': culture,
                 'deflang': defaultLanguage,
                 'wxl_file': file}
            print("Using Command: " + command)
            subprocess.check_call(command)

            command = '"%(wix)s\\torch.exe" -nologo -p -t language %(package_name)s %(lang)s.msi -o %(lang)s.mst' % \
                {'wix': wix_path,
                 'lang': culture,
                 'package_name': 'part.' + msi_name}
            print("Using Command: " + command)
            subprocess.check_call(command)

            command = 'cscript "%(winsdk)s\wisubstg.vbs" %(package_name)s %(lang)s.mst %(langid)s' % \
                {'winsdk': WinSDK_path,
                 'lang': culture,
                 'package_name': 'part.' + msi_name,
                 'langid': LCID}
            print("Using Command: " + command)
            subprocess.check_call(command)

            langIds = langIds + "," + LCID
            os.remove(culture + ".msi")
            os.remove(culture + ".mst")

        print("*** Add all supported languages to MSI Package attribute")
        command = 'cscript "%(winsdk)s\WiLangId.vbs" %(package_name)s Package %(langid)s' % \
            {'winsdk': WinSDK_path,
             'package_name': 'part.' + msi_name,
             'langid': langIds}
        print("Using Command: " + command)
        subprocess.check_call(command)

        bundlelocfile.write("        </PayloadGroup>\n")
        bundlelocfile.write("    </Fragment>\n")
        bundlelocfile.write("</Wix>\n")
        bundlelocfile.close()

        # Everything is OK, now rename the msi to final name
        if os.path.isfile(msi_name):
            os.remove(msi_name)
        os.rename('part.' + msi_name, msi_name)

        print("*** Compiling Bundle")
        # Compile bundle wix file
        command = '"%(wix)s\\candle.exe" -ext WixUtilExtension -ext WixBalExtension -nologo -dWINLIBPATH=%(winlibpath)s -dPlatform=%(arch)s -dMSIPackage=%(package_name)s -arch %(arch)s -out build\\wix\\bundle\\bundle.wixobj build\\wix\\bundle\\bundle.wxs' % \
            {'wix': wix_path,
             'winlibpath': build.winlib_path,
             'arch': winArch,
             'package_name': msi_name}
        print("Using Command: " + command)
        subprocess.check_call(command)
        # bundle localisation references
        command = '"%(wix)s\\candle.exe" -ext WixUtilExtension -ext WixBalExtension -nologo -dWINLIBPATH=%(winlibpath)s -dPlatform=%(arch)s -dMSIPackage=%(package_name)s -arch %(arch)s -out build\\wix\\bundle\\bundleloc.wixobj build\\wix\\bundle\\bundleloc.wxs' % \
            {'wix': wix_path,
             'winlibpath': build.winlib_path,
             'arch': winArch,
             'package_name': msi_name}
        print("Using Command: " + command)
        subprocess.check_call(command)
        exe_name = os.path.splitext(msi_name)[0] + '.exe'
        command = '"%(wix)s\\light.exe" -cc .\ -nologo -sw1076 -spdb -ext WixUtilExtension -ext WixBalExtension -dMSIPackage=%(msi_name)s -cultures:%(deflang)s -loc build\wix\Localization\mixxx_%(deflang)s.wxl -out %(package_name)s build\\wix\\bundle\\*.wixobj' % \
            {'wix': wix_path,
             'deflang': defaultLanguage,
             'msi_name': msi_name,
             'package_name': exe_name}
        print("Using Command: " + command)
        subprocess.check_call(command)

        if 'sign' in COMMAND_LINE_TARGETS:
            from build.windows import signtool
            codesign_subject_name = SCons.ARGUMENTS.get('windows_codesign_subject_name', '')
            if not codesign_subject_name:
                raise Exception('Code-signing was requested but windows_codesign_subject_name was not provided.')

            print("*** Signing Bundle")
            # In addition to simply signing the installer executable, we have to
            # extract and sign the "burn engine". See
            # http://wixtoolset.org/documentation/manual/v3/overview/insignia.html for details.
            command = ("%(wix)s\\insignia.exe -ib %(package_name)s -o setup.exe" % {
                    "wix": wix_path,
                    "package_name": exe_name,
            })
            print("Using Command: " + command)
            subprocess.check_call(command)

            # Imperatively sign the file since the whole WiX process is imperative.
            signtool.signtool_path(codesign_subject_name, 'setup.exe')

            command = ("%(wix)s\\insignia.exe -ab setup.exe %(package_name)s -o %(package_name)s" % {
                    "wix": wix_path,
                    "package_name": exe_name,
            })
            print("Using Command: " + command)
            subprocess.check_call(command)

            # Now sign the final package imperatively.
            signtool.signtool_path(codesign_subject_name, exe_name)

        # Some cleaning before leaving
        for file in glob.glob('*.cab'):
            os.remove(file)
        for file in glob.glob('build\wix\*.wixobj'):
            os.remove(file)
        for file in glob.glob('build\wix\subdirs\*.wixobj'):
            os.remove(file)
        for file in glob.glob('build\wix\subdirs\*.wxs'):
            os.remove(file)
        os.remove(msi_name)

    else:
        print("Aborted building installer")

# Do release things
versionbld = Builder(action = BuildRelease, suffix = '.foo', src_suffix = '.bar')
env.Append(BUILDERS = {'BuildRelease' : versionbld})

if 'makerelease' in COMMAND_LINE_TARGETS:
        makerelease = env.BuildRelease('', binary_files)
        env.Alias('makerelease', makerelease)

def ubuntu_append_changelog(debian_dir,
                            package_name, package_version,
                            description,
                            distro='lucid',
                            urgency='low',
                            author="Mixxx Buildbot <builds@mixxx.org>"):
        now_formatted = time.strftime("%a,  %d %b %Y %H:%M:%S +0000", time.gmtime())
        new_entry = [
                "%s (%s) %s; urgency=%s" % (package_name, package_version, distro, urgency),
                "",
                description,
                "",
                " -- %s  %s" % (author, now_formatted),
                "",
                ]
        lines = []
        with open(os.path.join(debian_dir, 'changelog'), 'r') as changelog:
                lines = list(changelog)
        with open(os.path.join(debian_dir, 'changelog'), 'w') as changelog:
                changelog.writelines(["%s\n" % x for x in new_entry])
                changelog.writelines(lines)

def ubuntu_cleanup():
        os.system('rm -rf ubuntu')
        os.mkdir('ubuntu')

# Build the Ubuntu package
def BuildUbuntuPackage(target, source, env):
        global mixxx_version
        print("==== Mixxx Post-Build Checks ====")
        print("You have built version " + mixxx_version)
        print("\n\n")

        print("Top line of README, check version:")
        os.system('head -n 1 README')
        print()
        print("Top 2 lines of LICENSE, check version and copyright dates:")
        os.system('head -n 2 LICENSE')
        print()
        print("Top line of debian/ubuntu changelog, check version:")
        os.system('head -n 1 build/debian/changelog')
        print()

        print("Now building DEB package...")
        print()

        arch = 'amd64' if build.machine_is_64bit else 'i386'

        package_target = ARGUMENTS.get('package', None)
        ubuntu_distros = ARGUMENTS.get('ubuntu_dist', None)
        if ubuntu_distros is None:
                print("You did not specify an Ubuntu distribution to target. Specify one with the ubuntu_dist flag.")
                # TODO(XXX) default to their current distro? the .pbuilderrc does this
                return
        ubuntu_version = ARGUMENTS.get('ubuntu_version', '0ubuntu1')
        ubuntu_ppa = ARGUMENTS.get('ubuntu_ppa', None)

        ubuntu_distros = ubuntu_distros.split(',')

        # Big hack for beta PPA upload. We need LP to believe that our original
        # package version is always changing otherwise it will reject our orig
        # source tarball.
        if ubuntu_ppa and 'mixxxbetas' in ubuntu_ppa:
                mixxx_version = '%s-%s%s' % (mixxx_version, vcs_name, vcs_revision)

        # Destroy ubuntu/ and create it
        ubuntu_cleanup()

        package_name = 'mixxx'

        # directory and original tarball need to have the upstream-release
        # version, NOT the package version. For example:
        # upstream version: 1.10.0-beta1
        # package version: 1.10.0-beta1-0ubuntu1~bzr2206
        # directory name: mixxx-1.10.0-beta1
        # original tarball: mixxx_1.10.0-beta1.orig.tar.gz

        mixxx_dir = '%s-%s' % (package_name, mixxx_version)
        # The underscore is super important here to make the deb package work
        mixxx_tarball = "%s_%s.orig.tar.gz" % (package_name, mixxx_version)

        build_dir = os.path.join('ubuntu', mixxx_dir)

        if os.path.exists(build_dir):
            print("* Cleaning up %s (cwd: %s)" % (build_dir, os.getcwd()))
            print
            os.system('rm -rf %s' % build_dir) # be careful.

        # TODO: make a get flags arg to accept a revision which can override this and checkout of a specific SVN rev for the package

        # Export the source folder
        print("* Exporting source folder from current workspace (%s rev: %s)" % (vcs_name,
                                                                                 vcs_revision))
        print()
        util.export_source('.', build_dir)

        # Copy a patch to be included in the exported build sources (this can also be something like src/SConscript, /build/debian/rules)
        if os.path.exists('post-export-patch'):
            print("* Applying post export patch")
            print()
            os.system('cp --dereference -r post-export-patch/* %s' % build_dir)

        # Write a build.h to the exported directory. Later code looks for a
        # build.h in the mixxx/ directory and moves it to build.build_dir/
        # instead of generating.
        util.write_build_header(os.path.join(build_dir, 'build.h'))

        os.chdir('ubuntu')

        # Tar the source code
        print("* Tarring source directory to '%s' ... (this can take a couple minutes)" % os.path.join(os.getcwd(), mixxx_tarball))
        print()
        os.system('rm -f "%s"' % mixxx_tarball) #Remove old tarball
        os.system('tar --exclude build/debian --exclude=debian --exclude=debian/* -czf "%s" %s' % (mixxx_tarball, mixxx_dir))

        os.chdir(mixxx_dir)
        # Copy the debian folder from /build/debian to exported source folder root
        print("* Copying Debian build directory from build/debian to debian (cwd: %s)" % os.getcwd())
        print()
        os.system('cp -r build/debian .')
        os.system('cp res/linux/mixxx-usb-uaccess.rules ./debian/mixxx.mixxx-usb.udev')

        scons_flags = ' '.join([
                'optimize=portable',
                'virtualize=0',
                'mad=1',
                'localecompare=1',
                'qt_sqlite_plugin=0',
                'build=' + build.build,
        ])

        # Replace environment variables in the rules file.
        # TODO(rryan) something more elegant? I don't know a better way. When
        # Ubuntu build servers build us we don't get the chance to pass
        # environment variables in.
        with open('debian/rules', 'r') as fr:
                rules = fr.read()
                rules = rules.replace('MIXXX_SCONS_FLAGS = ""',
                                      'MIXXX_SCONS_FLAGS = %s' % scons_flags)

                with open('debian/rules', 'w') as fw:
                        fw.write(rules)

        for ubuntu_distro in ubuntu_distros:
                # if a control.$distro file exists, use it
                if os.path.exists('debian/control.%s' % ubuntu_distro):
                        os.system('cp debian/control.%s debian/control' % ubuntu_distro)
                package_version = ubuntu_construct_version(build, mixxx_version,
                                                           branch_name, vcs_revision,
                                                           ubuntu_version, ubuntu_distro)

                ubuntu_signing_identity = SCons.ARGUMENTS.get('ubuntu_signing_identity', "Mixxx Buildbot <builds@mixxx.org>")

                # Add a changelog record for this package
                if build.build_is_debug:
                        description = "  * Experimental build of branch '%s' at revision %s" % (branch_name, vcs_revision)
                        ubuntu_append_changelog('debian', package_name, package_version,
                                                description, distro=ubuntu_distro,
                                                author=ubuntu_signing_identity)
                else:
                        description = "  * New upstream release."
                        ubuntu_append_changelog('debian', package_name, package_version,
                                                description,
                                                distro=ubuntu_distro,
                                                author=ubuntu_signing_identity)

                # Run pbuilder
                print("* Starting pbuilder ...  (cwd: %s)" % os.getcwd())
                print()

                num_jobs = GetOption('num_jobs')
                command = ['MIXXX_SCONS_FLAGS="%s"' % scons_flags,
                           'ARCH=%s' % arch,
                           'DIST=%s' % ubuntu_distro,
                           # Pass the scons -jX option in in
                           # DEB_BUILD_OPTIONS. The Debian package rules file
                           # reads this option to set the -jX flag on the scons
                           # commands it runs. We disable Debian package
                           # optimizations via noopt because we handle our own
                           # optimization flags.
                           'DEB_BUILD_OPTIONS="noopt parallel=%s"' % num_jobs]


                if package_target == 'source':
                        command.extend(['debuild',
                                        # Preserve the MIXXX_SCONS_FLAGS and DEB_BUILD_OPTIONS
                                        # environment variable.
                                        '-eMIXXX_SCONS_FLAGS',
                                        '-eDEB_BUILD_OPTIONS',
                                        '-S', '-sa',])
                else:
                        command.extend(['pdebuild', '--', '--buildresult ./'])
                result = os.system(' '.join(command))

                source_changes_file = os.path.join(
                        '..', '%s_%s_source.changes' % (package_name, package_version))
                if package_target == 'source':
                        if result == 0 and os.path.exists(source_changes_file):
                            print("* Done! Signed source package is in ubuntu/")
                            print()
                        else:
                            print("* Build failed.")
                            print()
                            raise Exception('Ubuntu source package build/signing failed.')
                else:
                        result_file = "%s_%s_%s.deb" % (package_name, package_version, arch)

                        # Since we might build for multiple distros we need to
                        # insert the distro name into the filename.
                        # HACK(rryan): filenames for Ubuntu packaging in general
                        # are a big mess but we only distribute files in this
                        # code path (package_target != 'source') via
                        # downloads.mixxx.org so we may as well make the
                        # filenames match the Windows/OSX builds.
                        version = construct_version(build, mixxx_version,
                                                    branch_name, vcs_revision)
                        dest_filename = '-'.join((package_name, version,
                                                  ubuntu_distro, arch))
                        dest_deb_filename = "%s.deb" % dest_filename
                        # Also rename the source tarball.
                        dest_tar_filename = "%s.tar.gz" % dest_filename

                        # ubuntu/ is one folder up
                        dest_deb_file = os.path.join('..', dest_deb_filename)
                        dest_tar_file = os.path.join('..', dest_tar_filename)

                        source_tar_file = os.path.join('..', mixxx_tarball)
                        if os.path.exists(source_tar_file):
                            shutil.move(source_tar_file, dest_tar_file)

                        if result == 0 and os.path.exists(result_file):
                            print("* Found package '%s'. Copying to ubuntu/" % result_file)
                            print()
                            shutil.move(result_file, dest_deb_file)
                        else:
                            print("* Build failed.")
                            print()
                            raise Exception('Ubuntu package build failed.')

                # print("Signing the .deb changes file...")
                # os.system('sudo debsign /var/cache/pbuilder/result/*.changes')

                if ubuntu_ppa is not None:
                    # dput this changes file to the PPA
                    dput_command = 'dput %s %s' % (ubuntu_ppa, source_changes_file)
                    print("* Uploading package for", ubuntu_distro, "to launchpad:", dput_command)
                    result = os.system(dput_command)
                    if result == 0:
                         print("* Package upload succeeded.")
                    else:
                        print("* Package upload failed.")
                        print()
                        raise Exception('Ubuntu package upload failed.')

        # Return back to the starting directory, otherwise you'll get a .sconsign.dblite error!
        os.chdir('../..')
        print("* Returning to starting working directory ...  (cwd: " + os.getcwd() + ")")
        print()

#Build the Ubuntu package if "makeubuntu" was passed as an argument
versiondebbld = Builder(action = BuildUbuntuPackage) #, suffix = '.foo', src_suffix = '.bar')
env.Append(BUILDERS = {'BuildUbuntuPackage' : versiondebbld})

if 'makeubuntu' in COMMAND_LINE_TARGETS:
        makeubuntu = env.BuildUbuntuPackage("blah", "src/defs_version.h" ) #(binary_files)
        env.Alias('makeubuntu', makeubuntu)
