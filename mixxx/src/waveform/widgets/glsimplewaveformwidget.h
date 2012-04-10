#ifndef GLSIMPLEWAVEFORMWIDGET_H
#define GLSIMPLEWAVEFORMWIDGET_H

#include <QtOpenGL/QGLWidget>
#include "waveformwidgetabstract.h"

class GLSimpleWaveformWidget : public WaveformWidgetAbstract, public QGLWidget
{
public:
    GLSimpleWaveformWidget( const char* group, QWidget* parent);
    virtual ~GLSimpleWaveformWidget();

    virtual QString getWaveformWidgetName() { return tr("Simple");}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLSimpleWaveform;}

    virtual bool useOpenGl() const { return true;}
    virtual bool useOpenGLShaders() const { return false;}

protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

private:
    GLSimpleWaveformWidget() {}
    friend class WaveformWidgetFactory;
};
#endif // GLSIMPLEWAVEFORMWIDGET_H
