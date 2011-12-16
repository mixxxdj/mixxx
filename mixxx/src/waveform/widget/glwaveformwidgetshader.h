#ifndef GLWAVEFORMWIDGETSHADER_H
#define GLWAVEFORMWIDGETSHADER_H

#include <QtOpenGL/QGLWidget>
#include "waveformwidgetabstract.h"

class GLWaveformWidgetShader : public WaveformWidgetAbstract, public QGLWidget
{
public:
    GLWaveformWidgetShader( const char* group, QWidget* parent);
    virtual ~GLWaveformWidgetShader();

    virtual QString getWaveformWidgetName() { return "Filtered";}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLWaveform;}

    virtual bool useOpenGl() const { return true;}
    virtual bool useOpenGLShaders() const { return true;}

protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

private:
    GLWaveformWidgetShader() {}
    friend class WaveformWidgetFactory;
};

#endif // GLWAVEFORMWIDGETSHADER_H
