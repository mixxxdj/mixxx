#pragma once

#include <QSqlDatabase>
#include <QString>
#include <atomic>

#include "library/itunes/itunesimporter.h"
#include "library/libraryfeature.h"

class ITunesMacOSImporter : public ITunesImporter {
  public:
    ITunesMacOSImporter(LibraryFeature* parentFeature,
            QSqlDatabase& database,
            std::atomic<bool>& cancelImport);

    ITunesImport importLibrary() override;

  private:
    LibraryFeature* m_parentFeature;
    QSqlDatabase& m_database;
    std::atomic<bool>& m_cancelImport;
};
