#pragma once

#include <QSqlDatabase>
#include <QString>
#include <atomic>
#include <memory>

#include "library/itunes/itunesimporter.h"

class ITunesDAO;
class ITunesFeature;

/// An importer that reads the user's default Music library
/// using the native `MediaPlayer` framework on iOS.
class ITunesIOSImporter : public ITunesImporter {
  public:
    ITunesIOSImporter(
            ITunesFeature* pParentFeature,
            std::unique_ptr<ITunesDAO> dao);

    ITunesImport importLibrary() override;

  private:
    std::unique_ptr<ITunesDAO> m_dao;

    void requestAuthorization();
};
