// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#ifndef WNUMBERPOS_H
#define WNUMBERPOS_H

#include <QByteArrayData>
#include <QMouseEvent>
#include <QString>

#include "preferences/dialog/dlgprefdeck.h"
#include "wnumber.h"

class ControlProxy;
class QMouseEvent;
class QObject;
class QWidget;

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

  private:

    TrackTime::DisplayMode m_displayMode;
    TrackTime::DisplayFormat m_displayFormat;

    double m_dOldTimeElapsed;
    ControlProxy* m_pTimeElapsed;
    ControlProxy* m_pTimeRemaining;
    ControlProxy* m_pShowTrackTimeRemaining;
    ControlProxy* m_pTimeFormat;
};

#endif
