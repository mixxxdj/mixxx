#pragma once

// #define MIXXX_USE_QGLWIDGET

#ifdef MIXXX_USE_QGLWIDGET

#include <QGLWidget>

class WGLWidget : public QGLWidget {
  public:
    WGLWidget(QWidget* parent);
    bool isContextValid() const;
    bool isContextSharing() const;
    void makeCurrentIfNeeded();
};

#else

#include <util/performancetimer.h>

#include <QOpenGLWindow>
#include <QWidget>

class WGLWidget;

class OpenGLWindow : public QOpenGLWindow {
    Q_OBJECT

    WGLWidget* m_pWidget;

  public:
    OpenGLWindow(WGLWidget* widget);
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    bool event(QEvent* ev) override;

    const PerformanceTimer& getTimer() const;
    int getMicrosUntilSwap() const;
  public slots:
    void onFrameSwapped();

  private:
    PerformanceTimer& getMyTimer() const;
};

class WGLWidget : public QWidget {
    OpenGLWindow* m_pOpenGLWindow{};
    QWidget* m_pContainerWidget{};

  public:
    WGLWidget(QWidget* parent);
    bool isContextValid() const;
    bool isContextSharing() const;
    void setAutoBufferSwap(bool);
    void makeCurrentIfNeeded();
    bool isValid() const;
    void resizeEvent(QResizeEvent* event);
    virtual void preRenderGL(OpenGLWindow* w);
    virtual void renderGL(OpenGLWindow* w);
    virtual void initializeGL();

    void handleEventFromWindow(QEvent* ev);

    void swapBuffers();
};

#endif
