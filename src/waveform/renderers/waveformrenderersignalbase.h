#pragma once

#include "waveformrendererabstract.h"
#include "waveformsignalcolors.h"
#include "skin/skincontext.h"

class ControlObject;
class ControlProxy;

class WaveformRendererSignalBase : public WaveformRendererAbstract {
public:
    explicit WaveformRendererSignalBase(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererSignalBase();

    virtual bool init();
    virtual void setup(const QDomNode& node, const SkinContext& context);

    virtual bool onInit() {return true;}
    virtual void onSetup(const QDomNode &node) = 0;

  protected:
    void deleteControls();

    void getGains(float* pAllGain, float* pLowGain, float* pMidGain,
                  float* highGain);

  protected:
    ControlProxy* m_pEQEnabled;
    ControlProxy* m_pLowFilterControlObject;
    ControlProxy* m_pMidFilterControlObject;
    ControlProxy* m_pHighFilterControlObject;
    ControlProxy* m_pLowKillControlObject;
    ControlProxy* m_pMidKillControlObject;
    ControlProxy* m_pHighKillControlObject;

    Qt::Alignment m_alignment;
    Qt::Orientation m_orientation;

    const WaveformSignalColors* m_pColors;
    qreal m_axesColor_r, m_axesColor_g, m_axesColor_b, m_axesColor_a;
    qreal m_signalColor_r, m_signalColor_g, m_signalColor_b;
    qreal m_lowColor_r, m_lowColor_g, m_lowColor_b;
    qreal m_midColor_r, m_midColor_g, m_midColor_b;
    qreal m_highColor_r, m_highColor_g, m_highColor_b;
    qreal m_rgbLowColor_r, m_rgbLowColor_g, m_rgbLowColor_b;
    qreal m_rgbMidColor_r, m_rgbMidColor_g, m_rgbMidColor_b;
    qreal m_rgbHighColor_r, m_rgbHighColor_g, m_rgbHighColor_b;
};
