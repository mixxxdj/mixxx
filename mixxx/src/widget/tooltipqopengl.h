#pragma once

#include <QPoint>
#include <QTimer>

class WGLWidget;

// Tooltips don't work for the qopengl-based WGLWidget. This
// singleton mimics the standard tooltip behaviour for them.
class ToolTipQOpenGL : public QObject {
    Q_OBJECT
  public:
    static ToolTipQOpenGL& singleton();
    void setActive(bool active);
    void start(WGLWidget* pWidget, QPoint pos);
    void stop();

  private slots:
    void onTimeout();

  private:
    ToolTipQOpenGL();

    bool m_active;
    QTimer m_timer;
    QPoint m_pos;
    WGLWidget* m_pWidget;
};
