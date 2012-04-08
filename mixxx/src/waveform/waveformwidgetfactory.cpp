#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/waveformwidgetabstract.h"

#include "widget/wwaveformviewer.h"

//WaveformWidgets
#include "waveform/widgets/emptywaveformwidget.h"
#include "waveform/widgets/softwarewaveformwidget.h"
#include "waveform/widgets/glwaveformwidget.h"
#include "waveform/widgets/glslwaveformwidget.h"
#include "waveform/widgets/glsimplewaveformwidget.h"

#include "controlpotmeter.h"
#include "defs.h"

#include <QTimer>
#include <QWidget>
#include <QTime>
#include <QStringList>

#include <QtOpenGL/QGLFormat>
#include <QtOpenGL/QGLShaderProgram>

#include <QDebug>

///////////////////////////////////////////

WaveformWidgetAbstractHandle::WaveformWidgetAbstractHandle() {
    m_active = true;
    m_type = WaveformWidgetType::Count_WaveformwidgetType;
}

///////////////////////////////////////////

WaveformWidgetHolder::WaveformWidgetHolder( WaveformWidgetAbstract* waveformWidget,
                                            WWaveformViewer* waveformViewer,
                                            const QDomNode& visualNodeCache)
    : m_waveformWidget(waveformWidget)
    , m_waveformViewer(waveformViewer){
    m_visualNodeCache = visualNodeCache.cloneNode();
}

///////////////////////////////////////////

WaveformWidgetFactory::WaveformWidgetFactory() {
    m_time = new QTime();
    m_config = 0;
    //m_skipRender = false;
    setFrameRate(33);
    m_defaultZoom = 1;
    m_zoomSync = false;

    m_lastFrameTime = 0;
    m_actualFrameRate = 0;

    //setup the opengl default format
    m_openGLAvailable = false;
    m_openGLShaderAvailable = false;

    if (QGLFormat::hasOpenGL()) {
        QGLFormat glFormat;
        glFormat.setDirectRendering(true);
        glFormat.setDoubleBuffer(true);
        glFormat.setDepth(false);
        glFormat.setSwapInterval(0); //enable vertical sync to avoid cue line to be cut
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

        if( majorVersion != 0 && minorVersion != 0)
            m_openGLVersion = QString::number(majorVersion) + "." +
                    QString::number(minorVersion);
        m_openGLAvailable = true;
        {
            QGLWidget glWidget;
            glWidget.makeCurrent();
            m_openGLShaderAvailable = QGLShaderProgram::hasOpenGLShaderPrograms();
            glWidget.doneCurrent();
        }
    }

    //default selection
    if (m_openGLAvailable) {
        if (m_openGLShaderAvailable) {
            //TODO: (vrince) enable when ready
            //m_type = WaveformWidgetType::GLSLWaveform;
            m_type = WaveformWidgetType::GLWaveform;
        } else {
            m_type = WaveformWidgetType::GLWaveform;
        }
    } else {
        m_type = WaveformWidgetType::SoftwareWaveform;
    }

    evaluateWidgets();
    start();
}

WaveformWidgetFactory::~WaveformWidgetFactory() {
    delete m_time;
}

bool WaveformWidgetFactory::setConfig(ConfigObject<ConfigValue> *config){
    m_config = config;
    if( !m_config)
        return false;

    QString framRate = m_config->getValueString(ConfigKey("[Waveform]","FrameRate"));
    if( !framRate.isEmpty()) {
        int frameRate = framRate.toInt();
        setFrameRate(frameRate);
    } else {
        m_config->set(ConfigKey("[Waveform]","FrameRate"), ConfigValue(m_frameRate));
    }

    QString defaultZoom = m_config->getValueString(ConfigKey("[Waveform]","DefaultZoom"));
    if( !defaultZoom.isEmpty()) {
        int zoom = defaultZoom.toInt();
        setDefaultZoom(zoom);
    } else{
        m_config->set(ConfigKey("[Waveform]","DefaultZoom"), ConfigValue(m_defaultZoom));
    }

    QString zoomSync = m_config->getValueString(ConfigKey("[Waveform]","ZoomSynchronization"));
    if( !zoomSync.isEmpty()) {
        bool sync = zoomSync.toInt();
        setZoomSync(sync);
    } else {
        m_config->set(ConfigKey("[Waveform]","ZoomSynchronization"), ConfigValue(m_zoomSync));
    }

    return true;
}

void WaveformWidgetFactory::start() {
    qDebug() << "WaveformWidgetFactory::start";
    killTimer(m_mainTimerId);
    m_mainTimerId = startTimer(1000.0/double(m_frameRate));
}

void WaveformWidgetFactory::stop() {
    killTimer(m_mainTimerId);
    m_mainTimerId = -1;
}

void WaveformWidgetFactory::timerEvent(QTimerEvent *timerEvent) {
    if( timerEvent->timerId() == m_mainTimerId) {
        refresh();
    }
}

void WaveformWidgetFactory::destroyWidgets() {
    for( unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++)
        delete m_waveformWidgetHolders[i].m_waveformWidget;
    m_waveformWidgetHolders.clear();
}

void WaveformWidgetFactory::addTimerListener(QWidget* pWidget) {
    // Do not hold the pointer to of timer listeners since they may be deleted
    connect(this, SIGNAL(waveformUpdateTick()),
            pWidget, SLOT(update()));
}

bool WaveformWidgetFactory::setWaveformWidget(WWaveformViewer* viewer, const QDomElement& node) {
    int index = findIndexOf(viewer);
    if( index != -1) {
        qDebug() << "WaveformWidgetFactory::setWaveformWidget - "\
                    "viewer already have a waveform widget but it's not found by the factory !";
        delete viewer->getWaveformWidget();
    }

    //Cast to widget done just after creation because it can't be perform in constructor (pure virtual)
    WaveformWidgetAbstract* waveformWidget = createWaveformWidget(m_type, viewer);
    waveformWidget->castToQWidget();
    viewer->setWaveformWidget(waveformWidget);
    viewer->setup(node);

    if( index == -1) {    //create new holder
        m_waveformWidgetHolders.push_back( WaveformWidgetHolder( waveformWidget, viewer, node));
        index = m_waveformWidgetHolders.size()-1;
    } else { //update holder
        m_waveformWidgetHolders[index] = WaveformWidgetHolder( waveformWidget, viewer, node);
    }

    qDebug() << "WaveformWidgetFactory::setWaveformWidget - waveform widget added in factory index" << index;

    return true;
}

void WaveformWidgetFactory::setFrameRate(int frameRate) {
    m_frameRate = math_min(60, math_max( 10, frameRate));
    if( m_config)
        m_config->set(ConfigKey("[Waveform]","FrameRate"), ConfigValue(m_frameRate));
}

bool WaveformWidgetFactory::setWidgetType(int handleIndex) {
    if (handleIndex < 0 && handleIndex > m_waveformWidgetHandles.size()) {
        qDebug() << "WaveformWidgetFactory::setWidgetType - invalid handle";
        return false;
    }

    WaveformWidgetAbstractHandle& handle = m_waveformWidgetHandles[handleIndex];
    if (handle.m_type == m_type) {
        qDebug() << "WaveformWidgetFactory::setWidgetType - type already in use";
        return false;
    }

    //change the type
    m_type = handle.m_type;

    //NOTE: (vRince)
    //I tried but I can't figure-out what is missing here that append in skin parser

    /*
    m_skipRender = true;
    qDebug() << "recreate start";

    //re-create/setup all waveform widgets
    for( unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++) {
        WaveformWidgetHolder& holder = m_waveformWidgetHolders[i];
        WaveformWidgetAbstract* previousWidget = holder.m_waveformWidget;

        holder.m_waveformWidget = createWaveformWidget(m_type, holder.m_waveformViewer);
        holder.m_waveformWidget->castToQWidget();
        holder.m_waveformViewer->setWaveformWidget(holder.m_waveformWidget);
        holder.m_waveformViewer->setup(holder.m_visualNodeCache);

        holder.m_waveformViewer->resize(holder.m_waveformViewer->size());

        holder.m_waveformWidget->setTrack(previousWidget->getTrackInfo());
        delete previousWidget;
    }

    m_skipRender = false;
    qDebug() << "recreate done";
    */

    return true;
}


void WaveformWidgetFactory::setDefaultZoom(int zoom){
    m_defaultZoom = math_max(1,math_min(4,zoom));
    if( m_config)
        m_config->set(ConfigKey("[Waveform]","DefaultZoom"), ConfigValue(m_defaultZoom));

    for( unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++)
        m_waveformWidgetHolders[i].m_waveformViewer->setZoom(m_defaultZoom);
}

void WaveformWidgetFactory::setZoomSync(bool sync) {
    m_zoomSync = sync;
    if( m_config)
        m_config->set(ConfigKey("[Waveform]","ZoomSynchronization"), ConfigValue(m_zoomSync));

    if( m_waveformWidgetHolders.size() == 0)
        return;

    int refZoom = m_waveformWidgetHolders[0].m_waveformWidget->getZoomFactor();
    for( unsigned int i = 1; i < m_waveformWidgetHolders.size(); i++)
        m_waveformWidgetHolders[i].m_waveformViewer->setZoom(refZoom);
}

void WaveformWidgetFactory::notifyZoomChange( WWaveformViewer* viewer) {
    if( isZoomSync()) {
        int refZoom = viewer->getWaveformWidget()->getZoomFactor();
        for (int i = 0; i < m_waveformWidgetHolders.size(); i++)
            if( m_waveformWidgetHolders[i].m_waveformViewer != viewer)
                m_waveformWidgetHolders[i].m_waveformViewer->setZoom(refZoom);
    }
}

void WaveformWidgetFactory::refresh() {
    //if( m_skipRender)
    //    return;

    for( unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++)
        m_waveformWidgetHolders[i].m_waveformWidget->preRender();

    for( unsigned int i = 0; i < m_waveformWidgetHolders.size(); i++)
        m_waveformWidgetHolders[i].m_waveformWidget->render();

    // Notify all other waveform-like widgets (e.g. WSpinny's) that they should
    // update.
    emit(waveformUpdateTick());

    m_lastFrameTime = m_time->elapsed();
    m_time->restart();

    m_actualFrameRate = 1000.0/(double)(m_lastFrameTime);
}

void WaveformWidgetFactory::evaluateWidgets() {
    m_waveformWidgetHandles.clear();
    for (int type = 0; type < WaveformWidgetType::Count_WaveformwidgetType; type++) {
        WaveformWidgetAbstract* widget = 0;
        switch(type) {
        case WaveformWidgetType::EmptyWaveform : widget = new EmptyWaveformWidget(); break;
        case WaveformWidgetType::SimpleSoftwareWaveform : break; //TODO: (vrince)
        case WaveformWidgetType::GLSimpleWaveform : widget = new GLSimpleWaveformWidget(); break;
        case WaveformWidgetType::SoftwareWaveform : widget = new SoftwareWaveformWidget(); break;
        case WaveformWidgetType::GLWaveform : widget = new GLWaveformWidget(); break;
        case WaveformWidgetType::GLSLWaveform : widget = new GLSLWaveformWidget(); break;
        }

        if (widget) {
            QString widgetName = widget->getWaveformWidgetName();
            if (widget->useOpenGLShaders()) {
                widgetName += " " + WaveformWidgetAbstract::s_openGlShaderFlag;
            } else if (widget->useOpenGl()) {
                widgetName += " " + WaveformWidgetAbstract::s_openGlFlag;
            }

            // add new handle for each available widget type
            WaveformWidgetAbstractHandle handle;
            handle.m_displayString = widgetName;
            handle.m_type = (WaveformWidgetType::Type)type;

            // NOTE: For the moment non active widget are not added to available handle
            // but it could be useful to have them anyway but not selectable in the combo box
            if ((widget->useOpenGl() && !isOpenGLAvailable()) ||
                    (widget->useOpenGLShaders() && !isOpenGlShaderAvailable())) {
                handle.m_active = false;
                continue;
            }
            m_waveformWidgetHandles.push_back(handle);
        }
        delete widget;
    }
}

WaveformWidgetAbstract* WaveformWidgetFactory::createWaveformWidget(WaveformWidgetType::Type type,
                                                                    WWaveformViewer* viewer) {
    if (viewer) {
        switch(type) {
        case WaveformWidgetType::EmptyWaveform : return new EmptyWaveformWidget(viewer->getGroup(), viewer);
        case WaveformWidgetType::SimpleSoftwareWaveform : return 0; //TODO: (vrince)
        case WaveformWidgetType::GLSimpleWaveform : return new GLSimpleWaveformWidget(viewer->getGroup(), viewer);
        case WaveformWidgetType::SoftwareWaveform : return new SoftwareWaveformWidget(viewer->getGroup(), viewer);
        case WaveformWidgetType::GLWaveform : return new GLWaveformWidget(viewer->getGroup(), viewer);
        case WaveformWidgetType::GLSLWaveform : return new GLSLWaveformWidget(viewer->getGroup(), viewer);
        default : return 0;
        }
    }
    return 0;
}

int WaveformWidgetFactory::findIndexOf( WWaveformViewer* viewer) const {
    for (int i = 0; i < m_waveformWidgetHolders.size(); i++)
        if( m_waveformWidgetHolders[i].m_waveformViewer == viewer)
            return i;
    return -1;
}
