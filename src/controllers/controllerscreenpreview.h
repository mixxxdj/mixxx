#pragma once

#include <QLabel>
#include <QWidget>
#include <array>
#include <chrono>

#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "util/duration.h"
#include "util/parented_ptr.h"

/// Widget to preview controller screen, used in preference window. This is
/// useful to help when developing new screen layout, without inducing any wear
/// and tear on a hardware device, or allow testing when not owning a device
/// using onboard screens. This can also be used to provide debugging
/// information as user can easily take a screenshot of what they see on the
/// device.
class ControllerScreenPreview : public QWidget {
    Q_OBJECT
  public:
    ControllerScreenPreview(QWidget* parent,
            const LegacyControllerMapping::ScreenInfo& screen);
  public slots:
    void updateFrame(const LegacyControllerMapping::ScreenInfo& screen, const QImage& frame);

  private:
    LegacyControllerMapping::ScreenInfo m_screenInfo;

    parented_ptr<QLabel> m_pFrame;
    parented_ptr<QLabel> m_pStat;

    double m_averageFrameDuration;
    using Clock = std::chrono::steady_clock;
    Clock::time_point m_lastFrameTimestamp;
};
