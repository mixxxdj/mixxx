#pragma once

#include <QLabel>
#include <QWidget>
#include <array>

#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "util/duration.h"
#include "util/parented_ptr.h"

/// Number of sample frame timestamp sample to calculate FPS label.
constexpr int kFrameHistorySize = 5;

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
    uint8_t m_frameDurationHistoryIdx;
    std::array<uint, kFrameHistorySize> m_frameDurationHistory;

    mixxx::Duration m_lastFrameTimestamp;
};
