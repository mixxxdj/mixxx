#ifndef GLWAVEFORMWIDGETSHADER_H
#define GLWAVEFORMWIDGETSHADER_H

#include "waveform/widgets/baseqopenglwidget.h"
#include "util/duration.h"

class GLSLWaveformRendererSignal;

class GLSLWaveformWidget : public BaseQOpenGLWidget {
    Q_OBJECT
  public:
    GLSLWaveformWidget(const char* group, QWidget* parent,
                       bool rgbRenderer);
    ~GLSLWaveformWidget() override;

    void resize(int width, int height, float devicePixelRatio) override;

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

  protected slots:
    mixxx::Duration render() override;

  private:
    GLSLWaveformRendererSignal* signalRenderer_;
    friend class WaveformWidgetFactory;
};

class GLSLFilteredWaveformWidget : public GLSLWaveformWidget {
    Q_OBJECT
  public:
    GLSLFilteredWaveformWidget(const char* group, QWidget* parent);
    ~GLSLFilteredWaveformWidget() override = default;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLSLFilteredWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return true; }
    static inline bool developerOnly() { return false; }
};

class GLSLRGBWaveformWidget : public GLSLWaveformWidget {
    Q_OBJECT
  public:
    GLSLRGBWaveformWidget(const char* group, QWidget* parent);
    ~GLSLRGBWaveformWidget() override = default;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLSLRGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return true; }
    static inline bool developerOnly() { return false; }
};


#endif // GLWAVEFORMWIDGETSHADER_H
