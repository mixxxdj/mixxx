
#include <QtDebug>
#include <QDomNode>
#include <QEvent>
#include <QDragEnterEvent>
#include <QUrl>
#include <QPainter>
#include <QMimeData>

#include "controlobject.h"
#include "controlobjectslave.h"
#include "trackinfoobject.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wwaveformviewer.h"
#include "waveform/waveformwidgetfactory.h"
#include "util/dnd.h"
#include "util/math.h"

WWaveformViewer::WWaveformViewer(const char *group, ConfigObject<ConfigValue>* pConfig, QWidget * parent)
        : WWidget(parent),
          m_pGroup(group),
          m_pConfig(pConfig),
          m_zoomZoneWidth(20),
          m_bScratching(false),
          m_bBending(false),
          m_waveformWidget(NULL) {
    setAcceptDrops(true);

    m_pZoom = new ControlObjectSlave(group, "waveform_zoom");
    m_pZoom->connectValueChanged(this, SLOT(onZoomChange(double)));

    m_pScratchPositionEnable = new ControlObjectSlave(
            group, "scratch_position_enable");
    m_pScratchPosition = new ControlObjectSlave(
            group, "scratch_position");
    m_pWheel = new ControlObjectSlave(
            group, "wheel");

    setAttribute(Qt::WA_OpaquePaintEvent);
}

WWaveformViewer::~WWaveformViewer() {
    //qDebug() << "~WWaveformViewer";

    delete m_pZoom;
    delete m_pScratchPositionEnable;
    delete m_pScratchPosition;
    delete m_pWheel;
}

void WWaveformViewer::setup(QDomNode node, const SkinContext& context) {
    Q_UNUSED(context);
    if (m_waveformWidget) {
        m_waveformWidget->setup(node, context);
    }
}

void WWaveformViewer::resizeEvent(QResizeEvent* /*event*/) {
    if (m_waveformWidget) {
        m_waveformWidget->resize(width(), height());
    }
}

void WWaveformViewer::mousePressEvent(QMouseEvent* event) {
    m_mouseAnchor = event->pos();

    if (event->button() == Qt::LeftButton && m_waveformWidget) {
        // If we are pitch-bending then disable and reset because the two
        // shouldn't be used at once.
        if (m_bBending) {
            m_pWheel->setParameter(0.5);
            m_bBending = false;
        }
        m_bScratching = true;
        double audioSamplePerPixel = m_waveformWidget->getAudioSamplePerPixel();
        double targetPosition = -1.0 * event->pos().x() * audioSamplePerPixel * 2;
        m_pScratchPosition->slotSet(targetPosition);
        m_pScratchPositionEnable->slotSet(1.0);
    } else if (event->button() == Qt::RightButton) {
        // If we are scratching then disable and reset because the two shouldn't
        // be used at once.
        if (m_bScratching) {
            m_pScratchPositionEnable->slotSet(0.0);
            m_bScratching = false;
        }
        m_pWheel->setParameter(0.5);
        m_bBending = true;
    }

    // Set the cursor to a hand while the mouse is down.
    setCursor(Qt::ClosedHandCursor);
}

void WWaveformViewer::mouseMoveEvent(QMouseEvent* event) {
    // Only send signals for mouse moving if the left button is pressed
    if (m_bScratching && m_waveformWidget) {
        // Adjusts for one-to-one movement.
        double audioSamplePerPixel = m_waveformWidget->getAudioSamplePerPixel();
        double targetPosition = -1.0 * event->pos().x() * audioSamplePerPixel * 2;
        //qDebug() << "Target:" << targetPosition;
        m_pScratchPosition->slotSet(targetPosition);
    } else if (m_bBending) {
        QPoint diff = event->pos() - m_mouseAnchor;
        // Start at the middle of [0.0, 1.0], and emit values based on how far
        // the mouse has traveled horizontally. Note, for legacy (MIDI) reasons,
        // this is tuned to 127.
        // NOTE(rryan): This is basically a direct connection to the "wheel"
        // control since we manually connect it in LegacySkinParser regardless
        // of whether the skin specifies it. See ControlTTRotaryBehavior to see
        // where this value is handled.
        double v = 0.5 + (diff.x() / 1270.0);
        // clamp to [0.0, 1.0]
        v = math_clamp(v, 0.0, 1.0);
        m_pWheel->setParameter(v);
    }
}

void WWaveformViewer::mouseReleaseEvent(QMouseEvent* /*event*/) {
    if (m_bScratching) {
        m_pScratchPositionEnable->slotSet(0.0);
        m_bScratching = false;
    }
    if (m_bBending) {
        m_pWheel->setParameter(0.5);
        m_bBending = false;
    }
    m_mouseAnchor = QPoint();

    // Set the cursor back to an arrow.
    setCursor(Qt::ArrowCursor);
}

void WWaveformViewer::wheelEvent(QWheelEvent *event) {
    if (m_waveformWidget) {
        //NOTE: (vrince) to limit the zoom action area uncomment the following line
        //if (event->x() > width() - m_zoomZoneWidth) {
            if (event->delta() > 0) {
                //qDebug() << "WaveformWidgetRenderer::wheelEvent +1";
                onZoomChange(m_waveformWidget->getZoomFactor() + 1);
            } else {
                //qDebug() << "WaveformWidgetRenderer::wheelEvent -1";
                onZoomChange(m_waveformWidget->getZoomFactor() - 1);
            }
        //}
    }
}

void WWaveformViewer::dragEnterEvent(QDragEnterEvent* event) {
    if (DragAndDropHelper::allowLoadToPlayer(m_pGroup, m_pConfig) &&
            DragAndDropHelper::dragEnterAccept(*event->mimeData(), m_pGroup,
                                               true, false)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void WWaveformViewer::dropEvent(QDropEvent* event) {
    if (DragAndDropHelper::allowLoadToPlayer(m_pGroup, m_pConfig)) {
        QList<QFileInfo> files = DragAndDropHelper::dropEventFiles(
                *event->mimeData(), m_pGroup, true, false);
        if (!files.isEmpty()) {
            event->accept();
            emit(trackDropped(files.at(0).canonicalFilePath(), m_pGroup));
            return;
        }
    }
    event->ignore();
}

void WWaveformViewer::onTrackLoaded(TrackPointer track) {
    if (m_waveformWidget) {
        m_waveformWidget->setTrack(track);
    }
}

void WWaveformViewer::onTrackUnloaded(TrackPointer /*track*/) {
    if (m_waveformWidget) {
        m_waveformWidget->setTrack(TrackPointer());
    }
}

void WWaveformViewer::onZoomChange(double zoom) {
    //qDebug() << "WaveformWidgetRenderer::onZoomChange" << this << zoom;
    setZoom(zoom);
    // notify back the factory to sync zoom if needed
    WaveformWidgetFactory::instance()->notifyZoomChange(this);
}

void WWaveformViewer::setZoom(int zoom) {
    //qDebug() << "WaveformWidgetRenderer::setZoom" << zoom;
    if (m_waveformWidget) {
        m_waveformWidget->setZoom(zoom);
    }

    // If multiple waveform widgets for the same group are created then it's
    // possible that this setZoom() is coming from another waveform with the
    // same group. That means that if we set the zoom control here, that
    // waveform will receive the update as a call to onZoomChange which will in
    // turn notify the WaveformWidgetFactory that zoom changed which will
    // infinite loop because we will receive another setZoom() from
    // WaveformWidgetFactory. To prevent this recursion, check for no-ops.
    if (m_pZoom->get() != zoom) {
        m_pZoom->set(zoom);
    }
}

void WWaveformViewer::setWaveformWidget(WaveformWidgetAbstract* waveformWidget) {
    if (m_waveformWidget) {
        QWidget* pWidget = m_waveformWidget->getWidget();
        disconnect(pWidget, SIGNAL(destroyed()),
                   this, SLOT(slotWidgetDead()));
    }
    m_waveformWidget = waveformWidget;
    if (m_waveformWidget) {
        QWidget* pWidget = m_waveformWidget->getWidget();
        connect(pWidget, SIGNAL(destroyed()),
                this, SLOT(slotWidgetDead()));
    }
}
