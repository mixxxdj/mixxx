#pragma once

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtHSVWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~QtHSVWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::QtHSVWaveform; }

    static inline QString getWaveformWidgetName() { return tr("HSV") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    QtHSVWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};
