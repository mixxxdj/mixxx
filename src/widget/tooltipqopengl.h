#pragma once

#include <QPoint>
#include <QTimer>
#include <QToolTip>

class WGLWidget;

// Tooltips don't work for the qopengl-based WGLWidget. This
// singleton mimics the standard tooltip behaviour for them.
class ToolTipQOpenGL : public QObject {
    Q_OBJECT

    bool m_active{true};
    QTimer m_timer;
    QPoint m_pos{};
    WGLWidget* m_widget{};
    ToolTipQOpenGL();

  public:
    static ToolTipQOpenGL& singleton();
    void setActive(bool active);
    void start(WGLWidget* widget, QPoint pos);
    void stop(WGLWidget* widget);
  private slots:
    void onTimeout();
};
