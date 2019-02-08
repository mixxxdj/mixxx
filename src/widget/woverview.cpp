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

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "woverview.h"
#include "wskincolor.h"
#include "widget/controlwidgetconnection.h"
#include "track/track.h"
#include "analyzer/analyzerprogress.h"
#include "util/math.h"
#include "util/timer.h"
#include "util/dnd.h"

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
        m_group(group),
        m_pConfig(pConfig),
        m_endOfTrack(false),
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
    m_trackSamplesControl =
            new ControlProxy(m_group, "track_samples", this);
    m_playControl = new ControlProxy(m_group, "play", this);
    setAcceptDrops(true);

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

    for (const auto& pMark: m_marks) {
        if (pMark->isValid()) {
            pMark->connectSamplePositionChanged(this,
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
    DEBUG_ASSERT(m_pCurrentTrack == pTrack);
    m_trackLoaded = true;
    update();
}

void WOverview::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
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
    update();
}

void WOverview::onMarkRangeChange(double /*v*/) {
    //qDebug() << "WOverview::onMarkRangeChange()" << v;
    update();
}

void WOverview::mouseMoveEvent(QMouseEvent* e) {
    if (m_orientation == Qt::Horizontal) {
        m_iPos = math_clamp(e->x(), 0, width() - 1);
    } else {
        m_iPos = math_clamp(e->y(), 0, height() - 1);
    }
    //qDebug() << "WOverview::mouseMoveEvent" << e->pos() << m_iPos;
    update();
}

void WOverview::mouseReleaseEvent(QMouseEvent* e) {
    mouseMoveEvent(e);
    double dValue = positionToValue(m_iPos);
    //qDebug() << "WOverview::mouseReleaseEvent" << e->pos() << m_iPos << ">>" << dValue;

    setControlParameterUp(dValue);
    m_bDrag = false;
}

void WOverview::mousePressEvent(QMouseEvent* e) {
    //qDebug() << "WOverview::mousePressEvent" << e->pos();
    mouseMoveEvent(e);
    m_bDrag = true;
}

void WOverview::paintEvent(QPaintEvent * /*unused*/) {
    //qDebug() << "WOverview::paintEvent";
    ScopedTimer t("WOverview::paintEvent");

    QPainter painter(this);
    painter.fillRect(rect(), m_qColorBackground);

    if (!m_backgroundPixmap.isNull()) {
        painter.drawPixmap(rect(), m_backgroundPixmap);
    }

    if (m_pCurrentTrack) {
        // Display viewer contour if end of track
        if (m_endOfTrack) {
            painter.setOpacity(0.8);
            painter.setPen(QPen(QBrush(m_endOfTrackColor), 1.5 * m_scaleFactor));
            painter.setBrush(QColor(0,0,0,0));
            painter.drawRect(rect().adjusted(0,0,-1,-1));
            painter.setOpacity(0.3);
            painter.setBrush(m_endOfTrackColor);
            painter.drawRect(rect().adjusted(1,1,-2,-2));
            painter.setOpacity(1);
        }

        // Draw Axis
        painter.setPen(QPen(m_signalColors.getAxesColor(), 1 * m_scaleFactor));
        if (m_orientation == Qt::Horizontal) {
            painter.drawLine(0, height() / 2, width(), height() / 2);
        } else {
            painter.drawLine(width() / 2 , 0, width() / 2, height());
        }

        // Draw waveform pixmap
        WaveformWidgetFactory* widgetFactory = WaveformWidgetFactory::instance();
        if (!m_waveformSourceImage.isNull()) {
            int diffGain;
            bool normalize = widgetFactory->isOverviewNormalized();
            if (normalize && m_pixmapDone && m_waveformPeak > 1) {
                diffGain = 255 - m_waveformPeak - 1;
            } else {
                const double visualGain = widgetFactory->getVisualGain(WaveformWidgetFactory::All);
                diffGain = 255.0 - 255.0 / visualGain;
            }

            if (m_diffGain != diffGain || m_waveformImageScaled.isNull()) {
                QRect sourceRect(0, diffGain, m_waveformSourceImage.width(),
                    m_waveformSourceImage.height() - 2 * diffGain);
                QImage croppedImage = m_waveformSourceImage.copy(sourceRect);
                if (m_orientation == Qt::Vertical) {
                    // Rotate pixmap
                    croppedImage = croppedImage.transformed(QTransform(0, 1, 1, 0, 0, 0));
                }
                m_waveformImageScaled = croppedImage.scaled(size(), Qt::IgnoreAspectRatio,
                                                            Qt::SmoothTransformation);
                m_diffGain = diffGain;
            }

            painter.drawImage(rect(), m_waveformImageScaled);

            // Overlay the played part of the overview-waveform with a skin defined color
            QColor playedOverlayColor = m_signalColors.getPlayedOverlayColor();
            if (playedOverlayColor.alpha() > 0) {
                if (m_orientation == Qt::Vertical) {
                    painter.fillRect(0, 0, m_waveformImageScaled.width(),  m_iPos, playedOverlayColor);
                } else {
                    painter.fillRect(0, 0, m_iPos, m_waveformImageScaled.height(), playedOverlayColor);
                }
            }
        }

        if ((m_analyzerProgress >= kAnalyzerProgressNone) &&
            (m_analyzerProgress < kAnalyzerProgressDone)) {
            // Paint analyzer Progress
            painter.setPen(QPen(m_signalColors.getAxesColor(), 3 * m_scaleFactor));

            if (m_analyzerProgress > kAnalyzerProgressNone) {
                if (m_orientation == Qt::Horizontal) {
                    painter.drawLine(
                            width() * m_analyzerProgress,
                            height() / 2,
                            width(),
                            height() / 2);
                } else {
                    painter.drawLine(
                            width() / 2 ,
                            height() * m_analyzerProgress,
                            width() / 2,
                            height());
                }
            }

            if (m_analyzerProgress <= kAnalyzerProgressHalf) { // remove text after progress by wf is recognizable
                if (m_trackLoaded) {
                    //: Text on waveform overview when file is playable but no waveform is visible
                    paintText(tr("Ready to play, analyzing .."), &painter);
                } else {
                    //: Text on waveform overview when file is cached from source
                    paintText(tr("Loading track .."), &painter);
                }
            } else if (m_analyzerProgress >= kAnalyzerProgressFinalizing) {
                //: Text on waveform overview during finalizing of waveform analysis
                paintText(tr("Finalizing .."), &painter);
            }
        } else if (!m_trackLoaded) {
            // This happens if the track samples are not loaded, but we have
            // a cached track
            //: Text on waveform overview when file is cached from source
            paintText(tr("Loading track .."), &painter);
        }

        double trackSamples = m_trackSamplesControl->get();
        if (m_trackLoaded && trackSamples > 0) {
            //qDebug() << "WOverview::paintEvent trackSamples > 0";
            const float offset = 1.0f;
            const float gain = static_cast<float>(length() - 2) / trackSamples;

            // Draw range (loop)
            for (auto&& currentMarkRange : m_markRanges) {
                // If the mark range is not active we should not draw it.
                if (!currentMarkRange.active()) {
                    continue;
                }

                // Active mark ranges by definition have starts/ends that are not
                // disabled.
                const double startValue = currentMarkRange.start();
                const double endValue = currentMarkRange.end();

                const float startPosition = offset + startValue * gain;
                const float endPosition = offset + endValue * gain;

                if (startPosition < 0.0 && endPosition < 0.0) {
                    continue;
                }

                if (currentMarkRange.enabled()) {
                    painter.setOpacity(0.4);
                    painter.setPen(currentMarkRange.m_activeColor);
                    painter.setBrush(currentMarkRange.m_activeColor);
                } else {
                    painter.setOpacity(0.2);
                    painter.setPen(currentMarkRange.m_disabledColor);
                    painter.setBrush(currentMarkRange.m_disabledColor);
                }

                // let top and bottom of the rect out of the widget
                if (m_orientation == Qt::Horizontal) {
                    painter.drawRect(QRectF(QPointF(startPosition, -2.0),
                                            QPointF(endPosition, height() + 1.0)));
                } else {
                    painter.drawRect(QRectF(QPointF(-2.0, startPosition),
                                            QPointF(width() + 1.0, endPosition)));
                }
            }

            // Draw markers (Cue & hotcues)
            QPen shadowPen(QBrush(m_qColorBackground), 2.5 * m_scaleFactor);

            QFont markerFont = painter.font();
            markerFont.setPixelSize(10 * m_scaleFactor);

            QFont shadowFont = painter.font();
            shadowFont.setWeight(99);
            shadowFont.setPixelSize(10 * m_scaleFactor);

            painter.setOpacity(0.9);

            for (const auto& currentMark: m_marks) {
                const WaveformMarkProperties& markProperties = currentMark->getProperties();
                if (currentMark->isValid() && currentMark->getSamplePosition() >= 0.0) {
                    //const float markPosition = 1.0 +
                    //        (currentMark.m_pointControl->get() / (float)m_trackSamplesControl->get()) * (float)(width()-2);
                    const float markPosition = offset + currentMark->getSamplePosition() * gain;

                    QLineF line;
                    if (m_orientation == Qt::Horizontal) {
                        line.setLine(markPosition, 0.0, markPosition, static_cast<float>(height()));
                    } else {
                        line.setLine(0.0, markPosition, static_cast<float>(width()), markPosition);
                    }
                    painter.setPen(shadowPen);
                    painter.drawLine(line);

                    painter.setPen(markProperties.m_color);
                    painter.drawLine(line);

                    if (!markProperties.m_text.isEmpty()) {
                        Qt::Alignment halign = markProperties.m_align & Qt::AlignHorizontal_Mask;
                        Qt::Alignment valign = markProperties.m_align & Qt::AlignVertical_Mask;
                        QFontMetricsF metric(markerFont);
                        QRectF textRect = metric.tightBoundingRect(markProperties.m_text);
                        QPointF textPoint;
                        if (m_orientation == Qt::Horizontal) {
                            if (halign == Qt::AlignLeft) {
                                textPoint.setX(markPosition - textRect.width());
                            } else if (halign == Qt::AlignHCenter) {
                                textPoint.setX(markPosition - textRect.width() / 2);
                            } else {  // AlignRight
                                textPoint.setX(markPosition + 0.5f);
                            }

                            if (valign == Qt::AlignTop) {
                                textPoint.setY(textRect.height() + 0.5f);
                            } else if (valign == Qt::AlignVCenter) {
                                textPoint.setY((textRect.height() + height()) / 2);
                            } else {  // AlignBottom
                                textPoint.setY(float(height()) - 0.5f);
                            }
                        } else {  // Vertical
                            if (halign == Qt::AlignLeft) {
                                textPoint.setX(1.0f);
                            } else if (halign == Qt::AlignHCenter) {
                                textPoint.setX((width() - textRect.width()) / 2);
                            } else {  // AlignRight
                                textPoint.setX(width() - textRect.width());
                            }

                            if (valign == Qt::AlignTop) {
                                textPoint.setY(markPosition - 1.0f);
                            } else if (valign == Qt::AlignVCenter) {
                                textPoint.setY(markPosition + textRect.height() / 2);
                            } else {  // AlignBottom
                                textPoint.setY(markPosition + metric.ascent());
                            }
                        }

                        painter.setPen(shadowPen);
                        painter.setFont(shadowFont);
                        painter.drawText(textPoint, markProperties.m_text);

                        painter.setPen(markProperties.m_textColor);
                        painter.setFont(markerFont);
                        painter.drawText(textPoint, markProperties.m_text);
                    }
                }
            }

            if (m_orientation == Qt::Vertical) {
                painter.setTransform(QTransform(0, 1, 1, 0, 0, 0));
            }

            // draw current position
            painter.setPen(QPen(QBrush(m_qColorBackground), 1 * m_scaleFactor));
            painter.setOpacity(0.5);
            painter.drawLine(m_iPos + 1, 0, m_iPos + 1, breadth());
            painter.drawLine(m_iPos - 1, 0, m_iPos - 1, breadth());

            painter.setPen(QPen(m_signalColors.getPlayPosColor(), 1 * m_scaleFactor));
            painter.setOpacity(1.0);
            painter.drawLine(m_iPos, 0, m_iPos, breadth());

            painter.drawLine(m_iPos - 2, 0, m_iPos, 2);
            painter.drawLine(m_iPos, 2, m_iPos + 2, 0);
            painter.drawLine(m_iPos - 2, 0, m_iPos + 2, 0);

            painter.drawLine(m_iPos - 2, breadth() - 1, m_iPos, breadth() - 3);
            painter.drawLine(m_iPos, breadth() - 3, m_iPos + 2, breadth() - 1);
            painter.drawLine(m_iPos - 2, breadth() - 1, m_iPos + 2, breadth() - 1);
        }
    }
    painter.end();
}

void WOverview::paintText(const QString &text, QPainter *painter) {
    QColor lowColor = m_signalColors.getLowColor();
    lowColor.setAlphaF(0.5);
    QPen lowColorPen(
            QBrush(lowColor), 1.25 * m_scaleFactor,
            Qt::SolidLine, Qt::RoundCap);
    painter->setPen(lowColorPen);
    QFont font = painter->font();
    QFontMetrics fm(font);
    int textWidth = fm.width(text);
    if (textWidth > length()) {
        qreal pointSize = font.pointSizeF();
        pointSize = pointSize * (length() - 5 * m_scaleFactor) / textWidth;
        if (pointSize < 6 * m_scaleFactor) {
            pointSize = 6 * m_scaleFactor;
        }
        font.setPointSizeF(pointSize);
        painter->setFont(font);
    }
    if (m_orientation == Qt::Vertical) {
        painter->setTransform(QTransform(0, 1, -1, 0, width(), 0));
    }
    painter->drawText(10 * m_scaleFactor, 12 * m_scaleFactor, text);
    painter->resetTransform();
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

    m_waveformImageScaled = QImage();
    m_diffGain = 0;
    Init();
}

void WOverview::dragEnterEvent(QDragEnterEvent* event) {
    if (DragAndDropHelper::allowLoadToPlayer(m_group,
                                             m_playControl->get() > 0.0,
                                             m_pConfig) &&
            DragAndDropHelper::dragEnterAccept(*event->mimeData(), m_group,
                                               true, false)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void WOverview::dropEvent(QDropEvent* event) {
    if (DragAndDropHelper::allowLoadToPlayer(m_group, m_playControl->get() > 0.0,
                                             m_pConfig)) {
        QList<QFileInfo> files = DragAndDropHelper::dropEventFiles(
                *event->mimeData(), m_group, true, false);
        if (!files.isEmpty()) {
            event->accept();
            emit(trackDropped(files.at(0).absoluteFilePath(), m_group));
            return;
        }
    }
    event->ignore();
}
