"""OSConsX.py - scons support for building applications on OS X using SCons.
Functions to build .app bundles and .dmg images, and to use otool(1) to trace out the needed libraries.

usage:

env = Environment(tools = ['OSConsX', toolpath=['path/to/osconsx'])
env.App('

By <nick@kousu.ca> January 16th 2009
License: 2-clause BSD (XXX put a proper notice here)


Please email me with questions/comments/patches!
"""



import sys, os, shutil
import SCons
from SCons.Builder import Builder
from SCons.Script import *
import otool

#Dev info:
#http://www.scons.org/wiki/MacOSX (not very featureful, but the tip about resource forks might be important (wait did I say important? I meant out of date. see CpMac(1)) 


#oooh, you can use warnings! just call "warn()"

def system(s):
	"wrap system() to give us feedback on what it's doing"
	"anything using this call should be fixed to use SCons's declarative style (once you figure that out, right nick?)"
	print s,
	sys.stdout.flush() #ignore line buffering..
	result = os.system(s)
	print
	return result
	
	

def no_sources(target, source, env):
	"an emitter that forces null sources, so that we don't need to have the user explicitly say there are no dependencies (SCons assumes that if you are building 'X.out' then you need 'X.in')"
	return target, [] 

	

def InstallDir(target, source, env): #XXX this belongs not in this module
	"copies the given source dir inside of the given target dir"
	#XXX could be rewritten better with schemey-recursion as "if source is File: env.Install(), elif source is Dir, scan the dir and recurse"
	#SCons doesn't really like using directories as targets. Like, at all.
	#Mkdir(os.path.join(str(target), str(source)))
	#translate install(a/, b/) to install(a/b/, [files in b])
	contents = Glob(os.path.join(str(source), "*")) #XXX there's probably a cleaner way that SCons has to do this
	#print "contents:",contents
	files = filter(lambda f: isinstance(f, SCons.Node.FS.File), contents)
	folders = filter(lambda f: isinstance(f, SCons.Node.FS.Dir), contents)
	#print map(str, folders)
	
	nodes = env.Install(Dir(os.path.join(str(target), str(source))), files)
	for f in folders:
		nodes+=InstallDir(target, f, env)
	return nodes

#okay, this works. It could be done better (make better use of SCons's declarativity, look at http://frungy.org/~tpot/weblog/2008/05/02#scons-rpm2 for ideas)
#Specifically, this does file copying by itself, instead of telling SCons about it.
#On the other hand, the files it is copying are not really part of the build process, they are tmp files, so maybe it works....
#BUG: scons doesn't track that it's built the .dmg. It decides it needs to build it every time "because it doesn't exist". Perhaps has to do with the lack s
def build_dmg(target, source, env):
	"takes the given source files, makes a temporary directory, copies them all there, and then packages that directory into a .dmg"
	#since we are building into a single .dmg, coerce target to point at the actual name 
	assert len(target) == 1 
	target = target[0]
	
	if os.path.exists(str(target)+".dmg"): #huhh? why do I have to say +.dmg here? I thought scons was supposed to handle that shit
		raise Exception(".dmg target already exists.")
	
	#if 'DMG_DIR' in env: .... etc fill me in please
	dmg = os.tmpnam()+"-"+env['VOLNAME'].strip()+"-dmg" #create a directory to build the .dmg root from
	
	#is os.system the best thing for this? Can't we declare that these files need to be moved somehow?
	#aah there must be a more SCons-ey (i.e. declarative) way to do all this; the trouble with doing
	os.mkdir(dmg)
	for f in source:
		print "Copying",f
		a, b = str(f), os.path.join(dmg, str(f.path))
		if isinstance(f, SCons.Node.FS.Dir): #XXX there's a lot of cases that could throw this off, particularly if you try to pass in subdirs
			copier = shutil.copytree
		elif isinstance(f, SCons.Node.FS.File):
			copier = shutil.copy
		else:
			raise Exception("%s is neither Dir nor File node? Bailing out." % f) 
		
		try:
			copier(a, b)
		except Exception, e:
			print "ERRRR", e
			raise Exception("Error copying %s: " % (a,), e)
	
	if env['ICON']:
		#XXX this doesn't seem to work, at least not on MacOS 10.5
		#the MacFUSE people have solved it, though, see "._" in http://www.google.com/codesearch/p?hl=en#OXKFx3-7cSY/tags/macfuse-1.0.0/filesystems-objc/FUSEObjC/FUSEFileSystem.m&q=volumeicon
		#appearently it requires making a special volume header file named "._$VOLNAME" with a binary blob in it
		#But also the Qt4 dmg has a working icon, and it has no ._$VOLNAME file 
		shutil.copy(env['ICON'], os.path.join(dmg, ".VolumeIcon.icns")) #XXX bug: will crash if not given an icon file
		system('SetFile -a C "%s"' % dmg) #is there an sconsey way to declare this? Would be nice so that it could write what
	
	
	if system("hdiutil create -srcfolder %s -format UDBZ -volname %s %s" % (dmg, env['VOLNAME'], target)):
		raise Exception("hdiutil create failed")
	
	shutil.rmtree(dmg)

Dmg = Builder(action = build_dmg, suffix=".dmg")

class Bundle(SCons.Node.Node):
	"until SCons gets its shit together and is able to handle having directories as targets, we use this"
	def __init__(self, path):
		path = str(path) #decast the object from being a File or a Dir
		self.path = path
		SCons.Node.Node.__init__(self)
		self.clear()
		assert self.path == path, "Node constructor overwrote .path :("
	def __str__(self):
		return self.path
	def __repr__(self):
		return 'Bundle("%s")' % self.path



def write_file(target, source, env):
	data = env['DATA']
	for t in target:
		f = open(str(t), "wb")
		f.write(data)
		f.close()


#should be in a different module, really
Writer = Builder(action = write_file, emitter = no_sources)





def build_app(target, source, env, ICON = None):
	#TODO: make it strip(1) the installed binary (saves about 1Mb)
	
	#EEEP: this code is pretty flakey because I can't figure out how to force; have asked the scons list about it
	
	
	bundle = target[0]
	binary = source[0]
	
	#this is copied from emit_app, which is unfortunate
	contents = Dir(os.path.join(str(bundle), "Contents"))
	MacOS = Dir(os.path.join(str(contents), "MacOS/"))
	frameworks = Dir(os.path.join(str(contents), "Frameworks")) #we put both frameworks and standard unix sharedlibs in here
	
	#installed_bin = source[-1] #env['APP_INSTALLED_BIN']
	installed_bin = os.path.join(str(MacOS), os.path.basename(str(binary)))  #have use os.path.basename in case there were other dirs tacked onto the front (env.Install() normally handles this for us, but since we are working around that we have to do it ourself)
	
	
	"todo: expose the ability to override the list of System dirs"
	#ugh, I really don't like this... I wish I could package it up nicer. I could use a Builder but then I would have to pass in to the builder installed_bin which seems backwards since 
	
	embed_deps = otool.embed_dependencies(str(binary))
	locals = [os.path.basename(a) for o,a in embed_deps] #precache the list of names of libs we are using so we can figure out if a lib is local or not (and therefore a ref to it needs to be updated) #XXX it seems kind of wrong to only look at the basename (even if, by the nature of libraries, that must be enough) but there is no easy way to compute the abspath
	
	
	#it would be nice if we could have the libs installed by SCons, but since what specific libs are needed depend on the binary already having been built (so that we can run otool against it) that kind of fails; the other option is to try to look at the build flags we are passing and guess what libs will be linked against but that seems like an even worse barrel of GIANT COBRAS
	
	#XXX stripping should be optional
	#This doesn't handle Frameworks correctly, only .dylibs
	#useful to know: http://developer.apple.com/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/FrameworkAnatomy.html#//apple_ref/doc/uid/20002253
 	#^ so you do have to copy in and _entire_ framework to be sure...
	#What we should really do is: record somewhere a hash keyed by the library's name, so you can go "this library is going to be embedded at this location"
	
	
	#Workarounds for a bug/feature in SCons such that it doesn't neccessarily run the source builders before the target builders (wtf scons??)
	Execute(Mkdir(contents))
	Execute(Mkdir(MacOS))
	Execute(Mkdir(frameworks))
	print "INSTALLING '%s' AS '%s'" % (binary, installed_bin)
	shutil.copy(str(binary), installed_bin) #e.g. this SHOULD be an env.Install() call, but if scons decides to run build_app before that env.Install then build_app fails and brings the rest of the build with it, of course
	system("strip '%s'" % installed_bin)
	
	for orig, abs in embed_deps:
		name = os.path.basename(abs)
		relative = os.path.join("@executable_path/../Frameworks/", name)
		embedded = os.path.join(str(frameworks), name)
		shutil.copy(abs, embedded)
		system("strip -S '%s' 2>/dev/null" % embedded), #(the stripping fails with ""symbols referenced by relocation entries that can't be stripped"" for some obscure Apple-only reason sometimes, related to their hacks to gcc---it depends on how the file was compiled; since we don't /really/ care about this we just let it silently fail)
		system("install_name_tool -change '%s' '%s' '%s'" % (orig, relative, installed_bin)) #change the reference to the library in the binary
		system("install_name_tool -id '%s' '%s'" % (relative, embedded)) #change the name the library knows itself as
		for d_dep in otool.otool(abs):
			if os.path.basename(d_dep) in locals:
				system("install_name_tool -change '%s' '%s' '%s'" % (d_dep, os.path.join("@executable_path/../Frameworks/", os.path.basename(d_dep)), embedded))
	
	
def emit_app(target, source, env):
	"""The first source is the binary program file, the rest are files/folders to include in the App's Resources directory.
	
	extra variables available:
	ICON - the filename of the icon file to use, used in package metadata. If not specified defaults to 'application.icns' (which should be in your sources list, but if you spec a file not in there it'll actually add it for you)
	SIGNATURE - the bundle signature, a four byte code. If not specified uses the first four characters of the bundle name. 
	PLUGINS - a list of files/folders to place in Contents/PlugIns (Adium uses Contents/PlugIns, Audacity uses plug-ins AND Contents/plug-ins..., Apple Mail uses Contents/PlugIns, so that's what we should stick with)
	"""
	
	#bah, unless we decide to change the interface so you pass the app icon in as a separate param, then we *have* to change Mixxx to work properly with the Resources/ dir	
	
	
	assert len(target) == 1
	bundle = target[0]
	#pull the binary off the front since it's a special case
	binary, resources = source[0], source[1:]
	try:
		icon = env['ICON']
	except KeyError:
		icon = "application.icns"
	
	#so, this doesn't work realistically because if the passed in icon is a remote path then shit clashes
	#but still it might be useful. XXX think this through.
	#if icon not in [str(ff) for ff in source]:
	#	source.append(File(icon))
	
	bundle_type = 'APPL'
	
	try:
		bundle_signature = env['SIGNATURE']
	except KeyError:
		bundle_signature = str(bundle)[:4].lower()
	assert len(bundle_signature) == 4, "Bundle signature must be four bytes"
	
	
	#coerce the target to a Bundle
	#XXX huh, that's weird, it builds fine now... oh shit it's because it caches types in its database
	#we don't need to tell it Bundle(), we just need to postpend the .app
	#it seems that if we tell Builder "suffix = '.app'" then it _at that point_ assumes that $NAME.app is a file, which then causes "TypeError: Tried to lookup File 'Mixxx.app' as a Dir.:"
	#so just work around that here
	if type(bundle) != Bundle:
		bundle = Bundle(str(bundle)+".app")
	
	#BUG: if the icon file is changed but nothing else then the plist doesn't get rebuilt (but since it's a str() and not a Node() there's no clean way to hook this in)
	
	#Precache some the important paths
	#idea: hide these in the env[]?
	bundle = Dir(str(bundle))	#coerce the bundle target into being a Dir
	contents = Dir(os.path.join(str(bundle), "Contents"))
	frameworks = Dir(os.path.join(str(contents), "Frameworks")) #we put both frameworks and standard unix sharedlibs in here
	plugins = Dir(os.path.join(str(contents), "PlugIns")) #TODO: plugins
	env['APP_RESOURCES'] = Dir(os.path.join(str(contents), "Resources"))
	
	
	#env['APP_INSTALLED_BIN'] = installed_bin

	
	
	#Generate the .plist and PkgInfo files
	
	#"""The contents of the PkgInfo file are the 4-byte package type followed by the 4-byte signature of your application.
	#Thus, for the TextEdit application, whose type is 'APPL' and whose signature is 'ttxt', the file would contain the ASCII string "APPLttxt"."""
	#So, we use the first four characters of the app
	env.Writer(File(os.path.join(str(contents),"PkgInfo")), [], DATA = "%s%s" % (bundle_type, bundle_signature))
	
	env.Plist(os.path.join(str(contents), "Info"), PLIST={'CFBundleExecutable': binary.name, 'CFBundleIconFile': icon, 'CFBundlePackageType': bundle_type, 'CFBundleSignature': bundle_signature})
	#NB: only need CFBundleExecutale if the binary name differs from the bundle name
	#todo:   
	"""Application Keys

	At a minimum, all applications should contain the following keys in their information property list file:
	CFBundleDisplayName
      CFBundleIdentifier
      CFBundleName
      CFBundlePackageType
      CFBundleShortVersionString
      CFBundleSignature
      CFBundleVersion
      LSHasLocalizedDisplayName
      NSHumanReadableCopyright
      NSAppleScriptEnabled"""
      #further: we should support generating document associations
      	
	for i in resources:
		if isinstance(i, SCons.Node.FS.Dir):
			#source+=
			InstallDir(Dir(bundle), i, env) #this is what we use now, but ideally Mixxx would be coded to use the Resources/ to store all it's resource files
			#InstallDir(env['APP_RESOURCES'], i, env) #this is what we are aiming for
			#so we use both! heh....
		elif isinstance(i, SCons.Node.FS.File):
			#source+=
			env.Install(env['APP_RESOURCES'], i)
	
	return bundle, source #+[installed_bin]




App = Builder(action = build_app, emitter = emit_app)



def build_plist(target, source, env):
	d = env['PLIST']
	assert len(target) == 1
	target = target[0]
	
	#XXX what happens if PLIST isn't passed in?
	#todo: make this support more than just <string>. but to do that means more research and fuck that at this point
	
	#this is a bad way to do this, should really use an XML writer... but fuck that
	outer_template = """<?xml version="1.0" encoding="UTF-8"?>
	<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
	<plist version="1.0">
	<dict>
	%s
	</dict>
	</plist>"""
	inner_template = """<key>%s</key><string>%s</string>"""
	
	inner = str.join('\n', [inner_template % (k,v) for k,v in d.iteritems()])
	plist = outer_template % inner
	
	f = open(str(target), "w")
	f.write(plist)
	f.close()
	

Plist = Builder(action = build_plist, emitter = no_sources, suffix="plist")


def generate(env):
	env['BUILDERS']['App'] = App
	env['BUILDERS']['Dmg'] = Dmg
	env['BUILDERS']['Plist'] = Plist
	env['BUILDERS']['Writer'] = Writer #this should be in a different module, really
	
def exists(env):
	return os.platform == 'darwin'


