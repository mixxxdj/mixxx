#ifndef QTRGBWAVEFORMWIDGET_H
#define QTRGBWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtRGBWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~QtRGBWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::QtRGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    QtRGBWaveformWidget(const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // QTRGBWAVEFORMWIDGET_H
