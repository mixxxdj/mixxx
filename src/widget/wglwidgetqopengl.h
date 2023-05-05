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
    QWidget* m_pWindowEventTarget{};

  public:
    WGLWidget(QWidget* parent);
    ~WGLWidget();

    bool isContextValid() const;
    bool isContextSharing() const;

    bool shouldRender() const;

    void makeCurrentIfNeeded();
    void doneCurrent();

    void swapBuffers();

    void setWindowEventTarget(QWidget* target);
    QWidget* windowEventTarget() const;

    // called (indirectly) by WaveformWidgetFactory
    virtual void paintGL();
    // called by OpenGLWindow
    virtual void resizeGL(int w, int h);
    virtual void initializeGL();

  protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    QPaintDevice* paintDevice();
};
