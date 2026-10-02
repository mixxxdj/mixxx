#include "widget/wwaveformviewer.h"

#include <QDragEnterEvent>
#include <QEvent>

#include "control/controlproxy.h"
#include "moc_wwaveformviewer.cpp"
#include "util/dnd.h"
#include "util/math.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wcuemenupopup.h"
#include "widget/wglwidget.h"

WWaveformViewer::WWaveformViewer(
        const QString& group,
        UserSettingsPointer pConfig,
        QWidget* parent)
        : WWidget(parent),
          m_group(group),
          m_pConfig(pConfig),
          m_zoomZoneWidth(20),
          m_bScratching(false),
          m_bBending(false),
          m_hotcueDragging(false),
          m_draggedHotcue(Cue::kNoHotCue),
          m_pCueMenuPopup(make_parented<WCueMenuPopup>(pConfig, this)),
          m_waveformWidget(nullptr) {
    setMouseTracking(true);
    setAcceptDrops(true);
    m_pZoom = new ControlProxy(group, "waveform_zoom", this, ControlFlag::NoAssertIfMissing);
    m_pZoom->connectValueChanged(this, &WWaveformViewer::onZoomChange);

    m_pScratchPositionEnable = new ControlProxy(
            group, "scratch_position_enable", this, ControlFlag::NoAssertIfMissing);
    m_pScratchPosition = new ControlProxy(
            group, "scratch_position", this, ControlFlag::NoAssertIfMissing);
    m_pWheel = new ControlProxy(
            group, "wheel", this, ControlFlag::NoAssertIfMissing);
    m_pPlayEnabled = new ControlProxy(group, "play", this, ControlFlag::NoAssertIfMissing);
    m_pPassthroughEnabled = make_parented<ControlProxy>(group, "passthrough", this);
    m_pPassthroughEnabled->connectValueChanged(this, &WWaveformViewer::passthroughChanged);

    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::NoFocus);
}

WWaveformViewer::~WWaveformViewer() {
    //qDebug() << "~WWaveformViewer";
}

void WWaveformViewer::setup(const QDomNode& node, const SkinContext& context) {
    if (m_waveformWidget) {
        m_waveformWidget->setup(node, context);
        m_dimBrightThreshold = m_waveformWidget->getDimBrightThreshold();
    }
}

void WWaveformViewer::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
    if (m_waveformWidget) {
        // Note m_waveformWidget is a WaveformWidgetAbstract,
        // so this calls the method of WaveformWidgetAbstract,
        // note of the derived waveform widgets which are also
        // a QWidget, though that will be called directly.
        m_waveformWidget->resize(width(), height());
    }
}

void WWaveformViewer::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
    if (m_waveformWidget) {
        // We leave it up to Qt to set the size of the derived
        // waveform widget, but we still need to set the size
        // of the renderer.
        m_waveformWidget->resizeRenderer(
                width(), height(), static_cast<float>(devicePixelRatioF()));
    }
}

void WWaveformViewer::mousePressEvent(QMouseEvent* event) {
    if (!m_waveformWidget || m_waveformWidget->getType() == WaveformWidgetType::Empty) {
        return;
    }
    const auto pCurrentTrack = m_waveformWidget->getTrackInfo();
    if (!pCurrentTrack) {
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
        // If we're clicking a hotcue mark -> start hotcue dragging
        if (!isPlaying() && m_pHoveredMark) {
            auto pCueAtClickPos = getCuePointerFromCueMark(m_pHoveredMark);
            if (pCueAtClickPos) {
                // Unhighlight mark so can darg the original cue mark image
                unhighlightMark(m_pHoveredMark);
                // Before we enable hotcue drag here and in WaveformWidgetRenderer
                // (and subsequently in allshader::WaveformRenderMark and
                // legacy WaveformRenderMark) we need to set the start position.
                //
                // We calculate the offset between pointer event ad cue position
                // so set the sample (target) position on every move and the dragged
                // mark can be rendered correctly.
                // We also store the initial event pos to avoid that a single-/
                // double-click shifts the cue due to random rounding offsets.
                const double cuePos = m_waveformWidget->transformSamplePositionInRendererWorld(
                        m_pHoveredMark->getSamplePosition());
                const int eventPos = m_waveformWidget->getOrientation() == Qt::Horizontal
                        ? m_mouseAnchor.x()
                        : m_mouseAnchor.y();
                m_hotcueDragMouseOffset = cuePos - eventPos;
                m_waveformWidget->setHotcueDragPos(eventPos + m_hotcueDragMouseOffset);
                m_hotcueDragging = true;
                m_draggedHotcue = pCueAtClickPos->getHotCue();
                m_waveformWidget->setHotcueDragIndex(m_draggedHotcue);
                m_waveformWidget->setHotcueDragInProgress(true);
            }
        } else {
            m_bScratching = true;
            int eventPosValue =
                    m_waveformWidget->getOrientation() == Qt::Horizontal
                    ? m_mouseAnchor.x()
                    : m_mouseAnchor.y();
            double audioSamplePerPixel = m_waveformWidget->getAudioSamplePerPixel();
            double targetPosition = -1.0 * eventPosValue * audioSamplePerPixel * 2;
            m_pScratchPosition->set(targetPosition);
            m_pScratchPositionEnable->set(1.0);
        }
    } else if (event->button() == Qt::RightButton) {
        if (!isPlaying() && m_pHoveredMark) {
            auto cueAtClickPos = getCuePointerFromCueMark(m_pHoveredMark);
            if (cueAtClickPos) {
                m_pCueMenuPopup->setTrackCueGroup(pCurrentTrack, cueAtClickPos, m_group);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                m_pCueMenuPopup->popup(event->globalPosition().toPoint());
#else
                m_pCueMenuPopup->popup(event->globalPos());
#endif
            }
        } else {
            // If we are scratching then disable and reset because the two shouldn't
            // be used at once.
            if (m_bScratching) {
                m_pScratchPositionEnable->set(0.0);
                m_bScratching = false;
            }
            if (m_hotcueDragging) {
                stopHotcueDragging();
            }
            m_pWheel->setParameter(0.5);
            m_bBending = true;
        }
    }

    // Set the cursor to a hand while the mouse is down (when cue menu is not open).
    if (!m_pCueMenuPopup->isVisible() && !m_hotcueDragging) {
        setCursor(Qt::ClosedHandCursor);
    }
}

void WWaveformViewer::mouseMoveEvent(QMouseEvent* event) {
    if (!m_waveformWidget || m_waveformWidget->getType() == WaveformWidgetType::Empty) {
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
    } else if (m_hotcueDragging) {
        if (isPlaying()) {
            // Abort hotcue dragging if play was toggled since drag start
            stopHotcueDragging();
            return;
        }
        // Get cursor pos, apply offset and send pos to widget
        const int eventPos = m_waveformWidget->getOrientation() == Qt::Horizontal
                ? event->pos().x()
                : event->pos().y();
        m_waveformWidget->setHotcueDragPos(eventPos + m_hotcueDragMouseOffset);
    } else {
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

void WWaveformViewer::mouseReleaseEvent(QMouseEvent* pEvent) {
    if (m_bScratching) {
        m_pScratchPositionEnable->set(0.0);
        m_bScratching = false;
    }
    if (m_bBending) {
        m_pWheel->setParameter(0.5);
        m_bBending = false;
    }
    if (m_hotcueDragging) {
        stopHotcueDragging();

        auto pHotcue = m_waveformWidget->getCuePointerFromIndex(m_draggedHotcue);
        if (!pHotcue) {
            return;
        }

        // Ignore no-op
        if (pEvent->pos() != m_mouseAnchor) {
            QPoint offset;
            if (m_waveformWidget->getOrientation() == Qt::Horizontal) {
                offset.setX(std::lround(m_hotcueDragMouseOffset));
            } else {
                offset.setY(std::lround(m_hotcueDragMouseOffset));
            }
            double newSamPos =
                    m_waveformWidget->transformRendererPositionToSamplePosition(
                            pEvent->pos() + offset);
            auto newCuePos = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(newSamPos);
            pHotcue->setStartPosition(newCuePos);
        }
    }
    m_mouseAnchor = QPoint();

    // Set the cursor back to an arrow.
    setCursor(Qt::ArrowCursor);
}

void WWaveformViewer::wheelEvent(QWheelEvent* event) {
    if (m_waveformWidget) {
        if (event->angleDelta().y() > 0) {
            onZoomChange(m_waveformWidget->getZoom() / 1.05);
        } else if (event->angleDelta().y() < 0) {
            onZoomChange(m_waveformWidget->getZoom() * 1.05);
        }
    }
}

void WWaveformViewer::dragEnterEvent(QDragEnterEvent* pEvent) {
    DragAndDropHelper::handleTrackDragEnterEvent(pEvent, m_group, m_pConfig);
}

void WWaveformViewer::dropEvent(QDropEvent* pEvent) {
    DragAndDropHelper::handleTrackDropEvent(pEvent, *this, m_group, m_pConfig);
}

bool WWaveformViewer::handleDragAndDropEventFromWindow(QEvent* pEvent) {
    return event(pEvent);
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

#ifdef __STEM__
void WWaveformViewer::slotSelectStem(mixxx::StemChannelSelection stemMask) {
    if (m_waveformWidget) {
        m_waveformWidget->selectStem(stemMask);
        update();
    }
}
#endif

void WWaveformViewer::slotTrackUnloaded(TrackPointer pOldTrack) {
    slotLoadingTrack(pOldTrack, TrackPointer());
}

void WWaveformViewer::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_waveformWidget) {
        m_waveformWidget->setTrack(TrackPointer());
    }
}

void WWaveformViewer::onZoomChange(double zoom) {
    // qDebug() << "WaveformWidgetRenderer::onZoomChange" << this << zoom;
    setZoom(zoom);
    // notify back the factory to sync zoom if needed
    WaveformWidgetFactory::instance()->notifyZoomChange(this);
}

void WWaveformViewer::setZoom(double zoom) {
    // qDebug() << "WaveformWidgetRenderer::setZoom" << zoom;
    // Stop hotcue dragging since zooming changes the sample/renderworld relation
    // the drag pos propagation is based on.
    stopHotcueDragging();
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
    if (m_waveformWidget) {
        m_waveformWidget->setDisplayBeatGridAlpha(alpha);
    }
}

void WWaveformViewer::setPlayMarkerPosition(double position) {
    if (m_waveformWidget) {
        m_waveformWidget->setPlayMarkerPosition(position);
    }
}

void WWaveformViewer::setWaveformWidget(WaveformWidgetAbstract* waveformWidget) {
    if (m_waveformWidget) {
        QWidget* pWidget = m_waveformWidget->getWidget();
        disconnect(pWidget);
    }
    m_waveformWidget = waveformWidget;
    if (m_waveformWidget) {
        QWidget* pWidget = m_waveformWidget->getWidget();
        DEBUG_ASSERT(pWidget);
        connect(pWidget,
                &QWidget::destroyed,
                this,
                [this]() {
                    // The pointer must be considered as dangling!
                    m_waveformWidget = nullptr;
                });
        m_waveformWidget->getWidget()->setMouseTracking(true);
#ifdef MIXXX_USE_QOPENGL
        if (m_waveformWidget->getGLWidget()) {
            // The OpenGLWindow used to display the waveform widget interferes with the
            // normal Qt tooltip mechanism and uses it's own mechanism. We set the tooltip
            // of the waveform widget to the tooltip of its parent WWaveformViewer so the
            // OpenGLWindow will display it.
            m_waveformWidget->getGLWidget()->setToolTip(toolTip());

            // Tell the WGLWidget that this is its drag&drop target
            m_waveformWidget->getGLWidget()->setTrackDropTarget(this);
        }
#endif
        // Make connection to show "Passthrough" label on the waveform, except for
        // "Empty" waveform type
        if (m_waveformWidget->getType() == WaveformWidgetType::Empty) {
            return;
        }
        connect(this,
                &WWaveformViewer::passthroughChanged,
                this,
                [this](double value) {
                    m_waveformWidget->setPassThroughEnabled(value > 0);
                });
        // Make sure the label is shown after the waveform type was changed
        emit passthroughChanged(m_pPassthroughEnabled->toBool());
    }
}

CuePointer WWaveformViewer::getCuePointerFromCueMark(WaveformMarkPointer pMark) const {
    if (m_waveformWidget && pMark) {
        return m_waveformWidget->getCuePointerFromIndex(pMark->getHotCue());
    }
    return {};
}

void WWaveformViewer::highlightMark(WaveformMarkPointer pMark) {
    QColor highlightColor = Color::chooseContrastColor(pMark->fillColor(),
            m_dimBrightThreshold);
    pMark->setBaseColor(highlightColor, m_dimBrightThreshold);
}

void WWaveformViewer::unhighlightMark(WaveformMarkPointer pMark) {
    auto pCue = getCuePointerFromCueMark(pMark);
    if (pCue) {
        QColor originalColor = mixxx::RgbColor::toQColor(pCue->getColor());
        pMark->setBaseColor(originalColor, m_dimBrightThreshold);
    }
}

void WWaveformViewer::stopHotcueDragging() {
    m_hotcueDragging = false;
    m_waveformWidget->setHotcueDragInProgress(false);
    m_waveformWidget->setHotcueDragIndex(Cue::kNoHotCue);
    // Both after Track::cuesUpdated() triggered a renderer update and
    // when this was a no-op the mark is now be underneath the cursor.
    // FIXME Doesn't work after shifting the cue. Though hover is not required
    // to drag it again.
    if (m_pHoveredMark) {
        highlightMark(m_pHoveredMark);
    }
}

bool WWaveformViewer::isPlaying() const {
    return m_pPlayEnabled->toBool();
}
