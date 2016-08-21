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

        if (!mark->m_pPointCos)
            continue;

        // Generate image on first paint can't be done in setup since we need
        // render widget to be resized yet ...
        if (mark->m_image.isNull()) {
            generateMarkImage(mark.data());
        }

        int samplePosition = mark->m_pPointCos->get();
        if (samplePosition > 0.0) {
            double currentMarkPoint = m_waveformRenderer->transformSampleIndexInRendererWorld(samplePosition);

            // NOTE: vRince I guess image width is odd to display the center on the exact line !
            //external image should respect that ...
            const int markHalfWidth = mark->m_image.width() / 2.0;

            //check if the current point need to be displayed
            if (currentMarkPoint > -markHalfWidth && currentMarkPoint < m_waveformRenderer->getWidth() + markHalfWidth) {
                painter->drawImage(QPoint(currentMarkPoint-markHalfWidth,0), mark->m_image);
            }
        }
    }
    painter->restore();
}

void WaveformRenderMark::onSetTrack() {
    slotCuesUpdated();

    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }
    connect(trackInfo.data(), SIGNAL(cuesUpdated(void)),
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

void WaveformRenderMark::generateMarkImage(WaveformMark* mark) {
    const WaveformMarkProperties& markProperties = mark->getProperties();

    // Load the pixmap from file -- takes precedence over text.
    if (!markProperties.m_pixmapPath.isEmpty()) {
        QString path = markProperties.m_pixmapPath;
        QImage image = QImage(path);
        // If loading the image didn't fail, then we're done. Otherwise fall
        // through and render a label.
        if (!image.isNull()) {
            mark->m_image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            WImageStore::correctImageColors(&mark->m_image);
            return;
        }
    }

    QPainter painter;

    int labelRectWidth = 0;
    int labelRectHeight = 0;

    // If no text is provided, leave m_markImage as a null image
    if (!markProperties.m_text.isNull()) {
        // Determine mark text.
        QString label = markProperties.m_text;
        if (mark->m_iHotCue != -1) {
            if (!label.isEmpty()) {
                label.prepend(": ");
            }
            label.prepend(QString::number(mark->m_iHotCue));
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
        wordRect.setWidth(wordRect.width() + (wordRect.width())%2);
        //even wordrect to have an even Image >> draw the line in the middle !

        labelRectWidth = wordRect.width() + 2*marginX + 4;
        labelRectHeight = wordRect.height() + 2*marginY + 4 ;

        QRectF labelRect(0, 0,
                (float)labelRectWidth, (float)labelRectHeight);

        mark->m_image = QImage(labelRectWidth+1,
                m_waveformRenderer->getHeight(),
                QImage::Format_ARGB32_Premultiplied);

        if (markProperties.m_align == Qt::AlignBottom) {
            labelRect.moveBottom(mark->m_image.height()-1);
        }

        // Fill with transparent pixels
        mark->m_image.fill(QColor(0,0,0,0).rgba());

        painter.begin(&mark->m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

        //draw the label rect
        QColor rectColor = markProperties.m_color;
        rectColor.setAlpha(150);
        painter.setPen(markProperties.m_color);
        painter.setBrush(QBrush(rectColor));
        painter.drawRoundedRect(labelRect, 2.0, 2.0);
        //painter.drawRect(labelRect);

        //draw text
        painter.setBrush(QBrush(QColor(0,0,0,0)));
        painter.setFont(font);
        painter.setPen(markProperties.m_textColor);
        painter.drawText(labelRect, Qt::AlignCenter, label);

        //draw line
        QColor lineColor = markProperties.m_color;
        lineColor.setAlpha(200);
        painter.setPen(lineColor);

        float middle = mark->m_image.width() / 2.0;
        //Default line align top
        float lineTop = labelRectHeight + 1;
        float lineBottom = mark->m_image.height();

        if (markProperties.m_align == Qt::AlignBottom) {
            lineTop = 0.0;
            lineBottom = mark->m_image.height() - labelRectHeight - 1;
        }

        painter.drawLine(middle, lineTop, middle, lineBottom);

        //other lines to increase contrast
        painter.setPen(QColor(0,0,0,120));
        painter.drawLine(middle - 1, lineTop, middle - 1, lineBottom);
        painter.drawLine(middle + 1, lineTop, middle + 1, lineBottom);

    }
    else //no text draw triangle
    {
        float triangleSize = 9.0;
        mark->m_image = QImage(labelRectWidth+1,
                m_waveformRenderer->getHeight(),
                QImage::Format_ARGB32_Premultiplied);
        mark->m_image.fill(QColor(0,0,0,0).rgba());

        painter.begin(&mark->m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

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
        triangle.append(QPointF(0.0,mark->m_image.height()));
        triangle.append(QPointF(triangleSize+0.5,mark->m_image.height()));
        triangle.append(QPointF(triangleSize*0.5 + 0.1, mark->m_image.height() - triangleSize*0.5 - 2.1));

        painter.drawPolygon(triangle);

        //TODO vRince duplicated code make a method
        //draw line
        QColor lineColor = markProperties.m_color;
        lineColor.setAlpha(140);
        painter.setPen(lineColor);
        float middle = mark->m_image.width() / 2.0;

        float lineTop = triangleSize * 0.5 + 1;
        float lineBottom = mark->m_image.height() - triangleSize * 0.5 - 1;

        painter.drawLine(middle, lineTop, middle, lineBottom);

        //other lines to increase contrast
        painter.setPen(QColor(0,0,0,100));
        painter.drawLine(middle - 1, lineTop, middle - 1, lineBottom);
        painter.drawLine(middle + 1, lineTop, middle + 1, lineBottom);
    }
}
