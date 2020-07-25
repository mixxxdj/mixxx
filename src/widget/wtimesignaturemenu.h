#pragma once

#include <util/widgethelper.h>

#include <QLineEdit>
#include <QMenu>
#include <QWidget>
#include <QtWidgets/QSpinBox>

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
    //
    //    signals:
    //     void timeSignatureUpdated(mixxx::TimeSignature timeSignature);
  private slots:
    void slotBeatCountChanged(int value);

  private:
    parented_ptr<QSpinBox> m_pBeatCountBox;
    mixxx::BeatsPointer m_pBeats;
    mixxx::Beat m_beat;
};
