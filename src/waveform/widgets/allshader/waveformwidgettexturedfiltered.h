#pragma once

#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderertextured.h"
#include "waveform/widgets/allshader/waveformwidget.h"

class WaveformWidgetFactory;

namespace allshader {
class WaveformWidgetTexturedFiltered;
}

class allshader::WaveformWidgetTexturedFiltered final : public allshader::WaveformWidget {
    Q_OBJECT
  public:
    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::AllShaderTexturedFiltered;
    }

    static inline QString getWaveformWidgetName() {
        return tr("Filtered");
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
    WaveformWidgetTexturedFiltered(const QString& group, QWidget* parent);
    friend class ::WaveformWidgetFactory;

    DISALLOW_COPY_AND_ASSIGN(WaveformWidgetTexturedFiltered);
};
