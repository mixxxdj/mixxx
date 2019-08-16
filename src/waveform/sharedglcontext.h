#ifndef WAVEFORM_SHAREDGLCONTEXT_H
#define WAVEFORM_SHAREDGLCONTEXT_H

class QGLWidget;

// Creating a QGLContext on its own doesn't work. We've tried that. You can't
// create a context on your own. It has to be associated with a real paint
// device. Source:
// http://lists.trolltech.com/qt-interest/2008-08/thread00046-0.html
class SharedGLContext {
  public:
    static QGLWidget* getWidget();
    static void setWidget(QGLWidget* pWidget);
  private:
    SharedGLContext() { }
    static QGLWidget* s_pSharedGLWidget;
};

#endif /* WAVEFORM_SHAREDGLCONTEXT_H */
