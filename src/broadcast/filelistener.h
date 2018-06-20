#pragma once

#include <QFile>
#include "preferences/dialog/dlgprefmetadata.h"
#include "control/controlpushbutton.h"
#include "broadcast/scrobblingservice.h"
#include "control/controlpushbutton.h"

class FileListener : public ScrobblingService {
    Q_OBJECT
  public:
    enum class FileListenerType {
        SAMBroadcaster
    };
    explicit FileListener(UserSettingsPointer pSettings);
    ~FileListener() override;
    void slotBroadcastCurrentTrack(TrackPointer pTrack) override;
    void slotScrobbleTrack(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
  protected:
  private slots:
    void slotFileSettingsChanged(double value);
  private:

    void updateStateFromSettings();
    void updateFile();

    QFile m_file;
    ControlPushButton m_COsettingsChanged;
    UserSettingsPointer m_pConfig;
    FileSettings m_latestSettings;
};