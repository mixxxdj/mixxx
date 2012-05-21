#ifndef QTSIMPLEWAVEFORMWIDGET_H
#define QTSIMPLEWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtSimpleWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtSimpleWaveformWidget(const char* group, QWidget* parent);
    virtual ~QtSimpleWaveformWidget();

    virtual QString getWaveformWidgetName() { return tr("Simple") + " - Qt";}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLSimpleWaveform;}

    virtual bool useOpenGl() const { return true;}
    virtual bool useOpenGLShaders() const { return false;}

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void postRender();

  private:
    QtSimpleWaveformWidget() {}
    friend class WaveformWidgetFactory;
};

#endif // QTSIMPLEWAVEFORMWIDGET_H
