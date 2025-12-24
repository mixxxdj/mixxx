//
// C++ Interface: parserpls
//
// Description: Interface header for the example parser PlsParser
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#pragma once

#include <QList>
#include <QString>
#include <QTextStream>

#include "library/parser.h"

class ParserPls : public Parser {
  public:
    static bool isPlaylistFilenameSupported(const QString& fileName);
    static QList<QString> parseAllLocations(const QString& playlistFile);
    /// Playlist Export
    static bool writePLSFile(const QString &file, const QList<QString> &items, bool useRelativePath);
};
