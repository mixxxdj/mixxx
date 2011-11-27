
#include <QGLWidget>
#include <QDebug>
#include <QDomNode>
#include <QEvent>
#include <QDragEnterEvent>
#include <QUrl>
#include <QPainter>
#include <QFile>
#include <QGLContext>

#include "mixxx.h"
#include "trackinfoobject.h"
#include "wglwaveformviewer.h"
#include "waveform/waveformrenderer.h"
#include "controlobjectthreadmain.h"
#include "sharedglcontext.h"

WGLWaveformViewer::WGLWaveformViewer(
        const char *group,
        WaveformRenderer *pWaveformRenderer,
        QWidget * pParent,
        const QGLWidget * pShareWidget,
        QGLContext *ctxt,
        Qt::WFlags f
    ) :
    QGLWidget(ctxt, pParent, pShareWidget)
{
    m_pWaveformRenderer = pWaveformRenderer;
    Q_ASSERT(m_pWaveformRenderer);

    m_pGroup = group;

    m_pScratchEnable = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group, "scratch_position_enable")));
    m_pScratch = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group, "scratch_position")));
    m_pTrackSamples = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group, "track_samples")));
    m_pTrackSampleRate = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group, "track_samplerate")));
    m_pRate = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pGroup, "rate")));
    m_pRateRange = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pGroup, "rateRange")));
    m_pRateDir = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pGroup, "rate_dir")));
    m_pWaveformZoomFactor = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey("[Master]", "waveform_zoom_factor")));

    setAcceptDrops(true);

    installEventFilter(this);

    m_painting = false;

    setSizePolicy(QSizePolicy::MinimumExpanding,
                  QSizePolicy::MinimumExpanding);
}

bool WGLWaveformViewer::directRendering()
{
    return format().directRendering();
}


WGLWaveformViewer::~WGLWaveformViewer() {
    delete m_pScratchEnable;
    delete m_pScratch;
    delete m_pTrackSamples;
    delete m_pTrackSampleRate;
    delete m_pRate;
    delete m_pRateRange;
    delete m_pRateDir;
}

void WGLWaveformViewer::setup(QDomNode node) {
    int w = width(), h = height();
    m_pWaveformRenderer->setup(node);
    m_pWaveformRenderer->resize(w, h);
}

void WGLWaveformViewer::resizeEvent(QResizeEvent* e)
{
    const QSize newSize = e->size();
    m_pWaveformRenderer->resize(newSize.width(),
                                newSize.height());
}


void WGLWaveformViewer::paintEvent(QPaintEvent *event) {
    QPainter painter;
    painter.begin(this);

    painter.setRenderHint(QPainter::Antialiasing);
    //painter.setRenderHint(QPainter::TextAntialiasing);

    // HighQualityAntialiasing makes some CPUs go crazy
    //painter.setRenderHint(QPainter::HighQualityAntialiasing);

    m_pWaveformRenderer->draw(&painter, event);

    painter.end();
    m_painting = false;
    // QPainter goes out of scope and is destructed
}

void WGLWaveformViewer::refresh() {
    //m_paintMutex.lock();
    if(!m_painting) {
        m_painting = true;

        // The docs say update is better than repaint.
        update();
        //updateGL();
    }
    //m_paintMutex.unlock();
}

/** SLOTS **/

void WGLWaveformViewer::setValue(double) {
    // unused, stops a bad connect from happening
}

bool WGLWaveformViewer::eventFilter(QObject *o, QEvent *e) {
    QMouseEvent* m = dynamic_cast<QMouseEvent*>(e);
    if(e->type() == QEvent::MouseButtonPress) {
        m_iMouseStart = m->x();
        if(m->button() == Qt::LeftButton) {
            m_bScratching = true;
            m_pScratch->slotSet(0);
            m_pScratchEnable->slotSet(1.0f);

            // Set the cursor to a hand while the mouse is down.
            setCursor(Qt::ClosedHandCursor);
        }
        //reset zoom on right click
        else if (m->button() == Qt::RightButton) {
            m_pWaveformZoomFactor->slotSet(1.0);
        }
    } else if(e->type() == QEvent::MouseMove) {
        // Only send signals for mouse moving if the left button is pressed
        if (m_iMouseStart != -1 && m_bScratching) {
            int curX = m->x();

            // Adjusts for one-to-one movement. Track sample rate in hundreds of
            // samples times two is the number of samples per pixel.  rryan
            // 4/2011
            double samplesPerPixel = m_pTrackSampleRate->get() / 100.0 * 2 * m_pWaveformZoomFactor->get();

            // To take care of one one movement when zoom changes with pitch
            double rateAdjust = m_pRateDir->get() *
                    math_min(0.99, m_pRate->get() * m_pRateRange->get());
            double targetPosition = (m_iMouseStart - curX) *
                    samplesPerPixel * (1 + rateAdjust);
            //qDebug() << "Target:" << targetPosition;
            m_pScratch->slotSet(targetPosition);
        }
    } else if(e->type() == QEvent::MouseButtonRelease) {
        if (m_bScratching) {
            m_pScratchEnable->slotSet(0.0f);
            m_bScratching = false;

            // Set the cursor back to an arrow.
            setCursor(Qt::ArrowCursor);
        }
        m_iMouseStart = -1;
    } else if(e->type() == QEvent::Wheel) {
        QWheelEvent* w = dynamic_cast<QWheelEvent*>(e);
        if (w->orientation() == Qt::Vertical)
        {
            double new_zoom = m_pWaveformZoomFactor->get() - (m_pWaveformZoomFactor->get() * (float)w->delta()) / 1200.0;
            new_zoom = math_max(new_zoom, 0.05);
            new_zoom = math_min(new_zoom, 6.0);
            m_pWaveformZoomFactor->slotSet(new_zoom);
        }
    } else {
        return QObject::eventFilter(o,e);
    }
    return true;
}

/** DRAG AND DROP **/


void WGLWaveformViewer::dragEnterEvent(QDragEnterEvent * event)
{
    // Accept the enter event if the thing is a filepath and nothing's playing
    // in this deck.
    if (event->mimeData()->hasUrls()) {
        ControlObject *pPlayCO = ControlObject::getControl(
            ConfigKey(m_pGroup, "play"));
        if (pPlayCO && pPlayCO->get()) {
            event->ignore();
        } else {
            event->acceptProposedAction();
        }
    }
}

void WGLWaveformViewer::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url = urls.first();
        QString name = url.toLocalFile();
		//total OWEN hack: because we strip out the library prefix
		//in the view, we have to add it back here again to properly receive
		//drops
        if (!QFile(name).exists())
        {
        	if(QFile(m_sPrefix+"/"+name).exists())
        		name = m_sPrefix+"/"+name;
        }
        //If the file is on a network share, try just converting the URL to a string...
        if (name == "")
            name = url.toString();

        event->accept();
        emit(trackDropped(name, m_pGroup));
    } else {
        event->ignore();
    }
}

void WGLWaveformViewer::setLibraryPrefix(QString sPrefix)
{
	m_sPrefix = "";
	m_sPrefix = sPrefix;
	if (sPrefix[sPrefix.length()-1] == '/' || sPrefix[sPrefix.length()-1] == '\\')
		m_sPrefix.chop(1);
}
