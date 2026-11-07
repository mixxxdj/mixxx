#pragma once

#include <QObject>

#include "waveform/renderers/waveformrendererabstract.h"

class WaveformRenderKeyCurveBase : public QObject, public WaveformRendererAbstract {
    Q_OBJECT

  public:
    explicit WaveformRenderKeyCurveBase(
            WaveformWidgetRenderer* waveformWidgetRenderer,
            bool updateImmediately);

    void setup(const QDomNode& node, const SkinContext& context) override;
    void onSetTrack() override;
    void onResize() override;

  protected:
    virtual void updateKeyTextures() = 0;

  private:
    const bool m_updateImmediately;
};
