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
        WaveformMark& mark = m_marks[i];

        if (!mark.m_pPointCos)
            continue;

        // Generate image on first paint can't be done in setup since we need
        // render widget to be resized yet ...
        if (mark.m_image.isNull()) {
            generateMarkImage(mark);
        }

        int samplePosition = mark.m_pPointCos->get();
        if (samplePosition > 0.0) {
            double currentMarkPoint = m_waveformRenderer->transformSampleIndexInRendererWorld(samplePosition);

            if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
                // NOTE: vRince I guess image width is odd to display the center on the exact line !
                // external image should respect that ...
                const int markHalfWidth = mark.m_image.width() / 2.0;

                // check if the current point need to be displayed
                if (currentMarkPoint > -markHalfWidth && currentMarkPoint < m_waveformRenderer->getWidth() + markHalfWidth) {
                    painter->drawImage(QPoint(currentMarkPoint - markHalfWidth,0), mark.m_image);
                }
            } else {
                const int markHalfHeight = mark.m_image.height() / 2.0;

                if (currentMarkPoint > -markHalfHeight && currentMarkPoint < m_waveformRenderer->getHeight() + markHalfHeight) {
                    painter->drawImage(QPoint(0,currentMarkPoint - markHalfHeight), mark.m_image);
                }
            }
        }
    }

    painter->restore();
}

void WaveformRenderMark::onResize() {
    // Delete all marks' images. New images will be created on next paint.
    for (int i = 0; i < m_marks.size(); i++) {
        m_marks[i].m_image = QImage();
    }
}

void WaveformRenderMark::generateMarkImage(WaveformMark& mark) {
    // Load the pixmap from file -- takes precedence over text.
    if (mark.m_pixmapPath != "") {
        QString path =  mark.m_pixmapPath;
        QImage image = QImage(path);
        // If loading the image didn't fail, then we're done. Otherwise fall
        // through and render a label.
        if (!image.isNull()) {
            mark.m_image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            WImageStore::correctImageColors(&mark.m_image);
            return;
        }
    }

    QPainter painter;

    // If no text is provided, leave m_markImage as a null image
    if (!mark.m_text.isNull()) {
        //QFont font("Bitstream Vera Sans");
        //QFont font("Helvetica");
        QFont font; // Uses the application default
        font.setPointSize(10);
        font.setStretch(100);

        QFontMetrics metrics(font);

        //fixed margin ...
        QRect wordRect = metrics.tightBoundingRect(mark.m_text);
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

        mark.m_image = QImage(width,
                              height,
                              QImage::Format_ARGB32_Premultiplied);

        Qt::Alignment markAlignH = mark.m_align & Qt::AlignHorizontal_Mask;
        Qt::Alignment markAlignV = mark.m_align & Qt::AlignVertical_Mask;

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
        mark.m_image.fill(QColor(0,0,0,0).rgba());

        painter.begin(&mark.m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

        // Prepare colors for drawing of marker lines
        QColor lineColor = mark.m_color;
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
        QColor rectColor = mark.m_color;
        rectColor.setAlpha(200);
        painter.setPen(mark.m_color);
        painter.setBrush(QBrush(rectColor));
        painter.drawRoundedRect(labelRect, 2.0, 2.0);

        // Draw text
        painter.setBrush(QBrush(QColor(0,0,0,0)));
        font.setWeight(75);
        painter.setFont(font);
        painter.setPen(mark.m_textColor);
        painter.drawText(labelRect, Qt::AlignCenter, mark.m_text);
    }
    else //no text draw triangle
    {
        float triangleSize = 9.0;
        float markLength = triangleSize + 1.0;
        float markBreadth = m_waveformRenderer->getBreadth();
        if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
            mark.m_image = QImage(markLength,
                                  markBreadth,
                                  QImage::Format_ARGB32_Premultiplied);
        } else {
            mark.m_image = QImage(markBreadth,
                                  markLength,
                                  QImage::Format_ARGB32_Premultiplied);
        }
        mark.m_image.fill(QColor(0,0,0,0).rgba());

        painter.begin(&mark.m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

        // Rotate if drawing vertical waveforms
        if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
            painter.setTransform(QTransform(0, 1, 1, 0, 0, 0));
        }

        QColor triangleColor = mark.m_color;
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
        QColor lineColor = mark.m_color;
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
