#ifndef GLWAVEFORMWIDGETSHADER_H
#define GLWAVEFORMWIDGETSHADER_H

#include <QGLWidget>
#include "waveformwidgetabstract.h"

class GLSLWaveformRendererSignal;

class GLSLWaveformWidget : public WaveformWidgetAbstract, public QGLWidget
{
public:
    GLSLWaveformWidget( const char* group, QWidget* parent);
    virtual ~GLSLWaveformWidget();

    virtual QString getWaveformWidgetName() { return "Filtered";}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLWaveform;}

    virtual bool useOpenGl() const { return true;}
    virtual bool useOpenGLShaders() const { return true;}

protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent *);

private:
    GLSLWaveformRendererSignal* signalRenderer_;

private:
    GLSLWaveformWidget() {}
    friend class WaveformWidgetFactory;
};

#endif // GLWAVEFORMWIDGETSHADER_H
