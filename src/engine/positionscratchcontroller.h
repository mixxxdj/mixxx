#ifndef POSITIONSCRATCHCONTROLLER_H
#define POSITIONSCRATCHCONTROLLER_H

#include <QObject>
#include <QString>

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"

class VelocityController;
class RateIIFilter;

class PositionScratchController : public QObject {
    Q_OBJECT
  public:
    PositionScratchController(const char* pGroup);
    virtual ~PositionScratchController();

    void process(double currentSample, double releaseRate,
                 int iBufferSize, double baserate);
    bool isEnabled();
    double getRate();
    void notifySeek(double currentSample);

  private slots:
    void slotAccumulate(double v);
    void slotSensitivity(double v);

  private:
    const QString m_group;
    ControlPushButton* m_pScratchEnable;
    ControlObject* m_pScratchPosition;
    ControlObject* m_pScratchAccumulator;
    ControlPotmeter* m_pScratchAccumulatorSensitivity;
    ControlObject* m_pMasterSampleRate;
    VelocityController* m_pVelocityController;
    RateIIFilter* m_pRateIIFilter;
    bool m_bScratching;
    bool m_bEnableInertia;
    double m_dLastPlaypos;
    double m_dPositionDeltaSum;
    double m_dTargetDelta;
    double m_dStartScratchPosition;
    double m_dRate;
    double m_dMoveDelay;
    double m_dMouseSampeTime;
};

#endif /* POSITIONSCRATCHCONTROLLER_H */
