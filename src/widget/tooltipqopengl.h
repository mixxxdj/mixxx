#pragma once

#include <QStyle>
#include <QTimer>
#include <QToolTip>

#include "widget/wglwidget.h"

// Tooltips don't work for the qopengl-based WGLWidget. This
// singleton mimics the standard tooltip behaviour for them.
class ToolTipQOpenGL : public QObject {
    bool m_active{true};
    QTimer* m_timer{};
    QPoint m_pos{};
    WGLWidget* m_widget{};
    ToolTipQOpenGL();

  public:
    ~ToolTipQOpenGL();
    static ToolTipQOpenGL* singleton();
    void setActive(bool active);
    void start(WGLWidget* widget, QPoint pos);
    void stop(WGLWidget* widget);
};
