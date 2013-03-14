// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#ifndef WNUMBERPOS_H
#define WNUMBERPOS_H

#include "wnumber.h"

class ControlObjectThreadWidget;
class ControlObjectThreadMain;

/**
@author Tue Haste Andersen
*/

class WNumberPos : public WNumber {
    Q_OBJECT
  public:
    WNumberPos(const char *group, QWidget *parent=0);
    virtual ~WNumberPos();

    void setValue(double dValue);
    /** Set if the display shows remaining time (true) or position (false) */
    void setRemain(bool bRemain);

  protected:
    void mousePressEvent(QMouseEvent* pEvent);

  private slots:
    void slotSetRemain(double dRemain);
    void slotSetTrackSampleRate(double dSampleRate);
    void slotSetTrackSamples(double dSamples);

  private:
    /** Old value set */
    double m_dOldValue;
    double m_dTrackSamples;
    double m_dTrackSampleRate;
    /** True if remaining content is being shown */
    bool m_bRemain;
    ControlObjectThreadMain* m_pShowTrackTimeRemaining;
    // Pointer to control object for rate and track info
    ControlObjectThreadWidget *m_pRateControl, *m_pRateDirControl, *m_pTrackSamples, *m_pTrackSampleRate;
};

#endif
