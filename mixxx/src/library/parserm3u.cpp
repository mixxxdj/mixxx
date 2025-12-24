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

#include "errordialoghandler.h"

namespace {
// according to http://en.wikipedia.org/wiki/M3U the default encoding of m3u is Windows-1252
// see also http://tools.ietf.org/html/draft-pantos-http-live-streaming-07
const char kStandardM3uTextEncoding[] = "Windows-1250";
const char kM3uHeader[] = "#EXTM3U";
const char kM3uCommentPrefix[] = "#";
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

// static
bool ParserM3u::isPlaylistFilenameSupported(const QString& fileName) {
    return fileName.endsWith(".m3u", Qt::CaseInsensitive) ||
            fileName.endsWith(".m3u8", Qt::CaseInsensitive);
}

QList<QString> ParserM3u::parseAllLocations(const QString& playlistFile) {
    QList<QString> paths;

    QFile file(playlistFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning()
                << "Failed to open playlist file"
                << playlistFile
                << file.errorString();
        return paths;
    }

    QByteArray byteArray = file.readAll();
    QString fileContents;
    if (Parser::isUtf8(byteArray.constData())) {
        fileContents = QString::fromUtf8(byteArray);
    } else {
        // FIXME: replace deprecated QTextCodec with direct usage of libicu
        fileContents = QTextCodec::codecForName(kStandardM3uTextEncoding)
                               ->toUnicode(byteArray);
    }

    if (!fileContents.startsWith(kM3uHeader)) {
        qWarning() << "M3U playlist file" << playlistFile << "does not start with" << kM3uHeader;
    }

    const QStringList fileLines = fileContents.split(kUniveralEndOfLineRegEx);
    for (const QString& line : fileLines) {
        if (line.startsWith(kM3uCommentPrefix)) {
            // Skip lines with comments
            continue;
        }
        paths.append(line);
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
    bool urlEncodingUsed = false;
    QDir baseDirectory(QFileInfo(file_str).canonicalPath());
    QTextCodec* codec = QTextCodec::codecForName(kStandardM3uTextEncoding);
    QString fileContents(QStringLiteral("#EXTM3U\n"));
    for (const QString& item : items) {
        fileContents += QStringLiteral("#EXTINF\n");
        if (useUtf8) {
            if (useRelativePath) {
                fileContents += baseDirectory.relativeFilePath(item) + QStringLiteral("\n");
            } else {
                fileContents += item + QStringLiteral("\n");
            }
        } else {
            QByteArray trackByteArray = codec->fromUnicode(item);
            QString trackName = codec->toUnicode(trackByteArray);
            if (trackName == item) {
                if (useRelativePath) { //Issue: URL Location is not working properly for Relative Paths
                    fileContents += baseDirectory.relativeFilePath(item) + QStringLiteral("\n");
                } else {
                    fileContents += item + QStringLiteral("\n");
                }
            } else {
                QUrl itemUrl = QUrl::fromLocalFile(item);
                QString itemUrlEncoded = itemUrl.toEncoded();
                fileContents += itemUrlEncoded + QStringLiteral("\n");
                urlEncodingUsed = true;
            }
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
        ErrorDialogHandler* pDialogHandler = ErrorDialogHandler::instance();
        ErrorDialogProperties* props = pDialogHandler->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(QObject::tr("Playlist Export Failed"));
        props->setText(QObject::tr("Could not create file") + " " + file_str);
        props->setDetails(file.errorString());
        pDialogHandler->requestErrorDialog(props);
        return false;
    }
    if (urlEncodingUsed) {
        QMessageBox::information(nullptr,
                QObject::tr("Playlist Export Has Special Characters"),
                QObject::tr("Some file paths in the playlist have special characters. "
                            "These file paths will be encoded as absolute path URLs. "
                            "Please select the m3u8 format for better and lossless exporting."));
    }
    file.write(outputByteArray);
    file.close();

    return true;
}
