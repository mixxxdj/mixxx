import os
import os.path
import string

import SCons.Action
import SCons.Builder
import SCons.Tool
import SCons.Util

# This is what we search for to find mingw:
prefixes = SCons.Util.Split("""
  i686-apple-darwin9-
  powerpc-apple-darwin9-
  x86_64-apple-darwin9-
""")

def find(env):
  for prefix in prefixes:
    # First search in the SCons path and then the OS path:
    if env.WhereIs(prefix + 'gcc') or SCons.Util.WhereIs(prefix + 'gcc'):
      return prefix

  return ''

def shlib_generator(target, source, env, for_signature):
  cmd = SCons.Util.CLVar(['$SHLINK', '$SHLINKFLAGS'])

  dll = env.FindIxes(target, 'SHLIBPREFIX', 'SHLIBSUFFIX')
  if dll: cmd.extend(['-o', dll])

  cmd.extend(['$SOURCES', '$_LIBDIRFLAGS', '$_LIBFLAGS'])

  implib = env.FindIxes(target, 'LIBPREFIX', 'LIBSUFFIX')
  if implib: cmd.append('-Wl,--out-implib,'+implib.get_string(for_signature))

  def_target = env.FindIxes(target, 'WIN32DEFPREFIX', 'WIN32DEFSUFFIX')
  if def_target: cmd.append('-Wl,--output-def,'+def_target.get_string(for_signature))

  return [cmd]

def shlib_emitter(target, source, env):
  dll = env.FindIxes(target, 'SHLIBPREFIX', 'SHLIBSUFFIX')
  no_import_lib = env.get('no_import_lib', 0)

  if not dll:
    raise SCons.Errors.UserError, "A shared library should have exactly one target with the suffix: %s" % env.subst("$SHLIBSUFFIX")

  if not no_import_lib and \
     not env.FindIxes(target, 'LIBPREFIX', 'LIBSUFFIX'):

    # Append an import library to the list of targets.
    target.append(env.ReplaceIxes(dll,
                    'SHLIBPREFIX', 'SHLIBSUFFIX',
                    'LIBPREFIX', 'LIBSUFFIX'))

  # Append a def file target if there isn't already a def file target
  # or a def file source. There is no option to disable def file
  # target emitting, because I can't figure out why someone would ever
  # want to turn it off.
  def_source = env.FindIxes(source, 'WIN32DEFPREFIX', 'WIN32DEFSUFFIX')
  def_target = env.FindIxes(target, 'WIN32DEFPREFIX', 'WIN32DEFSUFFIX')
  if not def_source and not def_target:
    target.append(env.ReplaceIxes(dll,
                  'SHLIBPREFIX', 'SHLIBSUFFIX',
                  'WIN32DEFPREFIX', 'WIN32DEFSUFFIX'))

  return (target, source)

# TODO: Backported to old scons version
#shlib_action = SCons.Action.CommandGenerator(shlib_generator)
shlib_action = SCons.Action.Action(shlib_generator,generator=1)

res_action = SCons.Action.Action('$RCCOM', '$RCCOMSTR')

res_builder = SCons.Builder.Builder(action=res_action, suffix='.res.o',
                  source_scanner=SCons.Tool.SourceFileScanner)
SCons.Tool.SourceFileScanner.add_scanner('.rc', SCons.Defaults.CScan)

def generate(env):
  gcc_prefix = find(env)

  if gcc_prefix:
    dir = os.path.dirname(env.WhereIs(gcc_prefix + 'gcc') or SCons.Util.WhereIs(gcc_prefix + 'gcc'))

    # The mingw bin directory must be added to the path:
    path = env['ENV'].get('PATH', [])
    if not path:
      path = []
    if SCons.Util.is_String(path):
      path = string.split(path, os.pathsep)

    env['ENV']['PATH'] = string.join([dir] + path, os.pathsep)

  # Most of mingw is the same as gcc and friends...
  gnu_tools = ['gcc', 'g++', 'gnulink', 'ar', 'gas']
  for tool in gnu_tools:
    SCons.Tool.Tool(tool)(env)

  #... but a few things differ:
  env['CC'] = gcc_prefix + 'gcc'
  env['SHCCFLAGS'] = SCons.Util.CLVar('$CCFLAGS')
  env['CXX'] = gcc_prefix + 'g++'
  env['SHCXXFLAGS'] = SCons.Util.CLVar('$CXXFLAGS')
  env['SHLINKFLAGS'] = SCons.Util.CLVar('$LINKFLAGS -shared')
  env['SHLINKCOM']   = shlib_action
  env.Append(SHLIBEMITTER = [shlib_emitter])
  # This line isn't required and breaks C++ linking
  #env['LINK'] = gcc_prefix + 'g++'
  env['AS'] = gcc_prefix + 'as'
  env['AR'] = gcc_prefix + 'ar'
  env['RANLIB'] = gcc_prefix + 'ranlib'
  env['WIN32DEFPREFIX']    = ''
  env['WIN32DEFSUFFIX']    = '.def'
  env['SHOBJSUFFIX'] = '.o'
  env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

  env['RC'] = gcc_prefix + 'windres'
  env['RCFLAGS'] = SCons.Util.CLVar('')
  env['RCINCFLAGS'] = '$( ${_concat(RCINCPREFIX, CPPPATH, RCINCSUFFIX, __env__, RDirs, TARGET)} $)'
  env['RCINCPREFIX'] = '--include-dir '
  env['RCINCSUFFIX'] = ''
  env['RCCOM'] = '$RC $RCINCFLAGS $RCINCPREFIX $SOURCE.dir $RCFLAGS -i $SOURCE -o $TARGET'
  #env['BUILDERS']['RES'] = res_builder

  # Some setting from the platform also have to be overridden:
  env['OBJPREFIX']    = ''
  env['OBJSUFFIX']    = '.o'
  env['LIBPREFIX']    = 'lib'
  env['LIBSUFFIX']    = '.a'
  env['SHOBJPREFIX']  = '$OBJPREFIX'
  env['SHOBJSUFFIX']  = '$OBJSUFFIX'
  env['PROGPREFIX']   = ''
  env['PROGSUFFIX']   = ''
  env['LIBPREFIX']    = 'lib'
  env['LIBSUFFIX']    = '.a'
  env['SHLIBPREFIX']  = 'lib'
  env['SHLIBSUFFIX']  = '.dylib'
  env['LIBPREFIXES']  = [ '$LIBPREFIX' ]
  env['LIBSUFFIXES']  = [ '$LIBSUFFIX' ]

def exists(env):
  return find(env)
