from SCons import Script
import os, sys, platform
import re

def get_bzr_revision():
    return os.popen("bzr revno").readline().strip()

def get_bzr_branch_name():
    output_lines = os.popen("bzr info").read().splitlines()

    matcher = re.compile(
        '\s*parent branch: http://bazaar.launchpad.net/(?P<owner>.*?)/mixxx/(?P<branch_name>.*?)/$')

    for line in output_lines:
        match = matcher.match(line)
        if match:
            match = match.groupdict()
            owner = match['owner']
            branch_name = match['branch_name']

            # Don't include the default name
            if owner == '~mixxxdevelopers':
                return branch_name
            return "%s_%s" % (owner.replace('~',''), branch_name)
    # Fall back on branch nick.
    return os.popen('bzr nick').readline().strip()


def get_build_dir(platformString, bitwidth):
    build_dir = '%s%s_build' % (platformString[0:3],bitwidth)
    return build_dir

def get_mixxx_version():
    """
    Parses defs.h to figure out the current Mixxx version.
    """
    #have to handle out-of-tree building, that's why the '#' :(
    defs = Script.File('#src/defs_version.h')

    for line in open(str(defs)).readlines():
        if line.strip().startswith("#define VERSION"):
            version = line
            break
    else:
        raise ValueError("Version not found")

    version = version.split()[-1].replace('"', '')
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
        if env.has_key(argflag):
            flags = env[argflag]
        else: #default value
            flags = default
    env[argflag] = flags
    return flags

# Checks for pkg-config on Linux
def CheckForPKGConfig( context, version='0.0.0' ):
    context.Message( "Checking for pkg-config (at least version %s)... " % version )
    ret = context.TryAction( "pkg-config --atleast-pkgconfig-version=%s" %version )[0]
    context.Result( ret )
    return ret

# Uses pkg-config to check for a minimum version
def CheckForPKG( context, name, version="" ):
    if version == "":
        context.Message( "Checking for %s... \t" % name )
        ret = context.TryAction( "pkg-config --exists '%s'" % name )[0]
    else:
        context.Message( "Checking for %s (%s or higher)... \t" % (name,version) )
        ret = context.TryAction( "pkg-config --atleast-version=%s '%s'" % (version,name) )[0]
        context.Result( ret )
    return ret
