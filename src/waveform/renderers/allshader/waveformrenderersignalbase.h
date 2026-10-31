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
    void draw(QPainter* painter, QPaintEvent* event) override final;

    static constexpr float m_maxValue{static_cast<float>(std::numeric_limits<uint8_t>::max())};

    explicit WaveformRendererSignalBase(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererSignalBase::Options options);

    virtual bool supportsSlip() const {
        return false;
    }

  public slots:
    void setAxesColor(const QColor& axesColor);
    void setColor(const QColor& lowColor);
    void setLowColor(const QColor& lowColor);
    void setMidColor(const QColor& midColor);
    void setHighColor(const QColor& highColor);

    void setIgnoreStem(bool value) {
        m_ignoreStem = value;
    }

  protected:
    bool m_ignoreStem;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSignalBase);
};
