#ifndef GLWAVEFORMWIDGET_H
#define GLWAVEFORMWIDGET_H

#include <QtOpenGL/QGLWidget>
#include "waveformwidgetabstract.h"

class GLWaveformWidget : public WaveformWidgetAbstract, public QGLWidget
{
public:
    GLWaveformWidget( const char* group, QWidget* parent);
    virtual ~GLWaveformWidget();

    virtual void castToQWidget();
    virtual QString getWaveformWidgetName() { return "Filtered";}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::FilteredOpenGlWaveform;}
    virtual bool useOpenGl() const { return true;}

protected:
    virtual void paintEvent(QPaintEvent* event);
};

#endif // GLWAVEFORMWIDGET_H
