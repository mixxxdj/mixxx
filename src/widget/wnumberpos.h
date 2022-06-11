#pragma once

#include <QMouseEvent>

#include "mixer/basetrackplayer.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "wnumber.h"

class ControlProxy;

class WNumberPos : public WNumber {
    Q_OBJECT

  public:
    explicit WNumberPos(const QString& group, QWidget* parent = nullptr);

  protected:
    void mousePressEvent(QMouseEvent* pEvent) override;

  private slots:
    void setValue(double dValue) override;
    void slotSetTimeElapsed(double);
    void slotTimeRemainingUpdated(double);
    void slotSetDisplayMode(double);
    void slotSetTimeFormat(double);
    void slotSetBeatCounter(double);

  private:

    TrackTime::DisplayMode m_displayMode;
    TrackTime::DisplayFormat m_displayFormat;

    double m_dOldTimeElapsed;
    ControlProxy* m_pTimeElapsed;
    ControlProxy* m_pTimeRemaining;
    ControlProxy* m_pShowTrackTimeRemaining;
    ControlProxy* m_pTimeFormat;

    ControlProxy* m_pBeatCountNextCue;
    double pBeatsToNextCue;
};
