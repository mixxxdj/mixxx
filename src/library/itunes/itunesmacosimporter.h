#pragma once

#include <QSqlDatabase>
#include <QString>
#include <atomic>
#include <memory>

#include "library/itunes/itunesimporter.h"

class ITunesDAO;
class ITunesFeature;

/// An importer that reads the user's default iTunes/Music.app library
/// using the native `iTunesLibrary` framework on macOS.
class ITunesMacOSImporter : public ITunesImporter {
  public:
    ITunesMacOSImporter(
            ITunesFeature* pParentFeature,
            std::unique_ptr<ITunesDAO> dao);

    ITunesImport importLibrary() override;

  private:
    std::unique_ptr<ITunesDAO> m_dao;
};
