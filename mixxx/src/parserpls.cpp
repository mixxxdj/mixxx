//
// C++ Implementation: parserpls
//
// Description: module to parse pls formated playlists
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "parser.h"
#include "parserpls.h"
//Added by qt3to4:
#include <Q3PtrList>
#include <QDebug>

/**
   @author Ingo Kossyk (kossyki@cs.tu-berlin.de)
 **/

/**
   ToDo:
    - parse ALL informations from the pls file if available ,
          not only the filepath;
 **/

ParserPls::ParserPls()
{
    m_psLocations = new Q3PtrList<QString>;
}

ParserPls::~ParserPls()
{

    delete m_psLocations;
}

Q3PtrList<QString> * ParserPls::parse(QString sFilename)
{
    //long numEntries =0;
    QFile * file = new QFile(sFilename);
    QString basepath = sFilename.section('/', 0, -2);

    clearLocations();

    if (file->open(QIODevice::ReadOnly) && !isBinary(sFilename) ) {

        Q3TextStream * textstream = new Q3TextStream( file );

        //numEntries = getNumEntries(textstream);

        //delete textstream;

        //textstream = new QTextStream( file );

        while(QString * psLine = new QString(getFilepath(textstream, basepath))){

            if(psLine->isNull() || (*psLine) == "NULL") {
                break;
            } else {
                m_psLocations->append(psLine);
            }

            //--numEntries;
        }

        file->close();

        if(m_psLocations->count() != 0)
            return m_psLocations;
        else
            return 0; // NULL pointer returned when no locations were found
    }

    file->close();
    return 0; //if we get here something went wrong :D
}

long ParserPls::getNumEntries(Q3TextStream * stream)
{

    QString textline;

    textline = stream->readLine();

    if(textline.contains("[playlist]")){

        while(!textline.contains("NumberOfEntries"))
            textline = stream->readLine();

        QString temp = textline.section("=",-1,-1);

        return temp.toLong();

    } else{
        qDebug() << "ParserPls: pls file is not a playlist! \n";
        return 0;
    }

}


QString ParserPls::getFilepath(Q3TextStream * stream, QString& basepath)
{
    QString textline,filename = "";
    textline = stream->readLine();
    while(!textline.isEmpty()){
        if(textline.isNull())
            break;

        if(textline.contains("File")) {
            int iPos = textline.find("=",0);
            ++iPos;

            filename = textline.right(textline.length()-iPos);

            if(isFilepath(filename)) {
                return filename;
            } else {
                // Try relative to m3u dir
                QString rel = basepath + "/" + filename;
                if (isFilepath(rel)) {
                    return rel;
                }
                // We couldn't match this to a real file so ignore it
            }
        }
        textline = stream->readLine();
    }

    // Signal we reached the end
    return 0;

}
