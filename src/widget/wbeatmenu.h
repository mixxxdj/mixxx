#pragma once

#include <QMenu>

#include "preferences/usersettings.h"
#include "track/track.h"

class WBeatMenu : public QMenu {
    Q_OBJECT
  public:
    WBeatMenu(UserSettingsPointer pConfig, QWidget* parent = nullptr);

    ~WBeatMenu() override;

    void setTrack(const TrackPointer& track) {
        m_pTrack = track;
    }
    void update();
  signals:
    void updateDownbeat();

  private:
    QAction* m_pSetAsDownbeat;
    TrackPointer m_pTrack;
    UserSettingsPointer m_pConfig;
};
