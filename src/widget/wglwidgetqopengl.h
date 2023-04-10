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

    // called (indirectly) by WaveformWidgetFactory
    virtual void renderGL();
    // called by OpenGLWindow
    virtual void resizeGL(int w, int h);
    // As we add a container widget with an OpenGLWindow as
    // child (on top of) the widget, we will not get events
    // such as mouse and drag/drop from the widget's window.
    // Instead we need to act on events from the OpenGLWindow.
    // Ideally we would have the OpenGLWindow simply call the
    // virtual bool event(QEvent* ev), but this is a protected
    // method, so we have to use this wrapper.
    virtual void handleEventFromWindow(QEvent* ev);
    virtual void initializeGL();
    virtual void windowExposed();

  protected:
    QPaintDevice* paintDevice();
};
