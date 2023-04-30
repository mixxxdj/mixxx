#pragma once

#include "widget/wglwidget.h"

class WGLWaveformWidget : public WGLWidget {
  public:
    WGLWaveformWidget(QWidget* parent);
#ifdef MIXXX_USE_QOPENGL
    bool event(QEvent* event) override;
#endif
};
