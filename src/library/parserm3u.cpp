//
// C++ Implementation: parserm3u
//
// Description: module to parse m3u(plaintext) formatted playlists
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
// Author: Tobias Rafreider trafreider@mixxx.org, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "library/parserm3u.h"

#include <QDir>
#include <QMessageBox>
#include <QTextCodec>
#include <QUrl>
#include <QtDebug>

#include "moc_parserm3u.cpp"

/**
   ToDo:
    - parse ALL information from the pls file if available ,
          not only the filepath;

          Userinformation :
          The M3U format is just a headerless plaintext format
          where every line of text either represents
          a file location or a comment. comments are being
          preceded by a '#'. This parser will try to parse all
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

QList<QString> ParserM3u::parse(const QString& sFilename) {
    clearLocations();

    QFile file(sFilename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning()
                << "Failed to open playlist file"
                << sFilename;
        return m_sLocations;
    }

    // Unfortunately QTextStream does not handle <CR> (=\r or asci value 13) line breaks.
    // This is important on OS X where iTunes, e.g., exports M3U playlists using <CR>
    // rather that <LF>.
    //
    // Using QFile::readAll() we obtain the complete content of the playlist as a ByteArray.
    // We replace any '\r' with '\n' if applicaple
    // This ensures that playlists from iTunes on OS X can be parsed
    QByteArray ba = file.readAll();
    //detect encoding
    bool isCRLF_encoded = ba.contains("\r\n");
    bool isCR_encoded = ba.contains("\r");
    if (isCR_encoded && !isCRLF_encoded) {
        ba.replace('\r', '\n');
    }

    QTextStream textstream(ba.constData());
    if (isUtf8(ba.constData())) {
        textstream.setCodec("UTF-8");
    } else {
        textstream.setCodec("windows-1252");
    }

    const auto basePath = sFilename.section('/', 0, -2);
    while (!textstream.atEnd()) {
        QString sLine = getFilePath(&textstream, basePath);
        if (sLine.isEmpty()) {
            continue;
        }
        m_sLocations.append(sLine);
    }

    return m_sLocations;
}

QString ParserM3u::getFilePath(QTextStream* stream, const QString& basePath) {
    QString textline;
    while (!(textline = stream->readLine().trimmed()).isEmpty()) {
        if (textline.startsWith("#")) {
            // Skip comments
            continue;
        }
        auto trackFile = playlistEntryToFileInfo(textline, basePath);
        if (trackFile.checkFileExists()) {
            return trackFile.location();
        }
        // We couldn't match this to a real file so ignore it
        qWarning() << trackFile << "not found";
    }
    // Signal we reached the end
    return QString();
}

bool ParserM3u::writeM3UFile(const QString &file_str, const QList<QString> &items, bool useRelativePath) {
    return writeM3UFile(file_str, items, useRelativePath, false);
}

bool ParserM3u::writeM3U8File(const QString &file_str, const QList<QString> &items, bool useRelativePath) {
    return writeM3UFile(file_str, items, useRelativePath, true);
}

bool ParserM3u::writeM3UFile(const QString &file_str, const QList<QString> &items, bool useRelativePath, bool useUtf8)
{
    // Important note:
    // On Windows \n will produce a <CR><CL> (=\r\n)
    // On Linux and OS X \n is <CR> (which remains \n)

    QTextCodec* codec;
    if (useUtf8) {
        codec = QTextCodec::codecForName("UTF-8");
    } else {
        // according to http://en.wikipedia.org/wiki/M3U the default encoding of m3u is Windows-1252
        // see also http://tools.ietf.org/html/draft-pantos-http-live-streaming-07
        // check if the all items can be properly encoded to Latin1.
        codec = QTextCodec::codecForName("windows-1252");
        for (int i = 0; i < items.size(); ++i) {
            if (!codec->canEncode(items.at(i))) {
                // filepath contains incompatible character
                QMessageBox::warning(nullptr,
                        tr("Playlist Export Failed"),
                        tr("File path contains characters, not allowed in m3u "
                           "playlists.\n") +
                                tr("Export a m3u8 playlist instead!\n") +
                                items.at(i));
                return false;
            }
        }
    }

    QFile file(file_str);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr,
                tr("Playlist Export Failed"),
                tr("Could not create file") + " " + file_str);
        return false;
    }

    // Base folder of file
    QString base = file_str.section('/', 0, -2);
    QDir base_dir(base);

    qDebug() << "Basepath: " << base;
    QTextStream out(&file);
    out.setCodec(codec);
    out << "#EXTM3U\n";
    for (int i = 0; i < items.size(); ++i) {
        out << "#EXTINF\n";
        // Write relative path if possible
        if (useRelativePath) {
            //QDir::relativePath() will return the absolutePath if it cannot compute the
            //relative Path
            out << base_dir.relativeFilePath(items.at(i)) << "\n";
        } else {
            out << items.at(i) << "\n";
        }
    }
    return true;
}
