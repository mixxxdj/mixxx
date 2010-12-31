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
#include <QTextStream>
#include <QDebug>
#include "parserm3u.h"

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

ParserM3u::ParserM3u() : Parser()
{
}

ParserM3u::~ParserM3u()
{

}


QList<QString> ParserM3u::parse(QString sFilename)
{
    QFile file(sFilename);
    QString basepath = sFilename.section('/', 0, -2);

    clearLocations();
    //qDebug() << "ParserM3u: Starting to parse.";
    if (file.open(QIODevice::ReadOnly) && !isBinary(sFilename)) {
    	/* Unfortunately, QT 4.7 does not handle <CR> line breaks.
    	 * This is important on OS X where iTunes, e.g., exports M3U playlists using <CR>
    	 *
    	 * Using QFile::readAll() we obtain the complete content of the playlist as a ByteArray.
    	 * We replace any '\r' with '\n' if applicaple
    	 * This ensures that playlists from iTunes on OS X can be parsed
    	 */
    	QByteArray ba = file.readAll();
    	ba.replace('\r',"\n");
        QTextStream textstream(ba.data());
        
        while(!textstream.atEnd()) {
            QString sLine = getFilepath(&textstream, basepath);
            if(sLine.isEmpty())
                break;

            //qDebug) << ("ParserM3u: parsed: " << (sLine);
            m_sLocations.append(sLine);
        }

        file.close();

        if(m_sLocations.count() != 0)
            return m_sLocations;
        else
            return QList<QString>(); // NULL pointer returned when no locations were found

    }

    file.close();
    return QList<QString>(); //if we get here something went wrong
}


QString ParserM3u::getFilepath(QTextStream *stream, QString basepath)
{
    QString textline,filename = "";

    textline = stream->readLine();
	textline.replace("\r", "\n");
    qDebug() << textline;
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
