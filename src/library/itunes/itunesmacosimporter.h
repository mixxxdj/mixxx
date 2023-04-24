#pragma once

#include <QSqlDatabase>
#include <QString>
#include <atomic>

#include "library/itunes/itunesdao.h"
#include "library/itunes/itunesimporter.h"
#include "library/libraryfeature.h"

/// An importer that reads the user's default iTunes/Music.app library
/// using the native `iTunesLibrary` framework on macOS.
class ITunesMacOSImporter : public ITunesImporter {
  public:
    ITunesMacOSImporter(LibraryFeature* parentFeature,
            const std::atomic<bool>& cancelImport,
            ITunesDAO& dao);

    ITunesImport importLibrary() override;

  private:
    LibraryFeature* m_parentFeature;
    const std::atomic<bool>& m_cancelImport;
    // The values behind these references are owned by the parent `ITunesFeature`,
    // (note that the DAO internally contains a database reference),
    // thus there is an implicit contract here that this `ITunesMacOSImporter` cannot
    // outlive the feature (which should not happen anyway, since importers are short-lived).
    ITunesDAO& m_dao;
};
