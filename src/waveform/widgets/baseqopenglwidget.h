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

    virtual mixxx::Duration render() override {
        WaveformWidgetAbstract::render();
        return mixxx::Duration();
    }

  private slots:
    void slotAboutToCompose();
    void slotFrameSwapped();

  private:
    mixxx::Duration m_lastRender;
    mixxx::Duration m_lastSwapRender;
    mixxx::Duration m_lastSwapDuration;
    mixxx::Duration m_lastSwapDurationMovingAverage;
};
