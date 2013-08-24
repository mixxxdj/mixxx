#ifndef GUITICK_H
#define GUITICK_H

#include <QObject>
#include <portaudio.h>

class ControlObject;

class GuiTick : public QObject {
    Q_OBJECT
  public:
    GuiTick(QObject* pParent=NULL);
    ~GuiTick();
    void process();

    static void setStreamTime(double streamTime);
    static double streamTime();

  private:
    ControlObject* m_pCOStreamTime;
    ControlObject* m_pCOGuiTick50ms;

    double m_lastUpdateTime;

    static double m_streamtime; // Stream Time in seconds
};

#endif // GUITICK_H
