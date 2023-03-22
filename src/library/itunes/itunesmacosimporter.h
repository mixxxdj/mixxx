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
    const QSqlDatabase& m_database;
    const std::atomic<bool>& m_cancelImport;
};
