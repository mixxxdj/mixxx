#pragma once

#include <util/widgethelper.h>

#include <QLineEdit>
#include <QMenu>
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
    void setTimeSignature(mixxx::TimeSignature timeSignature);
    mixxx::TimeSignature getTimeSignature() {
        return m_timeSignature;
    }
    void setBeatsPointer(mixxx::BeatsPointer pBeats) {
        m_pBeats = pBeats;
    }

    void setBeat(mixxx::Beat beat) {
        m_beat = beat;
        m_pBeatCountInputBox->setPlaceholderText(QString(beat.getTimeSignature().getBeatsPerBar()));
    }

    void popup(const QPoint& p) {
        //        auto parentWidget = static_cast<QWidget*>(parent());
        //        QPoint topLeft = mixxx::widgethelper::mapPopupToScreen(*parentWidget, p, size());
        //        move(topLeft);
        show();
    }
    //
    //    signals:
    //     void timeSignatureUpdated(mixxx::TimeSignature timeSignature);
  private slots:
    void slotBeatCountChanged(QString value);

  private:
    mixxx::TimeSignature m_timeSignature;
    parented_ptr<QLineEdit> m_pBeatCountInputBox;
    mixxx::BeatsPointer m_pBeats;
    mixxx::Beat m_beat;
};
