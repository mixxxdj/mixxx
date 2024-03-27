#pragma once

#include <QLabel>
#include <QWidget>

#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "util/duration.h"
#include "util/parented_ptr.h"

#define CONTROLLER_SCREEN_PREVIEW_FRAME_HISTORY_SIZE 5

/// Widget to preview controller screen
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
    uint m_frameDurationHistory[CONTROLLER_SCREEN_PREVIEW_FRAME_HISTORY_SIZE];

    mixxx::Duration m_lastFrameTimespamp;
};
