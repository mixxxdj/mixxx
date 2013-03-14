#include <QDomNode>
#include <QPainter>
#include <QPainterPath>

#include "waveform/renderers/waveformrendermark.h"

#include "controlobjectthreadmain.h"
#include "trackinfoobject.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"



WaveformRenderMark::WaveformRenderMark( WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer) {
}

void WaveformRenderMark::setup( const QDomNode& node) {
    m_marks.setup(m_waveformRenderer->getGroup(), node, *m_waveformRenderer->getWaveformSignalColors());
}

void WaveformRenderMark::draw( QPainter* painter, QPaintEvent* /*event*/) {
    painter->save();

    /*
    //DEBUG
    for( int i = 0; i < m_markPoints.size(); i++) {
        if( m_waveformWidget->getTrackSamples())
            painter->drawText(40*i,12+12*(i%3),QString::number(m_markPoints[i]->get() / (double)m_waveformWidget->getTrackSamples()));
    }
    */

    painter->setWorldMatrixEnabled(false);

    for( int i = 0; i < m_marks.size(); i++) {
        WaveformMark& mark = m_marks[i];

        if (!mark.m_pointControl)
            continue;

        // Generate image on first paint can't be done in setup since we need
        // render widget to be resized yet ...
        if( mark.m_image.isNull()) {
            generateMarkImage(mark);
        }

        int samplePosition = mark.m_pointControl->get();
        if (samplePosition >= 0.0) {
            m_waveformRenderer->regulateVisualSample(samplePosition);
            double currentMarkPoint = m_waveformRenderer->transformSampleIndexInRendererWorld(samplePosition);

            // NOTE: vRince I guess image width is odd to display the center on the exact line !
            //external image should respect that ...
            const int markHalfWidth = mark.m_image.width()/2.0;

            //check if the current point need to be displayed
            if( currentMarkPoint > -markHalfWidth && currentMarkPoint < m_waveformRenderer->getWidth() + markHalfWidth) {
                painter->drawImage(QPoint(currentMarkPoint-markHalfWidth,0), mark.m_image);
            }
        }
    }
    painter->restore();
}

void WaveformRenderMark::generateMarkImage( WaveformMark& mark) {
    // Load the pixmap from file -- takes precedence over text.
    if( mark.m_pixmapPath != "") {
        // TODO(XXX) We could use WPixmapStore here, which would recolor the
        // pixmap according to the theme. Then we would have to worry about
        // deleting it -- for now we'll just load the pixmap directly.
        QString path =  WWidget::getPath(mark.m_pixmapPath);
        QImage image = QImage(path);
        // If loading the image didn't fail, then we're done. Otherwise fall
        // through and render a label.
        if (!image.isNull()) {
            mark.m_image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            return;
        }
    }

    QPainter painter;

    int labelRectWidth = 0;
    int labelRectHeight = 0;

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
        wordRect.setWidth(wordRect.width() + (wordRect.width())%2);
        //even wordrect to have an even Image >> draw the line in the middle !

        labelRectWidth = wordRect.width() + 2*marginX + 4;
        labelRectHeight = wordRect.height() + 2*marginY + 4 ;

        QRectF labelRect(0, 0,
                (float)labelRectWidth, (float)labelRectHeight);

        mark.m_image = QImage(labelRectWidth+1,
                m_waveformRenderer->getHeight(),
                QImage::Format_ARGB32_Premultiplied);

        if (mark.m_align == Qt::AlignBottom) {
            labelRect.moveBottom(mark.m_image.height()-1);
        }

        // Fill with transparent pixels
        mark.m_image.fill(QColor(0,0,0,0).rgba());

        painter.begin(&mark.m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

        //draw the label rect
        QColor rectColor = mark.m_color;
        rectColor.setAlpha(150);
        rectColor.darker(200);
        painter.setPen(mark.m_color);
        painter.setBrush(QBrush(rectColor));
        painter.drawRoundedRect(labelRect, 2.0, 2.0);
        //painter.drawRect(labelRect);

        //draw text
        painter.setBrush(QBrush(QColor(0,0,0,0)));
        font.setWeight(75);
        painter.setFont(font);
        painter.setPen(mark.m_textColor);
        painter.drawText(labelRect, Qt::AlignCenter, mark.m_text);

        //draw line
        QColor lineColor = mark.m_color;
        lineColor.setAlpha(200);
        painter.setPen(lineColor);

        float middle = mark.m_image.width()/2;
        //Default line align top
        float lineTop = labelRectHeight + 1;
        float lineBottom = mark.m_image.height();

        if(mark.m_align == Qt::AlignBottom) {
            lineTop = 0.0;
            lineBottom = mark.m_image.height() - labelRectHeight - 1;
        }

        painter.drawLine( middle, lineTop, middle, lineBottom);

        //other lines to increase contrast
        painter.setPen(QColor(0,0,0,120));
        painter.drawLine( middle - 1, lineTop, middle - 1, lineBottom);
        painter.drawLine( middle + 1, lineTop, middle + 1, lineBottom);

    }
    else //no text draw triangle
    {
        float triangleSize = 9.0;
        mark.m_image = QImage(labelRectWidth+1,
                m_waveformRenderer->getHeight(),
                QImage::Format_ARGB32_Premultiplied);
        mark.m_image.fill(QColor(0,0,0,0).rgba());

        painter.begin(&mark.m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

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
        triangle.append(QPointF(0.0,mark.m_image.height()));
        triangle.append(QPointF(triangleSize+0.5,mark.m_image.height()));
        triangle.append(QPointF(triangleSize*0.5 + 0.1, mark.m_image.height() - triangleSize*0.5 - 2.1));

        painter.drawPolygon(triangle);

        //TODO vRince duplicated code make a method
        //draw line
        QColor lineColor = mark.m_color;
        lineColor.setAlpha(140);
        painter.setPen(lineColor);
        float middle = mark.m_image.width()/2;

        float lineTop = triangleSize*0.5 + 1;
        float lineBottom = mark.m_image.height() - triangleSize*0.5 - 1;

        painter.drawLine( middle, lineTop, middle, lineBottom);

        //other lines to increase contrast
        painter.setPen(QColor(0,0,0,100));
        painter.drawLine( middle - 1, lineTop, middle - 1, lineBottom);
        painter.drawLine( middle + 1, lineTop, middle + 1, lineBottom);
    }
}
