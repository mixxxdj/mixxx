//
// C++ Interface: parser
//
// Description: Interface header for the parser Parser
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
// Author: Tobias Rafreider trafreider@mixxx.org, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PARSER_H
#define PARSER_H

/**Developer Information:
This is the rootclass for all parser classes for the Importer class.
It can be used to write a new type-specific parser by deriving a new class
from it and overwrite the parse function and add class specific functions to
it afterwards fro proper functioning
**/

#include <qobject.h>
#include <qstring.h>
#include <qfile.h>


class Parser : public QObject
{
public:
    Parser();
    ~Parser();
    /**Can be called to parse a pls file
    Note for developers:
    This function should return an empty PtrList
     or 0 in order for the trackimporter to function**/
    virtual QList<QString> parse(QString) = 0;


protected:
    /**Pointer to the parsed Filelocations**/
    QList<QString> m_sLocations;
    /**Returns the number of parsed locations**/
    long countParsed();
    /**Clears m_psLocations**/
    void clearLocations();
    /**Checks if the file does contain binary content**/
    bool isBinary(QString);
    /**Checks if the given string represents a local filepath**/
    bool isFilepath(QString );
    // check for Utf8 encoding
    static bool isUtf8(const char* string);
};

#endif
