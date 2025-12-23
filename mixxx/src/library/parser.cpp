#include "library/parser.h"

#include <QDir>
#include <QUrl>
#include <QtDebug>

#include "library/parsercsv.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"

// static
bool Parser::isPlaylistFilenameSupported(const QString& playlistFile) {
    return ParserM3u::isPlaylistFilenameSupported(playlistFile) ||
            ParserPls::isPlaylistFilenameSupported(playlistFile) ||
            ParserCsv::isPlaylistFilenameSupported(playlistFile);
}

// static
QList<QString> Parser::parseAllLocations(const QString& playlistFile) {
    if (ParserM3u::isPlaylistFilenameSupported(playlistFile)) {
        return ParserM3u::parseAllLocations(playlistFile);
    }

    if (ParserPls::isPlaylistFilenameSupported(playlistFile)) {
        return ParserPls::parseAllLocations(playlistFile);
    }

    if (ParserCsv::isPlaylistFilenameSupported(playlistFile)) {
        return ParserCsv::parseAllLocations(playlistFile);
    }

    return QList<QString>();
}

// static
QList<QString> Parser::parse(const QString& playlistFile) {
    const QList<QString> allLocations = parseAllLocations(playlistFile);

    QFileInfo fileInfo(playlistFile);

    QList<QString> existingLocations;
    for (const auto& location : allLocations) {
        mixxx::FileInfo trackFile = Parser::playlistEntryToFileInfo(
                location, fileInfo.canonicalPath());
        if (trackFile.checkFileExists()) {
            existingLocations.append(trackFile.location());
        } else {
            qInfo() << "File" << trackFile.location() << "from playlist"
                    << playlistFile << "does not exist.";
        }
    }
    return existingLocations;
}

// The following public domain code is taken from
// http://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
// Thank you Christoph!
// static
bool Parser::isUtf8(const char* string) {
    if (!string) {
        return false;
    }

    const unsigned char* bytes = (const unsigned char *)string;
    while (*bytes) {
        if(     (// ASCII
                        bytes[0] == 0x09 ||
                        bytes[0] == 0x0A ||
                        bytes[0] == 0x0D ||
                        (0x20 <= bytes[0] && bytes[0] <= 0x7E)
                )
        ) {
                bytes += 1;
                continue;
        }

        if(     (// non-overlong 2-byte
                        (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
                        (0x80 <= bytes[1] && bytes[1] <= 0xBF)
                )
        ) {
                bytes += 2;
                continue;
        }

        if(     (// excluding overlongs
                        bytes[0] == 0xE0 &&
                        (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
                        (0x80 <= bytes[2] && bytes[2] <= 0xBF)
                ) ||
                (// straight 3-byte
                        ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
                                bytes[0] == 0xEE ||
                                bytes[0] == 0xEF) &&
                        (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                        (0x80 <= bytes[2] && bytes[2] <= 0xBF)
                ) ||
                (// excluding surrogates
                        bytes[0] == 0xED &&
                        (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
                        (0x80 <= bytes[2] && bytes[2] <= 0xBF)
                )
        ) {
                bytes += 3;
                continue;
        }

        if(     (// planes 1-3
                        bytes[0] == 0xF0 &&
                        (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
                        (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                        (0x80 <= bytes[3] && bytes[3] <= 0xBF)
                ) ||
                (// planes 4-15
                        (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
                        (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                        (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                        (0x80 <= bytes[3] && bytes[3] <= 0xBF)
                ) ||
                (// plane 16
                        bytes[0] == 0xF4 &&
                        (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
                        (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                        (0x80 <= bytes[3] && bytes[3] <= 0xBF)
                )
        ) {
                bytes += 4;
                continue;
        }

        return false;
    }

    return true;
}

// static
mixxx::FileInfo Parser::playlistEntryToFileInfo(
        const QString& playlistEntry,
        const QString& basePath) {
    if (playlistEntry.startsWith("file:")) {
        // URLs are always absolute
        return mixxx::FileInfo::fromQUrl(QUrl(playlistEntry));
    }
    auto filePath = QString(playlistEntry).replace('\\', '/');
    auto trackFile = mixxx::FileInfo(filePath);
    if (basePath.isEmpty() || trackFile.isAbsolute()) {
        return trackFile;
    } else {
        // Fallback: Relative to base path
        return mixxx::FileInfo(QDir(basePath), filePath);
    }
}
