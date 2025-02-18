#pragma once

#include "skin/legacy/skincontext.h"
#include "util/span.h"
#include "util/types.h"
#include "waveform/waveform.h"
#include "waveformrendererabstract.h"

class ControlProxy;
class WaveformSignalColors;

class WaveformRendererSignalBase : public QObject, public WaveformRendererAbstract {
    Q_OBJECT
  public:
    explicit WaveformRendererSignalBase(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererSignalBase();

    virtual bool init();
    virtual void setup(const QDomNode& node, const SkinContext& context);

    virtual bool onInit() {
        return true;
    }
    virtual void onSetup(const QDomNode& node) = 0;

  public slots:
    void setAllChannelVisualGain(double gain) {
        m_allChannelVisualGain = static_cast<CSAMPLE_GAIN>(gain);
    }
    void setLowVisualGain(double gain) {
        m_lowVisualGain = static_cast<CSAMPLE_GAIN>(gain);
    }
    void setMidVisualGain(double gain) {
        m_midVisualGain = static_cast<CSAMPLE_GAIN>(gain);
    }
    void setHighVisualGain(double gain) {
        m_highVisualGain = static_cast<CSAMPLE_GAIN>(gain);
    }

  protected:
    void deleteControls();

    void getGains(float* pAllGain,
            bool applyCompensation,
            float* pLowGain,
            float* pMidGain,
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

    CSAMPLE_GAIN m_allChannelVisualGain;
    CSAMPLE_GAIN m_lowVisualGain;
    CSAMPLE_GAIN m_midVisualGain;
    CSAMPLE_GAIN m_highVisualGain;

    float m_axesColor_r, m_axesColor_g, m_axesColor_b, m_axesColor_a;
    float m_signalColor_r, m_signalColor_g, m_signalColor_b;
    float m_lowColor_r, m_lowColor_g, m_lowColor_b;
    float m_midColor_r, m_midColor_g, m_midColor_b;
    float m_highColor_r, m_highColor_g, m_highColor_b;
    float m_rgbLowColor_r, m_rgbLowColor_g, m_rgbLowColor_b;
    float m_rgbMidColor_r, m_rgbMidColor_g, m_rgbMidColor_b;
    float m_rgbHighColor_r, m_rgbHighColor_g, m_rgbHighColor_b;
    float m_rgbLowFilteredColor_r, m_rgbLowFilteredColor_g, m_rgbLowFilteredColor_b;
    float m_rgbMidFilteredColor_r, m_rgbMidFilteredColor_g, m_rgbMidFilteredColor_b;
    float m_rgbHighFilteredColor_r, m_rgbHighFilteredColor_g, m_rgbHighFilteredColor_b;
};
