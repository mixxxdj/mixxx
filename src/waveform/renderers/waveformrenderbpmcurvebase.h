#pragma once

#include <QObject>

#include "waveform/renderers/waveformrendererabstract.h"

class WaveformRenderBpmCurveBase : public QObject, public WaveformRendererAbstract {
    Q_OBJECT

  public:
    explicit WaveformRenderBpmCurveBase(
            WaveformWidgetRenderer* pWaveformWidgetRenderer,
            bool updateImmediately);

    void setup(const QDomNode& node, const SkinContext& context) override;
    void onSetTrack() override;
    void onResize() override;

  protected:
    virtual void updateBpmTextures() = 0;

  private:
    const bool m_updateImmediately;
};
