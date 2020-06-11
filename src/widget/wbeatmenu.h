#pragma once

#include <QMenu>

#include "preferences/usersettings.h"
#include "track/beat.h"
#include "track/track.h"
#include "util/parented_ptr.h"

class WBeatMenu : public QMenu {
    Q_OBJECT
  public:
    WBeatMenu(UserSettingsPointer pConfig, QWidget* parent = nullptr);

    ~WBeatMenu() override = default;

    void setBeatgrid(mixxx::BeatsPointer pBeats) {
        m_pBeats = pBeats;
    }

    void setBeat(Beat beat) {
        m_beat = beat;
    }

    void update();

  private slots:
    void slotDownbeatUpdated();

  private:
    parented_ptr<QAction> m_pSetAsDownbeat;
    UserSettingsPointer m_pConfig;
    mixxx::BeatsPointer m_pBeats;
    Beat m_beat;
};
