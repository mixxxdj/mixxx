//
// C++ Implementation: parser
//
// Description: superclass for external formats parsers
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
// Author: Tobias Rafreider trafreider@mixxx.org, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtDebug>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QUrl>

#include "library/parser.h"

/**
   @author Ingo Kossyk (kossyki@cs.tu-berlin.de)
 **/


Parser::Parser() {
}

Parser::~Parser() {
}

void Parser::clearLocations() {
    m_sLocations.clear();
}

long Parser::countParsed() {
    return (long)m_sLocations.count();
}

bool Parser::isBinary(QString filename) {
    char firstByte;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly) && file.getChar(&firstByte)) {
        // If starting byte is not an ASCII character then the file
        // probably contains binary data.
        if (firstByte >= 32 && firstByte <= 126) {
            // Valid ASCII character
            return false;
        }
        // Check for UTF-8 BOM
        if (firstByte == static_cast<char>(0xEF)) {
            char nextChar;
            if (file.getChar(&nextChar) &&
                    nextChar == static_cast<char>(0xBB) &&
                    file.getChar(&nextChar) &&
                    nextChar == static_cast<char>(0xBF)) {
                // UTF-8 text file
                return false;
            }
            return true;
        }
    }
    qDebug() << "Parser: Error reading from" << filename;
    return true; //should this raise an exception?
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

TrackFile Parser::playlistEntryToTrackFile(
        const QString& playlistEntry,
        const QString& basePath) {
    if (playlistEntry.startsWith("file:")) {
        // URLs are always absolute
        return TrackFile::fromUrl(QUrl(playlistEntry));
    }
    auto filePath = QString(playlistEntry).replace('\\', '/');
    auto trackFile = TrackFile(filePath);
    if (basePath.isEmpty() || trackFile.asFileInfo().isAbsolute()) {
        return trackFile;
    } else {
        // Fallback: Relative to base path
        return TrackFile(QDir(basePath), filePath);
    }
}
