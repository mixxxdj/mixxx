#pragma once

#include "widget/wglwidget.h"

class WInitialGLWidget : public WGLWidget {
    Q_OBJECT
  public:
    WInitialGLWidget(QWidget* parent);

    void initializeGL() override;
  signals:
    void onInitialized();
};
