#pragma once

#include "util/class.h"
#include "waveform/widgets/allshader/waveformwidget.h"

class WaveformWidgetFactory;

namespace allshader {
class SimpleWaveformWidget;
}

class allshader::SimpleWaveformWidget final : public allshader::WaveformWidget {
    Q_OBJECT
  public:
    ~SimpleWaveformWidget() override;

    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::AllShaderSimpleWaveform;
    }

    static inline QString getWaveformWidgetName() {
        return tr("Simple");
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
    static constexpr WaveformWidgetCategory category() {
        return WaveformWidgetCategory::AllShader;
    }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    SimpleWaveformWidget(const QString& group, QWidget* parent);
    friend class ::WaveformWidgetFactory;

    DISALLOW_COPY_AND_ASSIGN(SimpleWaveformWidget);
};
