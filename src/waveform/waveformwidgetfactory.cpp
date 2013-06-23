#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QWidget>
#include <QtDebug>
#include <QtOpenGL/QGLFormat>
#include <QtOpenGL/QGLShaderProgram>

#include "waveform/waveformwidgetfactory.h"

#include "controlpotmeter.h"
#include "defs.h"
#include "waveform/widgets/emptywaveformwidget.h"
#include "waveform/widgets/softwarewaveformwidget.h"
#include "waveform/widgets/hsvwaveformwidget.h"
#include "waveform/widgets/glwaveformwidget.h"
#include "waveform/widgets/glsimplewaveformwidget.h"
#include "waveform/widgets/qtwaveformwidget.h"
#include "waveform/widgets/qtsimplewaveformwidget.h"
#include "waveform/widgets/glslwaveformwidget.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wwaveformviewer.h"
#include "util/timer.h"

///////////////////////////////////////////

WaveformWidgetAbstractHandle::WaveformWidgetAbstractHandle()
    : m_active(true),
      m_type(WaveformWidgetType::Count_WaveformwidgetType) {
}

///////////////////////////////////////////

WaveformWidgetHolder::WaveformWidgetHolder(WaveformWidgetAbstract* waveformWidget,
                                           WWaveformViewer* waveformViewer,
                                           const QDomNode& visualNodeCache)
    : m_waveformWidget(waveformWidget),
      m_waveformViewer(waveformViewer),
      m_visualNodeCache(visualNodeCache.cloneNode()) {
}

///////////////////////////////////////////

WaveformWidgetFactory::WaveformWidgetFactory() :
        m_type(WaveformWidgetType::Count_WaveformwidgetType),
        m_config(0),
        m_skipRender(false),
        m_frameRate(30),
        m_defaultZoom(3),
        m_zoomSync(false),
        m_overviewNormalized(false),
        m_openGLAvailable(false),
        m_openGLShaderAvailable(false),
        m_time(new QTime()),
        m_lastFrameTime(0),
        m_actualFrameRate(0) {

    m_visualGain[All] = 1.5;
    m_visualGain[Low] = 1.0;
    m_visualGain[Mid] = 1.0;
    m_visualGain[High] = 1.0;

    if (QGLFormat::hasOpenGL()) {
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
        // TOOD(XXX): What should we do on Windows?
        glFormat.setSwapInterval(0);
#endif


        glFormat.setRgba(true);
        QGLFormat::setDefaultFormat(glFormat);

        QGLFormat::OpenGLVersionFlags version = QGLFormat::openGLVersionFlags();

        int majorVersion = 0;
        int minorVersion = 0;
        if (version == QGLFormat::OpenGL_Version_None) {
            m_openGLVersion = "None";
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
        m_openGLShaderAvailable = QGLShaderProgram::hasOpenGLShaderPrograms(glWidget->context());
        delete glWidget;
    }

    evaluateWidgets();
    start();
}

WaveformWidgetFactory::~WaveformWidgetFactory() {
    delete m_time;
}

bool WaveformWidgetFactory::setConfig(ConfigObject<ConfigValue> *config){
    m_config = config;
    if (!m_config)
        return false;

    bool ok = false;

    int frameRate = m_config->getValueString(ConfigKey("[Waveform]","FrameRate")).toInt(&ok);
    if (ok) {
        setFrameRate(frameRate);
    } else {
        m_config->set(ConfigKey("[Waveform]","FrameRate"), ConfigValue(m_frameRate));
    }

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

    return true;
}

void WaveformWidgetFactory::start() {
    //qDebug() << "WaveformWidgetFactory::start";
    killTimer(m_mainTimerId);
    m_mainTimerId = startTimer(1000.0/double(m_frameRate));
}

void WaveformWidgetFactory::stop() {
    killTimer(m_mainTimerId);
    m_mainTimerId = -1;
}

void WaveformWidgetFactory::timerEvent(QTimerEvent *timerEvent) {
    if (timerEvent->timerId() == m_mainTimerId) {
        refresh();
    }
}

void WaveformWidgetFactory::destroyWidgets() {
    for (unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++) {
        WaveformWidgetAbstract* pWidget = m_waveformWidgetHolders[i].m_waveformWidget;;
        m_waveformWidgetHolders[i].m_waveformWidget = NULL;
        delete pWidget;
    }
    m_waveformWidgetHolders.clear();
}

void WaveformWidgetFactory::addTimerListener(QWidget* pWidget) {
    // Do not hold the pointer to of timer listeners since they may be deleted
    connect(this, SIGNAL(waveformUpdateTick()),
            pWidget, SLOT(repaint()),
            Qt::DirectConnection);
}

bool WaveformWidgetFactory::setWaveformWidget(WWaveformViewer* viewer, const QDomElement& node) {
    int index = findIndexOf(viewer);
    if (index != -1) {
        qDebug() << "WaveformWidgetFactory::setWaveformWidget - "\
                    "viewer already have a waveform widget but it's not found by the factory !";
        delete viewer->getWaveformWidget();
    }

    //Cast to widget done just after creation because it can't be perform in constructor (pure virtual)
    WaveformWidgetAbstract* waveformWidget = createWaveformWidget(m_type, viewer);
    viewer->setWaveformWidget(waveformWidget);
    viewer->setup(node);

    // create new holder
    if (index == -1) {
        m_waveformWidgetHolders.push_back(WaveformWidgetHolder(waveformWidget, viewer, node));
        index = m_waveformWidgetHolders.size()-1;
    } else { //update holder
        m_waveformWidgetHolders[index] = WaveformWidgetHolder(waveformWidget, viewer, node);
    }

    viewer->setZoom(m_defaultZoom);

    qDebug() << "WaveformWidgetFactory::setWaveformWidget - waveform widget added in factory, index" << index;

    return true;
}

void WaveformWidgetFactory::setFrameRate(int frameRate) {
    m_frameRate = math_min(60, math_max(10, frameRate));
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","FrameRate"), ConfigValue(m_frameRate));
    }
    start();
}

bool WaveformWidgetFactory::setWidgetType(WaveformWidgetType::Type type) {
    if (type == m_type)
        return true;

    // check if type is acceptable
    for (unsigned int i = 0; i < m_waveformWidgetHandles.size(); i++) {
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
    if (handleIndex < 0 && handleIndex > (int)m_waveformWidgetHandles.size()) {
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
    for (unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++) {
        WaveformWidgetHolder& holder = m_waveformWidgetHolders[i];
        WaveformWidgetAbstract* previousWidget = holder.m_waveformWidget;
        TrackPointer pTrack = previousWidget->getTrackInfo();
        //previousWidget->hold();
        int previousZoom = previousWidget->getZoomFactor();
        delete previousWidget;
        WWaveformViewer* viewer = holder.m_waveformViewer;
        WaveformWidgetAbstract* widget = createWaveformWidget(m_type, holder.m_waveformViewer);
        holder.m_waveformWidget = widget;
        viewer->setWaveformWidget(widget);
        viewer->setup(holder.m_visualNodeCache);
        viewer->setZoom(previousZoom);
        // resize() doesn't seem to get called on the widget. I think Qt skips
        // it since the size didn't change.
        //viewer->resize(viewer->size());
        widget->resize(viewer->width(), viewer->height());
        widget->setTrack(pTrack);
    }

    m_skipRender = false;
    //qDebug() << "recreate done";
    return true;
}

void WaveformWidgetFactory::setDefaultZoom(int zoom) {
    m_defaultZoom = math_max(WaveformWidgetRenderer::s_waveformMinZoom,
                             math_min(zoom, WaveformWidgetRenderer::s_waveformMaxZoom));
    if (m_config) {
        m_config->set(ConfigKey("[Waveform]","DefaultZoom"), ConfigValue(m_defaultZoom));
    }

    for (unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++) {
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
    for (unsigned int i = 1; i < m_waveformWidgetHolders.size(); i++) {
        m_waveformWidgetHolders[i].m_waveformViewer->setZoom(refZoom);
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

void WaveformWidgetFactory::notifyZoomChange(WWaveformViewer* viewer) {
    if (isZoomSync()) {
        //qDebug() << "WaveformWidgetFactory::notifyZoomChange";
        int refZoom = viewer->getWaveformWidget()->getZoomFactor();
        for (int i = 0; i < (int)m_waveformWidgetHolders.size(); i++) {
            if (m_waveformWidgetHolders[i].m_waveformViewer != viewer) {
                m_waveformWidgetHolders[i].m_waveformViewer->setZoom(refZoom);
            }
        }
    }
}

void WaveformWidgetFactory::refresh() {
    if (m_skipRender)
        return;

    ScopedTimer t(QString("WaveformWidgetFactory::refresh() %1waveforms")
                  .arg(m_waveformWidgetHolders.size()));

    for (unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++)
        m_waveformWidgetHolders[i].m_waveformWidget->preRender();

    for (unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++)
        m_waveformWidgetHolders[i].m_waveformWidget->render();

    for (unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++)
        m_waveformWidgetHolders[i].m_waveformWidget->postRender();

    // Notify all other waveform-like widgets (e.g. WSpinny's) that they should
    // update.
    emit(waveformUpdateTick());

    m_lastFrameTime = m_time->restart();
    if (m_lastFrameTime && m_lastFrameTime <= 1000) {
        m_actualFrameRate = 1000.0/(double)(m_lastFrameTime);
    }
}

WaveformWidgetType::Type WaveformWidgetFactory::autoChooseWidgetType() const {
    //default selection
    if (m_openGLAvailable) {
        if (m_openGLShaderAvailable) {
            //TODO: (vrince) enable when ready
            //return = WaveformWidgetType::GLSLWaveform;
            return WaveformWidgetType::GLWaveform;
        } else {
            return WaveformWidgetType::GLWaveform;
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

        switch(type) {
        case WaveformWidgetType::EmptyWaveform:
            widgetName = EmptyWaveformWidget::getWaveformWidgetName();
            useOpenGl = EmptyWaveformWidget::useOpenGl();
            useOpenGLShaders = EmptyWaveformWidget::useOpenGLShaders();
            break;
        case WaveformWidgetType::SoftwareSimpleWaveform:
            continue; // //TODO(vrince):
        case WaveformWidgetType::SoftwareWaveform:
            widgetName = SoftwareWaveformWidget::getWaveformWidgetName();
            useOpenGl = SoftwareWaveformWidget::useOpenGl();
            useOpenGLShaders = SoftwareWaveformWidget::useOpenGLShaders();
            break;
        case WaveformWidgetType::HSVWaveform:
            widgetName = HSVWaveformWidget::getWaveformWidgetName();
            useOpenGl = HSVWaveformWidget::useOpenGl();
            useOpenGLShaders = HSVWaveformWidget::useOpenGLShaders();
            break;
        case WaveformWidgetType::QtSimpleWaveform:
            widgetName = QtSimpleWaveformWidget::getWaveformWidgetName();
            useOpenGl = QtSimpleWaveformWidget::useOpenGl();
            useOpenGLShaders = QtSimpleWaveformWidget::useOpenGLShaders();
            break;
        case WaveformWidgetType::QtWaveform:
            widgetName = QtWaveformWidget::getWaveformWidgetName();
            useOpenGl = QtWaveformWidget::useOpenGl();
            useOpenGLShaders = QtWaveformWidget::useOpenGLShaders();
            break;
        case WaveformWidgetType::GLSimpleWaveform:
            widgetName = GLSimpleWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLSimpleWaveformWidget::useOpenGl();
            useOpenGLShaders = GLSimpleWaveformWidget::useOpenGLShaders();
            break;
        case WaveformWidgetType::GLWaveform:
            widgetName = GLWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLWaveformWidget::useOpenGl();
            useOpenGLShaders = GLWaveformWidget::useOpenGLShaders();
            break;
        case WaveformWidgetType::GLSLWaveform:
            widgetName = GLSLWaveformWidget::getWaveformWidgetName();
            useOpenGl = GLSLWaveformWidget::useOpenGl();
            useOpenGLShaders = GLSLWaveformWidget::useOpenGLShaders();
            break;
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
        m_waveformWidgetHandles.push_back(handle);
    }
}

WaveformWidgetAbstract* WaveformWidgetFactory::createWaveformWidget(
        WaveformWidgetType::Type type, WWaveformViewer* viewer) {
    WaveformWidgetAbstract* widget = NULL;
    if (viewer) {
        switch(type) {
        case WaveformWidgetType::SoftwareWaveform:
            widget = new SoftwareWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::HSVWaveform:
            widget = new HSVWaveformWidget(viewer->getGroup(), viewer);
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
        case WaveformWidgetType::GLWaveform:
            widget = new GLWaveformWidget(viewer->getGroup(), viewer);
            break;
        case WaveformWidgetType::GLSLWaveform:
            widget = new GLSLWaveformWidget(viewer->getGroup(), viewer);
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
    for (int i = 0; i < (int)m_waveformWidgetHolders.size(); i++)
        if (m_waveformWidgetHolders[i].m_waveformViewer == viewer)
            return i;
    return -1;
}
