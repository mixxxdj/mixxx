
import os

#This page is REALLY USEFUL: http://www.cocoadev.com/index.pl?ApplicationLinking
#question: why do dylibs embed their install name in themselves? that would seem to imply the system keeps a cache of all the "install names", but that's not what the docs seem to say.
 #todo: change the install names (-id) in a working .app to garbage and see if the app still runs.
#question: Frameworks can have included resources besides the DyLib (the basic structure is X.framework/X and X.framework/Resources).
#If you're embedding the framework, these resources might need to come along for the ride.
#but how the fuck does the framework find these files, if it needs them?
#for Qt it doesn't matter because none of their bundles have Resources
#http://developer.apple.com/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/FrameworkAnatomy.html says "resources such as a shared library, nib files, image files, strings files, information property lists, documentation, header files, and so on."
 #you wouldn't need to embed Headers, but all the other things are potentially very important!
 #see: http://developer.apple.com/documentation/MacOSX/Conceptual/BPFrameworks/Tasks/CreatingFrameworks.html#//apple_ref/doc/uid/20002258-106880

#OKAY BOWYZ: http://developer.apple.com/documentation/CoreFoundation/Conceptual/CFBundles/Concepts/SearchAlgorithm.html#//apple_ref/doc/uid/20001120
#so basically: Apple has a magic ObjC API that handles doing lookups in the current bundle for you (probably asks "Where was I loaded from plz?" and just uses that dir as the root dir)
#so YES, we do need to make sure to copy in the entirety of each framework's Resources/

#also good: http://lapcatsoftware.com/blog/2007/08/11/embedding-frameworks-in-loadable-bundles/
#also important: otool -D = get install name, install_name_tool -id = set install name

#so here's my algorithm:
#copy X.framework/Versions
#use `cp -R` to copy the symlinks (it's weird like that: -R is meant for Recursive but if you use it on a symlink and nothing else it makes a copy of the symlink instead of copying the file the symlink was pointing at)
 #also, the symlinks in a framework bundle are relative symlinks, so we shouldn't need to patch them
#so maybe: use `cp -R X.framework`

#QUESTIONS:
#Does Qt have an umbrella framework?
#Do Apple load lists mean that the libs are loaded at runtime for sure, or is it safe to ignore ones that aren't found (so long as the app doesn't try to actually load that library)?
#Is the working dir of an app set to it's .app dir or .app/Contents/MacOS?

#Assumption: the loadlinks inside of an embeddable framework are all relative


import sys,os



def system(s):
	"wrap system() to give us feedback on what it's doing"
	"anything using this call should be fixed to use SCons's declarative style (once you figure that out, right nick?)"
	print s,
	sys.stdout.flush() #ignore line buffering..
	result = os.system(s)
	print
	return result



SYSTEM_FRAMEWORKS = ["/System/Library/Frameworks"]
SYSTEM_LIBPATH = ["/usr/lib"] #anything else?
#paths to libs that we should copy in
LOCAL_FRAMEWORKS = [
    os.path.expanduser("~/Library/Frameworks"), 
    "/Library/Frameworks", 
    "/Network/Library/Frameworks"
]
LOCAL_LIBPATH = filter(lambda x: 
    os.path.isdir(x), 
    ["/usr/local/lib", "/opt/local/lib", "/sw/local/lib"]
)

#however
FRAMEWORKS = LOCAL_FRAMEWORKS + SYSTEM_FRAMEWORKS
LIBPATH = LOCAL_LIBPATH + SYSTEM_LIBPATH

# otool parsing
def otool(binary):
	"return in a list of strings the OS X 'install names' of Mach-O binaries (dylibs and programs)"
	"Do not run this on object code archive (.a) files, it is not designed for that."
	#if not os.path.exists(binary): raise Exception("'%s' not found." % binary)
	if not type(binary) == str: raise ValueError("otool() requires a path (as a string)")
	print "otool(%s)" % binary
	stdin, stdout, stderr = os.popen3('otool -L "%s"' % binary)
	try:
		header = stdout.readline() #discard the first line since it is just the name of the file or an error message (or if reading .as, the first item on the list)
		if not binary+":\n" == header:
			#as far as I know otool -L only parses .dylibs and .a (but if it does anything else we should cover that case here)
			if header.startswith("Archive : "):
				raise Exception("'%s' an archive (.a) file." % binary)
			else:
				raise Exception(stderr.readline().strip())
		def parse(l):
			return l[1:l.rfind("(")-1]
		return [parse(l[:-1]) for l in stdout] #[:-1] is for stripping the trailing \n
	finally:
		stdin.close()
		stdout.close()
		stderr.close()


def dependencies(binary):
	l = otool(binary)

	#sometimes otool -L outputs the -id field, as the first row, which is not what we are trying to ask for here. Strip it out.
	#XXX this is NOT strictly correct; IF there is a library with the exact same filename as this library
	#AND we depend on it AND it happens to be the first in the loadlist, then this will break mysteriously
	#there might be a combination of flags to otool that avoids this, or something, but bah
	#it seems that dylibs have -id's, and program binaries do not. but I don't want to rely on that since I haven't an Apple doc proclaiming it, and anyway to tell the two apart requires other otool trickery that I just don't trust.
	#This will work in probably every library that exists in reality, so it's "ok"
	if os.path.basename(l[0]) == os.path.basename(binary):
		id = l.pop(0)
		#print "Removing -id field result %s from %s" % (id, binary)
	return l







#TODO: allow passing a heuristic function (or maybe even just a list of prefixes) that will decide whether to add a lib to the dependency list. It's taking waay too long to search out everything.
def embed_dependencies(binary,
                       # Defaults from
                       # http://www.kernelthread.com/mac/osx/programming.html
                       LOCAL=[os.path.expanduser("~/Library/Frameworks"),
                              "/Library/Frameworks",
                              "/Network/Library/Frameworks",
                              "/usr/local/lib",
                              "/opt/local/lib",
                              "/sw/local/lib"],
                       SYSTEM=["/System/Library/Frameworks",
                               "/Network/Library/Frameworks",
                               "/usr/lib"]):

	#Todo: split out the detection of frameworks vs regular dylibs -- return them as separate lists
	"recursively locates all the dependent libs of a binary using `otool -L`"
	"all output will be absolute paths"
	"binary is a file path"
	"this will crash if it cannot examine a referenced dylib, just as the app you are trying to find dependencies for will crash"
	"LOCAL: a list of paths to consider to be for installed libs that must therefore be bundled. Override this if you are not getting the right libs."
	"SYSTEM: a list of system library search paths that should be searched for system libs but should not be recursed into; this is needed. Override this if you want to bunde the system libs for some reason." #XXX it's useful to expose LOCAL but is this really needed?
	"this is a badly named function"
	"Note: sometimes Mach-O binaries depend on themselves. Deal with it."
	#"ignore_missing means whether to ignore if we can't load a binary for examination (e.g. if you have references to plugins) XXX is the list"
	#binary = os.path.abspath(binary)
	todo = dependencies(binary)
	done = []
	orig = []
	#aaah this code is so bad. it can be factored but i can't can't can't so TODO: REFACTOR

	while todo: #this was written with recursion before, but because of the possibility of loops in the network of libs recursion would require passing a collector variable around in the recursion and when you get to that point you might as well just use a loop
		e = todo.pop() #because of how this is written, popping from the end is a depth-first search. Popping from the front would be a breadth-first search. Neat!
		#Figure out the absolute path to the library
		#todo: this would make sense as a separate function, but the @ case would be awkward to handle and it's iffy about what it should
		if e.startswith('/'):
			p = e
		elif e.startswith('@'): #it's a relative load path
			raise Exception("Unable to make heads nor tails, sah, of install name '%s'. Relative paths are for already-bundled binaries, this function does not support them." % e)
		else:
			#experiments show that giving an unspecified path is asking dyld(1) to find the library for us. This covers that case.
			#i will not do further experiments to figure out what dyld(1)'s specific search algorithm is (whether it looks at frameworks separate from dylibs) because that's not the public interface

			for P in ['']+LOCAL+SYSTEM: #XXX should local have priority over system or vice versa? (also, '' is the handle the relative case)
				p = os.path.abspath(os.path.join(P, e))
				#print "SEARCHING IN LIBPATH; TRYING", p
				if os.path.exists(p):
					break
			else:
				p = e #fallthrough to the exception below #XXX icky bad logic, there must be a way to avoid saying exists() twice

		if not os.path.exists(p):
			raise Exception("Dependent library '%s' not found. Make sure it is installed." % e)

		#sometimes the todo list will have redundant items placed on it; in the case that the items are named the same that could be avoided by relying on set() semantics
		#however there are many equivalent ways to name files in Unix, so we have to check here after getting abspath
		#proof that this makes the algorithm terminate: we record the absolute path to the file; even if there are multiple abspaths to one file (as can happen with symlinks sometimes) each differnt path means a different symlink file, of which there are only a finite number (since it's a finite disk)
		if p not in done and not any(p.startswith(P) for P in SYSTEM):
			done.append(p)
			orig.append(e)
			todo.extend(dependencies(p))

	assert all(e.startswith("/") for e in done), "embed_dependencies() is broken, some path in this list is not absolute: %s" % (done,)
	return sorted(zip(orig, done))



def change_id(binary, id):
	"there is no way to "
	return system("install_name_tool -id '%s' '%s'" % (id,  binary))

def change_ref(binary, orig, new):
	assert orig in dependencies(binary), "change_ref: '%s' not in otool -L '%s', the change will fail." % (orig, binary) #since install_name_tool(1) always fails silently, there's *no* way to tell if it worked or not; try to catch the one bad case we know of manually (and expensively)
	return system("install_name_tool -change '%s' '%s' '%s'" % (orig, new, binary))

#but what are we *really* doing here? The overall goal?
#I'm looking for
#So if I have Mixxx and I want to run it
#I look at what Mixxx depends on
#then in that list I filter out System libs
#then I take the remainder and copy them into the frameworks dir
#then I look at each of them and see what they depend on, and keep copying in and pruning
#then within all the binaries that I've taken in I use install_name_tool to change the local/ load paths to @executuable_path/../Frameworks/$name

#

#keywords: @executable_path, @loader_path, @rpath. TODO: document these.
