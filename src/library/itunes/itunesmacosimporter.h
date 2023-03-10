#pragma once

#include <QSqlDatabase>
#include <QString>

#include "library/itunes/itunesimporter.h"
#include "library/libraryfeature.h"

class ITunesMacOSImporter : public ITunesImporter {
  public:
    ITunesMacOSImporter(LibraryFeature* parentFeature,
            QString iTunesFile,
            QSqlDatabase& database,
            bool& cancelImport);

    ITunesImport importLibrary() override;
  private:
    LibraryFeature* m_parentFeature;
    QString m_iTunesFile;
    QSqlDatabase& m_database;
    bool& m_cancelImport;
};
