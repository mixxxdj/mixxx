#ifndef SHAREDGLCONTEXT_H_
#define SHAREDGLCONTEXT_H_

class QGLWidget;

// Creating a QGLContext on its own doesn't work. We've tried that. You can't
// create a context on your own. It has to be associated with a real paint
// device. Source:
// http://lists.trolltech.com/qt-interest/2008-08/thread00046-0.html
class SharedGLContext {
  public:
    static const QGLWidget* getWidget();
    static void setWidget(const QGLWidget* pWidget);
  private:
    SharedGLContext() { };
    static const QGLWidget* s_pSharedGLWidget;
};

#endif //SHAREDGLCONTEXT_H_
