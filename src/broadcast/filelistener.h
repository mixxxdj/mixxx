#pragma once

#include <QFile>

#include "broadcast/scrobblingservice.h"

class FileListener : public ScrobblingService {
  public:
    FileListener(const QString& path);
    void broadcastCurrentTrack(TrackPointer pTrack) override;
    void scrobbleTrack(TrackPointer pTrack) override;

  private:
    QFile m_file;
};