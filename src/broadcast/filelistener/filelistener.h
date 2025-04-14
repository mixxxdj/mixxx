#pragma once

#include <QFile>
#include <QThread>

#include "broadcast/scrobblingservice.h"
#include "control/controlpushbutton.h"
#include "preferences/dialog/dlgprefmetadata.h"
#include "preferences/metadatafilesettings.h"

/// Listens to metadata changes and current track and writes them to a
/// NowPlaying.txt file. This file can be read by third party applications
/// like RDS generators or such
class FileListener : public ScrobblingService {
    Q_OBJECT
  public:
    explicit FileListener(UserSettingsPointer pSettings);
    ~FileListener() override;
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
  signals:
    void deleteFile();
    void moveFile(const QString& destination);
    void writeMetadataToFile(const QByteArray& contents);
    void clearFile();
  private slots:
    void slotFileSettingsChanged(double value);

  private:
    struct WrittenMetadata {
        QString title, artist;
        bool isEmpty() {
            return title.isEmpty() && artist.isEmpty();
        }
    };

    void updateStateFromSettings();
    void updateFile();

    ControlPushButton m_COsettingsChanged;
    UserSettingsPointer m_pConfig;
    FileSettings m_latestSettings;
    QThread m_workerThread;
    WrittenMetadata m_fileContents;
    bool m_filePathChanged;
    bool m_tracksPaused;
};
