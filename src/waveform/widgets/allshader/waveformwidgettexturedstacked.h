#pragma once

#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderertextured.h"
#include "waveform/widgets/allshader/waveformwidget.h"

class WaveformWidgetFactory;

namespace allshader {
class WaveformWidgetTexturedStacked;
}

class allshader::WaveformWidgetTexturedStacked final : public allshader::WaveformWidget {
    Q_OBJECT
  public:
    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::AllShaderTexturedStacked;
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
    static constexpr bool useTextureForWaveform() {
        return true;
    }
    static constexpr WaveformWidgetCategory category() {
        return WaveformWidgetCategory::AllShader;
    }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    WaveformWidgetTexturedStacked(const QString& group, QWidget* parent);
    friend class ::WaveformWidgetFactory;

    DISALLOW_COPY_AND_ASSIGN(WaveformWidgetTexturedStacked);
};
