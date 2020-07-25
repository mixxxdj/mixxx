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
    void setBeat(mixxx::Beat beat);
    void popup(const QPoint& p);

  private slots:
    void slotBeatCountChanged(int value);
    void slotBeatSizeChanged(int index);
    void setTimeSignature(mixxx::TimeSignature timeSignature);

  private:
    parented_ptr<QSpinBox> m_pBeatCountBox;
    parented_ptr<QComboBox> m_pBeatLengthBox;
    mixxx::BeatsPointer m_pBeats;
    mixxx::Beat m_beat;
};
