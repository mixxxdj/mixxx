#pragma once

#include "widget/wglwidget.h"

class WInitialGLWidget : public WGLWidget {
    Q_OBJECT
  private:
    bool m_windowExposedCalled{};

  public:
    WInitialGLWidget(QWidget* parent);

    void windowExposed() override;
  signals:
    void onWindowExposed();
};
