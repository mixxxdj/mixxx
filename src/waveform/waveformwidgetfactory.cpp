#include "waveform/waveformwidgetfactory.h"

#include <QGLFormat>
#include <QGLShaderProgram>
#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QStringList>
#include <QTime>
#include <QWidget>
#include <QWindow>
#include <QtDebug>

#include "control/controlpotmeter.h"
#include "moc_waveformwidgetfactory.cpp"
#include "util/cmdlineargs.h"
#include "util/math.h"
#include "util/performancetimer.h"
#include "util/timer.h"
#include "waveform/guitick.h"
#include "waveform/sharedglcontext.h"
#include "waveform/visualsmanager.h"
#include "waveform/vsyncthread.h"
#include "waveform/widgets/emptywaveformwidget.h"
#include "waveform/widgets/glrgbwaveformwidget.h"
#include "waveform/widgets/glsimplewaveformwidget.h"
#include "waveform/widgets/glslwaveformwidget.h"
#include "waveform/widgets/glvsynctestwidget.h"
#include "waveform/widgets/glwaveformwidget.h"
#include "waveform/widgets/hsvwaveformwidget.h"
#include "waveform/widgets/qthsvwaveformwidget.h"
#include "waveform/widgets/qtrgbwaveformwidget.h"
#include "waveform/widgets/qtsimplewaveformwidget.h"
#include "waveform/widgets/qtvsynctestwidget.h"
#include "waveform/widgets/qtwaveformwidget.h"
#include "waveform/widgets/rgbwaveformwidget.h"
#include "waveform/widgets/softwarewaveformwidget.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wvumeter.h"
#include "widget/wwaveformviewer.h"

namespace {
// Returns true if the given waveform should be rendered.
bool shouldRenderWaveform(WaveformWidgetAbstract* pWaveformWidget) {
    if (pWaveformWidget == nullptr ||
        pWaveformWidget->getWidth() == 0 ||
        pWaveformWidget->getHeight() == 0) {
        return false;
    }

    auto* glw = qobject_cast<QGLWidget*>(pWaveformWidget->getWidget());
    if (glw == nullptr) {
        // Not a QGLWidget. We can simply use QWidget::isVisible.
        auto* qwidget = qobject_cast<QWidget*>(pWaveformWidget->getWidget());
        return qwidget != nullptr && qwidget->isVisible();
    }

    if (glw == nullptr || !glw->isValid() || !glw->isVisible()) {
        return false;
    }

    // Strangely, a widget can have non-zero width/height, be valid and visible,
    // yet still not show up on the screen. QWindow::isExposed tells us this.
    const QWindow* window = glw->windowHandle();
    if (window == nullptr || !window->isExposed()) {
        return false;
    }

    return true;
}
}  // anonymous namespace

///////////////////////////////////////////

WaveformWidgetAbstractHandle::WaveformWidgetAbstractHandle()
    : m_type(WaveformWidgetType::Count_WaveformwidgetType) {
}

///////////////////////////////////////////

WaveformWidgetHolder::WaveformWidgetHolder()
        : m_waveformWidget(nullptr),
          m_waveformViewer(nullptr),
          m_skinContextCache(UserSettingsPointer(), QString()) {
}

WaveformWidgetHolder::WaveformWidgetHolder(WaveformWidgetAbstract* waveformWidget,
                                           WWaveformViewer* waveformViewer,
                                           const QDomNode& node,
                                           const SkinContext& parentContext)
    : m_waveformWidget(waveformWidget),
      m_waveformViewer(waveformViewer),
      m_skinNodeCache(node.cloneNode()),
      m_skinContextCache(&parentContext) {
}

///////////////////////////////////////////

WaveformWidgetFactory::WaveformWidgetFactory()
        // Set an empty waveform initially. We will set the correct one when skin load finishes.
        // Concretely, we want to set a non-GL waveform when loading the skin so that the window
        // loads correctly.
        : m_type(WaveformWidgetType::EmptyWaveform),
          m_configType(WaveformWidgetType::EmptyWaveform),
          m_config(nullptr),
          m_skipRender(false),
          m_frameRate(30),
          m_endOfTrackWarningTime(30),
          m_defaultZoom(WaveformWidgetRenderer::s_waveformDefaultZoom),
          m_zoomSync(true),
          m_overviewNormalized(false),
          m_openGlAvailable(false),
          m_openGlesAvailable(false),
          m_openGLShaderAvailable(false),
          m_beatGridAlpha(90),
          m_vsyncThread(nullptr),
          m_pGuiTick(nullptr),
          m_pVisualsManager(nullptr),
          m_frameCnt(0),
          m_actualFrameRate(0),
          m_vSyncType(0),
          m_playMarkerPosition(WaveformWidgetRenderer::s_defaultPlayMarkerPosition) {
    m_visualGain[All] = 1.0;
    m_visualGain[Low] = 1.0;
    m_visualGain[Mid] = 1.0;
    m_visualGain[High] = 1.0;

    QGLWidget* pGlWidget = SharedGLContext::getWidget();
    if (pGlWidget && pGlWidget->isValid()) {
        // will be false if SafeMode is enabled

        pGlWidget->show();
        // Without a makeCurrent, hasOpenGLShaderPrograms returns false on Qt 5.
        // and QGLFormat::openGLVersionFlags() returns the maximum known version
        pGlWidget->makeCurrent();

        QGLFormat::OpenGLVersionFlags version = QGLFormat::openGLVersionFlags();

        auto rendererString = QString();
        if (QOpenGLContext::currentContext()) {
            auto glFunctions = QOpenGLFunctions();

            glFunctions.initializeOpenGLFunctions();
            QString versionString(QLatin1String(
                    reinterpret_cast<const char*>(glFunctions.glGetString(GL_VERSION))));
            QString vendorString(QLatin1String(
                    reinterpret_cast<const char*>(glFunctions.glGetString(GL_VENDOR))));
            rendererString = QString(QLatin1String(
                    reinterpret_cast<const char*>(glFunctions.glGetString(GL_RENDERER))));

            // Either GL or GL ES Version is set, not both.
            qDebug() << QString("openGLVersionFlags 0x%1").arg(version, 0, 16) << versionString << vendorString << rendererString;
        } else {
            qDebug() << "QOpenGLContext::currentContext() returns nullptr";
            qDebug() << "pGlWidget->->windowHandle() =" << pGlWidget->windowHandle();
        }

        int majorGlVersion = 0;
        int minorGlVersion = 0;
        int majorGlesVersion = 0;
        int minorGlesVersion = 0;
        if (version == QGLFormat::OpenGL_Version_None) {
            m_openGLVersion = "None";
        } else if (version & QGLFormat::OpenGL_Version_4_3) {
            majorGlVersion = 4;
            minorGlVersion = 3;
        } else if (version & QGLFormat::OpenGL_Version_4_2) {
            majorGlVersion = 4;
            minorGlVersion = 2;
        } else if (version & QGLFormat::OpenGL_Version_4_1) {
            majorGlVersion = 4;
            minorGlVersion = 1;
        } else if (version & QGLFormat::OpenGL_Version_4_0) {
            majorGlVersion = 4;
            minorGlVersion = 0;
        } else if (version & QGLFormat::OpenGL_Version_3_3) {
            majorGlVersion = 3;
            minorGlVersion = 3;
        } else if (version & QGLFormat::OpenGL_Version_3_2) {
            majorGlVersion = 3;
            minorGlVersion = 2;
        } else if (version & QGLFormat::OpenGL_Version_3_1) {
            majorGlVersion = 3;
            minorGlVersion = 1;
        } else if (version & QGLFormat::OpenGL_Version_3_0) {
            majorGlVersion = 3;
        } else if (version & QGLFormat::OpenGL_Version_2_1) {
            majorGlVersion = 2;
            minorGlVersion = 1;
        } else if (version & QGLFormat::OpenGL_Version_2_0) {
            majorGlVersion = 2;
            minorGlVersion = 0;
        } else if (version & QGLFormat::OpenGL_Version_1_5) {
            majorGlVersion = 1;
            minorGlVersion = 5;
        } else if (version & QGLFormat::OpenGL_Version_1_4) {
            majorGlVersion = 1;
            minorGlVersion = 4;
        } else if (version & QGLFormat::OpenGL_Version_1_3) {
            majorGlVersion = 1;
            minorGlVersion = 3;
        } else if (version & QGLFormat::OpenGL_Version_1_2) {
            majorGlVersion = 1;
            minorGlVersion = 2;
        } else if (version & QGLFormat::OpenGL_Version_1_1) {
            majorGlVersion = 1;
            minorGlVersion = 1;
        } else if (version & QGLFormat::OpenGL_ES_Version_2_0) {
            m_openGLVersion = "ES 2.0";
            majorGlesVersion = 2;
            minorGlesVersion = 0;
        } else if (version & QGLFormat::OpenGL_ES_CommonLite_Version_1_1) {
            if (version & QGLFormat::OpenGL_ES_Common_Version_1_1) {
                m_openGLVersion = "ES 1.1";
            } else {
                m_openGLVersion = "ES Common Lite 1.1";
            }
            majorGlesVersion = 1;
            minorGlesVersion = 1;
        } else if (version & QGLFormat::OpenGL_ES_Common_Version_1_1) {
            m_openGLVersion = "ES Common Lite 1.1";
            majorGlesVersion = 1;
            minorGlesVersion = 1;
        } else if (version & QGLFormat::OpenGL_ES_CommonLite_Version_1_0) {
            if (version & QGLFormat::OpenGL_ES_Common_Version_1_0) {
                m_openGLVersion = "ES 1.0";
            } else {
                m_openGLVersion = "ES Common Lite 1.0";
            }
            majorGlesVersion = 1;
            minorGlesVersion = 0;
        } else if (version & QGLFormat::OpenGL_ES_Common_Version_1_0) {
            m_openGLVersion = "ES Common Lite 1.0";
            majorGlesVersion = 1;
            minorGlesVersion = 0;
        } else {
            m_openGLVersion = QString("Unknown 0x%1")
                .arg(version, 0, 16);
        }

        if (majorGlVersion != 0) {
            m_openGLVersion = QString::number(majorGlVersion) + "."
                    + QString::number(minorGlVersion);

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
            if (majorGlVersion * 100 + minorGlVersion >= 201) {
                // Qt5 requires at least OpenGL 2.1 or OpenGL ES 2.0
                m_openGlAvailable = true;
            }
#endif
        } else {
            if (majorGlesVersion * 100 + minorGlesVersion >= 200) {
                // Qt5 requires at least OpenGL 2.1 or OpenGL ES 2.0
                m_openGlesAvailable = true;
            }
        }

        m_openGLShaderAvailable =
                QGLShaderProgram::hasOpenGLShaderPrograms(
                        pGlWidget->context());

        if (!rendererString.isEmpty()) {
            m_openGLVersion += " (" + rendererString + ")";
        }

        pGlWidget->hide();
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

    int frameRate = m_config->getValue(ConfigKey("[Waveform]","FrameRate"), m_frameRate);
    m_frameRate = math_clamp(frameRate, 1, 120);


    int endTime = m_config->getValueString(ConfigKey("[Waveform]","EndOfTrackWarningTime")).toInt(&ok);
    if (ok) {
        setEndOfTrackWarningTime(endTime);
    } else {
        m_config->set(ConfigKey("[Waveform]","EndOfTrackWarningTime"),
                ConfigValue(m_endOfTrackWarningTime));
    }

    m_vSyncType = m_config->getValue(ConfigKey("[Waveform]","VSync"), 0);

    double defaultZoom = m_config->getValueString(ConfigKey("[Waveform]","DefaultZoom")).toDouble(&ok);
    if (ok) {
        setDefaultZoom(defaultZoom);
    } else{
        m_config->set(ConfigKey("[Waveform]","DefaultZoom"), ConfigValue(m_defaultZoom));
    }

    bool zoomSync = m_config->getValue(ConfigKey("[Waveform]", "ZoomSynchronization"), m_zoomSync);
    setZoomSync(zoomSync);

    int beatGridAlpha = m_config->getValue(ConfigKey("[Waveform]", "beatGridAlpha"), m_beatGridAlpha);
    setDisplayBeatGridAlpha(beatGridAlpha);

    WaveformWidgetType::Type type = static_cast<WaveformWidgetType::Type>(
            m_config->getValueString(ConfigKey("[Waveform]","WaveformType")).toInt(&ok));
    // Store the widget type on m_configType for later initialization.
    // We will initialize the objects later because of a problem with GL on QT 5.14.2 on Windows
    if (!ok || !setWidgetType(type, &m_configType)) {
        setWidgetType(autoChooseWidgetType(), &m_configType);
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
    for (auto& holder : m_waveformWidgetHolders) {
        WaveformWidgetAbstract* pWidget = holder.m_waveformWidget;
        holder.m_waveformWidget = nullptr;
        delete pWidget;
    }
    m_waveformWidgetHolders.clear();
}

void WaveformWidgetFactory::addTimerListener(WVuMeter* pWidget) {
    // Do not hold the pointer to of timer listeners since they may be deleted.
    // We don't activate update() or repaint() directly so listener widgets
    // can decide whether to paint or not.
    connect(this,
            &WaveformWidgetFactory::waveformUpdateTick,
            pWidget,
            &WVuMeter::maybeUpdate,
            Qt::DirectConnection);
}

void WaveformWidgetFactory::slotSkinLoaded() {
    setWidgetTypeFromConfig();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0) && defined __WINDOWS__
    // This regenerates the waveforms twice because of a bug found on Windows
    // where the first one fails.
    // The problem is that the window of the widget thinks that it is not exposed.
    // (https://doc.qt.io/qt-5/qwindow.html#exposeEvent )
    setWidgetTypeFromConfig();
#endif
}

bool WaveformWidgetFactory::setWaveformWidget(WWaveformViewer* viewer,
                                              const QDomElement& node,
                                              const SkinContext& parentContext) {
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
    viewer->setup(node, parentContext);

    // create new holder
    WaveformWidgetHolder holder(waveformWidget, viewer, node, &parentContext);
    if (index == -1) {
        // add holder
        m_waveformWidgetHolders.push_back(std::move(holder));
        index = static_cast<int>(m_waveformWidgetHolders.size()) - 1;
    } else {
        // update holder
        DEBUG_ASSERT(index >= 0);
        m_waveformWidgetHolders[index] = std::move(holder);
    }

    viewer->setZoom(m_defaultZoom);
    viewer->setDisplayBeatGridAlpha(m_beatGridAlpha);
    viewer->setPlayMarkerPosition(m_playMarkerPosition);
    waveformWidget->resize(viewer->width(), viewer->height());
    waveformWidget->getWidget()->show();
    viewer->update();

    qDebug() << "WaveformWidgetFactory::setWaveformWidget - waveform widget added in factory, index" << index;

    return true;
}

void WaveformWidgetFactory::setFrameRate(int frameRate) {
    m_frameRate = math_clamp(frameRate, 1, 120);
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","FrameRate"), ConfigValue(m_frameRate));
    }
    if (m_vsyncThread) {
        m_vsyncThread->setSyncIntervalTimeMicros(static_cast<int>(1e6 / m_frameRate));
    }
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
    if (m_vsyncThread) {
        m_vsyncThread->setVSyncType(type);
    }
}

int WaveformWidgetFactory::getVSyncType() {
    return m_vSyncType;
}

bool WaveformWidgetFactory::setWidgetType(WaveformWidgetType::Type type) {
    return setWidgetType(type, &m_type);
}

bool WaveformWidgetFactory::setWidgetType(
        WaveformWidgetType::Type type,
        WaveformWidgetType::Type* pCurrentType) {
    if (type == *pCurrentType) {
        return true;
    }

    // check if type is acceptable
    int index = findHandleIndexFromType(type);
    if (index > -1) {
        // type is acceptable
        *pCurrentType = type;
        if (m_config) {
            m_config->setValue(
                    ConfigKey("[Waveform]", "WaveformType"),
                    static_cast<int>(*pCurrentType));
        }
        return true;
    }

    // fallback
    *pCurrentType = WaveformWidgetType::EmptyWaveform;
    if (m_config) {
        m_config->setValue(
                ConfigKey("[Waveform]", "WaveformType"),
                static_cast<int>(*pCurrentType));
    }
    return false;
}

bool WaveformWidgetFactory::setWidgetTypeFromConfig() {
    int empty = findHandleIndexFromType(WaveformWidgetType::EmptyWaveform);
    int desired = findHandleIndexFromType(m_configType);
    if (desired == -1) {
        desired = empty;
    }
    return setWidgetTypeFromHandle(desired, true);
}

bool WaveformWidgetFactory::setWidgetTypeFromHandle(int handleIndex, bool force) {
    if (handleIndex < 0 || handleIndex >= m_waveformWidgetHandles.size()) {
        qDebug() << "WaveformWidgetFactory::setWidgetType - invalid handle --> use of 'EmptyWaveform'";
        // fallback empty type
        setWidgetType(WaveformWidgetType::EmptyWaveform);
        return false;
    }

    WaveformWidgetAbstractHandle& handle = m_waveformWidgetHandles[handleIndex];
    if (handle.m_type == m_type && !force) {
        qDebug() << "WaveformWidgetFactory::setWidgetType - type already in use";
        return true;
    }

    // change the type
    setWidgetType(handle.m_type);

    m_skipRender = true;
    //qDebug() << "recreate start";

    //re-create/setup all waveform widgets
    for (auto& holder : m_waveformWidgetHolders) {
        WaveformWidgetAbstract* previousWidget = holder.m_waveformWidget;
        TrackPointer pTrack = previousWidget->getTrackInfo();
        //previousWidget->hold();
        double previousZoom = previousWidget->getZoomFactor();
        double previousPlayMarkerPosition = previousWidget->getPlayMarkerPosition();
        int previousbeatgridAlpha = previousWidget->getBeatGridAlpha();
        delete previousWidget;
        WWaveformViewer* viewer = holder.m_waveformViewer;
        WaveformWidgetAbstract* widget = createWaveformWidget(m_type, holder.m_waveformViewer);
        holder.m_waveformWidget = widget;
        viewer->setWaveformWidget(widget);
        viewer->setup(holder.m_skinNodeCache, holder.m_skinContextCache);
        viewer->setZoom(previousZoom);
        viewer->setPlayMarkerPosition(previousPlayMarkerPosition);
        viewer->setDisplayBeatGridAlpha(previousbeatgridAlpha);
        // resize() doesn't seem to get called on the widget. I think Qt skips
        // it since the size didn't change.
        //viewer->resize(viewer->size());
        widget->resize(viewer->width(), viewer->height());
        widget->setTrack(pTrack);
        widget->getWidget()->show();
        viewer->update();
    }

    m_skipRender = false;
    //qDebug() << "recreate done";
    return true;
}

void WaveformWidgetFactory::setDefaultZoom(double zoom) {
    m_defaultZoom = math_clamp(zoom, WaveformWidgetRenderer::s_waveformMinZoom,
                               WaveformWidgetRenderer::s_waveformMaxZoom);
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","DefaultZoom"), ConfigValue(m_defaultZoom));
    }

    for (const auto& holder : m_waveformWidgetHolders) {
        holder.m_waveformViewer->setZoom(m_defaultZoom);
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

    double refZoom = m_waveformWidgetHolders[0].m_waveformWidget->getZoomFactor();
    for (const auto& holder : m_waveformWidgetHolders) {
        holder.m_waveformViewer->setZoom(refZoom);
    }
}

void WaveformWidgetFactory::setDisplayBeatGridAlpha(int alpha) {
    m_beatGridAlpha = alpha;
    if (m_waveformWidgetHolders.size() == 0) {
        return;
    }

    for (const auto& holder : m_waveformWidgetHolders) {
        holder.m_waveformWidget->setDisplayBeatGridAlpha(m_beatGridAlpha);
    }
}

void WaveformWidgetFactory::setVisualGain(FilterIndex index, double gain) {
    m_visualGain[index] = gain;
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","VisualGain_" + QString::number(index)), QString::number(m_visualGain[index]));
    }
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
    if (pWaveformWidget != nullptr && isZoomSync()) {
        //qDebug() << "WaveformWidgetFactory::notifyZoomChange";
        double refZoom = pWaveformWidget->getZoomFactor();

        for (std::size_t i = 0; i < m_waveformWidgetHolders.size(); ++i) {
            WaveformWidgetHolder& holder = m_waveformWidgetHolders[i];
            if (holder.m_waveformViewer != viewer) {
                holder.m_waveformViewer->setZoom(refZoom);
            }
        }
    }
}

void WaveformWidgetFactory::render() {
    ScopedTimer t("WaveformWidgetFactory::render() %1waveforms",
            static_cast<int>(m_waveformWidgetHolders.size()));

    //int paintersSetupTime0 = 0;
    //int paintersSetupTime1 = 0;

    //qDebug() << "render()" << m_vsyncThread->elapsed();

    if (!m_skipRender) {
        if (m_type) {   // no regular updates for an empty waveform
            // next rendered frame is displayed after next buffer swap and than after VSync
            QVarLengthArray<bool, 10> shouldRenderWaveforms(
                    static_cast<int>(m_waveformWidgetHolders.size()));
            for (decltype(m_waveformWidgetHolders)::size_type i = 0;
                    i < m_waveformWidgetHolders.size();
                    i++) {
                WaveformWidgetAbstract* pWaveformWidget = m_waveformWidgetHolders[i].m_waveformWidget;
                // Don't bother doing the pre-render work if we aren't going to
                // render this widget.
                bool shouldRender = shouldRenderWaveform(pWaveformWidget);
                shouldRenderWaveforms[i] = shouldRender;
                if (!shouldRender) {
                    continue;
                }
                // Calculate play position for the new Frame in following run
                pWaveformWidget->preRender(m_vsyncThread);
            }
            //qDebug() << "prerender" << m_vsyncThread->elapsed();

            // It may happen that there is an artificially delayed due to
            // anti tearing driver settings
            // all render commands are delayed until the swap from the previous run is executed
            for (decltype(m_waveformWidgetHolders)::size_type i = 0;
                    i < m_waveformWidgetHolders.size();
                    i++) {
                WaveformWidgetAbstract* pWaveformWidget = m_waveformWidgetHolders[i].m_waveformWidget;
                if (!shouldRenderWaveforms[i]) {
                    continue;
                }
                pWaveformWidget->render();
                //qDebug() << "render" << i << m_vsyncThread->elapsed();
            }
        }

        // WSpinnys are also double-buffered QGLWidgets, like all the waveform
        // renderers. Render all the WSpinny widgets now.
        emit renderSpinnies(m_vsyncThread);

        // Notify all other waveform-like widgets (e.g. WSpinny's) that they should
        // update.
        //int t1 = m_vsyncThread->elapsed();
        emit waveformUpdateTick();
        //qDebug() << "emit" << m_vsyncThread->elapsed() - t1;

        m_frameCnt += 1.0f;
        mixxx::Duration timeCnt = m_time.elapsed();
        if (timeCnt > mixxx::Duration::fromSeconds(1)) {
            m_time.start();
            m_frameCnt = m_frameCnt * 1000 / timeCnt.toIntegerMillis(); // latency correction
            emit waveformMeasured(m_frameCnt, m_vsyncThread->droppedFrames());
            m_frameCnt = 0.0;
        }
    }

    m_pVisualsManager->process(m_endOfTrackWarningTime);
    m_pGuiTick->process();

    //qDebug() << "refresh end" << m_vsyncThread->elapsed();
    m_vsyncThread->vsyncSlotFinished();
}

void WaveformWidgetFactory::swap() {
    ScopedTimer t("WaveformWidgetFactory::swap() %1waveforms",
            static_cast<int>(m_waveformWidgetHolders.size()));

    // Do this in an extra slot to be sure to hit the desired interval
    if (!m_skipRender) {
        if (m_type) {   // no regular updates for an empty waveform
            // Show rendered buffer from last render() run
            //qDebug() << "swap() start" << m_vsyncThread->elapsed();
            for (const auto& holder : m_waveformWidgetHolders) {
                WaveformWidgetAbstract* pWaveformWidget = holder.m_waveformWidget;

                // Don't swap invalid / invisible widgets or widgets with an
                // unexposed window. Prevents continuous log spew of
                // "QOpenGLContext::swapBuffers() called with non-exposed
                // window, behavior is undefined" on Qt5. See Bug #1779487.
                if (!shouldRenderWaveform(pWaveformWidget)) {
                    continue;
                }
                QGLWidget* glw = qobject_cast<QGLWidget*>(pWaveformWidget->getWidget());
                if (glw != nullptr) {
                    if (glw->context() != QGLContext::currentContext()) {
                        glw->makeCurrent();
                    }
                    glw->swapBuffers();
                }
                //qDebug() << "swap x" << m_vsyncThread->elapsed();
            }
        }
        // WSpinnys are also double-buffered QGLWidgets, like all the waveform
        // renderers. Swap all the WSpinny widgets now.
        emit swapSpinnies();
    }
    //qDebug() << "swap end" << m_vsyncThread->elapsed();
    m_vsyncThread->vsyncSlotFinished();
}

WaveformWidgetType::Type WaveformWidgetFactory::autoChooseWidgetType() const {
    if (m_openGlAvailable) {
        if (m_openGLShaderAvailable) {
            return WaveformWidgetType::GLSLRGBWaveform;
        } else {
            return WaveformWidgetType::GLRGBWaveform;
        }
    }
    return WaveformWidgetType::RGBWaveform;
}

void WaveformWidgetFactory::evaluateWidgets() {
    m_waveformWidgetHandles.clear();
    for (int type = 0; type < WaveformWidgetType::Count_WaveformwidgetType; type++) {
        QString widgetName;
        bool useOpenGl;
        bool useOpenGles;
        bool useOpenGLShaders;
        bool developerOnly;

        switch(type) {
        case WaveformWidgetType::EmptyWaveform:
            widgetName = EmptyWaveformWidget::getWaveformWidgetName();
            useOpenGl = EmptyWaveformWidget::useOpenGl();
            useOpenGles = EmptyWaveformWidget::useOpenGles();
            useOpenGLShaders = EmptyWaveformWidget::useOpenGLShaders();
            developerOnly = EmptyWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::SoftwareSimpleWaveform:
            continue; // //TODO(vrince):
        case WaveformWidgetType::SoftwareWaveform:
            widgetName = SoftwareWaveformWidget::getWaveformWidgetName();
            useOpenGl = SoftwareWaveformWidget::useOpenGl();
            useOpenGles = SoftwareWaveformWidget::useOpenGles();
            useOpenGLShaders = SoftwareWaveformWidget::useOpenGLShaders();
            developerOnly = SoftwareWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::HSVWaveform:
            widgetName = HSVWaveformWidget::getWaveformWidgetName();
            useOpenGl = HSVWaveformWidget::useOpenGl();
            useOpenGles = HSVWaveformWidget::useOpenGles();
            useOpenGLShaders = HSVWaveformWidget::useOpenGLShaders();
            developerOnly = HSVWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::RGBWaveform:
            widgetName = RGBWaveformWidget::getWaveformWidgetName();
            useOpenGl = RGBWaveformWidget::useOpenGl();
            useOpenGles = RGBWaveformWidget::useOpenGles();
            useOpenGLShaders = RGBWaveformWidget::useOpenGLShaders();
            developerOnly = RGBWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::QtSimpleWaveform:
            widgetName = QtSimpleWaveformWidget::getWaveformWidgetName();
            useOpenGl = QtSimpleWaveformWidget::useOpenGl();
            useOpenGles = QtSimpleWaveformWidget::useOpenGles();
            useOpenGLShaders = QtSimpleWaveformWidget::useOpenGLShaders();
            developerOnly = QtSimpleWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::QtWaveform:
            widgetName = QtWaveformWidget::getWaveformWidgetName();
            useOpenGl = QtWaveformWidget::useOpenGl();
            useOpenGles = QtWaveformWidget::useOpenGles();
            useOpenGLShaders = QtWaveformWidget::useOpenGLShaders();
            developerOnly = QtWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLSimpleWaveform:
            widgetName = GLSimpleWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLSimpleWaveformWidget::useOpenGl();
            useOpenGles = GLSimpleWaveformWidget::useOpenGles();
            useOpenGLShaders = GLSimpleWaveformWidget::useOpenGLShaders();
            developerOnly = GLSimpleWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLFilteredWaveform:
            widgetName = GLWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLWaveformWidget::useOpenGl();
            useOpenGles = GLWaveformWidget::useOpenGles();
            useOpenGLShaders = GLWaveformWidget::useOpenGLShaders();
            developerOnly = GLWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLSLFilteredWaveform:
            widgetName = GLSLFilteredWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLSLFilteredWaveformWidget::useOpenGl();
            useOpenGles = GLSLFilteredWaveformWidget::useOpenGles();
            useOpenGLShaders = GLSLFilteredWaveformWidget::useOpenGLShaders();
            developerOnly = GLSLFilteredWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLSLRGBWaveform:
            widgetName = GLSLRGBWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLSLRGBWaveformWidget::useOpenGl();
            useOpenGles = GLSLRGBWaveformWidget::useOpenGles();
            useOpenGLShaders = GLSLRGBWaveformWidget::useOpenGLShaders();
            developerOnly = GLSLRGBWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::GLVSyncTest:
            widgetName = GLVSyncTestWidget::getWaveformWidgetName();
            useOpenGl = GLVSyncTestWidget::useOpenGl();
            useOpenGles =  GLVSyncTestWidget::useOpenGles();
            useOpenGLShaders = GLVSyncTestWidget::useOpenGLShaders();
            developerOnly = GLVSyncTestWidget::developerOnly();
            break;
        case WaveformWidgetType::GLRGBWaveform:
            widgetName = GLRGBWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLRGBWaveformWidget::useOpenGl();
            useOpenGles =  GLRGBWaveformWidget::useOpenGles();
            useOpenGLShaders = GLRGBWaveformWidget::useOpenGLShaders();
            developerOnly = GLRGBWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::QtVSyncTest:
            widgetName = QtVSyncTestWidget::getWaveformWidgetName();
            useOpenGl = QtVSyncTestWidget::useOpenGl();
            useOpenGles =  QtVSyncTestWidget::useOpenGles();
            useOpenGLShaders = QtVSyncTestWidget::useOpenGLShaders();
            developerOnly = QtVSyncTestWidget::developerOnly();
            break;
        case WaveformWidgetType::QtHSVWaveform:
            widgetName = QtHSVWaveformWidget::getWaveformWidgetName();
            useOpenGl = QtHSVWaveformWidget::useOpenGl();
            useOpenGles = QtHSVWaveformWidget::useOpenGles();
            useOpenGLShaders = QtHSVWaveformWidget::useOpenGLShaders();
            developerOnly = QtHSVWaveformWidget::developerOnly();
            break;
        case WaveformWidgetType::QtRGBWaveform:
            widgetName = QtRGBWaveformWidget::getWaveformWidgetName();
            useOpenGl = QtRGBWaveformWidget::useOpenGl();
            useOpenGles = QtRGBWaveformWidget::useOpenGles();
            useOpenGLShaders = QtRGBWaveformWidget::useOpenGLShaders();
            developerOnly = QtRGBWaveformWidget::developerOnly();
            break;
        default:
            DEBUG_ASSERT(!"Unexpected WaveformWidgetType");
            continue;
        }

        bool active = true;
        if (isOpenGlAvailable()) {
            if (useOpenGles && !useOpenGl) {
                active = false;
            } else if (useOpenGLShaders && !isOpenGlShaderAvailable()) {
                active = false;
            } else {
                if (useOpenGLShaders) {
                    widgetName += " " + tr("(GLSL)");
                } else if (useOpenGl) {
                    widgetName += " " + tr("(GL)");
                }
            }
        } else if (isOpenGlesAvailable()) {
            if (useOpenGl && !useOpenGles) {
                active = false;
            } else if (useOpenGLShaders && !isOpenGlShaderAvailable()) {
                active = false;
            } else {
                if (useOpenGLShaders) {
                    widgetName += " " + tr("(GLSL ES)");
                } else if (useOpenGles) {
                    widgetName += " " + tr("(GL ES)");
                }
            }
        } else {
            // No sufficiant GL supptor
            if (useOpenGles || useOpenGl || useOpenGLShaders) {
                active = false;
            }
        }

        if (developerOnly && !CmdlineArgs::Instance().getDeveloper()) {
            active = false;
        }

        if (active) {
            // add new handle for each available widget type
            WaveformWidgetAbstractHandle handle;
            handle.m_displayString = widgetName;
            handle.m_type = (WaveformWidgetType::Type)type;

            m_waveformWidgetHandles.push_back(handle);
        }
    }
}

WaveformWidgetAbstract* WaveformWidgetFactory::createWaveformWidget(
        WaveformWidgetType::Type type, WWaveformViewer* viewer) {
    WaveformWidgetAbstract* widget = nullptr;
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
        case WaveformWidgetType::QtVSyncTest:
            widget = new QtVSyncTestWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::QtHSVWaveform:
            widget = new QtHSVWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::QtRGBWaveform:
            widget = new QtRGBWaveformWidget(viewer->getGroup(), viewer);
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
                widget = nullptr;
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

void WaveformWidgetFactory::startVSync(GuiTick* pGuiTick, VisualsManager* pVisualsManager) {
    m_pGuiTick = pGuiTick;
    m_pVisualsManager = pVisualsManager;
    m_vsyncThread = new VSyncThread(this);
    m_vsyncThread->setObjectName(QStringLiteral("VSync"));
    m_vsyncThread->setVSyncType(m_vSyncType);
    m_vsyncThread->setSyncIntervalTimeMicros(static_cast<int>(1e6 / m_frameRate));

    connect(m_vsyncThread,
            &VSyncThread::vsyncRender,
            this,
            &WaveformWidgetFactory::render);
    connect(m_vsyncThread,
            &VSyncThread::vsyncSwap,
            this,
            &WaveformWidgetFactory::swap);

    m_vsyncThread->start(QThread::NormalPriority);
}

void WaveformWidgetFactory::getAvailableVSyncTypes(QList<QPair<int, QString > >* pList) {
    m_vsyncThread->getAvailableVSyncTypes(pList);
}

WaveformWidgetType::Type WaveformWidgetFactory::findTypeFromHandleIndex(int index) {
    WaveformWidgetType::Type type = WaveformWidgetType::Count_WaveformwidgetType;
    if (index >= 0 && index < m_waveformWidgetHandles.size()) {
        type = m_waveformWidgetHandles[index].m_type;
    }
    return type;
}

int WaveformWidgetFactory::findHandleIndexFromType(WaveformWidgetType::Type type) {
    int index = -1;
    for (int i = 0; i < m_waveformWidgetHandles.size(); i++) {
        WaveformWidgetAbstractHandle& handle = m_waveformWidgetHandles[i];
        if (handle.m_type == type) {
            index = i;
        }
    }
    return index;
}
