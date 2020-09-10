#pragma once

#include <QMenu>

#include "preferences/usersettings.h"
#include "track/beat.h"
#include "track/track.h"
#include "util/parented_ptr.h"
#include "widget/wcuemenupopup.h"
#include "widget/wtempomenu.h"
#include "widget/wtimesignaturemenu.h"

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

    void setBeatsPointer(mixxx::BeatsPointer pBeats);

    void setBeat(std::optional<mixxx::Beat> beat);

    void setOptions(Options selectedOptions) {
        m_eSelectedOptions = selectedOptions;
        updateMenu();
    }

    void addOptions(Options newOptions);

    void removeOptions(Options removeOptions);

  public slots:
    void slotBeatsUpdated();

  signals:
    void cueButtonClicked();

  private slots:
    void slotDownbeatUpdated();
    void slotDisplayTimeSignatureMenu();
    void slotDisplayTempoMenu();

  private:
    void updateMenu();

    UserSettingsPointer m_pConfig;
    parented_ptr<WTimeSignatureMenu> m_pTimeSignatureMenu;
    parented_ptr<WTempoMenu> m_pTempoMenu;
    mixxx::BeatsPointer m_pBeats;
    std::optional<mixxx::Beat> m_beat;
    Options m_eSelectedOptions;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WBeatMenu::Options)
