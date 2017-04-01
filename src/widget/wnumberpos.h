// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#ifndef WNUMBERPOS_H
#define WNUMBERPOS_H

#include <QMouseEvent>

#include "wnumber.h"
#include "preferences/dialog/dlgprefcontrols.h"

class ControlProxy;

class WNumberPos : public WNumber {
    Q_OBJECT
  public:
    explicit WNumberPos(const char *group, QWidget *parent=nullptr);

  protected:
    void mousePressEvent(QMouseEvent* pEvent) override;

  private slots:
    void setValue(double dValue) override;
    void slotSetPosition(double);
    void slotSetTrackSampleRate(double dSampleRate);
    void slotSetTrackSamples(double dSamples);
    void slotSetDisplayMode(double);

  private:

    TrackTime::DisplayMode m_displayMode;

    // Old value set
    double m_dOldPosition;
    double m_dTrackSamples;
    double m_dTrackSampleRate;
    // True if remaining content is being shown
    bool m_bRemain;
    ControlProxy* m_pShowTrackTimeRemaining;
    // Pointer to control object for position, rate, and track info
    ControlProxy* m_pVisualPlaypos;
    ControlProxy* m_pTrackSamples;
    ControlProxy* m_pTrackSampleRate;
};

#endif
