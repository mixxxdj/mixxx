#pragma once

#include "util/class.h"
#include "waveform/widgets/qopengl/waveformwidget.h"

class WaveformWidgetFactory;

namespace qopengl {
class SimpleWaveformWidget;
}

class qopengl::SimpleWaveformWidget final : public qopengl::WaveformWidget {
    Q_OBJECT
  public:
    ~SimpleWaveformWidget() override;

    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::QOpenGLSimpleWaveform;
    }

    static inline QString getWaveformWidgetName() {
        return tr("Simple (all-shaders)");
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
    SimpleWaveformWidget(const QString& group, QWidget* parent);
    friend class ::WaveformWidgetFactory;

    DISALLOW_COPY_AND_ASSIGN(SimpleWaveformWidget);
};
