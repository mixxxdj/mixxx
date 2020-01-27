from SCons import Script
import os
import os.path
import re
import stat

CURRENT_VCS = None


def get_current_vcs():
    if CURRENT_VCS is not None:
        return CURRENT_VCS
    if on_git():
        return "git"
    return "tar"


def on_git():
    cwd = os.getcwd()
    basename = " "
    while len(basename) > 0:
        try:
            os.stat(os.path.join(cwd, ".git"))
            return True
        except:
            pass
        cwd, basename = os.path.split(cwd)
    return False


def get_revision():
    global CURRENT_VCS
    if CURRENT_VCS is None:
        CURRENT_VCS = get_current_vcs()
    if CURRENT_VCS == "git":
        return get_git_revision()
    if CURRENT_VCS == "tar":
        return ""
    return None


def get_modified():
    global CURRENT_VCS
    if CURRENT_VCS is None:
        CURRENT_VCS = get_current_vcs()
    if CURRENT_VCS == "git":
        return get_git_modified()
    if CURRENT_VCS == "tar":
        return ""
    return None


def get_branch_name():
    global CURRENT_VCS
    if CURRENT_VCS is None:
        CURRENT_VCS = get_current_vcs()
    if CURRENT_VCS == "git":
        return get_git_branch_name()
    if CURRENT_VCS == "tar":
        return ""
    return None


def export_source(source, dest):
    global CURRENT_VCS
    if CURRENT_VCS is None:
        CURRENT_VCS = get_current_vcs()
    if CURRENT_VCS == "git":
        return export_git(source, dest)
    return None


def get_git_revision():
    return len(os.popen("git log --pretty=oneline --first-parent").read().splitlines())


def get_git_modified():
    modified_matcher = re.compile(
        "^#.*:   (?P<filename>.*?)$")  # note output might be translated
    modified_files = []
    for line in os.popen("git status").read().splitlines():
        match = modified_matcher.match(line)
        if match:
            match = match.groupdict()
            modified_files.append(match['filename'].strip())
    return "\n".join(modified_files)


def get_git_branch_name():
    # this returns the branch name or 'HEAD' in case of detached HEAD
    branch_name = os.popen(
        "git rev-parse --abbrev-ref HEAD").readline().strip()
    if branch_name == 'HEAD':
        # Use APPVEYOR_REPO_BRANCH variable if building on appveyor or (no branch) if unset
        branch_name = os.getenv("APPVEYOR_REPO_BRANCH", '(no branch)')
    # Add PR# to branch name if building a PR in appveyor to avoid package naming collision
    PRnum = os.getenv("APPVEYOR_PULL_REQUEST_NUMBER")
    if PRnum != None:
        branch_name += ("-PR" + PRnum)
    return branch_name


def export_git(source, dest):
    os.mkdir(dest)
    return os.system('git archive --format tar HEAD %s | tar x -C %s' % (source, dest))


def get_build_dir(platformString, bitwidth):
    build_dir = '%s%s_build' % (platformString[0:3], bitwidth)
    return build_dir


def get_mixxx_version():
    """Get Mixxx version number from defs_version.h"""
    # have to handle out-of-tree building, that's why the '#' :(
    defs = Script.File('#src/_version.h')
    version = ""

    for line in open(str(defs)).readlines():
        if line.strip().startswith("#define MIXXX_VERSION "):
            version = line
            break

    if version == "":
        raise ValueError("Version not found")

    version = version.split()[-1].replace('"', '')

    # Check if version respect constraints
    # 3 numbers separated by a dot, then maybe a (dash or tilde) and some string
    # See src/defs_version.h comment
    versionMask = '^\d+\.\d+\.\d+([-~].+)?$'
    if not re.match(versionMask, version):
        raise ValueError("Version format mismatch. See src/defs_version.h comment")

    return version


def get_flags(env, argflag, default=0):
    """
    * get value passed as an argument to scons as argflag=value
    * if no value is passed to scons use stored value
    * if no value is stored, use default
    Returns the value and stores it in env[argflag]
    """
    flags = Script.ARGUMENTS.get(argflag, -1)
    if int(flags) < 0:
        if argflag in env:
            flags = env[argflag]
        else:  # default value
            flags = default
    env[argflag] = flags
    return flags


# Checks for pkg-config on Linux
def CheckForPKGConfig(context, version='0.0.0'):
    context.Message(
        "Checking for pkg-config (at least version %s)... " % version)
    ret = context.TryAction(
        "pkg-config --atleast-pkgconfig-version=%s" % version)[0]
    context.Result(ret)
    return ret


# Uses pkg-config to check for a minimum version
def CheckForPKG(context, name, version=""):
    if version == "":
        context.Message("Checking for %s... \t" % name)
        ret = context.TryAction("pkg-config --exists '%s'" % name)[0]
    else:
        context.Message(
            "Checking for %s (%s or higher)... \t" % (name, version))
        ret = context.TryAction(
            "pkg-config --atleast-version=%s '%s'" % (version, name))[0]
        context.Result(ret)
    return ret


def write_build_header(path):
    f = open(path, 'w')
    try:
        branch_name = get_branch_name()
        modified = len(get_modified()) > 0
        # Do not emit BUILD_BRANCH on release branches.
        if branch_name and not branch_name.startswith('release'):
            f.write('#define BUILD_BRANCH "%s"\n' % branch_name)
        f.write('#define BUILD_REV "%s%s"\n' % (get_revision(),
                                                '+' if modified else ''))
    finally:
        f.close()
        os.chmod(path, stat.S_IRWXU | stat.S_IRWXG |stat.S_IRWXO)

def get_osx_min_version():
    """Gets the minimum required OS X version from product_definition.plist."""
    # Mixxx 2.0 supported OS X 10.6 and up.
    # Mixxx >2.0 requires C++11 which is only available with libc++ and OS X
    # 10.7 onwards. std::promise/std::future requires OS X 10.8.
    # Mixxx >2.2 switched to Qt 5, which requires macOS 10.11.
    # Use SCons to get the path relative to the repository root.
    product_definition = str(Script.File('#build/osx/product_definition.plist'))
    p = os.popen("/usr/libexec/PlistBuddy -c 'Print os:0' %s" % product_definition)
    min_version = p.readline().strip()
    result_code = p.close()
    assert result_code is None, "Can't read macOS min version: %s" % min_version
    return min_version


def find_d3dcompiler_dll(env):
    """Returns the path to d3dcompiler_xx.dll for bundling with Mixxx."""
    # On our Windows 7 build environment, d3dcompiler_xx.dll lives next to our
    # cl.exe for MSVC 14.0.
    #
    # https://code.woboq.org/qt5/qttools/src/shared/winutils/utils.cpp.html#924
    # windeployqt checks for d3dcompiler_xx.dll in:
    #
    # - The %SDK%/redist/D3D folder (Windows 8 SDK and above).
    # - The Qt SDK bin folder (Not present for our builds of Qt).
    # - The first folder in the %PATH% containing a file matching the pattern.
    #
    # Since we currently use the Windows 7 SDK, and the Qt SDK folder does not
    # have this DLL, let's search the path.
    paths = env['ENV']['PATH'].split(';')
    for path in paths:
        for version in range(47, 40, -1):
            dll_path = os.path.join(path, 'd3dcompiler_%d.dll' % version)
            if os.path.exists(dll_path):
                return dll_path
    return None
