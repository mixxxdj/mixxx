#ifndef WAVEFORMRENDERERSIGNALBASE_H
#define WAVEFORMRENDERERSIGNALBASE_H

#include "waveformrendererabstract.h"
#include "waveformsignalcolors.h"

class ControlObject;
class ControlObjectThreadMain;

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
    ControlObjectThreadMain* m_lowFilterControlObject;
    ControlObjectThreadMain* m_midFilterControlObject;
    ControlObjectThreadMain* m_highFilterControlObject;

    ControlObjectThreadMain* m_lowKillControlObject;
    ControlObjectThreadMain* m_midKillControlObject;
    ControlObjectThreadMain* m_highKillControlObject;

    const WaveformSignalColors* m_pColors;
    QColor m_axesColor;
    Qt::Alignment m_alignment;
};

#endif // WAVEFORMRENDERERSIGNALBASE_H
