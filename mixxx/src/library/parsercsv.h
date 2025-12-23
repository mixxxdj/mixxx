#pragma once

#include <QList>
#include <QString>
#include <QByteArray>

#include "library/parser.h"

class BaseSqlTableModel;

class ParserCsv : public Parser {
  public:
    // static
    static bool isPlaylistFilenameSupported(const QString& playlistFile);
    static QList<QString> parseAllLocations(const QString&);
    // Playlist Export
    static bool writeCSVFile(const QString &file, BaseSqlTableModel* pPlaylistTableModel, bool useRelativePath);
    // Readable Text export
    static bool writeReadableTextFile(const QString &file, BaseSqlTableModel* pPlaylistTableModel,  bool writeTimestamp);

  private:
    // Reads a line from the file and returns filepath if a valid file
    static QList<QList<QString>> tokenize(const QByteArray& str, char delimiter);
};
