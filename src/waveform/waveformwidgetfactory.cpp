#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QWidget>
#include <QtDebug>
#include <QGLFormat>
#include <QOpenGLShaderProgram>
#include <QGuiApplication>
#include <QApplication>
#include <QWindow>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "waveform/waveformwidgetfactory.h"

#include "control/controlpotmeter.h"
#include "waveform/widgets/emptywaveformwidget.h"
#include "waveform/widgets/softwarewaveformwidget.h"
#include "waveform/widgets/hsvwaveformwidget.h"
#include "waveform/widgets/rgbwaveformwidget.h"
#include "waveform/widgets/qthsvwaveformwidget.h"
#include "waveform/widgets/qtrgbwaveformwidget.h"
#include "waveform/widgets/glrgbwaveformwidget.h"
#include "waveform/widgets/glwaveformwidget.h"
#include "waveform/widgets/glsimplewaveformwidget.h"
#include "waveform/widgets/qtwaveformwidget.h"
#include "waveform/widgets/qtsimplewaveformwidget.h"
#include "waveform/widgets/glslwaveformwidget.h"
#include "waveform/widgets/glvsynctestwidget.h"
#include "waveform/widgets/qtvsynctestwidget.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wwaveformviewer.h"
#include "waveform/guitick.h"
#include "waveform/visualsmanager.h"
#include "waveform/renderthread.h"
#include "util/cmdlineargs.h"
#include "util/performancetimer.h"
#include "util/time.h"
#include "util/timer.h"
#include "util/math.h"
#include "util/memory.h"

namespace {
// Returns true if the given waveform should be rendered.
bool shouldRenderWaveform(WaveformWidgetAbstract* pWaveformWidget) {
    if (pWaveformWidget == nullptr ||
        pWaveformWidget->getWidth() == 0 ||
        pWaveformWidget->getHeight() == 0) {
        return false;
    }

    auto glw = dynamic_cast<QOpenGLWidget*>(pWaveformWidget->getWidget());
    if (glw == nullptr) {
        // Not a QOpenGLWidget. We can simply use QWidget::isVisible.
        auto qwidget = dynamic_cast<QWidget*>(pWaveformWidget->getWidget());
        return qwidget != nullptr && qwidget->isVisible();
    }

    if (glw == nullptr || !glw->isValid() || !glw->isVisible()) {
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
    : m_waveformWidget(NULL),
      m_waveformViewer(NULL),
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
        : m_type(WaveformWidgetType::Count_WaveformwidgetType),
          m_config(0),
          m_skipRender(false),
          m_frameRate(30),
          m_endOfTrackWarningTime(30),
          m_defaultZoom(WaveformWidgetRenderer::s_waveformDefaultZoom),
          m_zoomSync(false),
          m_overviewNormalized(false),
          m_openGlAvailable(false),
          m_openGlesAvailable(false),
          m_openGLShaderAvailable(false),
          m_beatGridAlpha(90),
          m_renderThread(nullptr),
          m_pGuiTick(nullptr),
          m_pVisualsManager(nullptr),
          m_frameCnt(0),
          m_actualFrameRate(0),
          m_renderType(0),
          m_playMarkerPosition(WaveformWidgetRenderer::s_defaultPlayMarkerPosition) {
    m_visualGain[All] = 1.0;
    m_visualGain[Low] = 1.0;
    m_visualGain[Mid] = 1.0;
    m_visualGain[High] = 1.0;

    const bool safeMode = CmdlineArgs::Instance().getSafeMode();
    QWindow* pWindow = QGuiApplication::focusWindow();

    if (safeMode) {
        m_openGLVersion = tr("Safe Mode");
    } else if (pWindow && pWindow->supportsOpenGL()) {
        m_openGlAvailable = true;

        QWidget* w = nullptr;
        foreach(QWidget* widget, QApplication::topLevelWidgets())
        if (widget->inherits("QMainWindow")) {
            w = widget;
            break;
        }
        std::unique_ptr<QOpenGLWidget> pGlW = std::make_unique<QOpenGLWidget>(w);
        //glw->makeCurrent(); // does not work here
        pGlW->show(); // the context is only valid if the wiget is shown.
        QOpenGLContext* pContext = pGlW->context();

        QString version;
        QString renderer;
        if (pContext) {
            version = QString::fromUtf8(reinterpret_cast<const char*>(
                    pContext->functions()->glGetString(GL_VERSION)));
            renderer = QString::fromUtf8(reinterpret_cast<const char*>(
                    pContext->functions()->glGetString(GL_RENDERER)));
        }

        m_openGLShaderAvailable = QOpenGLShaderProgram::hasOpenGLShaderPrograms(pContext);

        const QSurfaceFormat format = pWindow->format();

        const QSurfaceFormat::RenderableType renderableType = format.renderableType();
        QString strRenderableType;
        switch (renderableType) {
        case QSurfaceFormat::DefaultRenderableType:
            strRenderableType = "DefaultRenderableType";
            break;
        case QSurfaceFormat::OpenGL:
            strRenderableType = "OpenGL";
            break;
        case QSurfaceFormat::OpenGLES:
            strRenderableType = "OpenGLES";
            m_openGlesAvailable = true;
            break;
        case QSurfaceFormat::OpenVG:
            strRenderableType = "OpenVG";
            break;
        default:
            strRenderableType = "Unknown Type";
            break;
        }
        m_openGLVersion = strRenderableType + ": " + version + " (" + renderer + ")";
    } else {
        m_openGLVersion = tr("None");
    }

    evaluateWidgets();
    m_time.start();
}

WaveformWidgetFactory::~WaveformWidgetFactory() {
    if (m_renderThread) {
        delete m_renderThread;
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

    int render = m_config->getValue(ConfigKey("[Waveform]","Render"), 0);
    setRenderType(render);

    double defaultZoom = m_config->getValueString(ConfigKey("[Waveform]","DefaultZoom")).toDouble(&ok);
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
    for (std::size_t i = 0; i < m_waveformWidgetHolders.size(); i++) {
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
        index = m_waveformWidgetHolders.size() - 1;
    } else {
        // update holder
        DEBUG_ASSERT(index >= 0);
        m_waveformWidgetHolders[index] = std::move(holder);
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
    if (m_renderThread) {
        m_renderThread->setSyncIntervalTimeMicros(1e6 / m_frameRate);
    }
}

void WaveformWidgetFactory::setEndOfTrackWarningTime(int endTime) {
    m_endOfTrackWarningTime = endTime;
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","EndOfTrackWarningTime"), ConfigValue(m_endOfTrackWarningTime));
    }
}

void WaveformWidgetFactory::setRenderType(int type) {
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","Render"), ConfigValue((int)type));
    }

    m_renderType = type;
    if (m_renderThread) {
        m_renderThread->setRenderType(type);
    }
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

    //re-create/setup all waveform widgets
    for (std::size_t i = 0; i < m_waveformWidgetHolders.size(); i++) {
        WaveformWidgetHolder& holder = m_waveformWidgetHolders[i];
        WaveformWidgetAbstract* previousWidget = holder.m_waveformWidget;
        TrackPointer pTrack = previousWidget->getTrackInfo();
        //previousWidget->hold();
        double previousZoom = previousWidget->getZoomFactor();
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

    for (std::size_t i = 0; i < m_waveformWidgetHolders.size(); i++) {
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

    double refZoom = m_waveformWidgetHolders[0].m_waveformWidget->getZoomFactor();
    for (std::size_t i = 1; i < m_waveformWidgetHolders.size(); i++) {
        m_waveformWidgetHolders[i].m_waveformViewer->setZoom(refZoom);
    }
}

void WaveformWidgetFactory::setDisplayBeatGridAlpha(int alpha) {
    m_beatGridAlpha = alpha;
    if (m_waveformWidgetHolders.size() == 0) {
        return;
    }

    for (std::size_t i = 0; i < m_waveformWidgetHolders.size(); i++) {
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
    ScopedTimer t("WaveformWidgetFactory::render() %1waveforms", m_waveformWidgetHolders.size());

    //int paintersSetupTime0 = 0;
    //int paintersSetupTime1 = 0;

    //qDebug() << "render()" << m_renderThread->elapsed();

    if (!m_skipRender) {
        if (m_type) {   // no regular updates for an empty waveform
            // next rendered frame is displayed after next buffer swap and than after Render
            //qDebug() << "prerender" << m_renderThread->elapsed();

            // It may happen that there is an artificially delayed due to
            // anti tearing driver settings
            // all render commands are delayed until the swap from the previous run is executed
            for (std::size_t i = 0; i < m_waveformWidgetHolders.size(); i++) {
                WaveformWidgetAbstract* pWaveformWidget = m_waveformWidgetHolders[i].m_waveformWidget;
                if (!shouldRenderWaveform(pWaveformWidget)) {
                    continue;
                }
                pWaveformWidget->renderOnNextTick();
                //qDebug() << "render" << i << m_renderThread->elapsed();
            }
        }

        // WSpinnys are also double-buffered QOpenGLWidgets, like all the
        // waveform renderers. Render all the WSpinny widgets now.
        emit renderSpinnies();

        // Notify all other waveform-like widgets (e.g. WSpinny's) that they should
        // update.
        //int t1 = m_renderThread->elapsed();
        emit waveformUpdateTick();
        //qDebug() << "emit" << m_renderThread->elapsed() - t1;

        m_frameCnt += 1.0;
        mixxx::Duration timeCnt = m_time.elapsed();
        if (timeCnt > mixxx::Duration::fromSeconds(1)) {
            m_time.start();
            m_frameCnt = m_frameCnt * 1000 / timeCnt.toIntegerMillis(); // latency correction
            emit waveformMeasured(m_frameCnt, m_renderThread->droppedFrames());
            m_frameCnt = 0.0;
        }
    }

    m_pVisualsManager->process(m_endOfTrackWarningTime);
    m_pGuiTick->process();

    //qDebug() << "refresh end" << m_renderThread->elapsed();
    m_renderThread->renderSlotFinished();
}

WaveformWidgetType::Type WaveformWidgetFactory::autoChooseWidgetType() const {
    //default selection
    if (m_openGlAvailable) {
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
            // No sufficiant GL support
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
        if (!widget->isInitialized()) {
            qWarning() << "failed to init WaveformWidget" << type << "fall back to \"Empty\"";
            delete widget;
            widget = new EmptyWaveformWidget(viewer->getGroup(), viewer);
            widget->castToQWidget();
            if (!widget->isInitialized()) {
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

void WaveformWidgetFactory::startRenderThread(GuiTick* pGuiTick, VisualsManager* pVisualsManager) {
    m_pGuiTick = pGuiTick;
    m_pVisualsManager = pVisualsManager;
    m_renderThread = new RenderThread(this);
    m_renderThread->start(QThread::NormalPriority);
    m_renderThread->setRenderType(m_renderType);

    connect(m_renderThread, SIGNAL(render()),
            this, SLOT(render()));
}

void WaveformWidgetFactory::setDefaultSurfaceFormat() {
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QSurfaceFormat defaultFormat;
    defaultFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    defaultFormat.setSwapInterval(1);
    defaultFormat.setProfile(QSurfaceFormat::CompatibilityProfile);
    defaultFormat.setRenderableType(QSurfaceFormat::OpenGL);
    defaultFormat.setVersion(2, 1);
    QSurfaceFormat::setDefaultFormat(defaultFormat);
}
