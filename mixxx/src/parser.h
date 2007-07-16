//
// C++ Interface: parser
//
// Description: Interface header for the parser Parser
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//


/**Developer Information:
This is the rootclass for all parser classes for the Importer class.
It can be used to write a new type-specific parser by deriving a new class
from it and overwrite the parse function and add class specific functions to 
it afterwards fro proper functioning
**/

#include <qobject.h>
#include <qstring.h>
#include <qfile.h>
#include <q3ptrlist.h>
#include <q3textstream.h>

#ifndef PARSER_H
#define PARSER_H

class Parser : public QObject
{
    Q_OBJECT
public:
    Parser();
    ~Parser();
    /**Can be called to parse a pls file
    Note for developers:
    This function should return an empty PtrList
     or 0 in order for the trackimporter to function**/
    virtual Q3PtrList<QString> * parse(QString) = 0;


protected:
    /**Pointer to the parsed Filelocations**/
    Q3PtrList<QString> * m_psLocations;
    /**Returns the number of parsed locations**/
    long countParsed();
    /**Clears m_psLocations**/
    void clearLocations();
    /**Checks if the file does contain binary content**/
    bool isBinary(QString);
    /**Checks if the given string represents a local filepath**/
    bool isFilepath(QString );
};

#endif
