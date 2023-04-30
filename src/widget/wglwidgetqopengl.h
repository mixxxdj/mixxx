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
    QWidget* m_pEventReceiver;

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

    // By default the QOpenGLWindow will send its event to its owner
    // WGLWidget. This function allows to override that with a different
    // receiver. This is used for WWaveformViewer.
    void setEventReceiver(QWidget* target) {
        m_pEventReceiver = target;
    }

    QWidget* eventReceiver() const {
        return m_pEventReceiver;
    }

    // called (indirectly) by WaveformWidgetFactory
    virtual void renderGL();
    // called by OpenGLWindow
    virtual void resizeGL(int w, int h);
    virtual void initializeGL();
    virtual void windowExposed();

  protected:
    QPaintDevice* paintDevice();
    void clearPaintDevice();
};
