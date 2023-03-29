#pragma once

#include <QSqlDatabase>
#include <QXmlStreamReader>
#include <atomic>
#include <memory>

#include "library/itunes/itunesimporter.h"
#include "library/itunes/itunespathmapping.h"
#include "library/libraryfeature.h"

/// An importer that parses an iTunes XML library.
class ITunesXMLImporter : public ITunesImporter {
  public:
    ITunesXMLImporter(LibraryFeature* parentFeature,
            const QString& xmlFilePath,
            const QSqlDatabase& database,
            ITunesPathMapping& pathMapping,
            const std::atomic<bool>& cancelImport);

    ITunesImport importLibrary() override;

  private:
    LibraryFeature* m_parentFeature;
    const QString m_xmlFilePath;
    QFile m_xmlFile;
    QXmlStreamReader m_xml;
    // The values behind these references are owned by the parent `ITunesFeature`,
    // thus there is an implicit contract here that this `ITunesXMLImporter` cannot
    // outlive the feature (which should not happen anyway, since importers are short-lived).
    const QSqlDatabase& m_database;
    ITunesPathMapping& m_pathMapping;
    const std::atomic<bool>& m_cancelImport;

    void parseTracks();
    void guessMusicLibraryMountpoint();
    void parseTrack(QSqlQuery& query);
    std::unique_ptr<TreeItem> parsePlaylists();
    bool readNextStartElement();
    void parsePlaylist(QSqlQuery& queryInsertToPlaylists,
            QSqlQuery& queryInsertToPlaylistTracks,
            TreeItem& root);
};
