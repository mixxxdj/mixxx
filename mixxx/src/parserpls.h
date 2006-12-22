//
// C++ Interface: parserpls
//
// Description: Interface header for the example parser PlsParser
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "parser.h"

#ifndef PARSERPLS_H
#define PARSERPLS_H

class ParserPls : public Parser
{
    Q_OBJECT
public:
    ParserPls();
    ~ParserPls();
    /**Can be called to parse a pls file**/
    QPtrList<QString> * parse(QString);

private:
    /**Returns the Number of entries in the pls file**/
    long getNumEntries(QTextStream * );
    /**Reads a line from the file and returns filepath**/
    QString getFilepath(QTextStream *, QString&);

};

#endif
