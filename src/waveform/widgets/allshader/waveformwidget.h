#pragma once

#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wglwaveformwidget.h"

namespace allshader {
class WaveformWidget;
}

class allshader::WaveformWidget : public ::WGLWaveformWidget,
                                  public ::WaveformWidgetAbstract {
    Q_OBJECT
  public:
    explicit WaveformWidget(const QString& group, QWidget* parent);
    ~WaveformWidget() override;

    // override for WaveformWidgetAbstract
    mixxx::Duration render() override;

    // overrides for WGLWidget
    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    virtual WGLWidget* getGLWidget() override {
        return this;
    }

  private:
};
