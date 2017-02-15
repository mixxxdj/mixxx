#include <QDomNode>
#include <QPainter>
#include <QPainterPath>

#include "waveform/renderers/waveformrendermark.h"

#include "control/controlproxy.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "widget/wimagestore.h"

namespace {
    const int kMaxCueLabelLength = 23;
}

WaveformRenderMark::WaveformRenderMark(WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer) {
}

void WaveformRenderMark::setup(const QDomNode& node, const SkinContext& context) {
    m_marks.setup(m_waveformRenderer->getGroup(), node, context,
                  *m_waveformRenderer->getWaveformSignalColors());
}

void WaveformRenderMark::draw(QPainter* painter, QPaintEvent* /*event*/) {
    painter->save();

    /*
    //DEBUG
    for (int i = 0; i < m_markPoints.size(); i++) {
        if (m_waveformWidget->getTrackSamples())
            painter->drawText(40*i,12+12*(i%3),QString::number(m_markPoints[i]->get() / (double)m_waveformWidget->getTrackSamples()));
    }
    */

    painter->setWorldMatrixEnabled(false);

    for (int i = 0; i < m_marks.size(); i++) {
        WaveformMarkPointer mark = m_marks[i];

        if (!mark->isValid())
            continue;

        // Generate image on first paint can't be done in setup since we need
        // render widget to be resized yet ...
        if (mark->m_image.isNull()) {
            generateMarkImage(mark.data());
        }

        double samplePosition = mark->getSamplePosition();
        if (samplePosition != -1.0) {
            double currentMarkPoint =
                    m_waveformRenderer->transformSamplePositionInRendererWorld(samplePosition);

            if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
                // NOTE: vRince I guess image width is odd to display the center on the exact line !
                // external image should respect that ...
                const int markHalfWidth = mark->m_image.width() / 2.0;

                // Check if the current point need to be displayed
                if (currentMarkPoint > -markHalfWidth && currentMarkPoint < m_waveformRenderer->getWidth() + markHalfWidth) {
                    painter->drawImage(QPoint(currentMarkPoint - markHalfWidth, 0), mark->m_image);
                }
            } else {
                const int markHalfHeight = mark->m_image.height() / 2.0;

                if (currentMarkPoint > -markHalfHeight && currentMarkPoint < m_waveformRenderer->getHeight() + markHalfHeight) {
                    painter->drawImage(QPoint(0,currentMarkPoint - markHalfHeight), mark->m_image);
                }
            }
        }
    }

    painter->restore();
}

void WaveformRenderMark::onResize() {
    // Delete all marks' images. New images will be created on next paint.
    for (int i = 0; i < m_marks.size(); i++) {
        m_marks[i]->m_image = QImage();
    }
}

void WaveformRenderMark::onSetTrack() {
    slotCuesUpdated();

    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }
    connect(trackInfo.get(), SIGNAL(cuesUpdated(void)),
                  this, SLOT(slotCuesUpdated(void)));
}

void WaveformRenderMark::slotCuesUpdated() {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo){
        return;
    }

    QList<CuePointer> loadedCues = trackInfo->getCuePoints();
    for (const CuePointer pCue: loadedCues) {
        int hotCue = pCue->getHotCue();
        if (hotCue == -1) {
            continue;
        }

        QString newLabel = pCue->getLabel();
        QColor newColor = pCue->getColor();

        // Here we assume no two cues can have the same hotcue assigned,
        // because WaveformMarkSet stores one mark for each hotcue.
        WaveformMark* pMark = m_marks.getHotCueMark(hotCue).data();
        WaveformMarkProperties markProperties = pMark->getProperties();
        if (markProperties.m_text.isNull() || newLabel != markProperties.m_text ||
            !markProperties.m_color.isValid() || newColor != markProperties.m_color) {
            markProperties.m_text = newLabel;
            markProperties.m_color = newColor;
            pMark->setProperties(markProperties);
            generateMarkImage(pMark);
        }
    }
}

void WaveformRenderMark::generateMarkImage(WaveformMark* pMark) {
    const WaveformMarkProperties& markProperties = pMark->getProperties();

    // Load the pixmap from file -- takes precedence over text.
    if (!markProperties.m_pixmapPath.isEmpty()) {
        QString path = markProperties.m_pixmapPath;
        QImage image = QImage(path);
        // If loading the image didn't fail, then we're done. Otherwise fall
        // through and render a label.
        if (!image.isNull()) {
            pMark->m_image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            WImageStore::correctImageColors(&pMark->m_image);
            return;
        }
    }

    QPainter painter;

    // If no text is provided, leave m_markImage as a null image
    if (!markProperties.m_text.isNull()) {
        // Determine mark text.
        QString label = markProperties.m_text;
        if (pMark->m_iHotCue != -1) {
            if (!label.isEmpty()) {
                label.prepend(": ");
            }
            label.prepend(QString::number(pMark->m_iHotCue));
            if (label.size() > kMaxCueLabelLength) {
                label = label.left(kMaxCueLabelLength - 3) + "...";
            }
        }

        //QFont font("Bitstream Vera Sans");
        //QFont font("Helvetica");
        QFont font; // Uses the application default
        font.setPointSize(10);
        font.setStretch(100);
        font.setWeight(75);

        QFontMetrics metrics(font);

        //fixed margin ...
        QRect wordRect = metrics.tightBoundingRect(label);
        const int marginX = 1;
        const int marginY = 1;
        wordRect.moveTop(marginX + 1);
        wordRect.moveLeft(marginY + 1);
        wordRect.setHeight(wordRect.height() + (wordRect.height()%2));
        wordRect.setWidth(wordRect.width() + (wordRect.width())%2);
        //even wordrect to have an even Image >> draw the line in the middle !

        int labelRectWidth = wordRect.width() + 2 * marginX + 4;
        int labelRectHeight = wordRect.height() + 2 * marginY + 4 ;

        QRectF labelRect(0, 0,
                (float)labelRectWidth, (float)labelRectHeight);

        int width;
        int height;

        if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
            width = 2 * labelRectWidth + 1;
            height = m_waveformRenderer->getHeight();
        } else {
            width = m_waveformRenderer->getWidth();
            height = 2 * labelRectHeight + 1;
        }

        pMark->m_image = QImage(width, height, QImage::Format_ARGB32_Premultiplied);

        Qt::Alignment markAlignH = markProperties.m_align & Qt::AlignHorizontal_Mask;
        Qt::Alignment markAlignV = markProperties.m_align & Qt::AlignVertical_Mask;

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

        // Fill with transparent pixels
        pMark->m_image.fill(QColor(0,0,0,0).rgba());

        painter.begin(&pMark->m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

        // Prepare colors for drawing of marker lines
        QColor lineColor = markProperties.m_color;
        lineColor.setAlpha(200);
        QColor contrastLineColor(0,0,0,120);

        // Draw marker lines
        if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
            int middle = width / 2;
            if (markAlignH == Qt::AlignHCenter) {
                if (labelRect.top() > 0) {
                    painter.setPen(lineColor);
                    painter.drawLine(middle, 0, middle, labelRect.top());

                    painter.setPen(contrastLineColor);
                    painter.drawLine(middle - 1, 0, middle - 1, labelRect.top());
                    painter.drawLine(middle + 1, 0, middle + 1, labelRect.top());
                }

                if (labelRect.bottom() < height) {
                    painter.setPen(lineColor);
                    painter.drawLine(middle, labelRect.bottom(), middle, height);

                    painter.setPen(contrastLineColor);
                    painter.drawLine(middle - 1, labelRect.bottom(), middle - 1, height);
                    painter.drawLine(middle + 1, labelRect.bottom(), middle + 1, height);
                }
            } else {  // AlignLeft || AlignRight
                painter.setPen(lineColor);
                painter.drawLine(middle, 0, middle, height);

                painter.setPen(contrastLineColor);
                painter.drawLine(middle - 1, 0, middle - 1, height);
                painter.drawLine(middle + 1, 0, middle + 1, height);
            }
        } else {  // Vertical
            int middle = height / 2;
            if (markAlignV == Qt::AlignVCenter) {
                if (labelRect.left() > 0) {
                    painter.setPen(lineColor);
                    painter.drawLine(0, middle, labelRect.left(), middle);

                    painter.setPen(contrastLineColor);
                    painter.drawLine(0, middle - 1, labelRect.left(), middle - 1);
                    painter.drawLine(0, middle + 1, labelRect.left(), middle + 1);
                }

                if (labelRect.right() < width) {
                    painter.setPen(lineColor);
                    painter.drawLine(labelRect.right(), middle, width, middle);

                    painter.setPen(contrastLineColor);
                    painter.drawLine(labelRect.right(), middle - 1, width, middle - 1);
                    painter.drawLine(labelRect.right(), middle + 1, width, middle + 1);
                }
            } else {  // AlignTop || AlignBottom
                painter.setPen(lineColor);
                painter.drawLine(0, middle, width, middle);

                painter.setPen(contrastLineColor);
                painter.drawLine(0, middle - 1, width, middle - 1);
                painter.drawLine(0, middle + 1, width, middle + 1);
            }
        }

        // Draw the label rect
        QColor rectColor = markProperties.m_color;
        rectColor.setAlpha(200);
        painter.setPen(markProperties.m_color);
        painter.setBrush(QBrush(rectColor));
        painter.drawRoundedRect(labelRect, 2.0, 2.0);

        // Draw text
        painter.setBrush(QBrush(QColor(0,0,0,0)));
        painter.setFont(font);
        painter.setPen(markProperties.m_textColor);
        painter.drawText(labelRect, Qt::AlignCenter, label);
    }
    else //no text draw triangle
    {
        float triangleSize = 9.0;
        float markLength = triangleSize + 1.0;
        float markBreadth = m_waveformRenderer->getBreadth();

        int width, height;

        if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
            width = markLength;
            height = markBreadth;
        } else {
            width = markBreadth;
            height = markLength;
        }

        pMark->m_image = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
        pMark->m_image.fill(QColor(0,0,0,0).rgba());

        painter.begin(&pMark->m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

        // Rotate if drawing vertical waveforms
        if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
            painter.setTransform(QTransform(0, 1, 1, 0, 0, 0));
        }

        QColor triangleColor = markProperties.m_color;
        triangleColor.setAlpha(140);
        painter.setPen(QColor(0,0,0,0));
        painter.setBrush(QBrush(triangleColor));

        //vRince: again don't ask about the +-0.1 0.5 ...
        // just to make it nice in Qt ...

        QPolygonF triangle;
        triangle.append(QPointF(0.5,0));
        triangle.append(QPointF(triangleSize+0.5,0));
        triangle.append(QPointF(triangleSize*0.5 + 0.1, triangleSize*0.5));

        painter.drawPolygon(triangle);

        triangle.clear();
        triangle.append(QPointF(0.0,markBreadth));
        triangle.append(QPointF(triangleSize+0.5,markBreadth));
        triangle.append(QPointF(triangleSize*0.5 + 0.1, markBreadth - triangleSize*0.5 - 2.1));

        painter.drawPolygon(triangle);

        //TODO vRince duplicated code make a method
        //draw line
        QColor lineColor = markProperties.m_color;
        lineColor.setAlpha(140);
        painter.setPen(lineColor);

        float middle = markLength / 2.0;

        float lineTop = triangleSize * 0.5 + 1;
        float lineBottom = markBreadth - triangleSize * 0.5 - 1;

        painter.drawLine(middle, lineTop, middle, lineBottom);

        //other lines to increase contrast
        painter.setPen(QColor(0,0,0,100));
        painter.drawLine(middle - 1, lineTop, middle - 1, lineBottom);
        painter.drawLine(middle + 1, lineTop, middle + 1, lineBottom);
    }
}
