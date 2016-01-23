#ifndef GLWAVEFORMWIDGETSHADER_H
#define GLWAVEFORMWIDGETSHADER_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLSLWaveformRendererSignal;

class GLSLWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLSLWaveformWidget(const char* group, QWidget* parent,
                       bool rgbRenderer);
    virtual ~GLSLWaveformWidget();

    virtual void resize(int width, int height);

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual mixxx::Duration render();

  private:
    GLSLWaveformRendererSignal* signalRenderer_;

    friend class WaveformWidgetFactory;
};

class GLSLFilteredWaveformWidget : public GLSLWaveformWidget {
  public:
    GLSLFilteredWaveformWidget(const char* group, QWidget* parent);
    virtual ~GLSLFilteredWaveformWidget() {}

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLSLFilteredWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return true; }
    static inline bool developerOnly() { return false; }
};

class GLSLRGBWaveformWidget : public GLSLWaveformWidget {
  public:
    GLSLRGBWaveformWidget(const char* group, QWidget* parent);
    virtual ~GLSLRGBWaveformWidget() {}

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLSLRGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return true; }
    static inline bool developerOnly() { return false; }
};


#endif // GLWAVEFORMWIDGETSHADER_H
