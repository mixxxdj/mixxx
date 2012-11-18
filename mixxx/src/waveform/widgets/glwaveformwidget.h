#ifndef GLWAVEFORMWIDGET_H
#define GLWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLWaveformWidget(const char* group, QWidget* parent);
    virtual ~GLWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLSLWaveform;}

    static inline QString getWaveformWidgetName() { return tr("Filtered");}
    static inline bool useOpenGl() { return true;}
    static inline bool useOpenGLShaders() { return false;}

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void postRender();

  private:
    GLWaveformWidget() {}
    friend class WaveformWidgetFactory;
};

#endif // GLWAVEFORMWIDGET_H
