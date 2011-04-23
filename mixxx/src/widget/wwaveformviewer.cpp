
#include <QGLWidget>
#include <QDebug>
#include <QDomNode>
#include <QEvent>
#include <QDragEnterEvent>
#include <QUrl>
#include <QPainter>


#include "mixxx.h"
#include "trackinfoobject.h"
#include "wwaveformviewer.h"
#include "waveform/waveformrenderer.h"

#include "waveform/waveformwidget.h"
#include "waveform/waveformrenderbackground.h"
#include "waveform/waveformrendererfilteredsignal.h"

#include "waveform/glwaveformwidget.h"

WWaveformViewer::WWaveformViewer(const char *group, WaveformRenderer *pWaveformRenderer, QWidget * parent, Qt::WFlags f) :
    QWidget(parent)
{
    m_pWaveformRenderer = pWaveformRenderer;
    //Q_ASSERT(m_pWaveformRenderer);

    m_pGroup = group;

    setAcceptDrops(true);

    installEventFilter(this);
    m_painting = false;

    setAttribute(Qt::WA_OpaquePaintEvent,true);

    m_waveformWidgetRenderer = new WaveformWidgetRenderer( m_pGroup);

    //m_waveformWidgetRenderer->init();
    //m_waveformWidgetRenderer->addRenderer<WaveformRenderBackground>();
    //m_waveformWidgetRenderer->addRenderer<WaveformRendererFilteredSignal>();

    m_zoomZoneWidth = 20;

    m_waveformWidget = new GLWaveformWidget( m_pGroup, this);
    GLWaveformWidget* waveformWidget = (GLWaveformWidget*)m_waveformWidget;
    waveformWidget->getRenderer()->init();
}

WWaveformViewer::~WWaveformViewer() {
    delete m_waveformWidgetRenderer;
}

void WWaveformViewer::setup(QDomNode node) {
    GLWaveformWidget* waveformWidget = (GLWaveformWidget*)m_waveformWidget;
    waveformWidget->getRenderer()->setup( node);

    //m_waveformWidgetRenderer->setup( node);
    //m_waveformWidgetRenderer->resize( width(), height());
}

void WWaveformViewer::resizeEvent(QResizeEvent* /*event*/)
{
    m_waveformWidget->resize(size());
    //m_waveformWidgetRenderer->resize(width(),height());
    //QWidget::resizeEvent(event);
}
/*
void WWaveformViewer::paintEvent(QPaintEvent *event) {
 QPainter painter;
    painter.begin(this);
    //painter.setRenderHint(QPainter::Antialiasing);
    m_waveformWidgetRenderer->draw(&painter,event);
    painter.end();
    m_painting = false;
}
*/

void WWaveformViewer::refresh() {
    //m_paintMutex.lock();
    if(!m_painting) {
        m_painting = true;

        // The docs say update is better than repaint.
        update();
    }
    //m_paintMutex.unlock();
}

/** SLOTS **/

bool WWaveformViewer::eventFilter(QObject *o, QEvent *e) {
    if(e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *m = (QMouseEvent*)e;
        m_iMouseStart= -1;
        if(m->button() == Qt::LeftButton) {
            // The left button went down, so store the start position
            m_iMouseStart = m->x();
            emit(valueChangedLeftDown(64));
        }
    } else if(e->type() == QEvent::MouseMove) {
        // Only send signals for mouse moving if the left button is pressed
        if(m_iMouseStart != -1) {
            QMouseEvent *m = (QMouseEvent*)e;

            // start at the middle of 0-127, and emit values based on
            // how far the mouse has travelled horizontally
            double v = 64 + (double)(m->x()-m_iMouseStart)/10;
            // clamp to 0-127
            if(v<0)
                v = 0;
            else if(v > 127)
                v = 127;
            emit(valueChangedLeftDown(v));

        }
    } else if(e->type() == QEvent::MouseButtonRelease) {
        emit(valueChangedLeftDown(64));
    } else {
        return QObject::eventFilter(o,e);
    }
    return true;
}

void WWaveformViewer::wheelEvent(QWheelEvent *event)
{
    update();
    if( event->x() > width() - m_zoomZoneWidth)
    {
        GLWaveformWidget* waveformWidget = (GLWaveformWidget*)m_waveformWidget;
        if( event->delta() > 0)
            waveformWidget->getRenderer()->zoomIn();
        else
            waveformWidget->getRenderer()->zoomOut();
    }
}

/** DRAG AND DROP **/

void WWaveformViewer::dragEnterEvent(QDragEnterEvent * event)
{
    // Accept the enter event if the thing is a filepath.
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void WWaveformViewer::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url = urls.first();
        QString name = url.toLocalFile();
        //If the file is on a network share, try just converting the URL to a string...
        if (name == "")
            name = url.toString();

        event->accept();
        emit(trackDropped(name, m_pGroup));
    } else {
        event->ignore();
    }
}
