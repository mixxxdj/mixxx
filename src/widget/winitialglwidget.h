#pragma once

#include "widget/wglwidget.h"

class WInitialGLWidget : public WGLWidget {
    Q_OBJECT
  public:
    WInitialGLWidget(QWidget* parent);

    void initializeGL() override;
    void paintGL() override;
  signals:
    void onInitialized();
};
