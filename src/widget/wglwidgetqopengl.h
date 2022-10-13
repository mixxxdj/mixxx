#pragma once

#ifndef WGLWIDGET_H
#error "Do not include this file, include wglwidget.h instead"
#endif

#include <QWidget>

////////////////////////////////
// QOpenGLWindow based WGLWidget
////////////////////////////////

class QPaintDevice;
class OpenGLWindow;

class WGLWidget : public QWidget {
  private:
    OpenGLWindow* m_pOpenGLWindow{};
    QWidget* m_pContainerWidget{};

  public:
    WGLWidget(QWidget* parent);
    ~WGLWidget();

    bool isContextValid() const;
    bool isContextSharing() const;

    bool shouldRender() const;

    void makeCurrentIfNeeded();
    void doneCurrent();

    void swapBuffers();
    void resizeEvent(QResizeEvent* event) override;

    void showEvent(QShowEvent* event) override;

    // called by OpenGLWindow
    virtual void handleEventFromWindow(QEvent* ev);
    virtual void initializeGL();

  protected:
    QPaintDevice* paintDevice();
};
