#pragma once

#include <vector>

#include "skin/legacy/skincontext.h"
#include "waveform/renderers/waveformmarkrange.h"
#include "waveform/renderers/waveformrendererabstract.h"

class QPaintEvent;
class QPainter;
class WaveformWidgetRenderer;

class WaveformRenderMarkRange : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRenderMarkRange() override = default;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    void generateImages();

    std::vector<WaveformMarkRange> m_markRanges;
};
