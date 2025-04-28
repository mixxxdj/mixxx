#pragma once

#include <QFlags>
#include <limits>

#include "rendergraph/node.h"
#include "util/class.h"
#include "waveform/renderers/waveformrenderersignalbase.h"

class WaveformWidgetRenderer;

namespace allshader {
class WaveformRendererSignalBase;
} // namespace allshader

class allshader::WaveformRendererSignalBase : public ::WaveformRendererSignalBase {
  public:
    enum class Option {
        None = 0b0,
        SplitStereoSignal = 0b1,
        HighDetail = 0b10,
        AllOptionsCombined = SplitStereoSignal | HighDetail,
    };
    Q_DECLARE_FLAGS(Options, Option)

    void draw(QPainter* painter, QPaintEvent* event) override final;

    static constexpr float m_maxValue{static_cast<float>(std::numeric_limits<uint8_t>::max())};

    explicit WaveformRendererSignalBase(WaveformWidgetRenderer* waveformWidget);

    virtual bool supportsSlip() const {
        return false;
    }

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSignalBase);
};
