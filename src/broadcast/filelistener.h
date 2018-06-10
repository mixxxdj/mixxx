#pragma once

#include <QFile>

#include "broadcast/scrobblingservice.h"

class FileListener : public ScrobblingService {
  public:
    enum class FileListenerType {
        SAMBroadcaster
    };
    FileListener() = delete;
    ~FileListener() override;
    static std::unique_ptr<FileListener>
    makeFileListener(FileListenerType type, const QString& path);
    void broadcastCurrentTrack(TrackPointer pTrack) override;
    void scrobbleTrack(TrackPointer pTrack) override;

  protected:
    virtual void writeMetadata(QTextStream& stream, TrackPointer pTrack) = 0;
    FileListener(const QString& path);

  private:
    QFile m_file;
};

class SAMFileListener : public FileListener {
  public:
    SAMFileListener(const QString& path);
    ~SAMFileListener() override = default;
    void writeMetadata(QTextStream& stream, TrackPointer pTrack) override;
};