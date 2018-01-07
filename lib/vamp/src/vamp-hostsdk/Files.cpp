/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp

    An API for audio analysis and feature extraction plugins.

    Centre for Digital Music, Queen Mary, University of London.
    Copyright 2006-2015 Chris Cannam and QMUL.
  
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of the Centre for
    Digital Music; Queen Mary, University of London; and Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/

#include <vamp-hostsdk/PluginHostAdapter.h>

#include "Files.h"

#include <cctype> // tolower

#include <cstring>

#ifdef _WIN32

#include <windows.h>
#include <tchar.h>
#define PLUGIN_SUFFIX "dll"

#else /* ! _WIN32 */

#include <dirent.h>
#include <dlfcn.h>

#ifdef __APPLE__
#define PLUGIN_SUFFIX "dylib"
#else /* ! __APPLE__ */
#define PLUGIN_SUFFIX "so"
#endif /* ! __APPLE__ */

#endif /* ! _WIN32 */

using namespace std;

vector<string>
Files::listLibraryFiles()
{
    return listLibraryFilesMatching("");
}

vector<string>
Files::listLibraryFilesMatching(string libraryName)
{
    vector<string> path = Vamp::PluginHostAdapter::getPluginPath();
    vector<string> libraryFiles;

    // we match case-insensitively
    for (size_t i = 0; i < libraryName.length(); ++i) {
	libraryName[i] = tolower(libraryName[i]);
    }

    for (size_t i = 0; i < path.size(); ++i) {
        
        vector<string> files = listFiles(path[i], PLUGIN_SUFFIX);

        for (vector<string>::iterator fi = files.begin();
             fi != files.end(); ++fi) {
            
            if (libraryName != "") {
		// we match case-insensitively
                string temp = *fi;
                for (size_t i = 0; i < temp.length(); ++i) {
                    temp[i] = tolower(temp[i]);
                }
                // libraryName should be lacking an extension, as it
                // is supposed to have come from the plugin key
                string::size_type pi = temp.find('.');
                if (pi == string::npos) {
                    if (libraryName != temp) continue;
                } else {
                    if (libraryName != temp.substr(0, pi)) continue;
                }
            }

            string fullPath = path[i];
            fullPath = splicePath(fullPath, *fi);
	    libraryFiles.push_back(fullPath);
	}
    }

    return libraryFiles;
}

void *
Files::loadLibrary(string path)
{
    void *handle = 0;
#ifdef _WIN32
#ifdef UNICODE
    int len = path.length() + 1; // cannot be more wchars than length in bytes of utf8 string
    wchar_t *buffer = new wchar_t[len];
    int rv = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), len, buffer, len);
    if (rv <= 0) {
        cerr << "Vamp::HostExt: Unable to convert library path \""
             << path << "\" to wide characters " << endl;
        delete[] buffer;
        return handle;
    }
    handle = LoadLibrary(buffer);
    delete[] buffer;
#else
    handle = LoadLibrary(path.c_str());
#endif
    if (!handle) {
        cerr << "Vamp::HostExt: Unable to load library \""
             << path << "\"" << endl;
    }
#else
    handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        cerr << "Vamp::HostExt: Unable to load library \""
             << path << "\": " << dlerror() << endl;
    }
#endif
    return handle;
}

void
Files::unloadLibrary(void *handle)
{
#ifdef _WIN32
    FreeLibrary((HINSTANCE)handle);
#else
    dlclose(handle);
#endif
}

void *
Files::lookupInLibrary(void *handle, const char *symbol)
{
#ifdef _WIN32
    return (void *)GetProcAddress((HINSTANCE)handle, symbol);
#else
    return (void *)dlsym(handle, symbol);
#endif
}

string
Files::lcBasename(string path)
{
    string basename(path);
	
    string::size_type li = basename.rfind('/');
    if (li != string::npos) basename = basename.substr(li + 1);

#ifdef _WIN32
    li = basename.rfind('\\');
    if (li != string::npos) basename = basename.substr(li + 1);
#endif

    li = basename.find('.');
    if (li != string::npos) basename = basename.substr(0, li);

    for (size_t i = 0; i < basename.length(); ++i) {
        basename[i] = tolower(basename[i]);
    }

    return basename;
}

string
Files::splicePath(string a, string b)
{
#ifdef _WIN32
    return a + "\\" + b;
#else
    return a + "/" + b;
#endif
}

vector<string>
Files::listFiles(string dir, string extension)
{
    vector<string> files;

#ifdef _WIN32
    string expression = dir + "\\*." + extension;
#ifdef UNICODE
    int len = expression.length() + 1; // cannot be more wchars than length in bytes of utf8 string
    wchar_t *buffer = new wchar_t[len];
    int rv = MultiByteToWideChar(CP_UTF8, 0, expression.c_str(), len, buffer, len);
    if (rv <= 0) {
        cerr << "Vamp::HostExt: Unable to convert wildcard path \""
             << expression << "\" to wide characters" << endl;
        delete[] buffer;
        return files;
    }
    WIN32_FIND_DATA data;
    HANDLE fh = FindFirstFile(buffer, &data);
    if (fh == INVALID_HANDLE_VALUE) {
        delete[] buffer;
        return files;
    }

    bool ok = true;
    while (ok) {
        wchar_t *fn = data.cFileName;
        int wlen = wcslen(fn) + 1;
        int maxlen = wlen * 6;
        char *conv = new char[maxlen];
        int rv = WideCharToMultiByte(CP_UTF8, 0, fn, wlen, conv, maxlen, 0, 0);
        if (rv > 0) {
            files.push_back(conv);
        }
        delete[] conv;
        ok = FindNextFile(fh, &data);
    }

    FindClose(fh);
    delete[] buffer;
#else
    WIN32_FIND_DATA data;
    HANDLE fh = FindFirstFile(expression.c_str(), &data);
    if (fh == INVALID_HANDLE_VALUE) return files;

    bool ok = true;
    while (ok) {
        files.push_back(data.cFileName);
        ok = FindNextFile(fh, &data);
    }

    FindClose(fh);
#endif
#else

    size_t extlen = extension.length();
    DIR *d = opendir(dir.c_str());
    if (!d) return files;
            
    struct dirent *e = 0;
    while ((e = readdir(d))) {
 
        size_t len = strlen(e->d_name);
        if (len < extlen + 2 ||
            e->d_name + len - extlen - 1 != "." + extension) {
            continue;
        }

        files.push_back(e->d_name);
    }

    closedir(d);
#endif

    return files;
}
