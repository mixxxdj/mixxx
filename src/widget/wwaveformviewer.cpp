
#include <QtDebug>
#include <QDomNode>
#include <QEvent>
#include <QDragEnterEvent>
#include <QUrl>
#include <QPainter>
#include <QMimeData>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "track/track.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wwaveformviewer.h"
#include "waveform/waveformwidgetfactory.h"
#include "util/dnd.h"
#include "util/math.h"

WWaveformViewer::WWaveformViewer(const char *group, UserSettingsPointer pConfig, QWidget * parent)
        : WWidget(parent),
          m_pGroup(group),
          m_pConfig(pConfig),
          m_zoomZoneWidth(20),
          m_interactionMode(InteractionMode::NONE),
          m_waveformWidget(nullptr) {
    setAcceptDrops(true);

    m_pZoom = new ControlProxy(group, "waveform_zoom", this);
    m_pZoom->connectValueChanged(SLOT(onZoomChange(double)));

    m_pScratchPositionEnable = new ControlProxy(
            group, "scratch_position_enable", this);
    m_pScratchPosition = new ControlProxy(
            group, "scratch_position", this);
    m_pWheel = new ControlProxy(
            group, "wheel", this);

    setAttribute(Qt::WA_OpaquePaintEvent);
}

WWaveformViewer::~WWaveformViewer() {
    //qDebug() << "~WWaveformViewer";
}

void WWaveformViewer::setup(const QDomNode& node, const SkinContext& context) {
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
    if (!m_waveformWidget) {
        return;
    }

    m_mouseAnchor = event->pos();

    if (event->button() == Qt::LeftButton) {
        // If we are pitch-bending then disable and reset because the two
        // shouldn't be used at once.
        if (m_interactionMode == InteractionMode::BENDING) {
            m_pWheel->setParameter(0.5);
        }
        m_interactionMode = InteractionMode::SCRATCHING;
        m_pScratchPosition->set(0.0);
        m_pScratchPositionEnable->slotSet(1.0);
    } else if (event->button() == Qt::RightButton) {
        // If we are scratching then disable and reset because the two shouldn't
        // be used at once.
        if (m_interactionMode == SCRATCHING) {
            m_pScratchPositionEnable->slotSet(0.0);
        }
        m_interactionMode = BENDING;
        m_pWheel->setParameter(0.5);
    }

    // Set the cursor to a hand while the mouse is down.
    setCursor(Qt::ClosedHandCursor);
}

void WWaveformViewer::mouseMoveEvent(QMouseEvent* event) {
    if (!m_waveformWidget) {
        return;
    }

    Qt::Orientation orientation = m_waveformWidget->getOrientation();
    QPoint delta = event->pos() - m_mouseAnchor;
    int deltaValue = orientation == Qt::Horizontal ? delta.x() : delta.y();

    // Only send signals for mouse moving if the left button is pressed
    if (m_interactionMode == InteractionMode::SCRATCHING) {
        // Adjusts for one-to-one movement.
        double audioSamplePerPixel = m_waveformWidget->getAudioSamplePerPixel();
        double targetPosition = -1.0 * deltaValue * audioSamplePerPixel * 2;
        //qDebug() << "Target:" << targetPosition;
        m_pScratchPosition->set(targetPosition);
    } else if (m_interactionMode == InteractionMode::BENDING) {
        // Start at the middle of [0.0, 1.0], and emit values based on how far
        // the mouse has traveled horizontally. Note, for legacy (MIDI) reasons,
        // this is tuned to 127.
        // NOTE(rryan): This is basically a direct connection to the "wheel"
        // control since we manually connect it in LegacySkinParser regardless
        // of whether the skin specifies it. See ControlTTRotaryBehavior to see
        // where this value is handled.
        double v = 0.5 + (deltaValue / 1270.0);
        // clamp to [0.0, 1.0]
        v = math_clamp(v, 0.0, 1.0);
        m_pWheel->setParameter(v);
    }

    // Wrap cursor if outside of this window's bounds
    QPoint cursorDelta(0, 0);
    if (orientation == Qt::Horizontal) {
        if (event->x() < 0) {
            cursorDelta.setX(width());
        } else if (event->x() > width()) {
            cursorDelta.setX(-width());
        }
    } else if (orientation == Qt::Vertical) {
        if (event->y() < 0) {
            cursorDelta.setY(height());
        } else if (event->y() > height()) {
            cursorDelta.setY(-height());
        }
    }
    if (!cursorDelta.isNull()) {
        m_mouseAnchor += cursorDelta;
        QCursor::setPos(event->globalPos() + cursorDelta);
    }
}

void WWaveformViewer::mouseReleaseEvent(QMouseEvent* /*event*/) {
    if (m_interactionMode == InteractionMode::SCRATCHING) {
        m_pScratchPositionEnable->set(0.0);
    } else if (m_interactionMode == InteractionMode::BENDING) {
        m_pWheel->setParameter(0.5);
    }
    m_interactionMode = InteractionMode::NONE;
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
            emit(trackDropped(files.at(0).absoluteFilePath(), m_pGroup));
            return;
        }
    }
    event->ignore();
}

void WWaveformViewer::slotTrackLoaded(TrackPointer track) {
    if (m_waveformWidget) {
        m_waveformWidget->setTrack(track);
    }
}

void WWaveformViewer::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
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
