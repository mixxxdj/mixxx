#pragma once

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtWaveformWidget(const QString& group, QWidget* parent);
    virtual ~QtWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::QtWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    friend class WaveformWidgetFactory;
};
