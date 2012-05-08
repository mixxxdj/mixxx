#ifndef GLWAVEFORMWIDGETSHADER_H
#define GLWAVEFORMWIDGETSHADER_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLSLWaveformRendererSignal;

class GLSLWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLSLWaveformWidget(const char* group, QWidget* parent);
    virtual ~GLSLWaveformWidget();

    virtual QString getWaveformWidgetName() { return tr("Filtered (experimental)");}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLWaveform;}

    virtual bool useOpenGl() const { return true;}
    virtual bool useOpenGLShaders() const { return true;}

    virtual void resize( int width, int height);

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent *);

  private:
    GLSLWaveformRendererSignal* signalRenderer_;

    GLSLWaveformWidget() {}
    friend class WaveformWidgetFactory;
};

#endif // GLWAVEFORMWIDGETSHADER_H
