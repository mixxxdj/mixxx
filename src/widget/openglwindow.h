#pragma once

#include <QOpenGLWindow>

class WGLWidget;
class TrackDropTarget;

/// Helper class used by wglwidgetqopengl

class OpenGLWindow : public QOpenGLWindow {
    Q_OBJECT

  public:
    OpenGLWindow(WGLWidget* pWidget);
    ~OpenGLWindow();

    void widgetDestroyed();

  private:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    bool event(QEvent* pEv) override;

    WGLWidget* m_pWidget;
    TrackDropTarget* m_pTrackDropTarget;
};
