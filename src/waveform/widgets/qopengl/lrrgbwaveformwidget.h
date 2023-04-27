#pragma once

#include "util/class.h"
#include "waveform/widgets/qopengl/waveformwidget.h"

class WaveformWidgetFactory;

namespace qopengl {
class LRRGBWaveformWidget;
}

class qopengl::LRRGBWaveformWidget final : public qopengl::WaveformWidget {
    Q_OBJECT
  public:
    ~LRRGBWaveformWidget() override;

    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::QOpenGLLRRGBWaveform;
    }

    static inline QString getWaveformWidgetName() {
        return tr("RGB-L/R (all-shaders)");
    }
    static inline bool useOpenGl() {
        return true;
    }
    static inline bool useOpenGles() {
        return false;
    }
    static inline bool useOpenGLShaders() {
        return true;
    }
    static inline bool developerOnly() {
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
