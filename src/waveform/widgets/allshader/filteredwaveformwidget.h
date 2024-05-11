#pragma once

#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"
#include "waveform/widgets/allshader/waveformwidget.h"

class WaveformWidgetFactory;

namespace allshader {
class FilteredWaveformWidget;
}

class allshader::FilteredWaveformWidget final : public allshader::WaveformWidget {
    Q_OBJECT
  public:
    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::Filtered;
    }

    static constexpr bool useOpenGl() {
        return true;
    }
    static constexpr bool useOpenGles() {
        return true;
    }
    static constexpr bool useOpenGLShaders() {
        return true;
    }
    static constexpr WaveformWidgetCategory category() {
        return WaveformWidgetCategory::AllShader;
    }
    static constexpr int supportedOptions() {
        return WaveformRendererSignalBase::None;
    }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    FilteredWaveformWidget(const QString& group, QWidget* parent);
    friend class ::WaveformWidgetFactory;

    DISALLOW_COPY_AND_ASSIGN(FilteredWaveformWidget);
};
