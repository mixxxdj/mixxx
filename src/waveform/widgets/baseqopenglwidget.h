#pragma once

#include <QObject>
#include <QOpenGLWidget>

#include "util/duration.h"
#include "waveform/widgets/waveformwidgetabstract.h"

class BaseQOpenGLWidget : public QOpenGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    BaseQOpenGLWidget(const char* group, QWidget* pParent);
    virtual ~BaseQOpenGLWidget() = default;

    // Each QOpenGLWidget must define its own render
    // method. WaveformWidgetAbstract::render must not be called even though it
    // is defined.
    virtual mixxx::Duration render() = 0;

    void renderOnNextTick() override {
        m_shouldRenderOnNextTick = true;
        // Request a paint event from Qt.
        update();
    }

  private slots:
    void slotAboutToCompose();
    void slotFrameSwapped();

  private:
    bool m_shouldRenderOnNextTick;
    mixxx::Duration m_lastRender;
    mixxx::Duration m_lastSwapRender;
    mixxx::Duration m_lastSwapDuration;
    mixxx::Duration m_lastSwapDurationMovingAverage;
};
