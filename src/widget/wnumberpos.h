#pragma once

#include <QMouseEvent>

#include "wnumber.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "mixer/basetrackplayer.h"


class ControlProxy;

class WNumberPos : public WNumber {
    Q_OBJECT

  public:
    explicit WNumberPos(const QString& group, QWidget* parent = nullptr, BaseTrackPlayer* player = nullptr);

  protected:
    void mousePressEvent(QMouseEvent* pEvent) override;

  private slots:
    void setValue(double dValue) override;
    void slotSetTimeElapsed(double);
    void slotTimeRemainingUpdated(double);
    void slotSetDisplayMode(double);
    void slotSetTimeFormat(double);

  private:

    TrackTime::DisplayMode m_displayMode;
    TrackTime::DisplayFormat m_displayFormat;

    double m_dOldTimeElapsed;
    ControlProxy* m_pTimeElapsed;
    ControlProxy* m_pTimeRemaining;
    ControlProxy* m_pShowTrackTimeRemaining;
    ControlProxy* m_pTimeFormat;

    //We will use this to look for the track cues and update the
    //BeatsUntilNextCueAndRemaining display mode
    BaseTrackPlayer* m_pPlayer;
};
