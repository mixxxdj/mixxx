//
// C++ Interface: parserm3u
//
// Description: Interface header for the example parser ParserM3u
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
// Author: Tobias Rafreider trafreider@mixxx.org, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//
#pragma once

#include <QList>
#include <QString>

#include "library/parser.h"

class ParserM3u : public Parser {
  public:
    static bool isPlaylistFilenameSupported(const QString& fileName);
    static QList<QString> parseAllLocations(const QString& playlistFile);
    /// Playlist Export
    static bool writeM3UFile(const QString &file_str, const QList<QString> &items, bool useRelativePath, bool useUtf8);
    static bool writeM3UFile(const QString &file, const QList<QString> &items, bool useRelativePath);
    static bool writeM3U8File(const QString &file_str, const QList<QString> &items, bool useRelativePath);
};
