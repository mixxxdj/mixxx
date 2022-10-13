#pragma once

#include "waveform/widgets/qopengl/waveformwidget.h"

class WaveformWidgetFactory;

namespace qopengl {
class RGBWaveformWidget;
}

class qopengl::RGBWaveformWidget : public qopengl::WaveformWidget {
    Q_OBJECT
  public:
    ~RGBWaveformWidget() override;

    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::QOpenGLRGBWaveform;
    }

    static inline QString getWaveformWidgetName() {
        return tr("RGB (QOpenGL)");
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
    RGBWaveformWidget(const QString& group, QWidget* parent);
    friend class ::WaveformWidgetFactory;
};
