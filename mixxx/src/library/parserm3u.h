//
// C++ Interface: parserm3u
//
// Description: Interface header for the example parser ParserM3u
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "parser.h"

#ifndef PARSERM3U_H
#define PARSERM3U_H

class QTextStream;

class ParserM3u : public Parser
{
    Q_OBJECT
public:
    ParserM3u();
    ~ParserM3u();
    /**Overwriting function parse in class Parser**/
    QList<QString> parse(QString);

private:
    /**Reads a line from the file and returns filepath if a valid file**/
    QString getFilepath(QTextStream *, QString);


};

#endif
