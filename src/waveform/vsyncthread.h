#ifndef VSYNCTHREAD_H
#define VSYNCTHREAD_H

#include <QTime>
#include <QThread>
#include <QSemaphore>
#include <QPair>
#include <QOpenGLWidget>

#if defined(__APPLE__)

#elif defined(__WINDOWS__)

#else
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#ifndef QT_OPENGL_ES_2
    #include <qx11info_x11.h>
    #include <GL/glx.h>
    //#include "GL/glxext.h"
    // clean up after Xlib.h, which #defines values that conflict with QT.
    #undef Bool
    #undef Unsorted
    #undef None
    #undef Status
#endif // QT_OPENGL_ES_2
#endif // QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#endif

#include "util/performancetimer.h"

// TODO(rryan): Rename to RenderThread or something, or possibly replace with a
// generic BlockingThreadTimer class.
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

    VSyncThread(QObject* pParent);
    ~VSyncThread();

    void run();
    void stop();

    int elapsed();
    int toNextSyncMicros();
    void setSyncIntervalTimeMicros(int usSyncTimer);
    void setVSyncType(int mode);
    int droppedFrames();
    int fromTimerToNextSyncMicros(const PerformanceTimer& timer);
    void vsyncSlotFinished();
    void getAvailableVSyncTypes(QList<QPair<int, QString > >* list);

  signals:
    void vsyncRender();

  private:
    bool m_bDoRendering;

#if defined(__APPLE__)

#elif defined(__WINDOWS__)

#else
    //bool glXExtensionSupported(Display *dpy, int screen, const char *extension);

    /* Currently unused, but probably part of later a hardware sync solution
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
    */

    //int64_t m_target_msc;
    //Display* m_dpy;
    //GLXDrawable m_drawable;

#endif

    bool m_vSyncTypeChanged;
    int m_syncIntervalTimeMicros;
    int m_waitToSwapMicros;
    enum VSyncMode m_vSyncMode;
    bool m_syncOk;
    int m_droppedFrames;
    int m_swapWait;
    PerformanceTimer m_timer;
    QSemaphore m_semaVsyncSlot;
    double m_displayFrameRate;
    int m_vSyncPerRendering;
};


#endif // VSYNCTHREAD_H
