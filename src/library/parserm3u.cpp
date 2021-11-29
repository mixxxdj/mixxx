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
#include <QRegularExpression>
#include <QTextCodec>
#include <QUrl>
#include <QtDebug>

#include "moc_parserm3u.cpp"

namespace {
// according to http://en.wikipedia.org/wiki/M3U the default encoding of m3u is Windows-1252
// see also http://tools.ietf.org/html/draft-pantos-http-live-streaming-07
const char* kStandardM3uTextEncoding = "Windows-1250";
// Note: The RegEx pattern is compiled, when first used the first time
const auto kUniveralEndOfLineRegEx = QRegularExpression(QStringLiteral("\r\n|\r|\n"));
} // anonymous namespace

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

QList<QString> ParserM3u::parse(const QString& filename, bool keepMissingFiles) {
    QList<QString> paths;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning()
                << "Failed to open playlist file"
                << filename;
        return paths;
    }

    QByteArray byteArray = file.readAll();
    QString fileContents;
    if (isUtf8(byteArray.constData())) {
        fileContents = QString::fromUtf8(byteArray);
    } else {
        // FIXME: replace deprecated QTextCodec with direct usage of libicu
        fileContents = QTextCodec::codecForName(kStandardM3uTextEncoding)
                               ->toUnicode(byteArray);
    }

    QFileInfo fileInfo(filename);
    const QStringList fileLines = fileContents.split(kUniveralEndOfLineRegEx);
    for (const QString& line : fileLines) {
        auto trackFile = playlistEntryToFileInfo(line, fileInfo.canonicalPath());
        if (keepMissingFiles || trackFile.checkFileExists()) {
            paths.append(trackFile.location());
        } else {
            qInfo() << "File" << trackFile.location() << "from M3U playlist"
                    << filename << "does not exist.";
        }
    }

    return paths;
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

    QDir baseDirectory(QFileInfo(file_str).canonicalPath());

    QString fileContents(QStringLiteral("#EXTM3U\n"));
    for (const QString& item : items) {
        fileContents += QStringLiteral("#EXTINF\n");
        if (useRelativePath) {
            fileContents += baseDirectory.relativeFilePath(item) + QStringLiteral("\n");
        } else {
            fileContents += item + QStringLiteral("\n");
        }
    }

    QByteArray outputByteArray;
    if (useUtf8) {
        outputByteArray = fileContents.toUtf8();
    } else {
        // FIXME: replace deprecated QTextCodec with direct usage of libicu
        outputByteArray = QTextCodec::codecForName(kStandardM3uTextEncoding)
                                  ->fromUnicode(fileContents);
    }

    QFile file(file_str);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr,
                tr("Playlist Export Failed"),
                tr("Could not create file") + " " + file_str);
        return false;
    }
    file.write(outputByteArray);
    file.close();

    return true;
}
