#ifndef GLWAVEFORMWIDGET_H
#define GLWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLWaveformWidget(const char* group, QWidget* parent);
    virtual ~GLWaveformWidget();

    virtual QString getWaveformWidgetName() { return tr("Filtered");}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLSLWaveform;}

    virtual bool useOpenGl() const { return true;}
    virtual bool useOpenGLShaders() const { return false;}

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void postRender();

  private:
    GLWaveformWidget() {}
    friend class WaveformWidgetFactory;
};

#endif // GLWAVEFORMWIDGET_H
