#pragma once

#include <QMenu>

#include "preferences/usersettings.h"
#include "track/beat.h"
#include "track/track.h"
#include "util/parented_ptr.h"
#include "widget/wcuemenupopup.h"

class WBeatMenu : public QMenu {
    Q_OBJECT
  public:
    enum Option {
        SetDownbeat = 1 << 0,
        CueMenu = 1 << 1
    };
    Q_DECLARE_FLAGS(Options, Option)
    WBeatMenu(UserSettingsPointer pConfig, QWidget* parent = nullptr);

    ~WBeatMenu() override = default;

    void setBeatgrid(mixxx::BeatsPointer pBeats) {
        m_pBeats = pBeats;
    }

    void setBeat(mixxx::Beat beat) {
        m_beat = beat;
    }

    void setOptions(Options selectedOptions) {
        m_eSelectedOptions = selectedOptions;
        updateMenu();
    }

    void addOptions(Options newOptions);

    void removeOptions(Options removeOptions);

  signals:
    void cueButtonClicked();

  private slots:
    void slotDownbeatUpdated();

  private:
    void updateMenu();

    parented_ptr<QAction> m_pSetAsDownbeat;
    parented_ptr<QAction> m_pCueMenu;
    UserSettingsPointer m_pConfig;
    mixxx::BeatsPointer m_pBeats;
    mixxx::Beat m_beat;
    Options m_eSelectedOptions;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WBeatMenu::Options)
