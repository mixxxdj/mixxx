
"""SCons.Tool.qt

Tool-specific initialization for Qt.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004 The SCons Foundation
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

__revision__ = "/home/scons/scons/branch.0/branch.96/baseline/src/engine/SCons/Tool/qt.py 0.96.92.D001 2006/04/10 23:13:27 knight"

import os.path
import re

import SCons.Action
import SCons.Builder
import SCons.Defaults
import SCons.Scanner
import SCons.Tool
import SCons.Util

class ToolQtWarning(SCons.Warnings.Warning):
	pass

class GeneratedMocFileNotIncluded(ToolQtWarning):
	pass

class QtdirNotFound(ToolQtWarning):
	pass

SCons.Warnings.enableWarningClass(ToolQtWarning)

qrcinclude_re = re.compile(r'<file>([^<]*)</file>', re.M)


header_extensions = [".h", ".hxx", ".hpp", ".hh"]
if SCons.Util.case_sensitive_suffixes('.h', '.H'):
	header_extensions.append('.H')
#cplusplus = __import__('c++', globals(), locals(), [])
#cxx_suffixes = cplusplus.CXXSuffixes
cxx_suffixes = [".c", ".cxx", ".cpp", ".cc"]

def checkMocIncluded(target, source, env):
	moc = target[0]
	cpp = source[0]
	# looks like cpp.includes is cleared before the build stage :-(
	# not really sure about the path transformations (moc.cwd? cpp.cwd?) :-/
	path = SCons.Defaults.CScan.path_function(env, moc.cwd)
	includes = SCons.Defaults.CScan(cpp, env, path)
	if not moc in includes:
		SCons.Warnings.warn(
			GeneratedMocFileNotIncluded,
			"Generated moc file '%s' is not included by '%s'" %
			(str(moc), str(cpp)))

def find_file(filename, paths, node_factory):
	retval = None
	for dir in paths:
		node = node_factory(filename, dir)
		if node.rexists():
			return node
	return None

class _Automoc:
	"""
	Callable class, which works as an emitter for Programs, SharedLibraries and
	StaticLibraries.
	"""

	def __init__(self, objBuilderName):
		self.objBuilderName = objBuilderName

	def __call__(self, target, source, env):
		"""
		Smart autoscan function. Gets the list of objects for the Program
		or Lib. Adds objects and builders for the special qt files.
		"""
		try:
			if int(env.subst('$QT4_AUTOSCAN')) == 0:
				return target, source
		except ValueError:
			pass
		try:
			debug = int(env.subst('$QT4_DEBUG'))
		except ValueError:
			debug = 0

		# some shortcuts used in the scanner
		FS = SCons.Node.FS.default_fs
		splitext = SCons.Util.splitext
		objBuilder = getattr(env, self.objBuilderName)

		# some regular expressions:
		# Q_OBJECT detection
		q_object_search = re.compile(r'[^A-Za-z0-9]Q_OBJECT[^A-Za-z0-9]')
		# cxx and c comment 'eater'
		#comment = re.compile(r'(//.*)|(/\*(([^*])|(\*[^/]))*\*/)')
		# CW: something must be wrong with the regexp. See also bug #998222
		#     CURRENTLY THERE IS NO TEST CASE FOR THAT

		# The following is kind of hacky to get builders working properly (FIXME)
		objBuilderEnv = objBuilder.env
		objBuilder.env = env
		mocBuilderEnv = env.Moc4.env
		env.Moc4.env = env

		# make a deep copy for the result; MocH objects will be appended
		out_sources = source[:]

		for obj in source:
			if isinstance(obj,basestring):  # big kludge!
				print "scons: qt4: '%s' MAYBE USING AN OLD SCONS VERSION AND NOT CONVERTED TO 'File'. Discarded." % str(obj)
				continue
			if not obj.has_builder():
				# binary obj file provided
				if debug:
					print "scons: qt: '%s' seems to be a binary. Discarded." % str(obj)
				continue
			cpp = obj.sources[0]
			if not splitext(str(cpp))[1] in cxx_suffixes:
				if debug:
					print "scons: qt: '%s' is no cxx file. Discarded." % str(cpp)
				# c or fortran source
				continue
			#cpp_contents = comment.sub('', cpp.get_contents())
			try:
				cpp_contents = cpp.get_contents()
			except: continue # may be an still not generated source
			h=None
			for h_ext in header_extensions:
				# try to find the header file in the corresponding source
				# directory
				hname = splitext(cpp.name)[0] + h_ext
				h = find_file(hname, (cpp.get_dir(),), env.File)
				if h:
					if debug:
						print "scons: qt: Scanning '%s' (header of '%s')" % (str(h), str(cpp))
					#h_contents = comment.sub('', h.get_contents())
					h_contents = h.get_contents()
					break
			if not h and debug:
				print "scons: qt: no header for '%s'." % (str(cpp))
			if h and q_object_search.search(h_contents):
				# h file with the Q_OBJECT macro found -> add moc_cpp
				moc_cpp = env.Moc4(h)
				moc_o = objBuilder(moc_cpp)
				out_sources.append(moc_o)
				#moc_cpp.target_scanner = SCons.Defaults.CScan
				if debug:
					print "scons: qt: found Q_OBJECT macro in '%s', moc'ing to '%s'" % (str(h), str(moc_cpp))
			if cpp and q_object_search.search(cpp_contents):
				# cpp file with Q_OBJECT macro found -> add moc
				# (to be included in cpp)
				moc = env.Moc4(cpp)
				env.Ignore(moc, moc)
				if debug:
					print "scons: qt: found Q_OBJECT macro in '%s', moc'ing to '%s'" % (str(cpp), str(moc))
				#moc.source_scanner = SCons.Defaults.CScan
		# restore the original env attributes (FIXME)
		objBuilder.env = objBuilderEnv
		env.Moc4.env = mocBuilderEnv

		return (target, out_sources)

AutomocShared = _Automoc('SharedObject')
AutomocStatic = _Automoc('StaticObject')

def _detect(env):
	"""Not really safe, but fast method to detect the QT library"""

	QTDIR = env.get('QTDIR',None)
	if QTDIR!=None : return QTDIR

	QTDIR = os.environ.get('QTDIR',None)
	if QTDIR!=None : return QTDIR

	moc = env.WhereIs('moc-qt4') or env.WhereIs('moc4') or env.WhereIs('moc')
	if moc:
		SCons.Warnings.warn(
			QtdirNotFound,
			"QTDIR variable is not defined, using moc executable as a hint (QTDIR=%s)" % QTDIR)
		return os.path.dirname(os.path.dirname(moc))

	SCons.Warnings.warn(
		QtdirNotFound,
		"Could not detect qt, using empty QTDIR")
	return None

def generate(env):
	"""Add Builders and construction variables for qt to an Environment."""

	print "Loading qt4 tool..."

	def locateQt4Command(env, command, qtdir) :
		fullpath1 = os.path.join(qtdir,'bin',command +'-qt4')
		if os.access(fullpath1, os.X_OK) or \
			os.access(fullpath1+".exe", os.X_OK):
			return fullpath1
		fullpath3 = os.path.join(qtdir,'bin',command +'4')
		if os.access(fullpath3, os.X_OK) or \
			os.access(fullpath3+".exe", os.X_OK):
			return fullpath3
		fullpath2 = os.path.join(qtdir,'bin',command)
		if os.access(fullpath2, os.X_OK) or \
			os.access(fullpath2+".exe", os.X_OK):
			return fullpath2
		fullpath = env.Detect([command+'-qt4', command+'4', command])
		if not (fullpath is None) : return fullpath
		raise Exception("Qt4 command '" + command + "' not found. Tried: " + fullpath1 + " and "+ fullpath2)


	CLVar = SCons.Util.CLVar
	Action = SCons.Action.Action
	Builder = SCons.Builder.Builder
	splitext = SCons.Util.splitext

	env['QTDIR']  = _detect(env)
	# TODO: 'Replace' should be 'SetDefault'
#	env.SetDefault(
	env.Replace(
#		QTDIR  = _detect(env),
		QT4_BINPATH = os.path.join('$QTDIR', 'bin'),
		QT4_CPPPATH = os.path.join('$QTDIR', 'include'),
		QT4_LIBPATH = os.path.join('$QTDIR', 'lib'),
		# TODO: This is not reliable to QTDIR value changes but needed in order to support '-qt4' variants
		QT4_MOC = locateQt4Command(env,'moc', env['QTDIR']),
		QT4_UIC = locateQt4Command(env,'uic', env['QTDIR']),
		QT4_RCC = locateQt4Command(env,'rcc', env['QTDIR']),
		QT4_LUPDATE = locateQt4Command(env,'lupdate', env['QTDIR']),
		QT4_LRELEASE = locateQt4Command(env,'lrelease', env['QTDIR']),
		QT4_LIB = '', # KLUDGE to avoid linking qt3 library

		QT4_AUTOSCAN = 1, # Should the qt tool try to figure out, which sources are to be moc'ed?

		# Some QT specific flags. I don't expect someone wants to
		# manipulate those ...
		QT4_UICFLAGS = CLVar(''),
		QT4_MOCFROMHFLAGS = CLVar(''),
		QT4_MOCFROMCXXFLAGS = CLVar('-i'),
		QT4_QRCFLAGS = '',

		# suffixes/prefixes for the headers / sources to generate
		QT4_UISUFFIX = '.ui',
		QT4_UICDECLPREFIX = 'ui_',
		QT4_UICDECLSUFFIX = '.h',
		QT4_MOCHPREFIX = 'moc_',
		QT4_MOCHSUFFIX = '$CXXFILESUFFIX',
		QT4_MOCCXXPREFIX = 'moc_',
		QT4_MOCCXXSUFFIX = '.moc',
		QT4_QRCSUFFIX = '.qrc',
		QT4_QRCCXXSUFFIX = '$CXXFILESUFFIX',
		QT4_QRCCXXPREFIX = 'qrc_',

		# Commands for the qt support ...
		QT4_UICCOM = '$QT4_UIC $QT4_UICFLAGS -o $TARGET $SOURCE',
		QT4_MOCFROMHCOM = '$QT4_MOC $QT4_MOCFROMHFLAGS -o $TARGET $SOURCE',
		QT4_MOCFROMCXXCOM = [
			'$QT4_MOC $QT4_MOCFROMCXXFLAGS -o $TARGET $SOURCE',
			Action(checkMocIncluded,None)],
		QT4_LUPDATECOM = '$QT4_LUPDATE $SOURCE -ts $TARGET',
		QT4_LRELEASECOM = '$QT4_LRELEASE $SOURCE',
		QT4_RCCCOM = '$QT4_RCC $QT4_QRCFLAGS $SOURCE -o $TARGET',
		)

	# Translation builder
	tsbuilder = Builder(
		action = SCons.Action.Action('$QT4_LUPDATECOM', '$QT4_LUPDATECOMSTR'),
		multi=1
		)
	env.Append( BUILDERS = { 'Ts': tsbuilder } )
	qmbuilder = Builder(
		action = SCons.Action.Action('$QT4_LRELEASECOM', '$QT4_LRELEASECOMSTR'),
		src_suffix = '.ts',
		suffix = '.qm',
		single_source = True
		)
	env.Append( BUILDERS = { 'Qm': qmbuilder } )

	# Resource builder
	def scanResources(node, env, path, arg):
		contents = node.get_contents()
		includes = qrcinclude_re.findall(contents)
		return includes
	qrcscanner = SCons.Scanner.Scanner(name = 'qrcfile',
		function = scanResources,
		argument = None,
		skeys = ['.qrc'])
	qrcbuilder = Builder(
		action = SCons.Action.Action('$QT4_RCCCOM', '$QT4_RCCCOMSTR'),
		source_scanner = qrcscanner,
		src_suffix = '$QT4_QRCSUFFIX',
		suffix = '$QT4_QRCCXXSUFFIX',
		prefix = '$QT4_QRCCXXPREFIX',
		single_source = True
		)
	env.Append( BUILDERS = { 'Qrc': qrcbuilder } )

	# Interface builder
	uic4builder = Builder(
		action = SCons.Action.Action('$QT4_UICCOM', '$QT4_UICCOMSTR'),
		src_suffix='$QT4_UISUFFIX',
		suffix='$QT4_UICDECLSUFFIX',
		prefix='$QT4_UICDECLPREFIX',
		single_source = True
		#TODO: Consider the uiscanner on new scons version
		)
	env.Append( BUILDERS = { 'Uic4': uic4builder } )

	# Metaobject builder
	mocBld = Builder(action={}, prefix={}, suffix={})
	for h in header_extensions:
		act = SCons.Action.Action('$QT4_MOCFROMHCOM', '$QT4_MOCFROMHCOMSTR')
		mocBld.add_action(h, act)
		mocBld.prefix[h] = '$QT4_MOCHPREFIX'
		mocBld.suffix[h] = '$QT4_MOCHSUFFIX'
	for cxx in cxx_suffixes:
		act = SCons.Action.Action('$QT4_MOCFROMCXXCOM', '$QT4_MOCFROMCXXCOMSTR')
		mocBld.add_action(cxx, act)
		mocBld.prefix[cxx] = '$QT4_MOCCXXPREFIX'
		mocBld.suffix[cxx] = '$QT4_MOCCXXSUFFIX'
	env.Append( BUILDERS = { 'Moc4': mocBld } )

	# er... no idea what that was for
	static_obj, shared_obj = SCons.Tool.createObjBuilders(env)
	static_obj.src_builder.append('Uic4')
	shared_obj.src_builder.append('Uic4')

	# We use the emitters of Program / StaticLibrary / SharedLibrary
	# to scan for moc'able files
	# We can't refer to the builders directly, we have to fetch them
	# as Environment attributes because that sets them up to be called
	# correctly later by our emitter.
	env.AppendUnique(PROGEMITTER =[AutomocStatic],
					 SHLIBEMITTER=[AutomocShared],
					 LIBEMITTER  =[AutomocStatic],
					 # Of course, we need to link against the qt libraries
					 CPPPATH=["$QT4_CPPPATH"],
					 LIBPATH=["$QT4_LIBPATH"],
					 LIBS=['$QT4_LIB'])

	import new
	method = new.instancemethod(enable_modules, env, SCons.Environment)
	env.EnableQt4Modules=method

def enable_modules(self, modules, debug=False) :
	import sys

	validModules = [
		'QtCore',
		'QtGui',
		'QtOpenGL',
		'Qt3Support',
		# The next modules have not been tested yet so, please
		# maybe they require additional work on non Linux platforms
		'QtSql',
		'QtNetwork',
		'QtSvg',
		'QtTest',
		'QtXml',
		'QtUiTools',
		'QtDesigner',
		'QtDBUS',
		'QtScript',
		'QtWebKit',
		'QtXmlPatterns'
		]
	pclessModules = [
		'QtUiTools',
		'QtDesigner',
	]
	staticModules = [
		'QtUiTools',
	]
	invalidModules=[]
	for module in modules:
		if module not in validModules :
			invalidModules.append(module)
	if invalidModules :
		raise Exception("Modules %s are not Qt4 modules. Valid Qt4 modules are: %s"% \
			(str(invalidModules),str(validModules)))

	# TODO: Check whether we should add QT_CORE_LIB, QT_XML_LIB, QT_NETWORK_LIB...
	if 'QtGui' in modules:
		self.AppendUnique(CPPFLAGS=['-DQT_GUI_LIB'])

	debugSuffix = ''
	if sys.platform in ["linux2", "linux3"]:
		if debug : debugSuffix = '_debug'
		for module in modules :
			if module not in pclessModules : continue
			self.AppendUnique(LIBS=[module+debugSuffix]) # TODO: Add the debug suffix
			self.AppendUnique(LIBPATH=[os.path.join("$QTDIR","lib")])
			self.AppendUnique(CPPPATH=[os.path.join("$QTDIR","include","qt4")])
			self.AppendUnique(CPPPATH=[os.path.join("$QTDIR","include","qt4",module)])
		pcmodules = [module+debugSuffix for module in modules if module not in pclessModules ]
		self.ParseConfig('pkg-config %s --libs --cflags'% ' '.join(pcmodules))
		return
	if sys.platform == "win32" :
		if debug : debugSuffix = 'd'
		self.AppendUnique(LIBS=[lib+'4'+debugSuffix for lib in modules if lib not in staticModules])
		self.AppendUnique(LIBS=[lib+debugSuffix for lib in modules if lib in staticModules])
		if 'QtOpenGL' in modules:
			self.AppendUnique(LIBS=['opengl32'])
		self.AppendUnique(CPPPATH=[ '$QTDIR/include/'+module
			for module in modules])
		self.AppendUnique(LIBPATH=[os.path.join('$QTDIR','lib')])
		return
	if sys.platform=="darwin" :
		# TODO: Test debug version on Mac
		self.AppendUnique(LIBPATH=[os.path.join('$QTDIR','lib')])
		self.AppendUnique(LINKFLAGS="-F$QTDIR/lib")
		self.AppendUnique(LINKFLAGS="-L$QTDIR/lib") #TODO clean!
		if debug : debugSuffix = 'd'
		for module in modules :
			self.AppendUnique(CPPPATH=[os.path.join("$QTDIR","include")])
			self.AppendUnique(CPPPATH=[os.path.join("$QTDIR","include",module)])
			if module in staticModules :
				self.AppendUnique(LIBS=[module+debugSuffix]) # TODO: Add the debug suffix
				self.AppendUnique(LIBPATH=[os.path.join("$QTDIR","lib")])
			else :
				self.Append(LINKFLAGS=['-framework', module])
		if 'QtOpenGL' in modules:
			self.AppendUnique(LINKFLAGS="-F/System/Library/Frameworks")
			self.Append(LINKFLAGS=['-framework', 'AGL']) #TODO ughly kludge to avoid quotes
			self.Append(LINKFLAGS=['-framework', 'OpenGL'])
		return
# This should work for mac but doesn't
#	env.AppendUnique(FRAMEWORKPATH=[os.path.join(env['QTDIR'],'lib')])
#	env.AppendUnique(FRAMEWORKS=['QtCore','QtGui','QtOpenGL', 'AGL'])


def exists(env):
	return _detect(env)



