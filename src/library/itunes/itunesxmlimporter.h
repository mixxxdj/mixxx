#pragma once

#include <QHash>
#include <QSqlDatabase>
#include <QXmlStreamReader>
#include <atomic>
#include <memory>

#include "library/itunes/itunesdao.h"
#include "library/itunes/itunesimporter.h"
#include "library/itunes/itunespathmapping.h"
#include "library/libraryfeature.h"

/// An importer that parses an iTunes XML library.
class ITunesXMLImporter : public ITunesImporter {
  public:
    ITunesXMLImporter(LibraryFeature* parentFeature,
            const QString& xmlFilePath,
            const std::atomic<bool>& cancelImport,
            ITunesDAO& dao);

    ITunesImport importLibrary() override;

  private:
    LibraryFeature* m_parentFeature;
    const QString m_xmlFilePath;
    QFile m_xmlFile;
    QXmlStreamReader m_xml;
    const std::atomic<bool>& m_cancelImport;
    // The values behind the references are owned by the parent `ITunesFeature`
    // (note that the DAO internally contains a database reference),
    // thus there is an implicit contract here that this `ITunesXMLImporter` cannot
    // outlive the feature (which should not happen anyway, since importers are short-lived).
    ITunesDAO& m_dao;

    ITunesPathMapping m_pathMapping;
    QHash<QString, int> m_playlistIdByPersistentId;

    void parseTracks();
    void guessMusicLibraryMountpoint();
    void parseTrack();
    void parsePlaylists();
    bool readNextStartElement();
    void parsePlaylist();
};
