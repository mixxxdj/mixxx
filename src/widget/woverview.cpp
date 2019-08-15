//
// C++ Implementation: woverview
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QBrush>
#include <QtDebug>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QtDebug>
#include <QPixmap>
#include <QUrl>
#include <QMimeData>

#include "analyzer/analyzerprogress.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "engine/engine.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/compatibility.h"
#include "util/dnd.h"
#include "util/duration.h"
#include "util/math.h"
#include "util/painterscope.h"
#include "util/timer.h"
#include "widget/controlwidgetconnection.h"
#include "woverview.h"
#include "wskincolor.h"

#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

WOverview::WOverview(
        const char* group,
        PlayerManager* pPlayerManager,
        UserSettingsPointer pConfig,
        QWidget* parent) :
        WWidget(parent),
        m_actualCompletion(0),
        m_pixmapDone(false),
        m_waveformPeak(-1.0),
        m_diffGain(0),
        m_devicePixelRatio(1.0),
        m_group(group),
        m_pConfig(pConfig),
        m_endOfTrack(false),
        m_pCueMenu(std::make_unique<CueMenu>(this)),
        m_bDrag(false),
        m_iPos(0),
        m_orientation(Qt::Horizontal),
        m_a(1.0),
        m_b(0.0),
        m_analyzerProgress(kAnalyzerProgressUnknown),
        m_trackLoaded(false),
        m_scaleFactor(1.0) {
    m_endOfTrackControl = new ControlProxy(
            m_group, "end_of_track", this);
    m_endOfTrackControl->connectValueChanged(this, &WOverview::onEndOfTrackChange);
    m_pRateDirControl = new ControlProxy(m_group, "rate_dir", this);
    m_pRateRangeControl = new ControlProxy(m_group, "rateRange", this);
    m_pRateSliderControl = new ControlProxy(m_group, "rate", this);
    // Needed to recalculate range durations when rate slider is moved without the deck playing
    // TODO: connect to rate_ratio instead in PR #1765
    m_pRateSliderControl->connectValueChanged(this, &WOverview::onRateSliderChange);
    m_trackSampleRateControl = new ControlProxy(m_group, "track_samplerate", this);
    m_trackSamplesControl =
            new ControlProxy(m_group, "track_samples", this);
    setAcceptDrops(true);

    setMouseTracking(true);

    connect(pPlayerManager, &PlayerManager::trackAnalyzerProgress,
            this, &WOverview::onTrackAnalyzerProgress);
}

void WOverview::setup(const QDomNode& node, const SkinContext& context) {
    m_scaleFactor = context.getScaleFactor();
    m_signalColors.setup(node, context);

    m_qColorBackground = m_signalColors.getBgColor();

    // Clear the background pixmap, if it exists.
    m_backgroundPixmap = QPixmap();
    m_backgroundPixmapPath = context.selectString(node, "BgPixmap");
    if (!m_backgroundPixmapPath.isEmpty()) {
        m_backgroundPixmap = *WPixmapStore::getPixmapNoCache(
                context.makeSkinPath(m_backgroundPixmapPath),
                m_scaleFactor);
    }

    m_endOfTrackColor = QColor(200, 25, 20);
    const QString endOfTrackColorName = context.selectString(node, "EndOfTrackColor");
    if (!endOfTrackColorName.isNull()) {
        m_endOfTrackColor.setNamedColor(endOfTrackColorName);
        m_endOfTrackColor = WSkinColor::getCorrectColor(m_endOfTrackColor);
    }

    // setup hotcues and cue and loop(s)
    m_marks.setup(m_group, node, context, m_signalColors);
    WaveformMarkPointer defaultMark(m_marks.getDefaultMark());
    QColor defaultColor = defaultMark
            ? defaultMark->fillColor()
            : m_signalColors.getAxesColor();
    m_predefinedColorsRepresentation = context.getCueColorRepresentation(node, defaultColor);

    for (const auto& pMark: m_marks) {
        if (pMark->isValid()) {
            pMark->connectSamplePositionChanged(this,
                    &WOverview::onMarkChanged);
        }
        if (pMark->hasVisible()) {
            pMark->connectVisibleChanged(this,
                    &WOverview::onMarkChanged);
        }
    }

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

    QString orientationString = context.selectString(node, "Orientation").toLower();
    if (orientationString == "vertical") {
        m_orientation = Qt::Vertical;
    } else {
        m_orientation = Qt::Horizontal;
    }

    //qDebug() << "WOverview : m_marks" << m_marks.size();
    //qDebug() << "WOverview : m_markRanges" << m_markRanges.size();
    if (!m_connections.isEmpty()) {
        ControlParameterWidgetConnection* defaultConnection = m_connections.at(0);
        if (defaultConnection) {
            if (defaultConnection->getEmitOption() &
                    ControlParameterWidgetConnection::EMIT_DEFAULT) {
                // ON_PRESS means here value change on mouse move during press
                defaultConnection->setEmitOption(
                        ControlParameterWidgetConnection::EMIT_ON_RELEASE);
            }
        }
    }
}

void WOverview::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dValue);
    if (!m_bDrag) {
        // Calculate handle position. Clamp the value within 0-1 because that's
        // all we represent with this widget.
        dParameter = math_clamp(dParameter, 0.0, 1.0);

        int iPos = valueToPosition(dParameter);
        if (iPos != m_iPos) {
            m_iPos = iPos;
            //qDebug() << "WOverview::onConnectedControlChanged" << dParameter << ">>" << m_iPos;
            update();
        }
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
        disconnect(m_pCurrentTrack.get(), SIGNAL(waveformSummaryUpdated()),
                   this, SLOT(slotWaveformSummaryUpdated()));
    }

    m_waveformSourceImage = QImage();
    m_analyzerProgress = kAnalyzerProgressUnknown;
    m_actualCompletion = 0;
    m_waveformPeak = -1.0;
    m_pixmapDone = false;
    m_trackLoaded = false;
    m_endOfTrack = false;

    if (pNewTrack) {
        m_pCurrentTrack = pNewTrack;
        m_pWaveform = pNewTrack->getWaveformSummary();

        connect(pNewTrack.get(), SIGNAL(waveformSummaryUpdated()),
                this, SLOT(slotWaveformSummaryUpdated()));
        slotWaveformSummaryUpdated();
        connect(pNewTrack.get(), SIGNAL(cuesUpdated()),
                this, SLOT(receiveCuesUpdated()));
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

void WOverview::onMarkChanged(double /*v*/) {
    //qDebug() << "WOverview::onMarkChanged()" << v;
    if (m_pCurrentTrack) {
        updateCues(m_pCurrentTrack->getCuePoints());
        update();
    }
}

void WOverview::onMarkRangeChange(double /*v*/) {
    //qDebug() << "WOverview::onMarkRangeChange()" << v;
    update();
}

void WOverview::onRateSliderChange(double /*v*/) {
    update();
}

void WOverview::updateCues(const QList<CuePointer> &loadedCues) {
    m_marksToRender.clear();
    for (CuePointer currentCue: loadedCues) {
        const WaveformMarkPointer pMark = m_marks.getHotCueMark(currentCue->getHotCue());

        if (pMark != nullptr && pMark->isValid() && pMark->isVisible()
            && pMark->getSamplePosition() >= 0.0) {
            QColor newColor = m_predefinedColorsRepresentation.representationFor(currentCue->getColor());
            if (newColor != pMark->fillColor() || newColor != pMark->m_textColor) {
                pMark->setBaseColor(newColor);
            }

            int hotcueNumber = currentCue->getHotCue();
            if (currentCue->getType() == Cue::CUE && hotcueNumber != WaveformMark::kNoHotCue) {
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

            m_marksToRender.append(pMark);
        }
    }
    std::sort(m_marksToRender.begin(), m_marksToRender.end());
}

// connecting the tracks cuesUpdated and onMarkChanged is not possible
// due to the incompatible signatures. This is a "wrapper" workaround
void WOverview::receiveCuesUpdated() {
    onMarkChanged(0);
}

void WOverview::mouseMoveEvent(QMouseEvent* e) {
    if (m_bDrag) {
        if (m_orientation == Qt::Horizontal) {
            m_iPos = math_clamp(e->x(), 0, width() - 1);
        } else {
            m_iPos = math_clamp(e->y(), 0, height() - 1);
        }
    }

    // Without tracking the first hovered WaveformMark, the user could hover one
    // mark, then drag the cursor over another, and the second one's label text
    // would be drawn under the first.
    bool firstMarkHovered = false;
    for (const auto& pMark : m_marksToRender) {
        if (pMark->m_renderedArea.contains(e->pos()) && !firstMarkHovered) {
            pMark->m_bMouseHovering = true;
            firstMarkHovered = true;
        } else {
            pMark->m_bMouseHovering = false;
        }
    }

    //qDebug() << "WOverview::mouseMoveEvent" << e->pos() << m_iPos;
    update();
}

void WOverview::mouseReleaseEvent(QMouseEvent* e) {
    mouseMoveEvent(e);
    double dValue = positionToValue(m_iPos);
    //qDebug() << "WOverview::mouseReleaseEvent" << e->pos() << m_iPos << ">>" << dValue;

    if (e->button() == Qt::LeftButton) {
        // If a hotcue label is being hovered, jump to it instead of the point
        // under the cursor.
        for (const auto& pMark : m_marksToRender) {
            if (pMark->m_bMouseHovering) {
                dValue = pMark->getSamplePosition() / m_trackSamplesControl->get();
                break;
            }
        }
        setControlParameterUp(dValue);
        m_bDrag = false;
        // Do not seek when releasing a right click. This is important to
        // prevent accidental seeking when trying to right click a hotcue.
    }
}

void WOverview::mousePressEvent(QMouseEvent* e) {
    //qDebug() << "WOverview::mousePressEvent" << e->pos();
    mouseMoveEvent(e);
    bool dragging = true;
    if (m_pCurrentTrack != nullptr) {
        QList<CuePointer> cueList = m_pCurrentTrack->getCuePoints();
        for (const auto& pMark : m_marksToRender) {
            if (pMark->m_bMouseHovering) {
                if (e->button() == Qt::LeftButton) {
                    dragging = false;
                } else {
                    // Currently the only way WaveformMarks can be associated
                    // with their respective Cue objects is by using the hotcue
                    // number. If cues without assigned hotcue are drawn on
                    // WOverview in the future, another way to associate
                    // WaveformMarks with Cues will need to be implemented.
                    CuePointer pHoveredCue;
                    for (const auto& pCue : cueList) {
                        if (pCue->getHotCue() == pMark->getHotCue()) {
                            pHoveredCue = pCue;
                            break;
                        }
                    }
                    VERIFY_OR_DEBUG_ASSERT(pHoveredCue != nullptr) {
                        continue;
                    }
                    m_pCueMenu->setCue(pHoveredCue);
                    m_pCueMenu->setTrack(m_pCurrentTrack);
                    m_pCueMenu->popup(e->globalPos());
                }
            }
        }
    }
    if (e->button() == Qt::LeftButton) {
        m_bDrag = dragging;
    }
}

void WOverview::leaveEvent(QEvent* e) {
    Q_UNUSED(e);
    for (const auto& pMark : m_marks) {
        pMark->m_bMouseHovering = false;
    }
    m_bDrag = false;
    update();
}

void WOverview::paintEvent(QPaintEvent * /*unused*/) {
    ScopedTimer t("WOverview::paintEvent");

    QPainter painter(this);
    painter.fillRect(rect(), m_qColorBackground);

    if (!m_backgroundPixmap.isNull()) {
        painter.drawPixmap(rect(), m_backgroundPixmap);
    }

    if (m_pCurrentTrack) {
        // Refer to util/ScopePainter.h to understand the semantics of
        // ScopePainter.
        drawEndOfTrackBackground(&painter);
        drawAxis(&painter);
        drawWaveformPixmap(&painter);
        drawEndOfTrackFrame(&painter);
        drawAnalyzerProgress(&painter);

        double trackSamples = m_trackSamplesControl->get();
        if (m_trackLoaded && trackSamples > 0) {
            const float offset = 1.0f;
            const float gain = static_cast<float>(length() - 2) / m_trackSamplesControl->get();

            drawRangeMarks(&painter, offset, gain);
            drawMarks(&painter, offset, gain);
            drawCurrentPosition(&painter);
        }
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
    pPainter->setPen(QPen(m_signalColors.getAxesColor(), 1 * m_scaleFactor));
    if (m_orientation == Qt::Horizontal) {
        pPainter->drawLine(0, height() / 2, width(), height() / 2);
    } else {
        pPainter->drawLine(width() / 2, 0, width() / 2, height());
    }
}

void WOverview::drawWaveformPixmap(QPainter* pPainter) {
    WaveformWidgetFactory* widgetFactory = WaveformWidgetFactory::instance();
    if (!m_waveformSourceImage.isNull()) {
        PainterScope painterScope(pPainter);
        int diffGain;
        bool normalize = widgetFactory->isOverviewNormalized();
        if (normalize && m_pixmapDone && m_waveformPeak > 1) {
            diffGain = 255 - m_waveformPeak - 1;
        } else {
            const double visualGain = widgetFactory->getVisualGain(WaveformWidgetFactory::All);
            diffGain = 255.0 - 255.0 / visualGain;
        }

        if (m_diffGain != diffGain || m_waveformImageScaled.isNull()) {
            QRect sourceRect(0, diffGain, m_waveformSourceImage.width(), m_waveformSourceImage.height() - 2 * diffGain);
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

        // Overlay the played part of the overview-waveform with a skin defined color
        QColor playedOverlayColor = m_signalColors.getPlayedOverlayColor();
        if (playedOverlayColor.alpha() > 0) {
            if (m_orientation == Qt::Vertical) {
                pPainter->fillRect(0, 0, m_waveformImageScaled.width(), m_iPos, playedOverlayColor);
            } else {
                pPainter->fillRect(0, 0, m_iPos, m_waveformImageScaled.height(), playedOverlayColor);
            }
        }
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
        pPainter->setPen(QPen(m_signalColors.getAxesColor(), 3 * m_scaleFactor));

        if (m_analyzerProgress > kAnalyzerProgressNone) {
            if (m_orientation == Qt::Horizontal) {
                pPainter->drawLine(
                        width() * m_analyzerProgress,
                        height() / 2,
                        width(),
                        height() / 2);
            } else {
                pPainter->drawLine(
                        width() / 2,
                        height() * m_analyzerProgress,
                        width() / 2,
                        height());
            }
        }

        if (m_analyzerProgress <= kAnalyzerProgressHalf) { // remove text after progress by wf is recognizable
            if (m_trackLoaded) {
                //: Text on waveform overview when file is playable but no waveform is visible
                paintText(tr("Ready to play, analyzing .."), pPainter);
            } else {
                //: Text on waveform overview when file is cached from source
                paintText(tr("Loading track .."), pPainter);
            }
        } else if (m_analyzerProgress >= kAnalyzerProgressFinalizing) {
            //: Text on waveform overview during finalizing of waveform analysis
            paintText(tr("Finalizing .."), pPainter);
        }
    } else if (!m_trackLoaded) {
        // This happens if the track samples are not loaded, but we have
        // a cached track
        //: Text on waveform overview when file is cached from source
        paintText(tr("Loading track .."), pPainter);
    }
}

void WOverview::drawRangeMarks(QPainter* pPainter, const float& offset, const float& gain) {
    for (auto&& markRange : m_markRanges) {
        if (!markRange.active() || !markRange.visible()) {
            continue;
        }

        // Active mark ranges by definition have starts/ends that are not
        // disabled.
        const double startValue = markRange.start();
        const double endValue = markRange.end();

        const float startPosition = offset + startValue * gain;
        const float endPosition = offset + endValue * gain;

        if (startPosition < 0.0 && endPosition < 0.0) {
            continue;
        }

        PainterScope painterScope(pPainter);

        if (markRange.enabled()) {
            pPainter->setOpacity(0.4);
            pPainter->setPen(markRange.m_activeColor);
            pPainter->setBrush(markRange.m_activeColor);
        } else {
            pPainter->setOpacity(0.2);
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

        // draw duration of range
        if (markRange.showDuration()) {
            QString duration = mixxx::Duration::formatTime(
                    samplePositionToSeconds(endValue - startValue));

            QFontMetrics fm(pPainter->font());
            int textWidth = fm.width(duration);
            float padding = 3.0;
            float x;

            WaveformMarkRange::DurationTextLocation textLocation = markRange.durationTextLocation();
            if (textLocation == WaveformMarkRange::DurationTextLocation::Before) {
                x = startPosition - textWidth - padding;
            } else {
                x = endPosition + padding;
            }

            // Ensure the right end of the text does not get cut off by
            // the end of the track
            if (x + textWidth > width()) {
                x = width() - textWidth;
            }

            pPainter->setOpacity(1.0);
            pPainter->setPen(markRange.m_durationTextColor);
            pPainter->drawText(QPointF(x, fm.ascent()), duration);
        }
    }
}

void WOverview::drawMarks(QPainter* pPainter, const float offset, const float gain) {
    QFont markerFont = pPainter->font();
    markerFont.setPixelSize(10 * m_scaleFactor);

    QFont shadowFont = pPainter->font();
    shadowFont.setWeight(99);
    shadowFont.setPixelSize(10 * m_scaleFactor);

    // Text labels are rendered so they do not overlap with other WaveformMark's
    // labels. If the text would be too wide, it is elided. However, the user
    // can hover the mouse cursor over a label to show the whole label text,
    // temporarily hiding any following labels that would be drawn over it.
    // This requires looping over the WaveformMarks twice and the marks must be
    // sorted in the order they appear on the waveform.
    // In the first loop, the lines are drawn and the text to render plus its
    // location are calculated. The text must be drawn in the second loop to
    // prevent the lines of following WaveformMarks getting drawn over it.

    QList<QString> textToRender;
    QRectF expandedLabelRect;

    for (int i = 0; i < m_marksToRender.size(); ++i) {
        WaveformMarkPointer pMark = m_marksToRender.at(i);
        PainterScope painterScope(pPainter);

        //const float markPosition = 1.0 +
        //        (m_marksToRender.at(i).m_pointControl->get() / (float)m_trackSamplesControl->get()) * (float)(width()-2);
        const float markPosition = offset + m_marksToRender.at(i)->getSamplePosition() * gain;

        QPen shadowPen(QBrush(pMark->borderColor()), 2.5 * m_scaleFactor);

        QLineF line;
        if (m_orientation == Qt::Horizontal) {
            line.setLine(markPosition, 0.0, markPosition, static_cast<float>(height()));
        } else {
            line.setLine(0.0, markPosition, static_cast<float>(width()), markPosition);
        }
        pPainter->setPen(shadowPen);
        pPainter->drawLine(line);

        pPainter->setPen(pMark->fillColor());
        pPainter->drawLine(line);

        if (!pMark->m_text.isEmpty()) {
            Qt::Alignment halign = pMark->m_align & Qt::AlignHorizontal_Mask;
            Qt::Alignment valign = pMark->m_align & Qt::AlignVertical_Mask;

            QFontMetricsF metric(markerFont);
            QString text = pMark->m_text;

            // Only allow the text to overlap the following mark if the mouse is
            // hovering over it. Otherwise, elide it if it would render over
            // the next label.
            if (!pMark->m_bMouseHovering && i < m_marksToRender.size()-1) {
                const float nextMarkPosition = offset + m_marksToRender.at(i+1)->getSamplePosition() * gain;
                text = metric.elidedText(text, Qt::ElideRight, nextMarkPosition - markPosition - 5);
            }
            textToRender.append(text);

            QRectF textRect = metric.boundingRect(text);
            QPointF textPoint;
            if (m_orientation == Qt::Horizontal) {
                if (halign == Qt::AlignLeft) {
                    textPoint.setX(markPosition - textRect.width());
                } else if (halign == Qt::AlignHCenter) {
                    textPoint.setX(markPosition - textRect.width() / 2);
                } else { // AlignRight
                    textPoint.setX(markPosition + 0.5f);
                }

                if (valign == Qt::AlignTop) {
                    textPoint.setY(textRect.height() + 0.5f);
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
                    textPoint.setY(markPosition + metric.ascent());
                }
            }

            // QPainter::drawText starts drawing with the given QPointF as
            // the bottom left of the text, but QRectF::moveTo takes the new
            // top left of the QRectF.
            QPointF textTopLeft = QPointF(textPoint.x(), textPoint.y() - metric.height());
            textRect.moveTo(textTopLeft);
            pMark->m_renderedArea = textRect;

            if (pMark->m_bMouseHovering) {
                expandedLabelRect = textRect;
            }
        } else {
            // Placeholder to keep order
            textToRender.append(QString());
        }
    }

    for (int n = 0; n < m_marksToRender.size(); ++n) {
        WaveformMarkPointer pMark = m_marksToRender.at(n);
        QPen shadowPen(QBrush(pMark->borderColor()), 2.5 * m_scaleFactor);
        if (!pMark->m_renderedArea.intersects(expandedLabelRect)
            || pMark->m_bMouseHovering) {
            pPainter->setPen(shadowPen);
            pPainter->setFont(shadowFont);
            pPainter->drawText(pMark->m_renderedArea.bottomLeft(), textToRender.at(n));

            pPainter->setPen(pMark->m_textColor);
            pPainter->setFont(markerFont);
            pPainter->drawText(pMark->m_renderedArea.bottomLeft(), textToRender.at(n));
        }
        if (pMark->m_bMouseHovering) {
            // Show cue position when hovered
            // TODO: hide duration of intro/outro if the cue position text would
            // overlap
            double markPosition = m_marksToRender.at(n)->getSamplePosition();
            double markTime = samplePositionToSeconds(markPosition);
            QFontMetricsF metric(markerFont);
            Qt::Alignment valign = pMark->m_align & Qt::AlignVertical_Mask;
            QPointF textPoint(pMark->m_renderedArea.bottomLeft());
            if (valign == Qt::AlignTop) {
                textPoint.setY(float(height()) - 0.5f);
            } else {
                textPoint.setY(metric.height());
            }

            pPainter->setPen(pMark->m_textColor);
            pPainter->setFont(markerFont);
            pPainter->drawText(textPoint, mixxx::Duration::formatTime(markTime));
        }
    }
}

void WOverview::drawCurrentPosition(QPainter* pPainter) {
    PainterScope painterScope(pPainter);

    if (m_orientation == Qt::Vertical) {
        pPainter->setTransform(QTransform(0, 1, 1, 0, 0, 0));
    }

    pPainter->setPen(QPen(QBrush(m_qColorBackground), 1 * m_scaleFactor));
    pPainter->setOpacity(0.5);
    pPainter->drawLine(m_iPos + 1, 0, m_iPos + 1, breadth());
    pPainter->drawLine(m_iPos - 1, 0, m_iPos - 1, breadth());

    pPainter->setPen(QPen(m_signalColors.getPlayPosColor(), 1 * m_scaleFactor));
    pPainter->setOpacity(1.0);
    pPainter->drawLine(m_iPos, 0, m_iPos, breadth());

    pPainter->drawLine(m_iPos - 2, 0, m_iPos, 2);
    pPainter->drawLine(m_iPos, 2, m_iPos + 2, 0);
    pPainter->drawLine(m_iPos - 2, 0, m_iPos + 2, 0);

    pPainter->drawLine(m_iPos - 2, breadth() - 1, m_iPos, breadth() - 3);
    pPainter->drawLine(m_iPos, breadth() - 3, m_iPos + 2, breadth() - 1);
    pPainter->drawLine(m_iPos - 2, breadth() - 1, m_iPos + 2, breadth() - 1);
}

void WOverview::paintText(const QString& text, QPainter* pPainter) {
    PainterScope painterScope(pPainter);
    QColor lowColor = m_signalColors.getLowColor();
    lowColor.setAlphaF(0.5);
    QPen lowColorPen(
            QBrush(lowColor), 1.25 * m_scaleFactor,
            Qt::SolidLine, Qt::RoundCap);
    pPainter->setPen(lowColorPen);
    QFont font = pPainter->font();
    QFontMetrics fm(font);
    int textWidth = fm.width(text);
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
    pPainter->drawText(10 * m_scaleFactor, 12 * m_scaleFactor, text);
    pPainter->resetTransform();
}

double WOverview::samplePositionToSeconds(double sample) {
    // TODO: replace with rate_ratio in PR #1765
    double rateRatio = 1.0 + m_pRateDirControl->get()
            * m_pRateRangeControl->get() * m_pRateSliderControl->get();
    return sample / m_trackSampleRateControl->get()
            / mixxx::kEngineChannelCount / rateRatio;
}

void WOverview::resizeEvent(QResizeEvent * /*unused*/) {
    // Play-position potmeters range from 0 to 1 but they allow out-of-range
    // sets. This is to give VC access to the pre-roll area.
    const double kMaxPlayposRange = 1.0;
    const double kMinPlayposRange = 0.0;

    // Values of zero and one in normalized space.
    const double zero = (0.0 - kMinPlayposRange) / (kMaxPlayposRange - kMinPlayposRange);
    const double one = (1.0 - kMinPlayposRange) / (kMaxPlayposRange - kMinPlayposRange);

    // These coefficients convert between widget space and normalized value
    // space.
    m_a = (length() - 1) / (one - zero);
    m_b = zero * m_a;

    m_devicePixelRatio = getDevicePixelRatioF(this);

    m_waveformImageScaled = QImage();
    m_diffGain = 0;
    Init();
}

void WOverview::dragEnterEvent(QDragEnterEvent* event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_group, m_pConfig);
}
void WOverview::dropEvent(QDropEvent* event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_group, m_pConfig);
}
