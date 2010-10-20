//
// C++ Implementation: parser
//
// Description: superclass for external formats parsers
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtDebug>
#include "parser.h"

/**
   @author Ingo Kossyk (kossyki@cs.tu-berlin.de)
 **/


Parser::Parser() : QObject()
{
}

Parser::~Parser()
{


}

void Parser::clearLocations()
{
    while(!m_sLocations.isEmpty())
        m_sLocations.removeFirst();
}

long Parser::countParsed()
{
    return (long)m_sLocations.count();
}

bool Parser::isFilepath(QString sFilepath){
    QFile file(sFilepath);
    bool exists = file.exists();
    file.close();
    return exists;
}

bool Parser::isBinary(QString filename){
    QFile file(filename);

    if(file.open(QIODevice::ReadOnly)){
        char c;
        unsigned char uc;
        
        if(!file.getChar(&c))
        {
          qDebug() << "Parser: Error reading stream on " << filename;
          return true; //should this raise an exception?
        }
        
        uc = uchar(c);
        
        if(!(33<=uc && uc<=127))  //Starting byte is no character
        {
            file.close();
            return true;
        }

    } else{
        qDebug() << "Parser: Could not open file: " << filename;
    }
    //qDebug(QString("Parser: textstream starting character is: %1").arg(i));
    file.close();
    return false;
}
