#include "woverview.h"

#include <QBrush>
#include <QColor>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>

#include "analyzer/analyzerprogress.h"
#include "control/controlproxy.h"
#include "engine/engine.h"
#include "mixer/playermanager.h"
#include "moc_woverview.cpp"
#include "preferences/colorpalettesettings.h"
#include "track/track.h"
#include "util/colorcomponents.h"
#include "util/dnd.h"
#include "util/duration.h"
#include "util/math.h"
#include "util/painterscope.h"
#include "util/timer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/controlwidgetconnection.h"
#include "wskincolor.h"

namespace {
// Horizontal and vertical margin around the widget where we accept play pos dragging.
constexpr int kDragOutsideLimitX = 100;
constexpr int kDragOutsideLimitY = 50;
} // anonymous namespace

WOverview::WOverview(
        const QString& group,
        PlayerManager* pPlayerManager,
        UserSettingsPointer pConfig,
        QWidget* parent)
        : WWidget(parent),
          m_group(group),
          m_pConfig(pConfig),
          m_type(Type::RGB),
          m_actualCompletion(0),
          m_pixmapDone(false),
          m_waveformPeak(-1.0),
          m_diffGain(0),
          m_devicePixelRatio(1.0),
          m_endOfTrack(false),
          m_bPassthroughEnabled(false),
          m_pCueMenuPopup(make_parented<WCueMenuPopup>(pConfig, this)),
          m_bShowCueTimes(true),
          m_iPosSeconds(0),
          m_bLeftClickDragging(false),
          m_iPickupPos(0),
          m_iPlayPos(0),
          m_bTimeRulerActive(false),
          m_orientation(Qt::Horizontal),
          m_dragMarginH(kDragOutsideLimitX),
          m_dragMarginV(kDragOutsideLimitY),
          m_iLabelFontSize(10),
          m_maxPixelPos(1.0),
          m_analyzerProgress(kAnalyzerProgressUnknown),
          m_trackLoaded(false),
          m_pHoveredMark(nullptr),
          m_scaleFactor(1.0),
          m_trackSampleRateControl(
                  m_group,
                  QStringLiteral("track_samplerate")),
          m_trackSamplesControl(
                  m_group,
                  QStringLiteral("track_samples")),
          m_playpositionControl(
                  m_group,
                  QStringLiteral("playposition")) {
    m_endOfTrackControl = make_parented<ControlProxy>(
            m_group, QStringLiteral("end_of_track"), this, ControlFlag::NoAssertIfMissing);
    m_endOfTrackControl->connectValueChanged(this, &WOverview::onEndOfTrackChange);
    m_pRateRatioControl = make_parented<ControlProxy>(
            m_group, QStringLiteral("rate_ratio"), this, ControlFlag::NoAssertIfMissing);
    // Needed to recalculate range durations when rate slider is moved without the deck playing
    m_pRateRatioControl->connectValueChanged(
            this, &WOverview::onRateRatioChange);
    m_pPassthroughControl = make_parented<ControlProxy>(
            m_group, QStringLiteral("passthrough"), this, ControlFlag::NoAssertIfMissing);
    m_pPassthroughControl->connectValueChanged(this, &WOverview::onPassthroughChange);
    m_bPassthroughEnabled = m_pPassthroughControl->toBool();

    m_pTypeControl = make_parented<ControlProxy>(
            QStringLiteral("[Waveform]"),
            QStringLiteral("WaveformOverviewType"),
            this);
    m_pTypeControl->connectValueChanged(this, &WOverview::slotTypeControlChanged);
    slotTypeControlChanged(m_pTypeControl->get());

    m_pMinuteMarkersControl = make_parented<ControlProxy>(
            QStringLiteral("[Waveform]"),
            QStringLiteral("draw_overview_minute_markers"),
            this);
    m_pMinuteMarkersControl->connectValueChanged(this, &WOverview::slotMinuteMarkersChanged);
    slotMinuteMarkersChanged(static_cast<bool>(m_pMinuteMarkersControl->get()));

    // Update immediately when the normalize option or the visual gain have been
    // changed in the preferences.
    WaveformWidgetFactory* pWidgetFactory = WaveformWidgetFactory::instance();
    connect(pWidgetFactory,
            &WaveformWidgetFactory::overviewNormalizeChanged,
            this,
            &WOverview::slotNormalizeOrVisualGainChanged);
    connect(pWidgetFactory,
            &WaveformWidgetFactory::overallVisualGainChanged,
            this,
            &WOverview::slotNormalizeOrVisualGainChanged);
    // Also listen to ReplayGain changes to scale the waveform
    m_pReplayGain = make_parented<ControlProxy>(m_group, "replaygain", this);
    m_pReplayGain->connectValueChanged(this, &WOverview::slotNormalizeOrVisualGainChanged);

    m_pPassthroughLabel = make_parented<QLabel>(this);

    setAcceptDrops(true);

    setMouseTracking(true);

    connect(pPlayerManager, &PlayerManager::trackAnalyzerProgress,
            this, &WOverview::onTrackAnalyzerProgress);

    connect(m_pCueMenuPopup.get(), &WCueMenuPopup::aboutToHide, this, &WOverview::slotCueMenuPopupAboutToHide);
}

void WOverview::setup(const QDomNode& node, const SkinContext& context) {
    m_scaleFactor = context.getScaleFactor();
    m_signalColors.setup(node, context);

    m_backgroundColor = m_signalColors.getBgColor();
    m_axesColor = m_signalColors.getAxesColor();
    m_playPosColor = m_signalColors.getPlayPosColor();
    m_passthroughOverlayColor = m_signalColors.getPassthroughOverlayColor();
    m_playedOverlayColor = m_signalColors.getPlayedOverlayColor();
    m_lowColor = m_signalColors.getLowColor();
    m_dimBrightThreshold = m_signalColors.getDimBrightThreshold();

    m_labelBackgroundColor = context.selectColor(node, "LabelBackgroundColor");
    if (!m_labelBackgroundColor.isValid()) {
        m_labelBackgroundColor = m_backgroundColor;
        m_labelBackgroundColor.setAlpha(255 / 2); // 0 == fully transparent
    }

    m_labelTextColor = context.selectColor(node, "LabelTextColor");
    if (!m_labelTextColor.isValid()) {
        m_labelTextColor = Qt::white;
    }

    bool okay = false;
    int labelFontSize = context.selectInt(node, "LabelFontSize", &okay);
    if (okay) {
        m_iLabelFontSize = labelFontSize;
    }

    // Clear the background pixmap, if it exists.
    m_backgroundPixmap = QPixmap();
    m_backgroundPixmapPath = context.selectString(node, "BgPixmap");
    if (!m_backgroundPixmapPath.isEmpty()) {
        auto pPixmap = WPixmapStore::getPixmapNoCache(
                context.makeSkinPath(m_backgroundPixmapPath),
                m_scaleFactor);
        if (pPixmap) {
            m_backgroundPixmap = *pPixmap;
        }
    }

    m_endOfTrackColor = QColor(200, 25, 20);
    const QString endOfTrackColorName = context.selectString(node, "EndOfTrackColor");
    if (!endOfTrackColorName.isNull()) {
        m_endOfTrackColor = QColor(endOfTrackColorName);
        m_endOfTrackColor = WSkinColor::getCorrectColor(m_endOfTrackColor);
    }

    // setup hotcues and cue and loop(s)
    m_marks.setup(m_group, node, context, m_signalColors);

    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    auto colorPalette = colorPaletteSettings.getHotcueColorPalette();
    m_pCueMenuPopup->setColorPalette(colorPalette);

    m_marks.connectSamplePositionChanged(this, &WOverview::onMarkChanged);
    m_marks.connectSampleEndPositionChanged(this, &WOverview::onMarkChanged);
    m_marks.connectVisibleChanged(this, &WOverview::onMarkChanged);

    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == "MarkRange") {
            m_markRanges.push_back(WaveformMarkRange(m_group, child, context, m_signalColors));
            WaveformMarkRange& markRange = m_markRanges.back();

            if (markRange.m_markEnabledControl) {
                markRange.m_markEnabledControl->connectValueChanged(
                        this, &WOverview::onMarkRangeChange);
            }
            if (markRange.m_markVisibleControl) {
                markRange.m_markVisibleControl->connectValueChanged(
                        this, &WOverview::onMarkRangeChange);
            }
            if (markRange.m_markStartPointControl) {
                markRange.m_markStartPointControl->connectValueChanged(
                        this, &WOverview::onMarkRangeChange);
            }
            if (markRange.m_markEndPointControl) {
                markRange.m_markEndPointControl->connectValueChanged(
                        this, &WOverview::onMarkRangeChange);
            }
        }
        child = child.nextSibling();
    }

    DEBUG_ASSERT(m_pPassthroughLabel.get() != nullptr);
    m_pPassthroughLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // Shown on the overview waveform when vinyl passthrough is enabled
    m_pPassthroughLabel->setText(tr("Passthrough"));
    m_pPassthroughLabel->setStyleSheet(
            QStringLiteral("QLabel{"
                           "font-family: 'Open Sans';"
                           "font-size: %1px;"
                           "font-weight: bold;"
                           "color: %2;"
                           "margin-left };")
                    .arg(QString::number(int(m_iLabelFontSize * 1.5)),
                            QColor(m_signalColors.getPassthroughLabelColor())
                                    .name(QColor::HexRgb)));
    m_pPassthroughLabel->hide();
    QVBoxLayout* pPassthroughLayout = new QVBoxLayout(this);
    pPassthroughLayout->setContentsMargins(m_iLabelFontSize, 0, 0, 0); // l, t, r, b
    pPassthroughLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pPassthroughLayout->addWidget(m_pPassthroughLabel);
    setLayout(pPassthroughLayout);

    QString orientationString = context.selectString(node, "Orientation").toLower();
    if (orientationString == "vertical") {
        m_orientation = Qt::Vertical;
    } else {
        m_orientation = Qt::Horizontal;
    }

    m_bShowCueTimes = context.selectBool(node, "ShowCueTimes", true);

    // qDebug() << "WOverview : std::as_const(m_marks)" << m_marks.size();
    // qDebug() << "WOverview : m_markRanges" << m_markRanges.size();
    if (!m_connections.empty()) {
        auto& pDefaultConnection = m_connections.at(0);
        if (pDefaultConnection) {
            if (pDefaultConnection->getEmitOption() &
                    ControlParameterWidgetConnection::EMIT_DEFAULT) {
                // ON_PRESS means here value change on mouse move during press
                pDefaultConnection->setEmitOption(
                        ControlParameterWidgetConnection::EMIT_ON_RELEASE);
            }
        }
    }

    setFocusPolicy(Qt::NoFocus);
}

void WOverview::initWithTrack(TrackPointer pTrack) {
    Init();
    if (pTrack) {
        // if a track already loaded (after skin change)
        slotLoadingTrack(pTrack, TrackPointer());
        slotTrackLoaded(pTrack);
        slotWaveformSummaryUpdated();
    }
}

void WOverview::onConnectedControlChanged(double dParameter, double dValue) {
    // this is connected via skin to "playposition"
    Q_UNUSED(dValue);

    // Calculate handle position. Clamp the value within 0-1 because that's
    // all we represent with this widget.
    dParameter = math_clamp(dParameter, 0.0, 1.0);

    bool redraw = false;
    int oldPos = m_iPlayPos;
    m_iPlayPos = valueToPosition(dParameter);
    if (oldPos != m_iPlayPos) {
        redraw = true;
    }

    if (!m_bLeftClickDragging) {
        // if not dragged the pick-up moves with the play position
        m_iPickupPos = m_iPlayPos;
    }

    // In case the user is hovering a cue point or holding right click, the
    // calculated time between the playhead and cue/cursor should be updated at
    // least once per second, regardless of m_iPos which depends on the length
    // of the widget.
    int oldPositionSeconds = m_iPosSeconds;
    m_iPosSeconds = static_cast<int>(dParameter * getTrackSamples());
    if ((m_bTimeRulerActive || m_pHoveredMark != nullptr) && oldPositionSeconds != m_iPosSeconds) {
        redraw = true;
    }

    if (redraw) {
        update();
    }
}

void WOverview::slotWaveformSummaryUpdated() {
    //qDebug() << "WOverview::slotWaveformSummaryUpdated()";

    TrackPointer pTrack(m_pCurrentTrack);
    if (!pTrack) {
        return;
    }
    m_pWaveform = pTrack->getWaveformSummary();
    if (m_pWaveform) {
        // If the waveform is already complete, just draw it.
        if (m_pWaveform->getCompletion() == m_pWaveform->getDataSize()) {
            m_actualCompletion = 0;
            if (drawNextPixmapPart()) {
                update();
            }
        }
    } else {
        // Null waveform pointer means waveform was cleared.
        m_waveformSourceImage = QImage();
        m_analyzerProgress = kAnalyzerProgressUnknown;
        m_actualCompletion = 0;
        m_waveformPeak = -1.0;
        m_pixmapDone = false;

        update();
    }
}

void WOverview::onTrackAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress) {
    if (!m_pCurrentTrack || (m_pCurrentTrack->getId() != trackId)) {
        return;
    }

    bool updateNeeded = drawNextPixmapPart();
    if (updateNeeded || (m_analyzerProgress != analyzerProgress)) {
        m_analyzerProgress = analyzerProgress;
        update();
    }
}

void WOverview::slotTrackLoaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack); // only used in DEBUG_ASSERT
    //qDebug() << "WOverview::slotTrackLoaded()" << m_pCurrentTrack.get() << pTrack.get();
    DEBUG_ASSERT(m_pCurrentTrack == pTrack);
    m_trackLoaded = true;
    if (m_pCurrentTrack) {
        updateCues(m_pCurrentTrack->getCuePoints());
    }
    update();
}

void WOverview::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pOldTrack); // only used in DEBUG_ASSERT
    //qDebug() << this << "WOverview::slotLoadingTrack" << pNewTrack.get() << pOldTrack.get();
    DEBUG_ASSERT(m_pCurrentTrack == pOldTrack);
    if (m_pCurrentTrack != nullptr) {
        disconnect(m_pCurrentTrack.get(),
                &Track::waveformSummaryUpdated,
                this,
                &WOverview::slotWaveformSummaryUpdated);
        disconnect(m_pCurrentTrack.get(),
                &Track::cuesUpdated,
                this,
                &WOverview::receiveCuesUpdated);
    }

    m_waveformSourceImage = QImage();
    m_analyzerProgress = kAnalyzerProgressUnknown;
    m_actualCompletion = 0;
    m_waveformPeak = -1.0;
    m_pixmapDone = false;
    // Note: Here we already have the new track, but the engine and it's
    // Control Objects may still have the old one until the slotTrackLoaded()
    // signal has been received.
    m_trackLoaded = false;
    m_endOfTrack = false;

    if (pNewTrack) {
        m_pCurrentTrack = pNewTrack;
        m_pWaveform = pNewTrack->getWaveformSummary();

        connect(pNewTrack.get(),
                &Track::waveformSummaryUpdated,
                this,
                &WOverview::slotWaveformSummaryUpdated);
        slotWaveformSummaryUpdated();
        connect(pNewTrack.get(), &Track::cuesUpdated, this, &WOverview::receiveCuesUpdated);
    } else {
        m_pCurrentTrack.reset();
        m_pWaveform.clear();
    }
    update();
}

void WOverview::onEndOfTrackChange(double v) {
    //qDebug() << "WOverview::onEndOfTrackChange()" << v;
    m_endOfTrack = v > 0.0;
    update();
}

void WOverview::onMarkChanged(double v) {
    Q_UNUSED(v);
    //qDebug() << "WOverview::onMarkChanged()" << v;
    if (m_pCurrentTrack) {
        updateCues(m_pCurrentTrack->getCuePoints());
        update();
    }
}

void WOverview::onMarkRangeChange(double v) {
    Q_UNUSED(v);
    //qDebug() << "WOverview::onMarkRangeChange()" << v;
    update();
}

void WOverview::onRateRatioChange(double v) {
    Q_UNUSED(v);
    update();
}

void WOverview::onPassthroughChange(double v) {
    m_bPassthroughEnabled = static_cast<bool>(v);

    if (m_bPassthroughEnabled) {
        // Abort play position dragging
        m_bLeftClickDragging = false;
        m_bTimeRulerActive = false;
        m_iPickupPos = m_iPlayPos;
    } else {
        slotWaveformSummaryUpdated();
    }

    // Always call this to trigger a repaint even if no track is loaded
    update();
}

void WOverview::slotTypeControlChanged(double v) {
    // Assert that v is in enum range to prevent UB.
    DEBUG_ASSERT(v >= 0 && v < QMetaEnum::fromType<Type>().keyCount());
    Type type = static_cast<Type>(static_cast<int>(v));
    if (type == m_type) {
        return;
    }

    m_type = type;
    m_pWaveform.clear();
    slotWaveformSummaryUpdated();
}

void WOverview::slotMinuteMarkersChanged(bool /*unused*/) {
    update();
}

void WOverview::slotNormalizeOrVisualGainChanged() {
    update();
}

void WOverview::updateCues(const QList<CuePointer> &loadedCues) {
    for (const CuePointer& currentCue : loadedCues) {
        const WaveformMarkPointer pMark = m_marks.getHotCueMark(currentCue->getHotCue());

        if (pMark != nullptr && pMark->isValid() && pMark->isVisible()
            && pMark->getSamplePosition() != Cue::kNoPosition) {
            QColor newColor = mixxx::RgbColor::toQColor(currentCue->getColor());
            if (newColor != pMark->fillColor() || newColor != pMark->m_textColor) {
                pMark->setBaseColor(newColor, m_dimBrightThreshold);
            }

            int hotcueNumber = currentCue->getHotCue();
            if ((currentCue->getType() == mixxx::CueType::HotCue ||
                        currentCue->getType() == mixxx::CueType::Loop) &&
                    hotcueNumber != Cue::kNoHotCue) {
                // Prepend the hotcue number to hotcues' labels
                QString newLabel = currentCue->getLabel();
                if (newLabel.isEmpty()) {
                    newLabel = QString::number(hotcueNumber + 1);
                } else {
                    newLabel = QString("%1: %2").arg(hotcueNumber + 1).arg(newLabel);
                }

                if (pMark->m_text != newLabel) {
                    pMark->m_text = newLabel;
                }
            }
        }
    }

    m_marks.update();
}

// connecting the tracks cuesUpdated and onMarkChanged is not possible
// due to the incompatible signatures. This is a "wrapper" workaround
void WOverview::receiveCuesUpdated() {
    onMarkChanged(0);
}

void WOverview::mouseMoveEvent(QMouseEvent* e) {
    if (m_bLeftClickDragging) {
        if (isPosInAllowedPosDragZone(e->pos())) {
            m_bTimeRulerActive = true;
            m_timeRulerPos = e->pos();
            unsetCursor();
        } else {
            // Remove the time ruler to indicate dragging position is invalid,
            // don't abort dragging!
            m_iPickupPos = m_iPlayPos;
            m_bTimeRulerActive = false;

            setCursor(Qt::ForbiddenCursor);
            // Remember to restore cursor everywhere where we cancel dragging.
            // Update immediately.
            update();
            return;
        }

        if (m_orientation == Qt::Horizontal) {
            m_iPickupPos = math_clamp(e->pos().x(), 0, width() - 1);
        } else {
            m_iPickupPos = math_clamp(e->pos().y(), 0, height() - 1);
        }
    }

    // Do not activate cue hovering while right click is held down and the
    // button down event was not on a cue.
    if (m_bTimeRulerActive) {
        // Prevent showing times beyond the boundaries of the track when the
        // cursor is dragged outside this widget before releasing right click.
        m_timeRulerPos.setX(math_clamp(e->pos().x(), 0, width()));
        m_timeRulerPos.setY(math_clamp(e->pos().y(), 0, height()));
        update();
        return;
    }

    m_pHoveredMark = m_marks.findHoveredMark(e->pos(), m_orientation);

    // qDebug() << "WOverview::mouseMoveEvent" << e->pos();
    update();
}

void WOverview::mouseReleaseEvent(QMouseEvent* e) {
    mouseMoveEvent(e);
    if (m_bPassthroughEnabled) {
        m_bLeftClickDragging = false;
        // We may be dragging, and we may be outside the valid dragging area.
        // If so, we've set the 'invalid drag' cursor. Restore the cursor now.
        unsetCursor();
        return;
    }
    //qDebug() << "WOverview::mouseReleaseEvent" << e->pos() << m_iPos << ">>" << dValue;

    if (e->button() == Qt::LeftButton) {
        if (m_bLeftClickDragging) {
            unsetCursor();
            if (isPosInAllowedPosDragZone(e->pos())) {
                m_iPlayPos = m_iPickupPos;
                double dValue = positionToValue(m_iPickupPos);
                setControlParameterUp(dValue);
                m_bLeftClickDragging = false;
            } else {
                // Abort dragging if we are way outside the widget.
                m_iPickupPos = m_iPlayPos;
                m_bLeftClickDragging = false;
                m_bTimeRulerActive = false;
                return;
            }
        }
        m_bTimeRulerActive = false;
    } else if (e->button() == Qt::RightButton) {
        // Do not seek when releasing a right click. This is important to
        // prevent accidental seeking when trying to right click a hotcue.
        m_bTimeRulerActive = false;
    }
}

void WOverview::mousePressEvent(QMouseEvent* e) {
    //qDebug() << "WOverview::mousePressEvent" << e->pos();
    mouseMoveEvent(e);
    if (m_bPassthroughEnabled) {
        m_bLeftClickDragging = false;
        unsetCursor();
        return;
    }
    double trackSamples = getTrackSamples();
    if (m_pCurrentTrack == nullptr || trackSamples <= 0) {
        return;
    }
    if (e->button() == Qt::LeftButton) {
        if (m_orientation == Qt::Horizontal) {
            m_iPickupPos = math_clamp(e->pos().x(), 0, width() - 1);
        } else {
            m_iPickupPos = math_clamp(e->pos().y(), 0, height() - 1);
        }

        if (m_pHoveredMark != nullptr) {
            double dValue = m_pHoveredMark->getSamplePosition() / trackSamples;
            m_iPickupPos = valueToPosition(dValue);
            m_iPlayPos = m_iPickupPos;
            setControlParameterUp(dValue);
            m_bLeftClickDragging = false;
        } else {
            m_bLeftClickDragging = true;
            m_bTimeRulerActive = true;
            m_timeRulerPos = e->pos();
        }
    } else if (e->button() == Qt::RightButton) {
        if (m_bLeftClickDragging) {
            // Abort dragging
            m_iPickupPos = m_iPlayPos;
            m_bLeftClickDragging = false;
            m_bTimeRulerActive = false;
            unsetCursor();
        } else if (m_pHoveredMark == nullptr) {
            m_bTimeRulerActive = true;
            m_timeRulerPos = e->pos();
        } else if (m_pHoveredMark->getHotCue() != Cue::kNoHotCue) {
            // Currently the only way WaveformMarks can be associated
            // with their respective Cue objects is by using the hotcue
            // number. If cues without assigned hotcue are drawn on
            // WOverview in the future, another way to associate
            // WaveformMarks with Cues will need to be implemented.
            CuePointer pHoveredCue;
            const QList<CuePointer> cueList = m_pCurrentTrack->getCuePoints();
            for (const auto& pCue : cueList) {
                if (pCue->getHotCue() == m_pHoveredMark->getHotCue()) {
                    pHoveredCue = pCue;
                    break;
                }
            }
            if (pHoveredCue != nullptr) {
                if (e->modifiers().testFlag(Qt::ShiftModifier)) {
                    m_pCurrentTrack->removeCue(pHoveredCue);
                    return;
                } else {
                    // Clear the pickup position display, we have all cue info in the menu.
                    leaveEvent(nullptr);
                    m_pCueMenuPopup->setTrackCueGroup(m_pCurrentTrack, pHoveredCue, m_group);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                    m_pCueMenuPopup->popup(e->globalPosition().toPoint());
#else
                    m_pCueMenuPopup->popup(e->globalPos());
#endif
                }
            }
        }
    }
}

void WOverview::slotCueMenuPopupAboutToHide() {
    m_pHoveredMark.clear();
    update();
}

void WOverview::leaveEvent(QEvent* pEvent) {
    Q_UNUSED(pEvent);
    if (!m_pCueMenuPopup->isVisible()) {
        m_pHoveredMark.clear();
    }
    m_bLeftClickDragging = false;
    m_bTimeRulerActive = false;
    update();
}

void WOverview::paintEvent(QPaintEvent* pEvent) {
    Q_UNUSED(pEvent);
    ScopedTimer t(QStringLiteral("WOverview::paintEvent"));

    QPainter painter(this);
    painter.fillRect(rect(), m_backgroundColor);

    if (!m_backgroundPixmap.isNull()) {
        painter.drawPixmap(rect(), m_backgroundPixmap);
    }

    if (m_pCurrentTrack) {
        // Refer to util/ScopePainter.h to understand the semantics of
        // ScopePainter.
        drawEndOfTrackBackground(&painter);
        drawAxis(&painter);
        drawWaveformPixmap(&painter);
        drawPlayedOverlay(&painter);
        drawMinuteMarkers(&painter);
        drawPlayPosition(&painter);
        drawEndOfTrackFrame(&painter);
        drawAnalyzerProgress(&painter);

        double trackSamples = getTrackSamples();
        if (trackSamples > 0) {
            const float offset = 1.0f;
            const auto gain = static_cast<CSAMPLE_GAIN>(length() - 2) /
                    static_cast<CSAMPLE_GAIN>(trackSamples);

            drawRangeMarks(&painter, offset, gain);
            drawMarks(&painter, offset, gain);
            drawPickupPosition(&painter);
            drawTimeRuler(&painter);
            drawMarkLabels(&painter, offset, gain);
        }
    }

    if (m_bPassthroughEnabled) {
        drawPassthroughOverlay(&painter);
        m_pPassthroughLabel->show();
        unsetCursor();
    } else {
        m_pPassthroughLabel->hide();
    }
}

void WOverview::drawEndOfTrackBackground(QPainter* pPainter) {
    if (m_endOfTrack) {
        PainterScope painterScope(pPainter);
        pPainter->setOpacity(0.3);
        pPainter->setBrush(m_endOfTrackColor);
        pPainter->drawRect(rect().adjusted(1, 1, -2, -2));
    }
}

void WOverview::drawAxis(QPainter* pPainter) {
    PainterScope painterScope(pPainter);
    pPainter->setPen(QPen(m_axesColor, m_scaleFactor));
    if (m_orientation == Qt::Horizontal) {
        pPainter->drawLine(0, height() / 2, width(), height() / 2);
    } else {
        pPainter->drawLine(width() / 2, 0, width() / 2, height());
    }
}

void WOverview::drawWaveformPixmap(QPainter* pPainter) {
    if (!m_waveformSourceImage.isNull()) {
        WaveformWidgetFactory* pWidgetFactory = WaveformWidgetFactory::instance();
        float diffGain;
        bool normalize = pWidgetFactory->isOverviewNormalized();
        if (normalize && m_pixmapDone && m_waveformPeak > 1) {
            diffGain = 255 - m_waveformPeak - 1;
        } else {
            DEBUG_ASSERT(m_pCurrentTrack);
            const auto replayGain = m_pCurrentTrack->getReplayGain();
            const auto visualGain = static_cast<float>(
                    pWidgetFactory->getVisualGain(WaveformWidgetFactory::All) *
                    (replayGain.hasRatio()
                                    ? replayGain.getRatio()
                                    : 1.0));
            diffGain = 255.0f - (255.0f / visualGain);
        }

        if (m_diffGain != diffGain || m_waveformImageScaled.isNull()) {
            QRect sourceRect(0,
                    static_cast<int>(diffGain),
                    m_waveformSourceImage.width(),
                    m_waveformSourceImage.height() -
                            2 * static_cast<int>(diffGain));
            QImage croppedImage = m_waveformSourceImage.copy(sourceRect);
            if (m_orientation == Qt::Vertical) {
                // Rotate pixmap
                croppedImage = croppedImage.transformed(QTransform(0, 1, 1, 0, 0, 0));
            }
            m_waveformImageScaled = croppedImage.scaled(size() * m_devicePixelRatio,
                    Qt::IgnoreAspectRatio,
                    Qt::SmoothTransformation);
            m_diffGain = diffGain;
        }

        pPainter->drawImage(rect(), m_waveformImageScaled);
    }
}

void WOverview::drawMinuteMarkers(QPainter* pPainter) {
    if (!m_trackLoaded) {
        return;
    }

    if (!static_cast<bool>(m_pMinuteMarkersControl->get())) {
        return;
    }

    if (m_pRateRatioControl->get() == 0) {
        return;
    }

    // Faster than track->getDuration() and already has playback speed ratio compensated for
    const double trackSeconds = samplePositionToSeconds(getTrackSamples());

    QLineF line;
    pPainter->setPen(QPen(m_axesColor, m_scaleFactor));
    pPainter->setOpacity(1.0);

    const double overviewHeight = m_orientation == Qt::Horizontal ? height() : width();
    const double markerHeight = overviewHeight * 0.08;
    const double lowerMarkerYPos = overviewHeight * 0.92;
    double currentMarkerXPos;
    const int iWidth = m_orientation == Qt::Horizontal ? width() : height();
    for (double currentMarkerSeconds = 60; currentMarkerSeconds < trackSeconds;
            currentMarkerSeconds += 60) {
        currentMarkerXPos = currentMarkerSeconds / trackSeconds * iWidth;

        if (m_orientation == Qt::Horizontal) {
            line.setLine(currentMarkerXPos, 0.0, currentMarkerXPos, markerHeight);
            pPainter->drawLine(line);
            line.setLine(currentMarkerXPos, lowerMarkerYPos, currentMarkerXPos, overviewHeight);
            pPainter->drawLine(line);
        } else {
            // untested, best effort basis
            line.setLine(0.0, currentMarkerXPos, markerHeight, currentMarkerXPos);
            pPainter->drawLine(line);
            line.setLine(lowerMarkerYPos, currentMarkerXPos, overviewHeight, currentMarkerXPos);
            pPainter->drawLine(line);
        }
    }
}

void WOverview::drawPlayedOverlay(QPainter* pPainter) {
    // Overlay the played part of the overview-waveform with a skin defined color
    if (!m_waveformSourceImage.isNull() && m_playedOverlayColor.alpha() > 0) {
        if (m_orientation == Qt::Vertical) {
            pPainter->fillRect(0,
                    0,
                    m_waveformImageScaled.width(),
                    m_iPlayPos,
                    m_playedOverlayColor);
        } else {
            pPainter->fillRect(0,
                    0,
                    m_iPlayPos,
                    m_waveformImageScaled.height(),
                    m_playedOverlayColor);
        }
    }
}

void WOverview::drawPlayPosition(QPainter* pPainter) {
    // When the position line is currently dragged with the left mouse button
    // draw a thin line -without triangles or shadow- at the playposition.
    // The new playposition is drawn by drawPickupPosition()
    if (m_bLeftClickDragging) {
        PainterScope painterScope(pPainter);
        QLineF line;
        if (m_orientation == Qt::Horizontal) {
            line.setLine(m_iPlayPos, 0.0, m_iPlayPos, height());
        } else {
            line.setLine(0.0, m_iPlayPos, width(), m_iPlayPos);
        }
        pPainter->setPen(QPen(m_playPosColor, m_scaleFactor));
        pPainter->setOpacity(0.5);
        pPainter->drawLine(line);
    }
}

void WOverview::drawEndOfTrackFrame(QPainter* pPainter) {
    if (m_endOfTrack) {
        PainterScope painterScope(pPainter);
        pPainter->setOpacity(0.8);
        pPainter->setPen(QPen(QBrush(m_endOfTrackColor), 1.5 * m_scaleFactor));
        pPainter->setBrush(QColor(0, 0, 0, 0));
        pPainter->drawRect(rect().adjusted(0, 0, -1, -1));
    }
}

void WOverview::drawAnalyzerProgress(QPainter* pPainter) {
    if ((m_analyzerProgress >= kAnalyzerProgressNone) &&
            (m_analyzerProgress < kAnalyzerProgressDone)) {
        PainterScope painterScope(pPainter);
        pPainter->setPen(QPen(m_playPosColor, 3 * m_scaleFactor));

        if (m_analyzerProgress > kAnalyzerProgressNone) {
            if (m_orientation == Qt::Horizontal) {
                pPainter->drawLine(QLineF(width() * m_analyzerProgress,
                        height() / 2,
                        width(),
                        height() / 2));
            } else {
                pPainter->drawLine(QLineF(width() / 2,
                        height() * m_analyzerProgress,
                        width() / 2,
                        height()));
            }
        }

        if (m_analyzerProgress <= kAnalyzerProgressHalf) { // remove text after progress by wf is recognizable
            if (m_trackLoaded) {
                //: Text on waveform overview when file is playable but no waveform is visible
                paintText(tr("Ready to play, analyzing..."), pPainter);
            } else {
                //: Text on waveform overview when file is cached from source
                paintText(tr("Loading track..."), pPainter);
            }
        } else if (m_analyzerProgress >= kAnalyzerProgressFinalizing) {
            //: Text on waveform overview during finalizing of waveform analysis
            paintText(tr("Finalizing..."), pPainter);
        }
    } else if (!m_trackLoaded) {
        // This happens if the track samples are not loaded, but we have
        // a cached track
        //: Text on waveform overview when file is cached from source
        paintText(tr("Loading track..."), pPainter);
    }
}

void WOverview::drawRangeMarks(QPainter* pPainter, const float& offset, const float& gain) {
    for (auto&& markRange : m_markRanges) {
        if (!markRange.active() || !markRange.visible()) {
            continue;
        }

        // Active mark ranges by definition have starts/ends that are not
        // disabled.
        const qreal startValue = markRange.start();
        const qreal endValue = markRange.end();

        const qreal startPosition = offset + startValue * gain;
        const qreal endPosition = offset + endValue * gain;

        if (startPosition < 0.0 && endPosition < 0.0) {
            continue;
        }

        PainterScope painterScope(pPainter);

        if (markRange.enabled()) {
            pPainter->setOpacity(markRange.m_enabledOpacity);
            pPainter->setPen(markRange.m_activeColor);
            pPainter->setBrush(markRange.m_activeColor);
        } else {
            pPainter->setOpacity(markRange.m_disabledOpacity);
            pPainter->setPen(markRange.m_disabledColor);
            pPainter->setBrush(markRange.m_disabledColor);
        }

        // let top and bottom of the rect out of the widget
        if (m_orientation == Qt::Horizontal) {
            pPainter->drawRect(QRectF(QPointF(startPosition, -2.0),
                    QPointF(endPosition, height() + 1.0)));
        } else {
            pPainter->drawRect(QRectF(QPointF(-2.0, startPosition),
                    QPointF(width() + 1.0, endPosition)));
        }
    }
}

void WOverview::drawMarks(QPainter* pPainter, const float offset, const float gain) {
    QFont markerFont = pPainter->font();
    markerFont.setPixelSize(static_cast<int>(m_iLabelFontSize * m_scaleFactor));
    QFontMetricsF fontMetrics(markerFont);

    // Text labels are rendered so they do not overlap with other WaveformMarks'
    // labels. If the text would be too wide, it is elided. However, the user
    // can hover the mouse cursor over a label to show the whole label text,
    // temporarily hiding any following labels that would be drawn over it.
    // This requires looping over the WaveformMarks twice and the marks must be
    // sorted in the order they appear on the waveform.
    // In the first loop, the lines are drawn and the text to render plus its
    // location are calculated then stored in a WaveformMarkLabel. The text must
    // be drawn in the second loop to prevent the lines of following
    // WaveformMarks getting drawn over it. The second loop is in the separate
    // drawMarkLabels function so it can be called after drawCurrentPosition so
    // the view of labels is not obscured by the playhead.

    bool markHovered = false;

    for (auto it = m_marks.cbegin(); it != m_marks.cend(); ++it) {
        PainterScope painterScope(pPainter);
        const WaveformMarkPointer& pMark = *it;
        double samplePosition = pMark->getSamplePosition();
        const float markPosition = math_clamp(
                offset + static_cast<float>(samplePosition) * gain,
                0.0f,
                static_cast<float>(width()));
        pMark->m_linePosition = markPosition;

        QLineF line;
        QLineF bgLine;
        if (m_orientation == Qt::Horizontal) {
            line.setLine(markPosition, 0.0, markPosition, height());
            bgLine.setLine(markPosition - 1.0, 0.0, markPosition - 1.0, height());
        } else {
            line.setLine(0.0, markPosition, width(), markPosition);
            bgLine.setLine(0.0, markPosition - 1.0, width(), markPosition - 1.0);
        }

        QRectF rect;
        double sampleEndPosition = pMark->getSampleEndPosition();
        if (sampleEndPosition > 0) {
            const float markEndPosition = math_clamp(
                    offset + static_cast<float>(sampleEndPosition) * gain,
                    0.0f,
                    static_cast<float>(width()));

            if (m_orientation == Qt::Horizontal) {
                rect.setCoords(markPosition, 0, markEndPosition, height());
            } else {
                rect.setCoords(0, markPosition, width(), markEndPosition);
            }
        }

        pPainter->setPen(pMark->borderColor());
        pPainter->drawLine(bgLine);

        pPainter->setPen(pMark->fillColor());
        pPainter->drawLine(line);

        if (rect.isValid()) {
            QColor loopColor = pMark->fillColor();
            loopColor.setAlphaF(0.5f);
            pPainter->fillRect(rect, loopColor);
        }

        if (!pMark->m_text.isEmpty()) {
            Qt::Alignment halign = pMark->m_align & Qt::AlignHorizontal_Mask;
            Qt::Alignment valign = pMark->m_align & Qt::AlignVertical_Mask;

            QString text = pMark->m_text;

            // Only allow the text to overlap the following mark if the mouse is
            // hovering over it. Elide it if it would render over the next
            // label, but do not elide it if the next mark's label is not at the
            // same vertical position.

            if (pMark != m_pHoveredMark) {
                float nextMarkPosition = -1.0f;
                for (auto m = std::next(it); m != m_marks.cend(); ++m) {
                    const WaveformMarkPointer& otherMark = *m;
                    bool otherAtSameHeight = valign == (otherMark->m_align & Qt::AlignVertical_Mask);
                    // Hotcues always show at least their number.
                    bool otherHasLabel = !otherMark->m_text.isEmpty() || otherMark->getHotCue() != Cue::kNoHotCue;
                    if (otherAtSameHeight && otherHasLabel) {
                        nextMarkPosition = offset +
                                static_cast<float>(
                                        otherMark->getSamplePosition()) *
                                        gain;
                        break;
                    }
                }
                if (nextMarkPosition != -1.0) {
                    text = fontMetrics.elidedText(text, Qt::ElideRight, nextMarkPosition - markPosition - 5);
                }
            }
            // Sometimes QFontMetrics::elidedText turns the QString into just an
            // ellipsis character, so always show at least the hotcue number if
            // the label does not fit.
            if ((text.isEmpty() || text == "…") && pMark->getHotCue() != Cue::kNoHotCue) {
                text = QString::number(pMark->getHotCue() + 1);
            }

            QRectF textRect = fontMetrics.boundingRect(text);
            QPointF textPoint;
            if (m_orientation == Qt::Horizontal) {
                if (halign == Qt::AlignLeft) {
                    textPoint.setX(markPosition - textRect.width() - 5.5);
                } else if (halign == Qt::AlignHCenter) {
                    textPoint.setX(markPosition - textRect.width() / 2);
                } else { // AlignRight
                    textPoint.setX(markPosition + 1.5);
                }

                if (valign == Qt::AlignTop) {
                    textPoint.setY(fontMetrics.height());
                } else if (valign == Qt::AlignVCenter) {
                    textPoint.setY((textRect.height() + height()) / 2);
                } else { // AlignBottom
                    textPoint.setY(float(height()) - 0.5f);
                }
            } else { // Vertical
                if (halign == Qt::AlignLeft) {
                    textPoint.setX(1.0f);
                } else if (halign == Qt::AlignHCenter) {
                    textPoint.setX((width() - textRect.width()) / 2);
                } else { // AlignRight
                    textPoint.setX(width() - textRect.width());
                }

                if (valign == Qt::AlignTop) {
                    textPoint.setY(markPosition - 1.0f);
                } else if (valign == Qt::AlignVCenter) {
                    textPoint.setY(markPosition + textRect.height() / 2);
                } else { // AlignBottom
                    textPoint.setY(markPosition + fontMetrics.ascent());
                }
            }

            pMark->m_label.prerender(textPoint,
                    QPixmap(),
                    text,
                    markerFont,
                    m_labelTextColor,
                    m_labelBackgroundColor,
                    width(),
                    devicePixelRatioF());
        }

        // Show cue position when hovered
        // The area it will be drawn in needs to be calculated here
        // before drawMarkLabels so drawMarkLabels can avoid drawing
        // labels over the cue position.
        // This can happen for example if the user shows the cue position
        // of a hotcue which is near the intro end position because the
        // intro_end_position WaveformMark label is drawn at the top.
        // However, the drawing of this text needs to happen in
        // drawMarkLabels so none of the WaveformMark lines are drawn
        // on top of the position text.

        // WaveformMark::m_align refers to the alignment of the label,
        // so if the label is on bottom draw the position text on top and
        // vice versa.
        if (pMark == m_pHoveredMark) {
            Qt::Alignment valign = pMark->m_align & Qt::AlignVertical_Mask;
            QPointF positionTextPoint(markPosition + 1.5, 0);
            if (valign == Qt::AlignTop) {
                positionTextPoint.setY(float(height()) - 0.5f);
            } else {
                positionTextPoint.setY(fontMetrics.height());
            }

            double markSamples = pMark->getSamplePosition();
            double trackSamples = getTrackSamples();
            double currentPositionSamples = m_playpositionControl.get() * trackSamples;
            double markTime = samplePositionToSeconds(markSamples);
            double markTimeRemaining = samplePositionToSeconds(trackSamples - markSamples);
            double markTimeDistance = samplePositionToSeconds(markSamples - currentPositionSamples);
            QString cuePositionText = mixxx::Duration::formatTime(markTime) + " -" +
                    mixxx::Duration::formatTime(markTimeRemaining);
            QString cueTimeDistanceText = mixxx::Duration::formatTime(fabs(markTimeDistance));
            // Cast to int to avoid confusingly switching from -0:00 to 0:00 as
            // the playhead passes the cue
            if (static_cast<int>(markTimeDistance) < 0) {
                cueTimeDistanceText = "-" + cueTimeDistanceText;
            }

            m_cuePositionLabel.prerender(positionTextPoint,
                    QPixmap(),
                    cuePositionText,
                    markerFont,
                    m_labelTextColor,
                    m_labelBackgroundColor,
                    width(),
                    devicePixelRatioF());

            QPointF timeDistancePoint(positionTextPoint.x(),
                    (fontMetrics.height() + height()) / 2);

            m_cueTimeDistanceLabel.prerender(timeDistancePoint,
                    QPixmap(),
                    cueTimeDistanceText,
                    markerFont,
                    m_labelTextColor,
                    m_labelBackgroundColor,
                    width(),
                    devicePixelRatioF());
            markHovered = true;
        }
    }
    if (!markHovered) {
        m_cuePositionLabel.clear();
        m_cueTimeDistanceLabel.clear();
    }
}

void WOverview::drawPickupPosition(QPainter* pPainter) {
    PainterScope painterScope(pPainter);

    if (m_orientation == Qt::Vertical) {
        pPainter->setTransform(QTransform(0, 1, 1, 0, 0, 0));
    }

    // draw dark play position outlines
    pPainter->setPen(QPen(QBrush(m_backgroundColor), m_scaleFactor));
    pPainter->setOpacity(0.5);
    pPainter->drawLine(m_iPickupPos + 1, 0, m_iPickupPos + 1, breadth());
    pPainter->drawLine(m_iPickupPos - 1, 0, m_iPickupPos - 1, breadth());

    // draw colored play position line
    pPainter->setPen(QPen(m_playPosColor, m_scaleFactor));
    pPainter->setOpacity(1.0);
    pPainter->drawLine(m_iPickupPos, 0, m_iPickupPos, breadth());

    // draw triangle at the top
    pPainter->drawLine(m_iPickupPos - 2, 0, m_iPickupPos, 2);
    pPainter->drawLine(m_iPickupPos, 2, m_iPickupPos + 2, 0);
    pPainter->drawLine(m_iPickupPos - 2, 0, m_iPickupPos + 2, 0);

    // draw triangle at the bottom
    pPainter->drawLine(m_iPickupPos - 2, breadth() - 1, m_iPickupPos, breadth() - 3);
    pPainter->drawLine(m_iPickupPos, breadth() - 3, m_iPickupPos + 2, breadth() - 1);
    pPainter->drawLine(m_iPickupPos - 2, breadth() - 1, m_iPickupPos + 2, breadth() - 1);
}

void WOverview::drawTimeRuler(QPainter* pPainter) {
    QFont markerFont = pPainter->font();
    markerFont.setPixelSize(static_cast<int>(m_iLabelFontSize * m_scaleFactor));
    QFontMetricsF fontMetrics(markerFont);

    QFont shadowFont = pPainter->font();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    shadowFont.setWeight(99);
#else
    shadowFont.setWeight(QFont::Black);
#endif
    shadowFont.setPixelSize(static_cast<int>(m_iLabelFontSize * m_scaleFactor));
    QPen shadowPen(Qt::black, 2.5 * m_scaleFactor);

    if (m_bTimeRulerActive) {
        if (!m_bLeftClickDragging) {
            QLineF line;
            if (m_orientation == Qt::Horizontal) {
                line.setLine(m_timeRulerPos.x(), 0.0, m_timeRulerPos.x(), height());
            } else {
                line.setLine(0.0, m_timeRulerPos.x(), width(), m_timeRulerPos.x());
            }
            pPainter->setPen(shadowPen);
            pPainter->drawLine(line);

            pPainter->setPen(QPen(m_playPosColor, m_scaleFactor));
            pPainter->drawLine(line);
        }

        QPointF textPoint = m_timeRulerPos;
        QPointF textPointDistance = m_timeRulerPos;
        qreal widgetPositionFraction;
        qreal padding = 1.0; // spacing between line and text
        if (m_orientation == Qt::Horizontal) {
            textPoint = QPointF(textPoint.x() + padding, fontMetrics.height());
            textPointDistance = QPointF(textPointDistance.x() + padding,
                    (fontMetrics.height() + height()) / 2);
            widgetPositionFraction = m_timeRulerPos.x() / width();
        } else {
            textPoint.setX(0);
            textPointDistance.setX(0);
            widgetPositionFraction = m_timeRulerPos.y() / height();
        }
        qreal trackSamples = getTrackSamples();
        qreal timePosition = samplePositionToSeconds(
                widgetPositionFraction * trackSamples);
        qreal timePositionTillEnd = samplePositionToSeconds(
                (1 - widgetPositionFraction) * trackSamples);
        qreal timeDistance = samplePositionToSeconds(
                (widgetPositionFraction - m_playpositionControl.get()) * trackSamples);

        QString timeText = mixxx::Duration::formatTime(timePosition) + " -" + mixxx::Duration::formatTime(timePositionTillEnd);

        m_timeRulerPositionLabel.prerender(textPoint,
                QPixmap(),
                timeText,
                markerFont,
                m_labelTextColor,
                m_labelBackgroundColor,
                width(),
                devicePixelRatioF());
        m_timeRulerPositionLabel.draw(pPainter);

        QString timeDistanceText = mixxx::Duration::formatTime(fabs(timeDistance));
        // Cast to int to avoid confusingly switching from -0:00 to 0:00 as
        // the playhead passes the point
        if (static_cast<int>(timeDistance) < 0) {
            timeDistanceText = "-" + timeDistanceText;
        }
        m_timeRulerDistanceLabel.prerender(textPointDistance,
                QPixmap(),
                timeDistanceText,
                markerFont,
                m_labelTextColor,
                m_labelBackgroundColor,
                width(),
                devicePixelRatioF());
        m_timeRulerDistanceLabel.draw(pPainter);
    } else {
        m_timeRulerPositionLabel.clear();
        m_timeRulerDistanceLabel.clear();
    }
}

void WOverview::drawMarkLabels(QPainter* pPainter, const float offset, const float gain) {
    QFont markerFont = pPainter->font();
    markerFont.setPixelSize(static_cast<int>(m_iLabelFontSize * m_scaleFactor));
    QFontMetricsF fontMetrics(markerFont);

    // Draw WaveformMark labels
    for (const auto& pMark : std::as_const(m_marks)) {
        if (m_pHoveredMark != nullptr && pMark != m_pHoveredMark) {
            if (pMark->m_label.intersects(m_pHoveredMark->m_label)) {
                continue;
            }
        }
        if (m_bShowCueTimes &&
                (pMark->m_label.intersects(m_cuePositionLabel) || pMark->m_label.intersects(m_cueTimeDistanceLabel))) {
            continue;
        }
        if (pMark->m_label.intersects(m_timeRulerPositionLabel) || pMark->m_label.intersects(m_timeRulerDistanceLabel)) {
            continue;
        }

        pMark->m_label.draw(pPainter);
    }

    if (m_bShowCueTimes) {
        m_cuePositionLabel.draw(pPainter);
        m_cueTimeDistanceLabel.draw(pPainter);
    }

    // draw duration of WaveformMarkRanges
    for (auto&& markRange : m_markRanges) {
        if (markRange.showDuration() && markRange.active() && markRange.visible()) {
            // Active mark ranges by definition have starts/ends that are not
            // disabled.
            const qreal startValue = markRange.start();
            const qreal endValue = markRange.end();

            const qreal startPosition = offset + startValue * gain;
            const qreal endPosition = offset + endValue * gain;

            if (startPosition < 0.0 && endPosition < 0.0) {
                continue;
            }
            QString duration = mixxx::Duration::formatTime(
                    samplePositionToSeconds(endValue - startValue));

            QRectF durationRect = fontMetrics.boundingRect(duration);
            qreal x;

            WaveformMarkRange::DurationTextLocation textLocation = markRange.durationTextLocation();
            if (textLocation == WaveformMarkRange::DurationTextLocation::Before) {
                x = startPosition - durationRect.width() - 5.5;
            } else {
                x = endPosition + 1.5;
            }

            QPointF durationBottomLeft(x, fontMetrics.height());

            markRange.m_durationLabel.prerender(durationBottomLeft,
                    QPixmap(),
                    duration,
                    markerFont,
                    m_labelTextColor,
                    m_labelBackgroundColor,
                    width(),
                    devicePixelRatioF());

            if (!(markRange.m_durationLabel.intersects(m_cuePositionLabel) || markRange.m_durationLabel.intersects(m_cueTimeDistanceLabel) || markRange.m_durationLabel.intersects(m_timeRulerPositionLabel) || markRange.m_durationLabel.intersects(m_timeRulerDistanceLabel))) {
                markRange.m_durationLabel.draw(pPainter);
            }
        }
    }
}

void WOverview::drawPassthroughOverlay(QPainter* pPainter) {
    if (!m_waveformSourceImage.isNull() && m_passthroughOverlayColor.alpha() > 0) {
        // Overlay the entire overview-waveform with a skin defined color
        pPainter->fillRect(rect(), m_passthroughOverlayColor);
    }
}

bool WOverview::drawNextPixmapPart() {
    ConstWaveformPointer pWaveform = getWaveform();
    if (!pWaveform) {
        return false;
    }

    const int dataSize = pWaveform->getDataSize();
    const double audioVisualRatio = pWaveform->getAudioVisualRatio();
    const double trackSamples = getTrackSamples();
    if (dataSize <= 0 || audioVisualRatio <= 0 || trackSamples <= 0) {
        return false;
    }

    if (m_waveformSourceImage.isNull()) {
        // Waveform pixmap twice the height of the viewport to be scalable
        // by total_gain
        // We keep full range waveform data to scale it on paint
        m_waveformSourceImage = QImage(
                static_cast<int>(trackSamples / audioVisualRatio / 2) + 1,
                2 * 255,
                QImage::Format_ARGB32_Premultiplied);
        m_waveformSourceImage.fill(QColor(0, 0, 0, 0).value());
        if (dataSize / 2 != m_waveformSourceImage.width()) {
            qWarning() << "Track duration has changed since last analysis"
                       << m_waveformSourceImage.width() << "!=" << dataSize / 2;
        }
    }
    DEBUG_ASSERT(!m_waveformSourceImage.isNull());

    // Always multiple of 2
    const int waveformCompletion = pWaveform->getCompletion();
    // Test if there is some new to draw (at least of pixel width)
    const int completionIncrement = waveformCompletion - m_actualCompletion;

    int visiblePixelIncrement = completionIncrement * length() / dataSize;
    if (waveformCompletion < (dataSize - 2) &&
            (completionIncrement < 2 || visiblePixelIncrement == 0)) {
        return false;
    }

    const int nextCompletion = m_actualCompletion + completionIncrement;

    // qDebug() << "WOverview::drawNextPixmapPart() - nextCompletion:"
    //  << nextCompletion
    //  << "m_actualCompletion:" << m_actualCompletion
    //  << "waveformCompletion:" << waveformCompletion
    //  << "completionIncrement:" << completionIncrement;

    QPainter painter(&m_waveformSourceImage);
    painter.translate(0.0, static_cast<double>(m_waveformSourceImage.height()) / 2.0);

    if (m_type == Type::Filtered) {
        drawNextPixmapPartLMH(&painter, pWaveform, nextCompletion);
    } else if (m_type == Type::HSV) {
        drawNextPixmapPartHSV(&painter, pWaveform, nextCompletion);
    } else { // Type::RGB:
        drawNextPixmapPartRGB(&painter, pWaveform, nextCompletion);
    }

    m_waveformImageScaled = QImage();
    m_diffGain = 0;

    // Test if the complete waveform is done
    if (m_actualCompletion >= dataSize - 2) {
        m_pixmapDone = true;
        // qDebug() << "m_waveformPeakRatio" << m_waveformPeak;
    }

    return true;
}

void WOverview::drawNextPixmapPartHSV(QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        const int nextCompletion) {
    DEBUG_ASSERT(!m_waveformSourceImage.isNull());
    ScopedTimer t(QStringLiteral("WOverview::drawNextPixmapPartHSV"));

    // Get HSV of low color.
    float h, s, v;
    getHsvF(m_signalColors.getLowColor(), &h, &s, &v);

    QColor color;
    float lo, hi, total;

    unsigned char maxLow[2] = {0, 0};
    unsigned char maxHigh[2] = {0, 0};
    unsigned char maxMid[2] = {0, 0};
    unsigned char maxAll[2] = {0, 0};

    int currentCompletion = 0;
    for (int currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        maxAll[0] = pWaveform->getAll(currentCompletion);
        maxAll[1] = pWaveform->getAll(currentCompletion + 1);
        if (maxAll[0] || maxAll[1]) {
            maxLow[0] = pWaveform->getLow(currentCompletion);
            maxLow[1] = pWaveform->getLow(currentCompletion + 1);
            maxMid[0] = pWaveform->getMid(currentCompletion);
            maxMid[1] = pWaveform->getMid(currentCompletion + 1);
            maxHigh[0] = pWaveform->getHigh(currentCompletion);
            maxHigh[1] = pWaveform->getHigh(currentCompletion + 1);

            total = (maxLow[0] + maxLow[1] + maxMid[0] + maxMid[1] +
                            maxHigh[0] + maxHigh[1]) *
                    1.2f;

            // Prevent division by zero
            if (total > 0) {
                // Normalize low and high
                // (mid not need, because it not change the color)
                lo = (maxLow[0] + maxLow[1]) / total;
                hi = (maxHigh[0] + maxHigh[1]) / total;
            } else {
                lo = hi = 0.0;
            }

            // Set color
            color.setHsvF(h, 1.0f - hi, 1.0f - lo);

            pPainter->setPen(color);
            pPainter->drawLine(QPoint(currentCompletion / 2, -maxAll[0]),
                    QPoint(currentCompletion / 2, maxAll[1]));
        }
    }

    // Evaluate waveform ratio peak
    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        m_waveformPeak = math_max3(
                m_waveformPeak,
                static_cast<float>(pWaveform->getAll(currentCompletion)),
                static_cast<float>(pWaveform->getAll(currentCompletion + 1)));
    }

    m_actualCompletion = nextCompletion;
}

void WOverview::drawNextPixmapPartLMH(QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        const int nextCompletion) {
    DEBUG_ASSERT(!m_waveformSourceImage.isNull());
    ScopedTimer t(QStringLiteral("WOverview::drawNextPixmapPartLMH"));

    QColor lowColor = m_signalColors.getLowColor();
    QPen lowColorPen(QBrush(lowColor), 1);

    QColor midColor = m_signalColors.getMidColor();
    QPen midColorPen(QBrush(midColor), 1);

    QColor highColor = m_signalColors.getHighColor();
    QPen highColorPen(QBrush(highColor), 1);

    int currentCompletion = 0;
    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        unsigned char lowNeg = pWaveform->getLow(currentCompletion);
        unsigned char lowPos = pWaveform->getLow(currentCompletion + 1);
        if (lowPos || lowNeg) {
            pPainter->setPen(lowColorPen);
            pPainter->drawLine(QPoint(currentCompletion / 2, -lowNeg),
                    QPoint(currentCompletion / 2, lowPos));
        }
    }

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        pPainter->setPen(midColorPen);
        pPainter->drawLine(QPoint(currentCompletion / 2,
                                   -pWaveform->getMid(currentCompletion)),
                QPoint(currentCompletion / 2,
                        pWaveform->getMid(currentCompletion + 1)));
    }

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        pPainter->setPen(highColorPen);
        pPainter->drawLine(QPoint(currentCompletion / 2,
                                   -pWaveform->getHigh(currentCompletion)),
                QPoint(currentCompletion / 2,
                        pWaveform->getHigh(currentCompletion + 1)));
    }

    // Evaluate waveform ratio peak

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        m_waveformPeak = math_max3(
                m_waveformPeak,
                static_cast<float>(pWaveform->getAll(currentCompletion)),
                static_cast<float>(pWaveform->getAll(currentCompletion + 1)));
    }

    m_actualCompletion = nextCompletion;
}

void WOverview::drawNextPixmapPartRGB(QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        const int nextCompletion) {
    DEBUG_ASSERT(!m_waveformSourceImage.isNull());
    ScopedTimer t(QStringLiteral("WOverview::drawNextPixmapPartRGB"));

    QColor color;

    float lowColor_r, lowColor_g, lowColor_b;
    getRgbF(m_signalColors.getRgbLowColor(), &lowColor_r, &lowColor_g, &lowColor_b);

    float midColor_r, midColor_g, midColor_b;
    getRgbF(m_signalColors.getRgbMidColor(), &midColor_r, &midColor_g, &midColor_b);

    float highColor_r, highColor_g, highColor_b;
    getRgbF(m_signalColors.getRgbHighColor(), &highColor_r, &highColor_g, &highColor_b);

    int currentCompletion = 0;
    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        unsigned char left = pWaveform->getAll(currentCompletion);
        unsigned char right = pWaveform->getAll(currentCompletion + 1);

        // Retrieve "raw" LMH values from waveform
        float low = static_cast<float>(pWaveform->getLow(currentCompletion));
        float mid = static_cast<float>(pWaveform->getMid(currentCompletion));
        float high = static_cast<float>(pWaveform->getHigh(currentCompletion));

        // Do matrix multiplication
        float red = low * lowColor_r + mid * midColor_r + high * highColor_r;
        float green = low * lowColor_g + mid * midColor_g + high * highColor_g;
        float blue = low * lowColor_b + mid * midColor_b + high * highColor_b;

        // Normalize and draw
        float max = math_max3(red, green, blue);
        if (max > 0.0) {
            color.setRgbF(red / max, green / max, blue / max);
            pPainter->setPen(color);
            pPainter->drawLine(QPointF(currentCompletion / 2, -left),
                    QPointF(currentCompletion / 2, 0));
        }

        // Retrieve "raw" LMH values from waveform
        low = static_cast<float>(pWaveform->getLow(currentCompletion + 1));
        mid = static_cast<float>(pWaveform->getMid(currentCompletion + 1));
        high = static_cast<float>(pWaveform->getHigh(currentCompletion + 1));

        // Do matrix multiplication
        red = low * lowColor_r + mid * midColor_r + high * highColor_r;
        green = low * lowColor_g + mid * midColor_g + high * highColor_g;
        blue = low * lowColor_b + mid * midColor_b + high * highColor_b;

        // Normalize and draw
        max = math_max3(red, green, blue);
        if (max > 0.0) {
            color.setRgbF(red / max, green / max, blue / max);
            pPainter->setPen(color);
            pPainter->drawLine(QPointF(currentCompletion / 2, 0),
                    QPointF(currentCompletion / 2, right));
        }
    }

    // Evaluate waveform ratio peak
    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        m_waveformPeak = math_max3(
                m_waveformPeak,
                static_cast<float>(pWaveform->getAll(currentCompletion)),
                static_cast<float>(pWaveform->getAll(currentCompletion + 1)));
    }

    m_actualCompletion = nextCompletion;
}

void WOverview::paintText(const QString& text, QPainter* pPainter) {
    PainterScope painterScope(pPainter);
    m_lowColor.setAlphaF(0.5f);
    QPen lowColorPen(
            QBrush(m_lowColor), 1.25 * m_scaleFactor, Qt::SolidLine, Qt::RoundCap);
    pPainter->setPen(lowColorPen);
    QFont font = pPainter->font();
    QFontMetrics fm(font);

    int textWidth = fm.horizontalAdvance(text);

    if (textWidth > length()) {
        qreal pointSize = font.pointSizeF();
        pointSize = pointSize * (length() - 5 * m_scaleFactor) / textWidth;
        if (pointSize < 6 * m_scaleFactor) {
            pointSize = 6 * m_scaleFactor;
        }
        font.setPointSizeF(pointSize);
        pPainter->setFont(font);
    }
    if (m_orientation == Qt::Vertical) {
        pPainter->setTransform(QTransform(0, 1, -1, 0, width(), 0));
    }
    pPainter->drawText(QPointF(10 * m_scaleFactor, 12 * m_scaleFactor), text);
    pPainter->resetTransform();
}

double WOverview::samplePositionToSeconds(double sample) {
    double rate = m_pRateRatioControl->get();
    VERIFY_OR_DEBUG_ASSERT(rate != 0.0) {
        return 1;
    }

    double trackTime = sample /
            (m_trackSampleRateControl.get() * mixxx::kEngineChannelOutputCount);
    return trackTime / rate;
}

void WOverview::resizeEvent(QResizeEvent* pEvent) {
    Q_UNUSED(pEvent);
    m_maxPixelPos = length() - 1;

    m_devicePixelRatio = devicePixelRatioF();

    m_waveformImageScaled = QImage();
    m_diffGain = 0;
    Init();
}

void WOverview::dragEnterEvent(QDragEnterEvent* pEvent) {
    DragAndDropHelper::handleTrackDragEnterEvent(pEvent, m_group, m_pConfig);
}

void WOverview::dropEvent(QDropEvent* pEvent) {
    DragAndDropHelper::handleTrackDropEvent(pEvent, *this, m_group, m_pConfig);
}
