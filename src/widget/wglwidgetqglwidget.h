#pragma once

#ifndef WGLWIDGET_H
#error "Do not include this file, include wglwidget.h instead"
#endif

#include <QGLWidget>

class WGLWidget : public QGLWidget {
  public:
    WGLWidget(QWidget* parent);

    bool isContextValid() const;
    bool isContextSharing() const;

    bool shouldRender() const;

    void makeCurrentIfNeeded();

    QPaintDevice* paintDevice() {
        return this;
    }
};
