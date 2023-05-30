#pragma once

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(WGLWidget);

// Creating a QGLContext on its own doesn't work. We've tried that. You can't
// create a context on your own. It has to be associated with a real paint
// device. Source:
// http://lists.trolltech.com/qt-interest/2008-08/thread00046-0.html
class SharedGLContext {
  public:
    static WGLWidget* getWidget();
    static void setWidget(WGLWidget* pWidget);

  private:
    SharedGLContext() { }
    static WGLWidget* s_pSharedGLWidget;
};
