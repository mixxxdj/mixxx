#pragma once

#include <QFlags>
#include <limits>

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
    enum class Option {
        None = 0b0,
        SplitStereoSignal = 0b1,
        HighDetail = 0b10,
        BeatGridBar = 0b100,
        BeatGridMarker = 0b1000,
        AdvancedBeatGrid = BeatGridBar | BeatGridMarker,
        AllOptionsCombined = SplitStereoSignal | HighDetail | AdvancedBeatGrid,
    };
    Q_DECLARE_FLAGS(Options, Option)

    static constexpr float m_maxValue{static_cast<float>(std::numeric_limits<uint8_t>::max())};

    explicit WaveformRendererSignalBase(WaveformWidgetRenderer* waveformWidget);

    virtual bool supportsSlip() const {
        return false;
    }

    void draw(QPainter* painter, QPaintEvent* event) override {
        Q_UNUSED(painter);
        Q_UNUSED(event);
    }

    allshader::WaveformRendererAbstract* allshaderWaveformRenderer() override {
        return this;
    }

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSignalBase);
};
