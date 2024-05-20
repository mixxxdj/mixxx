#pragma once

#include "waveform/widgets/waveformwidgetabstract.h"
#include "waveform/widgets/waveformwidgetinfo.h"
#include "widget/wglwidget.h"

namespace allshader {
class WaveformWidget;
}

class allshader::WaveformWidget : public ::WGLWidget,
                                  public ::WaveformWidgetAbstract {
    Q_OBJECT
  public:
    explicit WaveformWidget(const WaveformWidgetInfoBase& info,
            const QString& group,
            QWidget* parent);
    ~WaveformWidget() override;

    // override for WaveformWidgetAbstract
    mixxx::Duration render() override;

    // overrides for WGLWidget
    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    WGLWidget* getGLWidget() override {
        return this;
    }
    WaveformWidgetType::Type getType() const override {
        return m_info.m_type;
    }
    void castToQWidget() override {
        m_widget = this;
    }

    static void registerInfos();

  private:
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;

    const WaveformWidgetInfoBase& m_info;
};
