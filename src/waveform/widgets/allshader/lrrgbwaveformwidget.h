#pragma once

#include "util/class.h"
#include "waveform/widgets/allshader/waveformwidget.h"

class WaveformWidgetFactory;

namespace allshader {
class LRRGBWaveformWidget;
}

class allshader::LRRGBWaveformWidget final : public allshader::WaveformWidget {
    Q_OBJECT
  public:
    ~LRRGBWaveformWidget() override;

    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::AllShaderLRRGBWaveform;
    }

    static inline QString getWaveformWidgetName() {
        return tr("RGB-L/R (all-shaders)");
    }
    static constexpr bool useOpenGl() {
        return true;
    }
    static constexpr bool useOpenGles() {
        return false;
    }
    static constexpr bool useOpenGLShaders() {
        return true;
    }
    static constexpr bool developerOnly() {
        return false;
    }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    LRRGBWaveformWidget(const QString& group, QWidget* parent);
    friend class ::WaveformWidgetFactory;

    DISALLOW_COPY_AND_ASSIGN(LRRGBWaveformWidget);
};
