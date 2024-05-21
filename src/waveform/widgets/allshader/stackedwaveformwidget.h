#pragma once

#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderertextured.h"
#include "waveform/widgets/allshader/waveformwidget.h"

class WaveformWidgetFactory;

namespace allshader {
class StackedWaveformWidget;
}

class allshader::StackedWaveformWidget final : public allshader::WaveformWidget {
    Q_OBJECT
  public:
    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::Stacked;
    }

    static inline QString getWaveformWidgetName() {
        return tr("RGB Stacked");
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
        return WaveformRendererSignalBase::HighDetail;
    }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    StackedWaveformWidget(const QString& group,
            QWidget* parent,
            WaveformRendererSignalBase::Options options =
                    WaveformRendererSignalBase::None);
    friend class ::WaveformWidgetFactory;

    DISALLOW_COPY_AND_ASSIGN(StackedWaveformWidget);
};
