#ifndef QTWAVEFORMWIDGET_H
#define QTWAVEFORMWIDGET_H

#include <QOpenGLWidget>

#include "waveformwidgetabstract.h"

class QtWaveformWidget : public QOpenGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtWaveformWidget(const char* group, QWidget* parent);
    virtual ~QtWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::QtWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    friend class WaveformWidgetFactory;
};

#endif // QTWAVEFORMWIDGET_H
