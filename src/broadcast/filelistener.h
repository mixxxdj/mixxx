#pragma once

#include <QFile>
#include <QThread>
#include "preferences/dialog/dlgprefmetadata.h"
#include "control/controlpushbutton.h"
#include "broadcast/scrobblingservice.h"
#include "control/controlpushbutton.h"
#include "preferences/dialog/dlgprefmetadata.h"

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
    void openFile();
    void moveFile(QString destination);
    void writeMetadataToFile(QByteArray contents);
    void clearFile();
  private slots:
    void slotFileSettingsChanged(double value);

  private:
    void updateStateFromSettings();
    void updateFile();
    static void writeMetadataToFile(const QByteArray* contents, std::shared_ptr<QFile> file);

    QString m_fileContents; //We need this to translate between codecs.
    ControlPushButton m_COsettingsChanged;
    UserSettingsPointer m_pConfig;
    FileSettings m_latestSettings;
    QThread m_workerThread;
    bool filePathChanged = false;
    bool fileOpen = false;
    bool tracksPaused = false;
};