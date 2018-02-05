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

#ifndef VAMP_FILES_H
#define VAMP_FILES_H

#include <vector>
#include <string>

/**
 * This is a private implementation class for the Vamp Host SDK.
 */
class Files
{
public:
    static std::vector<std::string> listLibraryFiles();

    struct Filter {
        enum { All, Matching, NotMatching } type;
        std::vector<std::string> libraryNames;
        Filter() : type(All) { }
    };
    static std::vector<std::string> listLibraryFilesMatching(Filter);

    static void *loadLibrary(std::string filename);
    static void unloadLibrary(void *);
    static void *lookupInLibrary(void *, const char *symbol);

    static std::string lcBasename(std::string path);
    static std::string splicePath(std::string a, std::string b);
    static std::vector<std::string> listFiles(std::string dir, std::string ext);
};

#endif

    
