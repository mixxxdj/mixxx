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


#ifndef PARSERPLS_H
#define PARSERPLS_H

#include "parser.h"

class QTextStream;

class ParserPls : public Parser
{
    Q_OBJECT
public:
    ParserPls();
    ~ParserPls();
    /**Can be called to parse a pls file**/
    QList<QString> parse(QString);
    //Playlist Export
    static bool writePLSFile(QString &file, QList<QString> &items, bool useRelativePath);

private:
    /**Returns the Number of entries in the pls file**/
    long getNumEntries(QTextStream * );
    /**Reads a line from the file and returns filepath**/
    QString getFilepath(QTextStream *, QString);

};

#endif
