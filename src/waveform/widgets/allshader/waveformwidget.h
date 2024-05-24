#pragma once

#include "waveform/renderers/allshader/waveformrenderersignalbase.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "waveform/widgets/waveformwidgetvars.h"
#include "widget/wglwidget.h"

namespace allshader {
class WaveformWidget;
}

class allshader::WaveformWidget final : public ::WGLWidget,
                                        public ::WaveformWidgetAbstract {
    Q_OBJECT
  public:
    explicit WaveformWidget(QWidget* parent,
            WaveformWidgetType::Type type,
            const QString& group,
            WaveformRendererSignalBase::Options options);
    ~WaveformWidget() override;

    WaveformWidgetType::Type getType() const override {
        return m_type;
    }

    // override for WaveformWidgetAbstract
    mixxx::Duration render() override;

    // overrides for WGLWidget
    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    WGLWidget* getGLWidget() override {
        return this;
    }
    static WaveformWidgetVars vars();
    static WaveformRendererSignalBase::Options supportedOptions(WaveformWidgetType::Type type);

  private:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;

    allshader::WaveformRendererSignalBase* addWaveformSignalRenderer(
            WaveformWidgetType::Type type,
            WaveformRendererSignalBase::Options options,
            ::WaveformRendererAbstract::PositionSource positionSource);

    WaveformWidgetType::Type m_type;

    DISALLOW_COPY_AND_ASSIGN(WaveformWidget);
};
