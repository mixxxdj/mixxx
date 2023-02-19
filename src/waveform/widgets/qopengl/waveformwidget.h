#pragma once

#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wglwidget.h"

class WWaveformViewer;

namespace qopengl {
class WaveformWidget;
}

class qopengl::WaveformWidget : public ::WGLWidget,
                                public ::WaveformWidgetAbstract {
    Q_OBJECT
  public:
    explicit WaveformWidget(const QString& group, QWidget* parent);
    ~WaveformWidget() override;

    // override for WaveformWidgetAbstract
    mixxx::Duration render() override;

    // override for WGLWidget
    void renderGL() override;

    // overrides for WGLWidget
    void initializeGL() override;

    virtual WGLWidget* getGLWidget() override {
        return this;
    }

  private:
    // We need to forward events coming from the QOpenGLWindow
    // (drag&drop, mouse) to the viewer
    void handleEventFromWindow(QEvent* ev) override;
};
