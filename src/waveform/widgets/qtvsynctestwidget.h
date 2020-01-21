#pragma once

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class QtVSyncTestWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtVSyncTestWidget(const QString& group, QWidget* parent);
    virtual ~QtVSyncTestWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::QtVSyncTest; }

    static inline QString getWaveformWidgetName() { return tr("VSyncTest") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return true; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    friend class WaveformWidgetFactory;
};
