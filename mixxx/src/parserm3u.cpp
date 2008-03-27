//
// C++ Implementation: parserm3u
//
// Description: module to parse m3u(plaintext) formated playlists
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "parserm3u.h"
//Added by qt3to4:
#include <Q3PtrList>

/**
   @author Ingo Kossyk (kossyki@cs.tu-berlin.de)
 **/

/**
   ToDo:
    - parse ALL informations from the pls file if available ,
          not only the filepath;

          Userinformation :
          The M3U format is just a headerless plaintext format
          where every line of text either represents
          a file location or a comment. comments are being
          preceeded by a '#'. This parser will try to parse all
          file information from the given file and add the filepaths
          to the locations ptrlist when the file is existing locally
          or on a mounted harddrive.
 **/

ParserM3u::ParserM3u()
{
    m_psLocations = new Q3PtrList<QString>;
}

ParserM3u::~ParserM3u()
{

    //delete m_psLocations;

}


Q3PtrList<QString> * ParserM3u::parse(QString sFilename)
{


    QFile * file = new QFile(sFilename);
    QString basepath = sFilename.section('/', 0, -2);

    clearLocations();
    //qDebug() << "ParserM3u: Starting to parse.";
    if (file->open(QIODevice::ReadOnly) && !isBinary(sFilename)) {

        Q3TextStream * textstream = new Q3TextStream( file );



        while(QString * psLine = new QString(getFilepath(textstream, basepath))){

            if(psLine->isNull() || (*psLine) == "NULL")
                break;

            //qDebug) << ("ParserM3u: parsed: " << (*psLine);
            m_psLocations->append(psLine);

        }

        file->close();

        if(m_psLocations->count() != 0)
            return m_psLocations;
        else
            return 0; // NULL pointer returned when no locations were found

    }

    file->close();
    return 0; //if we get here something went wrong
}


QString ParserM3u::getFilepath(Q3TextStream * stream, QString& basepath)
{
    QString textline,filename = "";

    textline = stream->readLine();
    while(!textline.isEmpty()){
        if(textline.isNull())
            break;

        if(!textline.contains("#") && !textline.isEmpty()){
            filename = textline;
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
