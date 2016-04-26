// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#ifndef WNUMBERPOS_H
#define WNUMBERPOS_H

#include <QMouseEvent>

#include "wnumber.h"

class ControlObjectSlave;

class WNumberPos : public WNumber {
    Q_OBJECT
  public:
    explicit WNumberPos(const char *group, QWidget *parent=nullptr);

    // Set if the display shows remaining time (true) or position (false)
    void setRemain(bool bRemain);

  protected:
    void mousePressEvent(QMouseEvent* pEvent) override;

  private slots:
    void setValue(double dValue) override;
    void slotSetValue(double);
    void slotSetRemain(double remain);
    void slotSetTrackSampleRate(double dSampleRate);
    void slotSetTrackSamples(double dSamples);

  private:
    // Old value set
    double m_dOldValue;
    double m_dTrackSamples;
    double m_dTrackSampleRate;
    // True if remaining content is being shown
    bool m_bRemain;
    ControlObjectSlave* m_pShowTrackTimeRemaining;
    // Pointer to control object for position, rate, and track info
    ControlObjectSlave* m_pVisualPlaypos;
    ControlObjectSlave* m_pTrackSamples;
    ControlObjectSlave* m_pTrackSampleRate;
};

#endif
