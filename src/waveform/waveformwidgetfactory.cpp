#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QWidget>
#include <QtDebug>
#include <QGLFormat>
#include <QGLShaderProgram>
#include <QGuiApplication>
#include <QWindow>

#include "waveform/waveformwidgetfactory.h"

#include "control/controlpotmeter.h"
#include "waveform/widgets/emptywaveformwidget.h"
#include "waveform/widgets/softwarewaveformwidget.h"
#include "waveform/widgets/hsvwaveformwidget.h"
#include "waveform/widgets/rgbwaveformwidget.h"
#include "waveform/widgets/glrgbwaveformwidget.h"
#include "waveform/widgets/glwaveformwidget.h"
#include "waveform/widgets/glsimplewaveformwidget.h"
#include "waveform/widgets/qtwaveformwidget.h"
#include "waveform/widgets/qtsimplewaveformwidget.h"
#include "waveform/widgets/glslwaveformwidget.h"
#include "waveform/widgets/glvsynctestwidget.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wwaveformviewer.h"
#include "waveform/guitick.h"
#include "waveform/vsyncthread.h"
#include "util/cmdlineargs.h"
#include "util/performancetimer.h"
#include "util/timer.h"
#include "util/math.h"

///////////////////////////////////////////

WaveformWidgetAbstractHandle::WaveformWidgetAbstractHandle()
    : m_active(true),
      m_type(WaveformWidgetType::Count_WaveformwidgetType) {
}

///////////////////////////////////////////

WaveformWidgetHolder::WaveformWidgetHolder()
    : m_waveformWidget(NULL),
      m_waveformViewer(NULL),
      m_skinContextCache(UserSettingsPointer(), QString()) {
}

WaveformWidgetHolder::WaveformWidgetHolder(WaveformWidgetAbstract* waveformWidget,
                                           WWaveformViewer* waveformViewer,
                                           const QDomNode& node,
                                           const SkinContext& skinContext)
    : m_waveformWidget(waveformWidget),
      m_waveformViewer(waveformViewer),
      m_skinNodeCache(node.cloneNode()),
      m_skinContextCache(skinContext) {
}

///////////////////////////////////////////

WaveformWidgetFactory::WaveformWidgetFactory() :
        m_type(WaveformWidgetType::Count_WaveformwidgetType),
        m_config(0),
        m_skipRender(false),
        m_frameRate(30),
        m_endOfTrackWarningTime(30),
        m_defaultZoom(WaveformWidgetRenderer::s_waveformDefaultZoom),
        m_zoomSync(false),
        m_overviewNormalized(false),
        m_openGLAvailable(false),
        m_openGLShaderAvailable(false),
        m_beatGridAlpha(90),
        m_vsyncThread(NULL),
        m_frameCnt(0),
        m_actualFrameRate(0),
        m_vSyncType(0),
        m_playMarkerPosition(WaveformWidgetRenderer::s_defaultPlayMarkerPosition) {

    m_visualGain[All] = 1.0;
    m_visualGain[Low] = 1.0;
    m_visualGain[Mid] = 1.0;
    m_visualGain[High] = 1.0;

    if (!CmdlineArgs::Instance().getSafeMode() && QGLFormat::hasOpenGL()) {
        QGLFormat glFormat;
        glFormat.setDirectRendering(true);
        glFormat.setDoubleBuffer(true);
        glFormat.setDepth(false);
        // Disable waiting for vertical Sync
        // This can be enabled when using a single Threads for each QGLContext
        // Setting 1 causes QGLContext::swapBuffer to sleep until the next VSync
#if defined(__APPLE__)
        // On OS X, syncing to vsync has good performance FPS-wise and
        // eliminates tearing.
        glFormat.setSwapInterval(1);
#else
        // Otherwise, turn VSync off because it could cause horrible FPS on
        // Linux.
        // TODO(XXX): Make this configurable.
        // TODO(XXX): What should we do on Windows?
        glFormat.setSwapInterval(0);
#endif


        glFormat.setRgba(true);
        QGLFormat::setDefaultFormat(glFormat);

        QGLFormat::OpenGLVersionFlags version = QGLFormat::openGLVersionFlags();

        int majorVersion = 0;
        int minorVersion = 0;
        if (version == QGLFormat::OpenGL_Version_None) {
            m_openGLVersion = "None";
// Flags introduced in Qt 5.2.
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
        } else if (version & QGLFormat::OpenGL_Version_4_3) {
            majorVersion = 4;
            minorVersion = 3;
        } else if (version & QGLFormat::OpenGL_Version_4_2) {
            majorVersion = 4;
            minorVersion = 2;
        } else if (version & QGLFormat::OpenGL_Version_4_1) {
            majorVersion = 4;
            minorVersion = 1;
#endif
// Flags introduced in Qt 4.7.
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
        } else if (version & QGLFormat::OpenGL_Version_4_0) {
            majorVersion = 4;
            minorVersion = 0;
        } else if (version & QGLFormat::OpenGL_Version_3_3) {
            majorVersion = 3;
            minorVersion = 3;
        } else if (version & QGLFormat::OpenGL_Version_3_2) {
            majorVersion = 3;
            minorVersion = 2;
        } else if (version & QGLFormat::OpenGL_Version_3_1) {
            majorVersion = 3;
            minorVersion = 1;
#endif
        } else if (version & QGLFormat::OpenGL_Version_3_0) {
            majorVersion = 3;
        } else if (version & QGLFormat::OpenGL_Version_2_1) {
            majorVersion = 2;
            minorVersion = 1;
        } else if (version & QGLFormat::OpenGL_Version_2_0) {
            majorVersion = 2;
            minorVersion = 0;
        } else if (version & QGLFormat::OpenGL_Version_1_5) {
            majorVersion = 1;
            minorVersion = 5;
        } else if (version & QGLFormat::OpenGL_Version_1_4) {
            majorVersion = 1;
            minorVersion = 4;
        } else if (version & QGLFormat::OpenGL_Version_1_3) {
            majorVersion = 1;
            minorVersion = 3;
        } else if (version & QGLFormat::OpenGL_Version_1_2) {
            majorVersion = 1;
            minorVersion = 2;
        } else if (version & QGLFormat::OpenGL_Version_1_1) {
            majorVersion = 1;
            minorVersion = 1;
        }

        if (majorVersion != 0) {
            m_openGLVersion = QString::number(majorVersion) + "." +
                    QString::number(minorVersion);
        }

        m_openGLAvailable = true;

        QGLWidget* glWidget = new QGLWidget(); // create paint device
        // QGLShaderProgram::hasOpenGLShaderPrograms(); valgind error
        // Without a makeCurrent, hasOpenGLShaderPrograms returns false on Qt 5.
        glWidget->context()->makeCurrent();
        m_openGLShaderAvailable =
                QGLShaderProgram::hasOpenGLShaderPrograms(glWidget->context());
        delete glWidget;
    }

    evaluateWidgets();
    m_time.start();
}

WaveformWidgetFactory::~WaveformWidgetFactory() {
    if (m_vsyncThread) {
        delete m_vsyncThread;
    }
}

bool WaveformWidgetFactory::setConfig(UserSettingsPointer config) {
    m_config = config;
    if (!m_config) {
        return false;
    }

    bool ok = false;

    int frameRate = m_config->getValueString(ConfigKey("[Waveform]","FrameRate")).toInt(&ok);
    if (ok) {
        setFrameRate(frameRate);
    } else {
        m_config->set(ConfigKey("[Waveform]","FrameRate"), ConfigValue(m_frameRate));
    }

    int endTime = m_config->getValueString(ConfigKey("[Waveform]","EndOfTrackWarningTime")).toInt(&ok);
    if (ok) {
        setEndOfTrackWarningTime(endTime);
    } else {
        m_config->set(ConfigKey("[Waveform]","EndOfTrackWarningTime"),
                ConfigValue(m_endOfTrackWarningTime));
    }

    int vsync = m_config->getValue(ConfigKey("[Waveform]","VSync"), 0);
    setVSyncType(vsync);

    int defaultZoom = m_config->getValueString(ConfigKey("[Waveform]","DefaultZoom")).toInt(&ok);
    if (ok) {
        setDefaultZoom(defaultZoom);
    } else{
        m_config->set(ConfigKey("[Waveform]","DefaultZoom"), ConfigValue(m_defaultZoom));
    }

    int zoomSync = m_config->getValueString(ConfigKey("[Waveform]","ZoomSynchronization")).toInt(&ok);
    if (ok) {
        setZoomSync(static_cast<bool>(zoomSync));
    } else {
        m_config->set(ConfigKey("[Waveform]","ZoomSynchronization"), ConfigValue(m_zoomSync));
    }

    int beatGridAlpha = m_config->getValue(ConfigKey("[Waveform]", "beatGridAlpha"), m_beatGridAlpha);
    setDisplayBeatGridAlpha(beatGridAlpha);

    WaveformWidgetType::Type type = static_cast<WaveformWidgetType::Type>(
            m_config->getValueString(ConfigKey("[Waveform]","WaveformType")).toInt(&ok));
    if (!ok || !setWidgetType(type)) {
        setWidgetType(autoChooseWidgetType());
    }

    for (int i = 0; i < FilterCount; i++) {
        double visualGain = m_config->getValueString(
                ConfigKey("[Waveform]","VisualGain_" + QString::number(i))).toDouble(&ok);

        if (ok) {
            setVisualGain(FilterIndex(i), visualGain);
        } else {
            m_config->set(ConfigKey("[Waveform]","VisualGain_" + QString::number(i)),
                          QString::number(m_visualGain[i]));
        }
    }

    int overviewNormalized = m_config->getValueString(ConfigKey("[Waveform]","OverviewNormalized")).toInt(&ok);
    if (ok) {
        setOverviewNormalized(static_cast<bool>(overviewNormalized));
    } else {
        m_config->set(ConfigKey("[Waveform]","OverviewNormalized"), ConfigValue(m_overviewNormalized));
    }

    m_playMarkerPosition = m_config->getValue(ConfigKey("[Waveform]","PlayMarkerPosition"),
            WaveformWidgetRenderer::s_defaultPlayMarkerPosition);
    setPlayMarkerPosition(m_playMarkerPosition);

    return true;
}

void WaveformWidgetFactory::destroyWidgets() {
    for (int i = 0; i < m_waveformWidgetHolders.size(); i++) {
        WaveformWidgetAbstract* pWidget = m_waveformWidgetHolders[i].m_waveformWidget;;
        m_waveformWidgetHolders[i].m_waveformWidget = NULL;
        delete pWidget;
    }
    m_waveformWidgetHolders.clear();
}

void WaveformWidgetFactory::addTimerListener(QWidget* pWidget) {
    // Do not hold the pointer to of timer listeners since they may be deleted.
    // We don't activate update() or repaint() directly so listener widgets
    // can decide whether to paint or not.
    connect(this, SIGNAL(waveformUpdateTick()),
            pWidget, SLOT(maybeUpdate()),
            Qt::DirectConnection);
}

bool WaveformWidgetFactory::setWaveformWidget(WWaveformViewer* viewer,
                                              const QDomElement& node,
                                              const SkinContext& context) {
    int index = findIndexOf(viewer);
    if (index != -1) {
        qDebug() << "WaveformWidgetFactory::setWaveformWidget - "\
                    "viewer already have a waveform widget but it's not found by the factory !";
        delete viewer->getWaveformWidget();
    }

    // Cast to widget done just after creation because it can't be perform in
    // constructor (pure virtual)
    WaveformWidgetAbstract* waveformWidget = createWaveformWidget(m_type, viewer);
    viewer->setWaveformWidget(waveformWidget);
    viewer->setup(node, context);

    // create new holder
    if (index == -1) {
        m_waveformWidgetHolders.push_back(
            WaveformWidgetHolder(waveformWidget, viewer, node, context));
        index = m_waveformWidgetHolders.size() - 1;
    } else { //update holder
        m_waveformWidgetHolders[index] =
                WaveformWidgetHolder(waveformWidget, viewer, node, context);
    }

    viewer->setZoom(m_defaultZoom);
    viewer->setDisplayBeatGridAlpha(m_beatGridAlpha);
    viewer->setPlayMarkerPosition(m_playMarkerPosition);
    viewer->update();

    qDebug() << "WaveformWidgetFactory::setWaveformWidget - waveform widget added in factory, index" << index;

    return true;
}

void WaveformWidgetFactory::setFrameRate(int frameRate) {
    m_frameRate = math_clamp(frameRate, 1, 120);
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","FrameRate"), ConfigValue(m_frameRate));
    }
    m_vsyncThread->setSyncIntervalTimeMicros(1e6 / m_frameRate);
}

void WaveformWidgetFactory::setEndOfTrackWarningTime(int endTime) {
    m_endOfTrackWarningTime = endTime;
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","EndOfTrackWarningTime"), ConfigValue(m_endOfTrackWarningTime));
    }
}

void WaveformWidgetFactory::setVSyncType(int type) {
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","VSync"), ConfigValue((int)type));
    }

    m_vSyncType = type;
    m_vsyncThread->setVSyncType(type);
}

int WaveformWidgetFactory::getVSyncType() {
    return m_vSyncType;
}

bool WaveformWidgetFactory::setWidgetType(WaveformWidgetType::Type type) {
    if (type == m_type)
        return true;

    // check if type is acceptable
    for (int i = 0; i < m_waveformWidgetHandles.size(); i++) {
        WaveformWidgetAbstractHandle& handle = m_waveformWidgetHandles[i];
        if (handle.m_type == type) {
            // type is acceptable
            m_type = type;
            if (m_config) {
                m_config->set(ConfigKey("[Waveform]","WaveformType"), ConfigValue((int)m_type));
            }
            return true;
        }
    }

    // fallback
    m_type = WaveformWidgetType::EmptyWaveform;
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","WaveformType"), ConfigValue((int)m_type));
    }
    return false;
}

bool WaveformWidgetFactory::setWidgetTypeFromHandle(int handleIndex) {
    if (handleIndex < 0 || handleIndex >= (int)m_waveformWidgetHandles.size()) {
        qDebug() << "WaveformWidgetFactory::setWidgetType - invalid handle --> use of 'EmptyWaveform'";
        // fallback empty type
        setWidgetType(WaveformWidgetType::EmptyWaveform);
        return false;
    }

    WaveformWidgetAbstractHandle& handle = m_waveformWidgetHandles[handleIndex];
    if (handle.m_type == m_type) {
        qDebug() << "WaveformWidgetFactory::setWidgetType - type already in use";
        return true;
    }

    // change the type
    setWidgetType(handle.m_type);

    m_skipRender = true;
    //qDebug() << "recreate start";

    const float devicePixelRatio = getDevicePixelRatio();

    //re-create/setup all waveform widgets
    for (int i = 0; i < m_waveformWidgetHolders.size(); i++) {
        WaveformWidgetHolder& holder = m_waveformWidgetHolders[i];
        WaveformWidgetAbstract* previousWidget = holder.m_waveformWidget;
        TrackPointer pTrack = previousWidget->getTrackInfo();
        //previousWidget->hold();
        int previousZoom = previousWidget->getZoomFactor();
        double previousPlayMarkerPosition = previousWidget->getPlayMarkerPosition();
        delete previousWidget;
        WWaveformViewer* viewer = holder.m_waveformViewer;
        WaveformWidgetAbstract* widget = createWaveformWidget(m_type, holder.m_waveformViewer);
        holder.m_waveformWidget = widget;
        viewer->setWaveformWidget(widget);
        viewer->setup(holder.m_skinNodeCache, holder.m_skinContextCache);
        viewer->setZoom(previousZoom);
        viewer->setPlayMarkerPosition(previousPlayMarkerPosition);
        // resize() doesn't seem to get called on the widget. I think Qt skips
        // it since the size didn't change.
        //viewer->resize(viewer->size());
        widget->resize(viewer->width(), viewer->height(), devicePixelRatio);
        widget->setTrack(pTrack);
        widget->getWidget()->show();
        viewer->update();
    }

    m_skipRender = false;
    //qDebug() << "recreate done";
    return true;
}

void WaveformWidgetFactory::setDefaultZoom(int zoom) {
    m_defaultZoom = math_clamp(zoom, WaveformWidgetRenderer::s_waveformMinZoom,
                               WaveformWidgetRenderer::s_waveformMaxZoom);
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","DefaultZoom"), ConfigValue(m_defaultZoom));
    }

    for (int i = 0; i < m_waveformWidgetHolders.size(); i++) {
        m_waveformWidgetHolders[i].m_waveformViewer->setZoom(m_defaultZoom);
    }
}

void WaveformWidgetFactory::setZoomSync(bool sync) {
    m_zoomSync = sync;
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","ZoomSynchronization"), ConfigValue(m_zoomSync));
    }

    if (m_waveformWidgetHolders.size() == 0) {
        return;
    }

    int refZoom = m_waveformWidgetHolders[0].m_waveformWidget->getZoomFactor();
    for (int i = 1; i < m_waveformWidgetHolders.size(); i++) {
        m_waveformWidgetHolders[i].m_waveformViewer->setZoom(refZoom);
    }
}

void WaveformWidgetFactory::setDisplayBeatGridAlpha(int alpha) {
    m_beatGridAlpha = alpha;
    if (m_waveformWidgetHolders.size() == 0) {
        return;
    }

    for (int i = 0; i < m_waveformWidgetHolders.size(); i++) {
        m_waveformWidgetHolders[i].m_waveformWidget->setDisplayBeatGridAlpha(m_beatGridAlpha);
    }

}

void WaveformWidgetFactory::setVisualGain(FilterIndex index, double gain) {
    m_visualGain[index] = gain;
    if (m_config)
        m_config->set(ConfigKey("[Waveform]","VisualGain_" + QString::number(index)), QString::number(m_visualGain[index]));
}

double WaveformWidgetFactory::getVisualGain(FilterIndex index) const {
    return m_visualGain[index];
}

void WaveformWidgetFactory::setOverviewNormalized(bool normalize) {
    m_overviewNormalized = normalize;
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","OverviewNormalized"), ConfigValue(m_overviewNormalized));
    }
}

void WaveformWidgetFactory::setPlayMarkerPosition(double position) {
    //qDebug() << "setPlayMarkerPosition, position=" << position;
    m_playMarkerPosition = position;
    if (m_config) {
        m_config->setValue(ConfigKey("[Waveform]", "PlayMarkerPosition"), m_playMarkerPosition);
    }

    for (const auto& holder : m_waveformWidgetHolders) {
        holder.m_waveformWidget->setPlayMarkerPosition(m_playMarkerPosition);
    }
}

void WaveformWidgetFactory::notifyZoomChange(WWaveformViewer* viewer) {
    WaveformWidgetAbstract* pWaveformWidget = viewer->getWaveformWidget();
    if (pWaveformWidget != NULL && isZoomSync()) {
        //qDebug() << "WaveformWidgetFactory::notifyZoomChange";
        int refZoom = pWaveformWidget->getZoomFactor();

        for (int i = 0; i < m_waveformWidgetHolders.size(); ++i) {
            WaveformWidgetHolder& holder = m_waveformWidgetHolders[i];
            if (holder.m_waveformViewer != viewer) {
                holder.m_waveformViewer->setZoom(refZoom);
            }
        }
    }
}

void WaveformWidgetFactory::render() {
    ScopedTimer t("WaveformWidgetFactory::render() %1waveforms", m_waveformWidgetHolders.size());

    //int paintersSetupTime0 = 0;
    //int paintersSetupTime1 = 0;

    if (!m_skipRender) {
        if (m_type) {   // no regular updates for an empty waveform
            // next rendered frame is displayed after next buffer swap and than after VSync
            for (int i = 0; i < m_waveformWidgetHolders.size(); i++) {
                // Calculate play position for the new Frame in following run
                m_waveformWidgetHolders[i].m_waveformWidget->preRender(m_vsyncThread);
            }
            //qDebug() << "prerender" << m_vsyncThread->elapsed();

            // It may happen that there is an artificially delayed due to
            // anti tearing driver settings
            // all render commands are delayed until the swap from the previous run is executed
            for (int i = 0; i < m_waveformWidgetHolders.size(); i++) {
                WaveformWidgetAbstract* pWaveformWidget = m_waveformWidgetHolders[i].m_waveformWidget;
                if (pWaveformWidget->getWidth() > 0 &&
                        pWaveformWidget->getWidget()->isVisible()) {
                    (void)pWaveformWidget->render();
                }
                //qDebug() << "render" << i << m_vsyncThread->elapsed();
            }
        }

        // Notify all other waveform-like widgets (e.g. WSpinny's) that they should
        // update.
        //int t1 = m_vsyncThread->elapsed();
        emit(waveformUpdateTick());
        //qDebug() << "emit" << m_vsyncThread->elapsed() - t1;

        m_frameCnt += 1.0;
        mixxx::Duration timeCnt = m_time.elapsed();
        if (timeCnt > mixxx::Duration::fromSeconds(1)) {
            m_time.start();
            m_frameCnt = m_frameCnt * 1000 / timeCnt.toIntegerMillis(); // latency correction
            emit(waveformMeasured(m_frameCnt, m_vsyncThread->droppedFrames()));
            m_frameCnt = 0.0;
        }
    }
    //qDebug() << "refresh end" << m_vsyncThread->elapsed();
    m_vsyncThread->vsyncSlotFinished();
}

void WaveformWidgetFactory::swap() {
    ScopedTimer t("WaveformWidgetFactory::swap() %1waveforms", m_waveformWidgetHolders.size());

    // Do this in an extra slot to be sure to hit the desired interval
    if (!m_skipRender) {
        if (m_type) {   // no regular updates for an empty waveform
            // Show rendered buffer from last render() run
            //qDebug() << "swap() start" << m_vsyncThread->elapsed();
            for (int i = 0; i < m_waveformWidgetHolders.size(); i++) {
                WaveformWidgetAbstract* pWaveformWidget = m_waveformWidgetHolders[i].m_waveformWidget;
                if (pWaveformWidget->getWidth() > 0) {
                    QGLWidget* glw = dynamic_cast<QGLWidget*>(pWaveformWidget->getWidget());
                    // Don't swap invalid / invisible widgets or widgets with an
                    // unexposed window. Prevents continuous log spew of
                    // "QOpenGLContext::swapBuffers() called with non-exposed
                    // window, behavior is undefined" on Qt5. See Bug #1779487.
                    if (glw && glw->isValid() && glw->isVisible()) {
                        auto window = glw->windowHandle();
                        if (window && window->isExposed()) {
                            VSyncThread::swapGl(glw, i);
                        }
                    }
                }

                //qDebug() << "swap x" << m_vsyncThread->elapsed();
            }
        }
    }
    //qDebug() << "swap end" << m_vsyncThread->elapsed();
    m_vsyncThread->vsyncSlotFinished();
}

WaveformWidgetType::Type WaveformWidgetFactory::autoChooseWidgetType() const {
    //default selection
    if (m_openGLAvailable) {
        if (m_openGLShaderAvailable) {
            return WaveformWidgetType::GLSLRGBWaveform;
        } else {
            return WaveformWidgetType::GLRGBWaveform;
        }
    }
    return WaveformWidgetType::SoftwareWaveform;
}

void WaveformWidgetFactory::evaluateWidgets() {
    m_waveformWidgetHandles.clear();
    for (int type = 0; type < WaveformWidgetType::Count_WaveformwidgetType; type++) {
        QString widgetName;
        bool useOpenGl;
        bool useOpenGLShaders;
        bool developerOnly;

        switch(type) {
        case WaveformWidgetType::EmptyWaveform:
            widgetName = EmptyWaveformWidget::getWaveformWidgetName();
            useOpenGl = EmptyWaveformWidget::useOpenGl();
            useOpenGLShaders = EmptyWaveformWidget::useOpenGLShaders();
            developerOnly = EmptyWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::SoftwareSimpleWaveform:
            continue; // //TODO(vrince):
        case WaveformWidgetType::SoftwareWaveform:
            widgetName = SoftwareWaveformWidget::getWaveformWidgetName();
            useOpenGl = SoftwareWaveformWidget::useOpenGl();
            useOpenGLShaders = SoftwareWaveformWidget::useOpenGLShaders();
            developerOnly = SoftwareWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::HSVWaveform:
            widgetName = HSVWaveformWidget::getWaveformWidgetName();
            useOpenGl = HSVWaveformWidget::useOpenGl();
            useOpenGLShaders = HSVWaveformWidget::useOpenGLShaders();
            developerOnly = HSVWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::RGBWaveform:
            widgetName = RGBWaveformWidget::getWaveformWidgetName();
            useOpenGl = RGBWaveformWidget::useOpenGl();
            useOpenGLShaders = RGBWaveformWidget::useOpenGLShaders();
            developerOnly = RGBWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::QtSimpleWaveform:
            widgetName = QtSimpleWaveformWidget::getWaveformWidgetName();
            useOpenGl = QtSimpleWaveformWidget::useOpenGl();
            useOpenGLShaders = QtSimpleWaveformWidget::useOpenGLShaders();
            developerOnly = QtSimpleWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::QtWaveform:
            widgetName = QtWaveformWidget::getWaveformWidgetName();
            useOpenGl = QtWaveformWidget::useOpenGl();
            useOpenGLShaders = QtWaveformWidget::useOpenGLShaders();
            developerOnly = QtWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLSimpleWaveform:
            widgetName = GLSimpleWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLSimpleWaveformWidget::useOpenGl();
            useOpenGLShaders = GLSimpleWaveformWidget::useOpenGLShaders();
            developerOnly = GLSimpleWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLFilteredWaveform:
            widgetName = GLWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLWaveformWidget::useOpenGl();
            useOpenGLShaders = GLWaveformWidget::useOpenGLShaders();
            developerOnly = GLWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLSLFilteredWaveform:
            widgetName = GLSLFilteredWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLSLFilteredWaveformWidget::useOpenGl();
            useOpenGLShaders = GLSLFilteredWaveformWidget::useOpenGLShaders();
            developerOnly = GLSLFilteredWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLSLRGBWaveform:
            widgetName = GLSLRGBWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLSLRGBWaveformWidget::useOpenGl();
            useOpenGLShaders = GLSLRGBWaveformWidget::useOpenGLShaders();
            developerOnly = GLSLRGBWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLVSyncTest:
            widgetName = GLVSyncTestWidget::getWaveformWidgetName();
            useOpenGl = GLVSyncTestWidget::useOpenGl();
            useOpenGLShaders = GLVSyncTestWidget::useOpenGLShaders();
            developerOnly = GLVSyncTestWidget::developerOnly();
            break;
        case WaveformWidgetType::GLRGBWaveform:
            widgetName = GLRGBWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLRGBWaveformWidget::useOpenGl();
            useOpenGLShaders = GLRGBWaveformWidget::useOpenGLShaders();
            developerOnly = GLRGBWaveformWidget::developerOnly();
            break;
        default:
            continue;
        }

        if (useOpenGLShaders) {
            widgetName += " " + tr("(GLSL)");
        } else if (useOpenGl) {
            widgetName += " " + tr("(GL)");
        }

        // add new handle for each available widget type
        WaveformWidgetAbstractHandle handle;
        handle.m_displayString = widgetName;
        handle.m_type = (WaveformWidgetType::Type)type;

        // NOTE: For the moment non active widget are not added to available handle
        // but it could be useful to have them anyway but not selectable in the combo box
        if ((useOpenGl && !isOpenGLAvailable()) ||
                (useOpenGLShaders && !isOpenGlShaderAvailable())) {
            handle.m_active = false;
            continue;
        }

        if (developerOnly && !CmdlineArgs::Instance().getDeveloper()) {
            handle.m_active = false;
            continue;
        }

        m_waveformWidgetHandles.push_back(handle);
    }
}

WaveformWidgetAbstract* WaveformWidgetFactory::createWaveformWidget(
        WaveformWidgetType::Type type, WWaveformViewer* viewer) {
    WaveformWidgetAbstract* widget = NULL;
    if (viewer) {
        if (CmdlineArgs::Instance().getSafeMode()) {
            type = WaveformWidgetType::EmptyWaveform;
        }

        switch(type) {
        case WaveformWidgetType::SoftwareWaveform:
            widget = new SoftwareWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::HSVWaveform:
            widget = new HSVWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::RGBWaveform:
            widget = new RGBWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::QtSimpleWaveform:
            widget = new QtSimpleWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::QtWaveform:
            widget = new QtWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::GLSimpleWaveform:
            widget = new GLSimpleWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::GLFilteredWaveform:
            widget = new GLWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::GLRGBWaveform:
            widget = new GLRGBWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::GLSLFilteredWaveform:
            widget = new GLSLFilteredWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::GLSLRGBWaveform:
            widget = new GLSLRGBWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::GLVSyncTest:
            widget = new GLVSyncTestWidget(viewer->getGroup(), viewer);
            break;
        default:
        //case WaveformWidgetType::SoftwareSimpleWaveform: TODO: (vrince)
        //case WaveformWidgetType::EmptyWaveform:
            widget = new EmptyWaveformWidget(viewer->getGroup(), viewer);
            break;
        }
        widget->castToQWidget();
        if (!widget->isValid()) {
            qWarning() << "failed to init WafeformWidget" << type << "fall back to \"Empty\"";
            delete widget;
            widget = new EmptyWaveformWidget(viewer->getGroup(), viewer);
            widget->castToQWidget();
            if (!widget->isValid()) {
                qWarning() << "failed to init EmptyWaveformWidget";
                delete widget;
                widget = NULL;
            }
        }
    }
    return widget;
}

int WaveformWidgetFactory::findIndexOf(WWaveformViewer* viewer) const {
    for (int i = 0; i < (int)m_waveformWidgetHolders.size(); i++) {
        if (m_waveformWidgetHolders[i].m_waveformViewer == viewer) {
            return i;
        }
    }
    return -1;
}

void WaveformWidgetFactory::startVSync(GuiTick* pGuiTick) {
    m_vsyncThread = new VSyncThread(this, pGuiTick);
    m_vsyncThread->start(QThread::NormalPriority);

    connect(m_vsyncThread, SIGNAL(vsyncRender()),
            this, SLOT(render()));
    connect(m_vsyncThread, SIGNAL(vsyncSwap()),
            this, SLOT(swap()));
}

void WaveformWidgetFactory::getAvailableVSyncTypes(QList<QPair<int, QString > >* pList) {
    m_vsyncThread->getAvailableVSyncTypes(pList);
}

// static
float WaveformWidgetFactory::getDevicePixelRatio() {
    float devicePixelRatio = 1.0;
    QWindow* pWindow = QGuiApplication::focusWindow();
    if (pWindow != nullptr) {
        devicePixelRatio = pWindow->devicePixelRatio();
    }
    return devicePixelRatio;
}
