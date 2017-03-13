#ifndef VSYNCTHREAD_H
#define VSYNCTHREAD_H

#include <QTime>
#include <QThread>
#include <QSemaphore>
#include <QPair>

#if defined(__APPLE__)

#elif defined(__WINDOWS__)

#else
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    #include <qx11info_x11.h>
#endif // QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#endif

#include "util/performancetimer.h"


#if defined(__APPLE__)

#elif defined(__WINDOWS__)

#else
    #include <GL/glx.h>
    #include "GL/glxext.h"
    // clean up after Xlib.h, which #defines values that conflict with QT.
    #undef Bool
    #undef Unsorted
#endif

class QGLWidget;
class GuiTick;
class MixxxMainWindow;

class VSyncThread : public QThread {
    Q_OBJECT
  public:
    enum VSyncMode {
        ST_TIMER = 0,
        ST_MESA_VBLANK_MODE_1,
        ST_SGI_VIDEO_SYNC,
        ST_OML_SYNC_CONTROL,
        ST_FREE,
        ST_COUNT // Dummy Type at last, counting possible types
    };

    static void swapGl(QGLWidget* glw, int index);

    VSyncThread(MixxxMainWindow* mixxMainWindow);
    ~VSyncThread();

    void run();
    void stop();

    bool waitForVideoSync(QGLWidget* glw);
    int elapsed();
    int usToNextSync();
    void setUsSyncIntervalTime(int usSyncTimer);
    void setVSyncType(int mode);
    int droppedFrames();
    void setSwapWait(int sw);
    int usFromTimerToNextSync(PerformanceTimer* timer);
    void vsyncSlotFinished();
    void getAvailableVSyncTypes(QList<QPair<int, QString > >* list);
    void setupSync(QGLWidget* glw, int index);
    void waitUntilSwap(QGLWidget* glw);

  signals:
    void vsyncRender();
    void vsyncSwap();

  private:
    bool m_bDoRendering;
    QGLWidget *m_glw;

#if defined(__APPLE__)

#elif defined(__WINDOWS__)

#else
    void initGlxext(QGLWidget* glw);
    bool glXExtensionSupported(Display *dpy, int screen, const char *extension);

    PFNGLXGETVIDEOSYNCSGIPROC glXGetVideoSyncSGI;
    PFNGLXWAITVIDEOSYNCSGIPROC glXWaitVideoSyncSGI;

    PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI;

    PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;

    PFNGLXGETSYNCVALUESOMLPROC glXGetSyncValuesOML;
    PFNGLXGETMSCRATEOMLPROC glXGetMscRateOML;
    PFNGLXSWAPBUFFERSMSCOMLPROC glXSwapBuffersMscOML;
    PFNGLXWAITFORMSCOMLPROC glXWaitForMscOML;
    PFNGLXWAITFORSBCOMLPROC  glXWaitForSbcOML;

    PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalMESA;

    uint m_counter;

    int64_t m_target_msc;
    Display* m_dpy;
    GLXDrawable m_drawable;

#endif

    bool m_vSyncTypeChanged;
    int m_usSyncIntervalTime;
    int m_usWaitToSwap;
    enum VSyncMode m_vSyncMode;
    bool m_syncOk;
    int m_droppedFrames;
    int m_swapWait;
    PerformanceTimer m_timer;
    QSemaphore m_semaVsyncSlot;
    double m_displayFrameRate;
    int m_vSyncPerRendering;


    GuiTick* m_pGuiTick;
};


#endif // VSYNCTHREAD_H
