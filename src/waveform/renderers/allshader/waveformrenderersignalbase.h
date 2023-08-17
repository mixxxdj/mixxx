#pragma once

#include "util/class.h"
#include "waveform/renderers/allshader/waveformrendererabstract.h"
#include "waveform/renderers/waveformrenderersignalbase.h"

class WaveformWidgetRenderer;

namespace allshader {
class WaveformRendererSignalBase;
}

class allshader::WaveformRendererSignalBase : public ::WaveformRendererSignalBase,
                                              public allshader::WaveformRendererAbstract {
  public:
    explicit WaveformRendererSignalBase(WaveformWidgetRenderer* waveformWidget);

    void draw(QPainter* painter, QPaintEvent* event) override {
    }

    allshader::WaveformRendererAbstract* allshaderWaveformRenderer() override {
        return this;
    }

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSignalBase);
};
