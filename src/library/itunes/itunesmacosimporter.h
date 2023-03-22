#pragma once

#include <QSqlDatabase>
#include <QString>
#include <atomic>

#include "library/itunes/itunesimporter.h"
#include "library/libraryfeature.h"

class ITunesMacOSImporter : public ITunesImporter {
  public:
    ITunesMacOSImporter(LibraryFeature* parentFeature,
            const QSqlDatabase& database,
            const std::atomic<bool>& cancelImport);

    ITunesImport importLibrary() override;

  private:
    LibraryFeature* m_parentFeature;
    // The values behind these references are owned by the parent `ITunesFeature`,
    // thus there is an implicit contract here that this `ITunesMacOSImporter` cannot
    // outlive the feature (which should not happen anyway, since importers are short-lived).
    const QSqlDatabase& m_database;
    const std::atomic<bool>& m_cancelImport;
};
