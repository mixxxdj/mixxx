// loopingcontrol.h
// Created on Sep 23, 2008
// Author: asantoni, rryan

#ifndef LOOPINGCONTROL_H
#define LOOPINGCONTROL_H

#include <QObject>

#include "configobject.h"
#include "engine/enginecontrol.h"

class ControlPushButton;
class ControlObject;

class LoopingControl : public EngineControl {
    Q_OBJECT
  public:
    LoopingControl(const char * _group, ConfigObject<ConfigValue> * _config);
    virtual ~LoopingControl();
    double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int iBufferSize);
    double nextTrigger(const double dRate,
                       const double currentSample,
                       const double totalSamples,
                       const int iBufferSize);
    double getTrigger(const double dRate,
                      const double currentSample,
                      const double totalSamples,
                      const int iBufferSize);

    void hintReader(QList<Hint>& hintList);

  public slots:
    void slotLoopIn(double);
    void slotLoopOut(double);
    void slotReloopExit(double);

  private:
    ControlObject* m_pCOLoopStartPosition;
    ControlObject* m_pCOLoopEndPosition;
    ControlPushButton* m_pLoopInButton;
    ControlPushButton* m_pLoopOutButton;
    ControlPushButton* m_pReloopExitButton;
    bool m_bLoopingEnabled;
    unsigned long m_iLoopEndSample;
    unsigned long m_iLoopStartSample;
    unsigned long m_iCurrentSample;
};

#endif /* LOOPINGCONTROL_H */
