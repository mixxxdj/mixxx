#pragma once

#include <QString>
#include "library/itunes/itunesimporter.h"

class ITunesMacOSImporter : public ITunesImporter {
  public:
    ITunesMacOSImporter(QString iTunesFile);

    ITunesImport importLibrary() override;
  private:
    QString m_iTunesFile;
};
