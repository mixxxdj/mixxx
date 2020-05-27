#include "widget/wwaveformviewer.h"

#include <QDomNode>
#include <QDragEnterEvent>
#include <QEvent>
#include <QMimeData>
#include <QPainter>
#include <QUrl>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "track/track.h"
#include "util/dnd.h"
#include "util/math.h"
#include "util/widgethelper.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/waveformwidgetabstract.h"

WWaveformViewer::WWaveformViewer(const char* group, UserSettingsPointer pConfig, QWidget* parent)
        : WWidget(parent),
          m_pGroup(group),
          m_pConfig(pConfig),
          m_zoomZoneWidth(20),
          m_bScratching(false),
          m_bBending(false),
          m_pCueMenuPopup(make_parented<WCueMenuPopup>(pConfig, this)),
          m_waveformWidget(nullptr) {
    setMouseTracking(true);
    setAcceptDrops(true);
    m_pZoom = new ControlProxy(group, "waveform_zoom", this);
    m_pZoom->connectValueChanged(this, &WWaveformViewer::onZoomChange);

    m_pScratchPositionEnable = new ControlProxy(
            group, "scratch_position_enable", this);
    m_pScratchPosition = new ControlProxy(
            group, "scratch_position", this);
    m_pWheel = new ControlProxy(
            group, "wheel", this);
    m_pPlayEnabled = new ControlProxy(group, "play", this);

    setAttribute(Qt::WA_OpaquePaintEvent);
}

WWaveformViewer::~WWaveformViewer() {
    //qDebug() << "~WWaveformViewer";
}

void WWaveformViewer::setup(const QDomNode& node, const SkinContext& context) {
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
        if (m_bBending) {
            m_pWheel->setParameter(0.5);
            m_bBending = false;
        }
        m_bScratching = true;
        int eventPosValue = m_waveformWidget->getOrientation() == Qt::Horizontal ?
                    event->pos().x() : event->pos().y();
        double audioSamplePerPixel = m_waveformWidget->getAudioSamplePerPixel();
        double targetPosition = -1.0 * eventPosValue * audioSamplePerPixel * 2;
        m_pScratchPosition->set(targetPosition);
        m_pScratchPositionEnable->slotSet(1.0);
    } else if (event->button() == Qt::RightButton) {
        const auto currentTrack = m_waveformWidget->getTrackInfo();
        if (!isPlaying() && m_pHoveredMark) {
            auto cueAtClickPos = getCuePointerFromCueMark(m_pHoveredMark);
            if (cueAtClickPos) {
                m_pCueMenuPopup->setTrackAndCue(currentTrack, cueAtClickPos);
                QPoint cueMenuTopLeft = mixxx::widgethelper::mapPopupToScreen(
                        *this,
                        event->globalPos(),
                        m_pCueMenuPopup->size());
                m_pCueMenuPopup->popup(cueMenuTopLeft);
            }
        } else {
            // If we are scratching then disable and reset because the two shouldn't
            // be used at once.
            if (m_bScratching) {
                m_pScratchPositionEnable->slotSet(0.0);
                m_bScratching = false;
            }
            m_pWheel->setParameter(0.5);
            m_bBending = true;
        }
    }

    // Set the cursor to a hand while the mouse is down (when cue menu is not open).
    if (!m_pCueMenuPopup->isVisible()) {
        setCursor(Qt::ClosedHandCursor);
    }
}

void WWaveformViewer::mouseMoveEvent(QMouseEvent* event) {
    if (!m_waveformWidget) {
        return;
    }

    // Only send signals for mouse moving if the left button is pressed
    if (m_bScratching) {
        int eventPosValue = m_waveformWidget->getOrientation() == Qt::Horizontal ?
                    event->pos().x() : event->pos().y();
        // Adjusts for one-to-one movement.
        double audioSamplePerPixel = m_waveformWidget->getAudioSamplePerPixel();
        double targetPosition = -1.0 * eventPosValue * audioSamplePerPixel * 2;
        //qDebug() << "Target:" << targetPosition;
        m_pScratchPosition->set(targetPosition);
    } else if (m_bBending) {
        QPoint diff = event->pos() - m_mouseAnchor;
        int diffValue = m_waveformWidget->getOrientation() == Qt::Horizontal ?
                    diff.x() : diff.y();
        // Start at the middle of [0.0, 1.0], and emit values based on how far
        // the mouse has traveled horizontally. Note, for legacy (MIDI) reasons,
        // this is tuned to 127.
        // NOTE(rryan): This is basically a direct connection to the "wheel"
        // control since we manually connect it in LegacySkinParser regardless
        // of whether the skin specifies it. See ControlTTRotaryBehavior to see
        // where this value is handled.
        double v = 0.5 + (diffValue / 1270.0);
        // clamp to [0.0, 1.0]
        v = math_clamp(v, 0.0, 1.0);
        m_pWheel->setParameter(v);
    } else if (!isPlaying()) {
        WaveformMarkPointer pMark;
        pMark = m_waveformWidget->getCueMarkAtPoint(event->pos());
        if (pMark && getCuePointerFromCueMark(pMark)) {
            if (!m_pHoveredMark) {
                m_pHoveredMark = pMark;
                highlightMark(pMark);
            } else if (pMark != m_pHoveredMark) {
                unhighlightMark(m_pHoveredMark);
                m_pHoveredMark = pMark;
                highlightMark(pMark);
            }
        } else {
            if (m_pHoveredMark) {
                unhighlightMark(m_pHoveredMark);
                m_pHoveredMark = nullptr;
            }
        }
    }
}

void WWaveformViewer::mouseReleaseEvent(QMouseEvent* /*event*/) {
    if (m_bScratching) {
        m_pScratchPositionEnable->set(0.0);
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
        if (event->delta() > 0) {
            onZoomChange(m_waveformWidget->getZoomFactor() * 1.05);
        } else {
            onZoomChange(m_waveformWidget->getZoomFactor() / 1.05);
        }
    }
}

void WWaveformViewer::dragEnterEvent(QDragEnterEvent* event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_pGroup, m_pConfig);
}

void WWaveformViewer::dropEvent(QDropEvent* event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_pGroup, m_pConfig);
}

void WWaveformViewer::leaveEvent(QEvent*) {
    if (m_pHoveredMark) {
        unhighlightMark(m_pHoveredMark);
        m_pHoveredMark = nullptr;
    }
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

void WWaveformViewer::setZoom(double zoom) {
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

void WWaveformViewer::setDisplayBeatGridAlpha(int alpha) {
    m_waveformWidget->setDisplayBeatGridAlpha(alpha);
}

void WWaveformViewer::setPlayMarkerPosition(double position) {
    if (m_waveformWidget) {
        m_waveformWidget->setPlayMarkerPosition(position);
    }
}

void WWaveformViewer::setWaveformWidget(WaveformWidgetAbstract* waveformWidget) {
    if (m_waveformWidget) {
        QWidget* pWidget = m_waveformWidget->getWidget();
        disconnect(pWidget, &QWidget::destroyed, this, &WWaveformViewer::slotWidgetDead);
    }
    m_waveformWidget = waveformWidget;
    if (m_waveformWidget) {
        QWidget* pWidget = m_waveformWidget->getWidget();
        connect(pWidget, &QWidget::destroyed, this, &WWaveformViewer::slotWidgetDead);
        m_waveformWidget->getWidget()->setMouseTracking(true);
    }
}

CuePointer WWaveformViewer::getCuePointerFromCueMark(WaveformMarkPointer pMark) const {
    if (pMark && pMark->getHotCue() != Cue::kNoHotCue) {
        QList<CuePointer> cueList = m_waveformWidget->getTrackInfo()->getCuePoints();
        for (const auto& pCue : cueList) {
            if (pCue->getHotCue() == pMark->getHotCue()) {
                return pCue;
            }
        }
    }
    return CuePointer();
}

void WWaveformViewer::highlightMark(WaveformMarkPointer pMark) {
    QColor highlightColor = Color::chooseContrastColor(pMark->fillColor());
    pMark->setBaseColor(highlightColor);
}

void WWaveformViewer::unhighlightMark(WaveformMarkPointer pMark) {
    QColor originalColor = mixxx::RgbColor::toQColor(getCuePointerFromCueMark(pMark)->getColor());
    pMark->setBaseColor(originalColor);
}

bool WWaveformViewer::isPlaying() const {
    return m_pPlayEnabled->get();
}
