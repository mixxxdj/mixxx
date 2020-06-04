#pragma once

#include <QMenu>

#include "preferences/usersettings.h"
#include "track/track.h"

class WBeatMenu : public QMenu {
    Q_OBJECT
  public:
    WBeatMenu(UserSettingsPointer pConfig, QWidget* parent = nullptr);

    virtual ~WBeatMenu() override;

    void setTrack(const TrackPointer& track) {
        m_pTrack = track;
    }
  signals:
    void updateDownbeat();

  private:
    QAction* m_pSetAsDownbeat;
    TrackPointer m_pTrack;
};
