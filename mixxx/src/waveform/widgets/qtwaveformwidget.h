#ifndef QTWAVEFORMWIDGET_H
#define QTWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtWaveformWidget(const char* group, QWidget* parent);
    virtual ~QtWaveformWidget();

    virtual QString getWaveformWidgetName() { return tr("Filtered") + " - Qt";}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLSLWaveform;}

    virtual bool useOpenGl() const { return true;}
    virtual bool useOpenGLShaders() const { return false;}

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void postRender();

  private:
    QtWaveformWidget() {}
    friend class WaveformWidgetFactory;
};

#endif // QTWAVEFORMWIDGET_H
