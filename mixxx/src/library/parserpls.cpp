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
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QUrl>

/**
   @author Ingo Kossyk (kossyki@cs.tu-berlin.de)
 **/

/**
   ToDo:
    - parse ALL informations from the pls file if available ,
          not only the filepath;
 **/

ParserPls::ParserPls() : Parser()
{
}

ParserPls::~ParserPls()
{
}

QList<QString> ParserPls::parse(QString sFilename)
{
    //long numEntries =0;
    QFile file(sFilename);
    QString basepath = sFilename.section('/', 0, -2);

    clearLocations();

    if (file.open(QIODevice::ReadOnly) && !isBinary(sFilename) ) {

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
            QString psLine = getFilepath(&textstream, basepath);
            if(psLine.isNull()) {
                break;
            } else {
                m_sLocations.append(psLine);
            }

            //--numEntries;
        }

        file.close();

        if(m_sLocations.count() != 0)
            return m_sLocations;
        else
            return QList<QString>(); // NULL pointer returned when no locations were found
    }

    file.close();
    return QList<QString>(); //if we get here something went wrong :D
}

long ParserPls::getNumEntries(QTextStream *stream)
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


QString ParserPls::getFilepath(QTextStream *stream, QString basepath)
{
    QString textline,filename = "";
    textline = stream->readLine();
    while(!textline.isEmpty()){
        if(textline.isNull())
            break;

        if(textline.contains("File")) {
            int iPos = textline.indexOf("=",0);
            ++iPos;

            filename = textline.right(textline.length()-iPos);
           
            //Rythmbox playlists starts with file://<path>
            //We remove the file protocol if found. 
            filename.remove("file://");
            QByteArray strlocbytes = filename.toUtf8();
            QUrl location = QUrl::fromEncoded(strlocbytes);
            QString trackLocation = location.toLocalFile();
            //qDebug() << trackLocation;

            if(isFilepath(trackLocation)) {
                return trackLocation;
            } else {
                // Try relative to m3u dir
                QString rel = basepath + "/" + trackLocation;
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
