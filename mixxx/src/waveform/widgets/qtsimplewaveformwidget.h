#ifndef QTSIMPLEWAVEFORMWIDGET_H
#define QTSIMPLEWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtSimpleWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtSimpleWaveformWidget(const char* group, QWidget* parent);
    virtual ~QtSimpleWaveformWidget();


    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLSimpleWaveform;}

    static inline QString getWaveformWidgetName() { return tr("Simple") + " - Qt";}
    static inline bool useOpenGl() { return true;}
    static inline bool useOpenGLShaders() { return false;}

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void postRender();
    virtual int render();

  private:
    friend class WaveformWidgetFactory;
};

#endif // QTSIMPLEWAVEFORMWIDGET_H
