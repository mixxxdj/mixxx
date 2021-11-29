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

class ParserPls : Parser {
  public:
    static bool isPlaylistFilenameSupported(const QString& fileName);
    QList<QString> parse(const QString& playlistFile, bool keepMissingFiles);
    /// Playlist Export
    static bool writePLSFile(const QString &file, const QList<QString> &items, bool useRelativePath);

  private:
    /// Reads a line from the file and returns filepath
    static QString getFilePath(QTextStream*, const QString& basePath);
};
