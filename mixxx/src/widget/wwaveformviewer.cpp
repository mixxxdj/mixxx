
#include <QDebug>
#include <QtXml/QDomNode>
#include <QEvent>
#include <QDragEnterEvent>
#include <QUrl>
#include <QPainter>

#include "mixxx.h"
#include "trackinfoobject.h"
#include "wwaveformviewer.h"

#include "waveform/widgets/waveformwidgetabstract.h"

WWaveformViewer::WWaveformViewer(const char *group, QWidget * parent, Qt::WFlags f) :
    QWidget(parent) {
    m_pGroup = group;

    setAcceptDrops(true);

    //installEventFilter(this);

    setAttribute(Qt::WA_ForceUpdatesDisabled);
    setAttribute(Qt::WA_OpaquePaintEvent);

    m_zoomZoneWidth = 20;
    m_waveformWidget = 0;
}

WWaveformViewer::~WWaveformViewer() {
}

void WWaveformViewer::setup(QDomNode node) {
    if( m_waveformWidget)
        m_waveformWidget->setup(node);
}

void WWaveformViewer::resizeEvent(QResizeEvent* /*event*/) {
    if( m_waveformWidget)
        m_waveformWidget->resize(width(),height());
}

void WWaveformViewer::mousePressEvent(QMouseEvent* event) {
    m_mouseAnchor = event->pos();
}

void WWaveformViewer::mouseMoveEvent(QMouseEvent* event) {

    if( event->buttons() & Qt::RightButton) {
    }

    if( event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - m_mouseAnchor;
        // start at the middle of 0-127, and emit values based on
        // how far the mouse has travelled horizontally
        double valueDiff = 64 + (double)(diff.x())/10;
        valueDiff = math_max( 0.0, math_min( 127.0, valueDiff));
        emit(valueChangedLeftDown(valueDiff));
    }
}

void WWaveformViewer::mouseReleaseEvent(QMouseEvent* event){
    if( !(event->buttons() & Qt::LeftButton)) {
        emit(valueChangedLeftDown(64));
    }
}

void WWaveformViewer::wheelEvent(QWheelEvent *event) {
    if( m_waveformWidget) {
        if( event->x() > width() - m_zoomZoneWidth) {
            if( event->delta() > 0)
                m_waveformWidget->zoomIn();
            else
                m_waveformWidget->zoomOut();
        }
        m_waveformWidget->render();
    }
}

/** DRAG AND DROP **/

void WWaveformViewer::dragEnterEvent(QDragEnterEvent * event) {
    // Accept the enter event if the thing is a filepath.
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void WWaveformViewer::dropEvent(QDropEvent * event) {
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

void WWaveformViewer::onTrackLoaded( TrackPointer track) {
    if( m_waveformWidget)
        m_waveformWidget->setTrack(track);
}

void WWaveformViewer::onTrackUnloaded( TrackPointer /*track*/) {
    if( m_waveformWidget)
        m_waveformWidget->setTrack( TrackPointer(0));
}
