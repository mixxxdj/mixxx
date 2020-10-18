#include <QDomNode>
#include <QPainter>
#include <QPainterPath>

#include "waveform/renderers/waveformrendermark.h"

#include "control/controlproxy.h"
#include "engine/controls/cuecontrol.h"
#include "track/track.h"
#include "util/color/color.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "widget/wimagestore.h"
#include "util/painterscope.h"

namespace {
    const int kMaxCueLabelLength = 23;
}

WaveformRenderMark::WaveformRenderMark(
        WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer) {
}

void WaveformRenderMark::setup(const QDomNode& node, const SkinContext& context) {
    WaveformSignalColors signalColors = *m_waveformRenderer->getWaveformSignalColors();
    m_marks.setup(m_waveformRenderer->getGroup(), node, context, signalColors);
}

void WaveformRenderMark::draw(QPainter* painter, QPaintEvent* /*event*/) {
    PainterScope PainterScope(painter);
    // Maps mark objects to their positions in the widget.
    QMap<WaveformMarkPointer, int> marksOnScreen;
    /*
    //DEBUG
    for (int i = 0; i < m_markPoints.size(); i++) {
        if (m_waveformWidget->getTrackSamples())
            painter->drawText(40*i,12+12*(i%3),QString::number(m_markPoints[i]->get() / (double)m_waveformWidget->getTrackSamples()));
    }
    */

    painter->setWorldMatrixEnabled(false);

    for (auto& pMark: m_marks) {
        if (!pMark->isValid())
            continue;

        if (pMark->hasVisible() && !pMark->isVisible()) {
            continue;
        }

        // Generate image on first paint can't be done in setup since we need
        // render widget to be resized yet ...
        if (pMark->m_image.isNull()) {
            generateMarkImage(pMark);
        }

        double samplePosition = pMark->getSamplePosition();
        if (samplePosition != -1.0) {
            double currentMarkPoint =
                    m_waveformRenderer->transformSamplePositionInRendererWorld(samplePosition);
            if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
                // NOTE: vRince I guess image width is odd to display the center on the exact line !
                // external image should respect that ...
                const int markHalfWidth = pMark->m_image.width() / 2.0
                        / m_waveformRenderer->getDevicePixelRatio();

                // Check if the current point needs to be displayed.
                if (currentMarkPoint > -markHalfWidth && currentMarkPoint < m_waveformRenderer->getWidth() + markHalfWidth) {
                    int drawOffset = currentMarkPoint - markHalfWidth;
                    painter->drawImage(QPoint(drawOffset, 0), pMark->m_image);
                    marksOnScreen[pMark] = drawOffset;
                }
            } else {
                const int markHalfHeight = pMark->m_image.height() / 2.0;
                if (currentMarkPoint > -markHalfHeight &&
                        currentMarkPoint < m_waveformRenderer->getHeight() +
                                        markHalfHeight) {
                    int drawOffset = currentMarkPoint - markHalfHeight;
                    painter->drawImage(QPoint(0, drawOffset), pMark->m_image);
                    marksOnScreen[pMark] = drawOffset;
                }
            }
        }
    }
    m_waveformRenderer->setMarkPositions(marksOnScreen);
}

void WaveformRenderMark::onResize() {
    // Delete all marks' images. New images will be created on next paint.
    for (const auto& pMark : m_marks) {
        pMark->m_image = QImage();
    }
}

void WaveformRenderMark::onSetTrack() {
    slotCuesUpdated();

    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }
    connect(trackInfo.get(),
            &Track::cuesUpdated,
            this,
            &WaveformRenderMark::slotCuesUpdated);
}

void WaveformRenderMark::slotCuesUpdated() {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }

    QList<CuePointer> loadedCues = trackInfo->getCuePoints();
    for (const CuePointer& pCue : loadedCues) {
        int hotCue = pCue->getHotCue();
        if (hotCue == Cue::kNoHotCue) {
            continue;
        }

        // Here we assume no two cues can have the same hotcue assigned,
        // because WaveformMarkSet stores one mark for each hotcue.
        WaveformMarkPointer pMark = m_marks.getHotCueMark(hotCue);
        if (pMark.isNull()) {
            continue;
        }

        QString newLabel = pCue->getLabel();
        QColor newColor = mixxx::RgbColor::toQColor(pCue->getColor());
        if (pMark->m_text.isNull() || newLabel != pMark->m_text ||
                !pMark->fillColor().isValid() ||
                newColor != pMark->fillColor()) {
            pMark->m_text = newLabel;
            pMark->setBaseColor(newColor);
            generateMarkImage(pMark);
        }
    }
}

void WaveformRenderMark::generateMarkImage(WaveformMarkPointer pMark) {
    // Load the pixmap from file -- takes precedence over text.
    if (!pMark->m_pixmapPath.isEmpty()) {
        QString path = pMark->m_pixmapPath;
        QImage image = *WImageStore::getImage(path, scaleFactor());
        //QImage image = QImage(path);
        // If loading the image didn't fail, then we're done. Otherwise fall
        // through and render a label.
        if (!image.isNull()) {
            pMark->m_image =
                    image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            //WImageStore::correctImageColors(&pMark->m_image);
            return;
        }
    }

    QPainter painter;

    // Determine mark text.
    QString label = pMark->m_text;
    if (pMark->getHotCue() >= 0) {
        if (!label.isEmpty()) {
            label.prepend(": ");
        }
        label.prepend(QString::number(pMark->getHotCue() + 1));
        if (label.size() > kMaxCueLabelLength) {
            label = label.left(kMaxCueLabelLength - 3) + "...";
        }
    }

    //QFont font("Bitstream Vera Sans");
    //QFont font("Helvetica");
    QFont font; // Uses the application default
    font.setPointSizeF(10 * scaleFactor());
    font.setStretch(100);
    font.setWeight(75);

    QFontMetrics metrics(font);

    //fixed margin ...
    QRect wordRect = metrics.tightBoundingRect(label);
    const int marginX = 1;
    const int marginY = 1;
    wordRect.moveTop(marginX + 1);
    wordRect.moveLeft(marginY + 1);
    wordRect.setHeight(wordRect.height() + (wordRect.height() % 2));
    wordRect.setWidth(wordRect.width() + (wordRect.width()) % 2);
    //even wordrect to have an even Image >> draw the line in the middle !

    int labelRectWidth = wordRect.width() + 2 * marginX + 4;
    int labelRectHeight = wordRect.height() + 2 * marginY + 4;

    QRectF labelRect(0, 0, (float)labelRectWidth, (float)labelRectHeight);

    int width;
    int height;

    if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
        width = 2 * labelRectWidth + 1;
        height = m_waveformRenderer->getHeight();
    } else {
        width = m_waveformRenderer->getWidth();
        height = 2 * labelRectHeight + 1;
    }

    pMark->m_image = QImage(width * m_waveformRenderer->getDevicePixelRatio(),
            height * m_waveformRenderer->getDevicePixelRatio(),
            QImage::Format_ARGB32_Premultiplied);
    pMark->m_image.setDevicePixelRatio(
            m_waveformRenderer->getDevicePixelRatio());

    Qt::Alignment markAlignH = pMark->m_align & Qt::AlignHorizontal_Mask;
    Qt::Alignment markAlignV = pMark->m_align & Qt::AlignVertical_Mask;

    if (markAlignH == Qt::AlignHCenter) {
        labelRect.moveLeft((width - labelRectWidth) / 2);
    } else if (markAlignH == Qt::AlignRight) {
        labelRect.moveRight(width - 1);
    }

    if (markAlignV == Qt::AlignVCenter) {
        labelRect.moveTop((height - labelRectHeight) / 2);
    } else if (markAlignV == Qt::AlignBottom) {
        labelRect.moveBottom(height - 1);
    }

    pMark->m_label.setAreaRect(labelRect);

    // Fill with transparent pixels
    pMark->m_image.fill(QColor(0, 0, 0, 0).rgba());

    painter.begin(&pMark->m_image);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.setWorldMatrixEnabled(false);

    // Draw marker lines
    if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
        int middle = width / 2;
        pMark->m_linePosition = middle;
        if (markAlignH == Qt::AlignHCenter) {
            if (labelRect.top() > 0) {
                painter.setPen(pMark->fillColor());
                painter.drawLine(QLineF(middle, 0, middle, labelRect.top()));

                painter.setPen(pMark->borderColor());
                painter.drawLine(QLineF(middle - 1, 0, middle - 1, labelRect.top()));
                painter.drawLine(QLineF(middle + 1, 0, middle + 1, labelRect.top()));
            }

            if (labelRect.bottom() < height) {
                painter.setPen(pMark->fillColor());
                painter.drawLine(QLineF(middle, labelRect.bottom(), middle, height));

                painter.setPen(pMark->borderColor());
                painter.drawLine(QLineF(middle - 1, labelRect.bottom(), middle - 1, height));
                painter.drawLine(QLineF(middle + 1, labelRect.bottom(), middle + 1, height));
            }
        } else { // AlignLeft || AlignRight
            painter.setPen(pMark->fillColor());
            painter.drawLine(middle, 0, middle, height);

            painter.setPen(pMark->borderColor());
            painter.drawLine(middle - 1, 0, middle - 1, height);
            painter.drawLine(middle + 1, 0, middle + 1, height);
        }
    } else { // Vertical
        int middle = height / 2;
        pMark->m_linePosition = middle;
        if (markAlignV == Qt::AlignVCenter) {
            if (labelRect.left() > 0) {
                painter.setPen(pMark->fillColor());
                painter.drawLine(QLineF(0, middle, labelRect.left(), middle));

                painter.setPen(pMark->borderColor());
                painter.drawLine(QLineF(0, middle - 1, labelRect.left(), middle - 1));
                painter.drawLine(QLineF(0, middle + 1, labelRect.left(), middle + 1));
            }

            if (labelRect.right() < width) {
                painter.setPen(pMark->fillColor());
                painter.drawLine(QLineF(labelRect.right(), middle, width, middle));

                painter.setPen(pMark->borderColor());
                painter.drawLine(QLineF(labelRect.right(), middle - 1, width, middle - 1));
                painter.drawLine(QLineF(labelRect.right(), middle + 1, width, middle + 1));
            }
        } else { // AlignTop || AlignBottom
            painter.setPen(pMark->fillColor());
            painter.drawLine(0, middle, width, middle);

            painter.setPen(pMark->borderColor());
            painter.drawLine(0, middle - 1, width, middle - 1);
            painter.drawLine(0, middle + 1, width, middle + 1);
        }
    }

    // Draw the label rect
    painter.setPen(pMark->borderColor());
    painter.setBrush(QBrush(pMark->fillColor()));
    painter.drawRoundedRect(labelRect, 2.0, 2.0);

    // Draw text
    painter.setBrush(QBrush(QColor(0, 0, 0, 0)));
    painter.setFont(font);
    painter.setPen(pMark->labelColor());
    painter.drawText(labelRect, Qt::AlignCenter, label);
}
