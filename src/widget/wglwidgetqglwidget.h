#pragma once

#ifndef WGLWIDGET_H
#error "Do not include this file, include wglwidget.h instead"
#endif

#include <QGLWidget>

class WGLWidget : public QGLWidget {
  public:
    WGLWidget(QWidget* pParent);

    bool isContextValid() const;
    bool shouldRender() const;
    void makeCurrentIfNeeded();

  protected:
    QPaintDevice* paintDevice() {
        return this;
    }
};
