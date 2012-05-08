#ifndef GLSIMPLEWAVEFORMWIDGET_H
#define GLSIMPLEWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLSimpleWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLSimpleWaveformWidget(const char* group, QWidget* parent);
    virtual ~GLSimpleWaveformWidget();

    virtual QString getWaveformWidgetName() { return tr("Simple");}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLSimpleWaveform;}

    virtual bool useOpenGl() const { return true;}
    virtual bool useOpenGLShaders() const { return false;}

    //reimplemet it to discard use of rate adjust
    virtual void updateVisualSamplingPerPixel();

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void postRender();

  private:
    GLSimpleWaveformWidget() {}
    friend class WaveformWidgetFactory;
};
#endif // GLSIMPLEWAVEFORMWIDGET_H
