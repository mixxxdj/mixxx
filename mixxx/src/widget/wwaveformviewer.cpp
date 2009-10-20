
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


WWaveformViewer::WWaveformViewer(const char *group, WaveformRenderer *pWaveformRenderer, QWidget * parent, Qt::WFlags f) : QWidget(parent, f)
{

    m_pWaveformRenderer = pWaveformRenderer;
    Q_ASSERT(m_pWaveformRenderer);

    m_pGroup = group;
    
    setAcceptDrops(true);

    installEventFilter(this);

    // Start a timer based on our desired FPS
    // TODO Eventually make this user-configurable.
    int desired_fps = 1;
    int update_interval = 1000 / desired_fps;
    m_iTimerID = startTimer(update_interval);
    
    m_painting = false;
}

WWaveformViewer::~WWaveformViewer() {
    // Stop the timer we started
    killTimer(m_iTimerID);
}

void WWaveformViewer::setup(QDomNode node) {
    qDebug() << "Waveform viewer setup";
    // Acquire position
    QString pos = WWidget::selectNodeQString(node, "Pos");
    int sep = pos.indexOf(",");
    int x = pos.left(sep).toInt();
    int y = pos.mid(sep+1).toInt();
    
    move(x,y);

    // Acquire size
    QString size = WWidget::selectNodeQString(node, "Size");
    sep = size.indexOf(",");
    x = size.left(sep).toInt();
    y = size.mid(sep+1).toInt();

    setFixedSize(x,y);

    m_pWaveformRenderer->resize(x,y);

    m_pWaveformRenderer->setup(node);
}

void WWaveformViewer::paintEvent(QPaintEvent *event) {
    QPainter painter;
    painter.begin(this);
    
    painter.setRenderHint(QPainter::Antialiasing);

    m_pWaveformRenderer->draw(&painter, event);
    
    painter.end();
    m_painting = false;
    // QPainter goes out of scope and is destructed
}

void WWaveformViewer::timerEvent(QTimerEvent *qte) {
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

        event->accept();
        emit(trackDropped(name));
    } else {
        event->ignore();
    }
}
