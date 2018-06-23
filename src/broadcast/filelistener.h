#pragma once

#include <QFile>

#include "broadcast/scrobblingservice.h"
#include "control/controlpushbutton.h"
#include "preferences/dialog/dlgprefmetadata.h"

class FileListener : public ScrobblingService {
    Q_OBJECT
  public:
    explicit FileListener(UserSettingsPointer pSettings);
    ~FileListener() override = default;
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
  protected:
  private slots:
    void slotFileSettingsChanged(double value);

  private:
    void updateStateFromSettings();
    void updateFile();
    static void writeMetadataToFile(const QByteArray* contents, std::shared_ptr<QFile> file);

    std::shared_ptr<QFile> m_pFile;
    QString m_fileContents; //We need this to translate between codecs.
    ControlPushButton m_COsettingsChanged;
    UserSettingsPointer m_pConfig;
    FileSettings m_latestSettings;
    bool filePathChanged = false;
};