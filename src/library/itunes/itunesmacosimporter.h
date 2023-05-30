#pragma once

#include <QSqlDatabase>
#include <QString>
#include <atomic>
#include <memory>

#include "library/itunes/itunesdao.h"
#include "library/itunes/itunesimporter.h"
#include "library/libraryfeature.h"

/// An importer that reads the user's default iTunes/Music.app library
/// using the native `iTunesLibrary` framework on macOS.
class ITunesMacOSImporter : public ITunesImporter {
  public:
    ITunesMacOSImporter(LibraryFeature* parentFeature,
            const std::atomic<bool>& cancelImport,
            std::unique_ptr<ITunesDAO> dao);

    ITunesImport importLibrary() override;

  private:
    LibraryFeature* m_parentFeature;
    // The values behind these references are owned by the parent `ITunesFeature`,
    // thus there is an implicit contract here that this `ITunesMacOSImporter` cannot
    // outlive the feature (which should not happen anyway, since importers are short-lived).
    const std::atomic<bool>& m_cancelImport;
    std::unique_ptr<ITunesDAO> m_dao;
};
