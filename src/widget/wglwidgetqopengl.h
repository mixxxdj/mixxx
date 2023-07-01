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
class TrackDropTarget;

class WGLWidget : public QWidget {
  public:
    WGLWidget(QWidget* parent);
    ~WGLWidget();

    bool isContextValid() const;
    bool isContextSharing() const;

    bool shouldRender() const;

    void makeCurrentIfNeeded();
    void doneCurrent();

    void swapBuffers();

    void updateGL();

    // called (indirectly) by WaveformWidgetFactory
    virtual void paintGL();
    // called by OpenGLWindow
    virtual void resizeGL(int w, int h);
    virtual void initializeGL();

    void setTrackDropTarget(TrackDropTarget* pTarget);
    TrackDropTarget* trackDropTarget() const;

  protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    QPaintDevice* paintDevice();

  private:
    OpenGLWindow* m_pOpenGLWindow;
    QWidget* m_pContainerWidget;
    TrackDropTarget* m_pTrackDropTarget;
};
