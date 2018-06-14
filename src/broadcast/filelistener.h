#pragma once

#include <QFile>

#include "broadcast/scrobblingservice.h"
#include "control/controlpushbutton.h"

class FileListener : public ScrobblingService {
    Q_OBJECT
  public:
    enum class FileListenerType {
        SAMBroadcaster
    };
    FileListener() = delete;
    ~FileListener() override;
    static std::unique_ptr<FileListener>
    makeFileListener(FileListenerType type, UserSettingsPointer pConfig);
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
    static ConfigKey getFileModifiedControlKey();
    static ConfigKey getFilePathConfigKey();

  protected:
    virtual void writeMetadata(QTextStream& stream, TrackPointer pTrack) = 0;
    explicit FileListener(UserSettingsPointer pConfig);
  public slots:
    void slotFilePathChanged(double value);

  private:
    QFile m_file;
    ControlPushButton m_filePathChanged;
    UserSettingsPointer m_pConfig;
};

class SAMFileListener : public FileListener {
  public:
    explicit SAMFileListener(UserSettingsPointer pConfig);
    ~SAMFileListener() override = default;
    void writeMetadata(QTextStream& stream, TrackPointer pTrack) override;
};