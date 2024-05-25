#pragma once

#include <QFile>
#include <QHash>
#include <QXmlStreamReader>
#include <memory>

#include "library/itunes/itunesdao.h"
#include "library/itunes/itunesimporter.h"
#include "library/itunes/itunespathmapping.h"

class ITunesFeature;
class TrackRef;

/// An importer that parses an iTunes XML library.
class ITunesXMLImporter : public ITunesImporter {
  public:
    ITunesXMLImporter(
            ITunesFeature* pParentFeature,
            const QString& xmlFilePath,
            std::unique_ptr<ITunesDAO> dao);

    ITunesImport importLibrary() override;

  private:
    const QString m_xmlFilePath;
    QFile m_xmlFile;
    QXmlStreamReader m_xml;
    std::unique_ptr<ITunesDAO> m_dao;

    ITunesPathMapping m_pathMapping;
    QHash<QString, int> m_playlistIdByPersistentId;

    void parseTracks();
    void guessMusicLibraryMountpoint();
    void parseTrack();
    void parsePlaylists();
    bool readNextStartElement();
    void parsePlaylist();
};
