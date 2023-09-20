#include "waveform/waveformwidgetfactory.h"

#ifdef MIXXX_USE_QOPENGL
#include <QOpenGLShaderProgram>
#include <QOpenGLWindow>
#else
#include <QGLFormat>
#include <QGLShaderProgram>
#endif

#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QRegularExpression>
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
#ifdef MIXXX_USE_QOPENGL
#include "waveform/widgets/allshader/filteredwaveformwidget.h"
#include "waveform/widgets/allshader/hsvwaveformwidget.h"
#include "waveform/widgets/allshader/lrrgbwaveformwidget.h"
#include "waveform/widgets/allshader/rgbwaveformwidget.h"
#include "waveform/widgets/allshader/simplewaveformwidget.h"
#else
#include "waveform/widgets/qthsvwaveformwidget.h"
#include "waveform/widgets/qtrgbwaveformwidget.h"
#include "waveform/widgets/qtsimplewaveformwidget.h"
#include "waveform/widgets/qtvsynctestwidget.h"
#include "waveform/widgets/qtwaveformwidget.h"
#endif
#include "waveform/widgets/emptywaveformwidget.h"
#include "waveform/widgets/glrgbwaveformwidget.h"
#include "waveform/widgets/glsimplewaveformwidget.h"
#include "waveform/widgets/glslwaveformwidget.h"
#include "waveform/widgets/glvsynctestwidget.h"
#include "waveform/widgets/glwaveformwidget.h"
#include "waveform/widgets/hsvwaveformwidget.h"
#include "waveform/widgets/nonglwaveformwidgetabstract.h"
#include "waveform/widgets/rgbwaveformwidget.h"
#include "waveform/widgets/softwarewaveformwidget.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wvumeter.h"
#include "widget/wvumeterlegacy.h"
#include "widget/wwaveformviewer.h"

namespace {
// Returns true if the given waveform should be rendered.
bool shouldRenderWaveform(WaveformWidgetAbstract* pWaveformWidget) {
    if (pWaveformWidget == nullptr ||
        pWaveformWidget->getWidth() == 0 ||
        pWaveformWidget->getHeight() == 0) {
        return false;
    }

    auto* glw = pWaveformWidget->getGLWidget();
    if (glw == nullptr) {
        // Not a WGLWidget. We can simply use QWidget::isVisible.
        auto* qwidget = qobject_cast<QWidget*>(pWaveformWidget->getWidget());
        return qwidget != nullptr && qwidget->isVisible();
    }

    return glw->shouldRender();
}

const QRegularExpression openGLVersionRegex(QStringLiteral("^(\\d+)\\.(\\d+).*$"));
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
          m_frameRate(60),
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

#ifdef MIXXX_USE_QOPENGL
    WGLWidget* widget = SharedGLContext::getWidget();
    if (widget) {
        widget->makeCurrentIfNeeded();
        auto* pContext = QOpenGLContext::currentContext();
        if (pContext) {
            auto* glFunctions = pContext->functions();
            glFunctions->initializeOpenGLFunctions();
            QString versionString(QLatin1String(
                    reinterpret_cast<const char*>(glFunctions->glGetString(GL_VERSION))));
            QString vendorString(QLatin1String(
                    reinterpret_cast<const char*>(glFunctions->glGetString(GL_VENDOR))));
            QString rendererString = QString(QLatin1String(
                    reinterpret_cast<const char*>(glFunctions->glGetString(GL_RENDERER))));
            qDebug().noquote() << QStringLiteral(
                    "OpenGL driver version string \"%1\", vendor \"%2\", "
                    "renderer \"%3\"")
                                          .arg(versionString, vendorString, rendererString);

            GLint majorVersion, minorVersion = GL_INVALID_ENUM;
            glFunctions->glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
            glFunctions->glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
            if (majorVersion == GL_INVALID_ENUM || minorVersion == GL_INVALID_ENUM) {
                // GL_MAJOR/MINOR_VERSION are not supported below OpenGL 3.0, so
                // parse GL_VERSION string as a fallback.
                // https://www.khronos.org/opengl/wiki/OpenGL_Context#OpenGL_version_number
                auto match = openGLVersionRegex.match(versionString);
                DEBUG_ASSERT(match.hasMatch());
                majorVersion = match.captured(1).toInt();
                minorVersion = match.captured(2).toInt();
            }

            qDebug().noquote()
                    << QStringLiteral("Supported OpenGL version: %1.%2")
                               .arg(QString::number(majorVersion), QString::number(minorVersion));

            m_openGLShaderAvailable = QOpenGLShaderProgram::hasOpenGLShaderPrograms(pContext);

            m_openGLVersion = pContext->isOpenGLES() ? "ES " : "";
            m_openGLVersion += majorVersion == 0 ? QString("None") : versionString;

            // Qt5 requires at least OpenGL 2.1 or OpenGL ES 2.0
            if (pContext->isOpenGLES()) {
                if (majorVersion * 100 + minorVersion >= 200) {
                    m_openGlesAvailable = true;
                }
            } else {
                if (majorVersion * 100 + minorVersion >= 201) {
                    m_openGlAvailable = true;
                }
            }

            if (!rendererString.isEmpty()) {
                m_openGLVersion += " (" + rendererString + ")";
            }
        } else {
            qDebug() << "QOpenGLContext::currentContext() returns nullptr";
        }
        widget->doneCurrent();
        widget->hide();
    }
#else
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
#endif
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

void WaveformWidgetFactory::addVuMeter(WVuMeterLegacy* pVuMeter) {
    // Do not hold the pointer to of timer listeners since they may be deleted.
    // We don't activate update() or repaint() directly so listener widgets
    // can decide whether to paint or not.
    connect(this,
            &WaveformWidgetFactory::waveformUpdateTick,
            pVuMeter,
            &WVuMeterLegacy::maybeUpdate,
            Qt::DirectConnection);
}

void WaveformWidgetFactory::addVuMeter(WVuMeterBase* pVuMeter) {
    // WVuMeterGLs to be rendered and swapped from the vsync thread
    connect(this,
            &WaveformWidgetFactory::renderVuMeters,
            pVuMeter,
            &WVuMeterBase::render);
    connect(this,
            &WaveformWidgetFactory::swapVuMeters,
            pVuMeter,
            &WVuMeterBase::swap);
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
    bool isAcceptable = index > -1;
    *pCurrentType = isAcceptable ? type : WaveformWidgetType::EmptyWaveform;
    if (m_config) {
        m_config->setValue(
                ConfigKey("[Waveform]", "WaveformType"), *pCurrentType);
    }
    return isAcceptable;
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
    ScopedTimer t(u"WaveformWidgetFactory::render() %1waveforms",
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
                shouldRenderWaveforms[static_cast<int>(i)] = shouldRender;
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
                if (!shouldRenderWaveforms[static_cast<int>(i)]) {
                    continue;
                }
                pWaveformWidget->render();
                //qDebug() << "render" << i << m_vsyncThread->elapsed();
            }
        }

        // WSpinnys are also double-buffered WGLWidgets, like all the waveform
        // renderers. Render all the WSpinny widgets now.
        emit renderSpinnies(m_vsyncThread);
        // Same for WVuMeterGL. Note that we are either using WVuMeter or WVuMeterGL.
        // If we are using WVuMeter, this does nothing
        emit renderVuMeters(m_vsyncThread);

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
    ScopedTimer t(u"WaveformWidgetFactory::swap() %1waveforms",
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
                // window, behavior is undefined" on Qt5. See issue #9360.
                if (!shouldRenderWaveform(pWaveformWidget)) {
                    continue;
                }
                WGLWidget* glw = pWaveformWidget->getGLWidget();
                if (glw != nullptr) {
                    glw->makeCurrentIfNeeded();
                    glw->swapBuffers();
                    glw->doneCurrent();
                }
                //qDebug() << "swap x" << m_vsyncThread->elapsed();
            }
        }
        // WSpinnys are also double-buffered QGLWidgets, like all the waveform
        // renderers. Swap all the WSpinny widgets now.
        emit swapSpinnies();
        // Same for WVuMeterGL. Note that we are either using WVuMeter or WVuMeterGL
        // If we are using WVuMeter, this does nothing
        emit swapVuMeters();
    }
    //qDebug() << "swap end" << m_vsyncThread->elapsed();
    m_vsyncThread->vsyncSlotFinished();
}

WaveformWidgetType::Type WaveformWidgetFactory::autoChooseWidgetType() const {
    if (isOpenGlShaderAvailable()) {
#ifndef MIXXX_USE_QOPENGL
        return WaveformWidgetType::GLSLRGBWaveform;
#else
        return WaveformWidgetType::AllShaderRGBWaveform;
#endif
    }
    if (isOpenGlAvailable() || isOpenGlesAvailable()) {
        return WaveformWidgetType::GLRGBWaveform;
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
        WaveformWidgetCategory category;

        // this lambda needs its type specified explicitly,
        // requiring it to be called with via `.operator()<WaveformT>()`
        auto setWaveformVarsByType = [&]<typename WaveformT>() {
            widgetName = buildWidgetDisplayName<WaveformT>();
            useOpenGl = WaveformT::useOpenGl();
            useOpenGles = WaveformT::useOpenGles();
            useOpenGLShaders = WaveformT::useOpenGLShaders();
            category = WaveformT::category();
        };

        switch(type) {
        case WaveformWidgetType::EmptyWaveform:
            setWaveformVarsByType.operator()<EmptyWaveformWidget>();
            break;
        case WaveformWidgetType::SoftwareSimpleWaveform:
            continue; // //TODO(vrince):
        case WaveformWidgetType::SoftwareWaveform:
#ifdef __APPLE__
            // Don't offer the simple renderers on macOS, they do not work with skins
            // that load GL widgets (spinnies, waveforms) in singletons.
            // Also excluded in enum WaveformWidgetType
            // https://bugs.launchpad.net/bugs/1928772
            continue;
#else
            setWaveformVarsByType.operator()<SoftwareWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::HSVWaveform:
#ifdef __APPLE__
            continue;
#else
            setWaveformVarsByType.operator()<HSVWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::RGBWaveform:
#ifdef __APPLE__
            continue;
#else
            setWaveformVarsByType.operator()<RGBWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::QtSimpleWaveform:
#ifdef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<QtSimpleWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::QtWaveform:
#ifdef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<QtWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::GLSimpleWaveform:
            setWaveformVarsByType.operator()<GLSimpleWaveformWidget>();
            break;
        case WaveformWidgetType::GLFilteredWaveform:
            setWaveformVarsByType.operator()<GLWaveformWidget>();
            break;
        case WaveformWidgetType::GLSLFilteredWaveform:
            setWaveformVarsByType.operator()<GLSLFilteredWaveformWidget>();
            break;
        case WaveformWidgetType::GLSLRGBWaveform:
            setWaveformVarsByType.operator()<GLSLRGBWaveformWidget>();
            break;
        case WaveformWidgetType::GLSLRGBStackedWaveform:
            setWaveformVarsByType.operator()<GLSLRGBStackedWaveformWidget>();
            break;
        case WaveformWidgetType::GLVSyncTest:
            setWaveformVarsByType.operator()<GLVSyncTestWidget>();
            break;
        case WaveformWidgetType::GLRGBWaveform:
            setWaveformVarsByType.operator()<GLRGBWaveformWidget>();
            break;
        case WaveformWidgetType::QtVSyncTest:
#ifdef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<QtVSyncTestWidget>();
#endif
            break;
        case WaveformWidgetType::QtHSVWaveform:
#ifdef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<QtHSVWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::QtRGBWaveform:
#ifdef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<QtRGBWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::AllShaderRGBWaveform:
#ifndef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<allshader::RGBWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::AllShaderLRRGBWaveform:
#ifndef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<allshader::LRRGBWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::AllShaderFilteredWaveform:
#ifndef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<allshader::FilteredWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::AllShaderSimpleWaveform:
#ifndef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<allshader::SimpleWaveformWidget>();
            break;
#endif
        case WaveformWidgetType::AllShaderHSVWaveform:
#ifndef MIXXX_USE_QOPENGL
            continue;
#else
            setWaveformVarsByType.operator()<allshader::HSVWaveformWidget>();
            break;
#endif
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
            }
        } else if (isOpenGlesAvailable()) {
            if (useOpenGl && !useOpenGles) {
                active = false;
            } else if (useOpenGLShaders && !isOpenGlShaderAvailable()) {
                active = false;
            }
        } else {
            // No sufficient GL support
            if (useOpenGles || useOpenGl || useOpenGLShaders) {
                active = false;
            }
        }

        if (category == WaveformWidgetCategory::DeveloperOnly &&
                !CmdlineArgs::Instance().getDeveloper()) {
            active = false;
        }

        if (active) {
            // add new handle for each available widget type
            WaveformWidgetAbstractHandle handle;
            handle.m_displayString = widgetName;
            handle.m_type = static_cast<WaveformWidgetType::Type>(type);

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
        case WaveformWidgetType::GLSLRGBStackedWaveform:
            widget = new GLSLRGBStackedWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::GLVSyncTest:
            widget = new GLVSyncTestWidget(viewer->getGroup(), viewer);
            break;
#ifdef MIXXX_USE_QOPENGL
        case WaveformWidgetType::AllShaderRGBWaveform:
            widget = new allshader::RGBWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::AllShaderLRRGBWaveform:
            widget = new allshader::LRRGBWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::AllShaderFilteredWaveform:
            widget = new allshader::FilteredWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::AllShaderSimpleWaveform:
            widget = new allshader::SimpleWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::AllShaderHSVWaveform:
            widget = new allshader::HSVWaveformWidget(viewer->getGroup(), viewer);
            break;
#else
        case WaveformWidgetType::QtSimpleWaveform:
            widget = new QtSimpleWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::QtWaveform:
            widget = new QtWaveformWidget(viewer->getGroup(), viewer);
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
#endif
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

void WaveformWidgetFactory::getAvailableVSyncTypes(QList<QPair<int, QString>>* pList) {
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

template<typename WaveformT>
QString WaveformWidgetFactory::buildWidgetDisplayName() const {
    const bool isLegacy = WaveformT::category() == WaveformWidgetCategory::Legacy;
    QStringList extras;
    if (isLegacy) {
        extras.push_back(tr("legacy"));
    }
    if (isOpenGlesAvailable()) {
        if (WaveformT::useOpenGLShaders()) {
            extras.push_back(QStringLiteral("GLSL ES"));
        } else if (WaveformT::useOpenGles()) {
            extras.push_back(QStringLiteral("GLES"));
        }
    } else if (isOpenGlAvailable()) {
        if (WaveformT::useOpenGLShaders()) {
            extras.push_back(QStringLiteral("GLSL"));
        } else if (WaveformT::useOpenGl()) {
            extras.push_back(QStringLiteral("GL"));
        }
    }
    QString name = WaveformT::getWaveformWidgetName();
    if (extras.isEmpty()) {
        return name;
    }
    return QStringLiteral("%1 (%2)").arg(name, extras.join(QStringLiteral(", ")));
}

// static
QSurfaceFormat WaveformWidgetFactory::getSurfaceFormat() {
    QSurfaceFormat format;
    // Qt5 requires at least OpenGL 2.1 or OpenGL ES 2.0, default is 2.0
    // format.setVersion(2, 1);
    // Core and Compatibility contexts have been introduced in openGL 3.2
    // From 3.0 to 3.1 we have implicit the Core profile and Before 3.0 we have the
    // Compatibility profile
    // format.setProfile(QSurfaceFormat::CoreProfile);

    // setSwapInterval sets the application preferred swap interval
    // in minimum number of video frames that are displayed before a buffer swap occurs
    // - 0 will turn the vertical refresh syncing off
    // - 1 (default) means swapping after drawig a video frame to the buffer
    // - n means swapping after drawing n video frames to the buffer
    //
    // The vertical sync setting requested by the OpenGL application, can be overwritten
    // if a user changes the "Wait for vertical refresh" setting in AMD graphic drivers
    // for Windows.

#if defined(__APPLE__)
    // On OS X, syncing to vsync has good performance FPS-wise and
    // eliminates tearing. (This is an comment from pre QOpenGLWindow times)
    format.setSwapInterval(1);
#else
    // It seems that on Windows (at least for some AMD drivers), the setting 1 is not
    // not properly handled. We saw frame rates divided by exact integers, like it should
    // be with values >1 (see https://github.com/mixxxdj/mixxx/issues/11617)
    // Reported as https://bugreports.qt.io/browse/QTBUG-114882
    // On Linux, horrible FPS were seen with "VSync off" before switching to QOpenGLWindow too
    format.setSwapInterval(0);
#endif
    return format;
}
