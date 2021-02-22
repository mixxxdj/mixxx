#pragma once

#include <util/widgethelper.h>

#include <QComboBox>
#include <QLineEdit>
#include <QMenu>
#include <QSpinBox>
#include <QWidget>

#include "preferences/usersettings.h"
#include "track/beats.h"
#include "track/timesignature.h"
#include "util/parented_ptr.h"

class WTimeSignatureMenu : public QWidget {
    Q_OBJECT
  public:
    WTimeSignatureMenu(QWidget* parent = nullptr);
    ~WTimeSignatureMenu() override;
    void setBeatsPointer(mixxx::BeatsPointer pBeats) {
        m_pBeats = pBeats;
    }
    void setBeat(std::optional<mixxx::Beat> beat);
    void popup(const QPoint& p);

  private slots:
    void slotBeatCountChanged(int value);
    void slotBeatSizeChanged(int index);
    void setTimeSignature(const mixxx::TimeSignature& timeSignature);
    void slotTimeSignatureHalved();
    void slotTimeSignatureDoubled();

  private:
    bool canHalveBothValues() const;
    bool canDoubleBothValues() const;
    void updateHalfDoubleButtonsActiveStatus();

    parented_ptr<QSpinBox> m_pBeatCountBox;
    parented_ptr<QComboBox> m_pBeatLengthBox;
    parented_ptr<QPushButton> m_pHalfButton;
    parented_ptr<QPushButton> m_pDoubleButton;
    mixxx::BeatsPointer m_pBeats;
    std::optional<mixxx::Beat> m_beat;
};
