#ifndef QTWAVEFORMWIDGET_H
#define QTWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtWaveformWidget(const char* group, QWidget* parent);
    virtual ~QtWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::QtWaveform;}

    static inline QString getWaveformWidgetName() { return tr("Filtered") + " - Qt";}
    static inline bool useOpenGl() { return true;}
    static inline bool useOpenGLShaders() { return false;}

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual void postRender();

  private:
    QtWaveformWidget() {}
    friend class WaveformWidgetFactory;
};

#endif // QTWAVEFORMWIDGET_H
