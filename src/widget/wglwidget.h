#pragma once

#include <QOpenGLWindow>
#include <QWidget>

class WGLWidget : public QWidget {
    QOpenGLWindow* m_pOpenGLWindow;

  public:
    WGLWidget(QWidget* parent, WGLWidget* shareWidget = nullptr);

    void makeCurrent();
    void setAutoBufferSwap(bool);
    QOpenGLContext* context() const;
    virtual void initializeGL();
    bool isValid() const {
        return true;
    }
};
