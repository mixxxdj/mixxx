//
// C++ Implementation: parserpls
//
// Description: module to parse pls formatted playlists
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
// Author: Tobias Rafreider trafreider@mixxx.org, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "library/parserpls.h"

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QUrl>
#include <QtDebug>

#include "moc_parserpls.cpp"

/**
   ToDo:
    - parse ALL information from the pls file if available ,
          not only the filepath;
 **/

ParserPls::ParserPls() : Parser() {
}

ParserPls::~ParserPls() {
}

QList<QString> ParserPls::parse(const QString& sFilename) {
    //long numEntries =0;
    QFile file(sFilename);
    const auto basePath = sFilename.section('/', 0, -2);

    clearLocations();

    if (file.open(QIODevice::ReadOnly) && !isBinary(sFilename)) {

        /* Unfortunately, QT 4.7 does not handle <CR> (=\r or asci value 13) line breaks.
         * This is important on OS X where iTunes, e.g., exports M3U playlists using <CR>
         * rather that <LF>
         *
         * Using QFile::readAll() we obtain the complete content of the playlist as a ByteArray.
         * We replace any '\r' with '\n' if applicaple
         * This ensures that playlists from iTunes on OS X can be parsed
         */
        QByteArray ba = file.readAll();
        //detect encoding
        bool isCRLF_encoded = ba.contains("\r\n");
        bool isCR_encoded = ba.contains("\r");
        if (isCR_encoded && !isCRLF_encoded) {
            ba.replace('\r','\n');
        }
        QTextStream textstream(ba.constData());

        while(!textstream.atEnd()) {
            QString psLine = getFilePath(&textstream, basePath);
            if(psLine.isNull()) {
                break;
            } else {
                m_sLocations.append(psLine);
            }

        }

        file.close();

        if (m_sLocations.count() != 0) {
            return m_sLocations;
        } else {
            return QList<QString>(); // NULL pointer returned when no locations were found
        }
    }

    file.close();
    return QList<QString>(); //if we get here something went wrong :D
}

long ParserPls::getNumEntries(QTextStream *stream) {
    QString textline;
    textline = stream->readLine();

    if (textline.contains("[playlist]")) {
        while (!textline.contains("NumberOfEntries")) {
            textline = stream->readLine();
        }

        QString temp = textline.section("=",-1,-1);

        return temp.toLong();

    } else{
        qDebug() << "ParserPls: pls file is not a playlist! \n";
        return 0;
    }

}


QString ParserPls::getFilePath(QTextStream *stream, const QString& basePath) {
    QString textline = stream->readLine();
    while (!textline.isEmpty()) {
        if (textline.isNull()) {
            break;
        }

        if(textline.contains("File")) {
            int iPos = textline.indexOf("=", 0);
            ++iPos;

            QString filename = textline.right(textline.length() - iPos);
            auto trackFile = playlistEntryToTrackFile(filename, basePath);
            if (trackFile.checkFileExists()) {
                return trackFile.location();
            }
            // We couldn't match this to a real file so ignore it
            qWarning() << trackFile << "not found";
        }
        textline = stream->readLine();
    }

    // Signal we reached the end
    return QString();
}

bool ParserPls::writePLSFile(const QString &file_str, const QList<QString> &items, bool useRelativePath)
{
    QFile file(file_str);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr,
                tr("Playlist Export Failed"),
                tr("Could not create file") + " " + file_str);
        return false;
    }
    //Base folder of file
    QString base = file_str.section('/', 0, -2);
    QDir base_dir(base);

    QTextStream out(&file);
    out << "[playlist]\n";
    out << "NumberOfEntries=" << items.size() << "\n";
    for (int i = 0; i < items.size(); ++i) {
        //Write relative path if possible
        if (useRelativePath) {
            //QDir::relativePath() will return the absolutePath if it cannot compute the
            //relative Path
            out << "File" << i << "=" << base_dir.relativeFilePath(items.at(i)) << "\n";
        } else {
            out << "File" << i << "=" << items.at(i) << "\n";
        }
    }

    return true;
}
