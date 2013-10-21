#ifndef WAVEFORMRENDERERSIGNALBASE_H
#define WAVEFORMRENDERERSIGNALBASE_H

#include "waveformrendererabstract.h"
#include "waveformsignalcolors.h"

class ControlObject;
class ControlObjectThread;

class WaveformRendererSignalBase : public WaveformRendererAbstract {
public:
    explicit WaveformRendererSignalBase( WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererSignalBase();

    virtual bool init();
    virtual void setup(const QDomNode &node);

    virtual bool onInit() {return true;}
    virtual void onSetup(const QDomNode &node) = 0;

protected:
    void deleteControls();

protected:
    ControlObjectThread* m_pLowFilterControlObject;
    ControlObjectThread* m_pMidFilterControlObject;
    ControlObjectThread* m_pHighFilterControlObject;

    ControlObjectThread* m_pLowKillControlObject;
    ControlObjectThread* m_pMidKillControlObject;
    ControlObjectThread* m_pHighKillControlObject;

    const WaveformSignalColors* m_pColors;
    QColor m_axesColor;
    Qt::Alignment m_alignment;
};

#endif // WAVEFORMRENDERERSIGNALBASE_H
