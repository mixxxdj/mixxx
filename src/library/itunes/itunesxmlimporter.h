#pragma once

#include <QSqlDatabase>
#include <QXmlStreamReader>
#include <memory>

#include "library/itunes/itunesimporter.h"
#include "library/itunes/itunespathmapping.h"
#include "library/libraryfeature.h"

/// An importer that parses an iTunes XML library.
class ITunesXMLImporter : public ITunesImporter {
  public:
    ITunesXMLImporter(LibraryFeature* parentFeature,
            const QString& filePath,
            QSqlDatabase& database,
            ITunesPathMapping& pathMapping,
            bool& cancelImport);

    ITunesImport importLibrary() override;

  private:
    LibraryFeature* m_parentFeature;
    QFile m_file;
    QXmlStreamReader m_xml;
    QString m_dbfile;
    QSqlDatabase& m_database;
    ITunesPathMapping& m_pathMapping;
    bool& m_cancelImport;

    void parseTracks();
    void guessMusicLibraryMountpoint();
    void parseTrack(QSqlQuery& query);
    std::unique_ptr<TreeItem> parsePlaylists();
    bool readNextStartElement();
    void parsePlaylist(QSqlQuery& query_insert_to_playlists,
            QSqlQuery& query_insert_to_playlist_tracks,
            TreeItem& root);
};
