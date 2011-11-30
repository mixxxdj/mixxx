
#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>

#include "waveformrendermark.h"

#include "waveformrenderer.h"
#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "trackinfoobject.h"

WaveformRenderMark::WaveformRenderMark(const char* pGroup,
                                       WaveformRenderer *parent)
        : m_pGroup(pGroup),
          m_pParent(parent),
          m_pMarkPoint(NULL),
          m_pTrackSamples(NULL),
          m_iMarkPoint(-1),
          m_iWidth(0),
          m_iHeight(0),
          m_bHasCustomPixmap(false),
          m_dSamplesPerDownsample(-1),
          m_iNumSamples(0),
          m_iSampleRate(-1) {
    m_pTrackSamples = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(pGroup, "track_samples")));
    slotUpdateTrackSamples(m_pTrackSamples->get());
    connect(m_pTrackSamples, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateTrackSamples(double)));
    m_pTrackSampleRate = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(pGroup, "track_samplerate")));
    slotUpdateTrackSampleRate(m_pTrackSampleRate->get());
    connect(m_pTrackSampleRate, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateTrackSampleRate(double)));
}

WaveformRenderMark::~WaveformRenderMark() {
    qDebug() << this << "~WaveformRenderMark()";
    //m_markPixmap = QPixmap();
    delete m_pTrackSamples;
    delete m_pTrackSampleRate;
    delete m_pMarkPoint;
}

void WaveformRenderMark::slotUpdateMarkPoint(double v) {
    //qDebug() << "WaveformRenderMark :: MarkPoint = " << v;
    m_iMarkPoint = (int)v;
}

void WaveformRenderMark::slotUpdateTrackSamples(double samples) {
    //qDebug() << "WaveformRenderMark :: samples = " << int(samples);
    m_iNumSamples = (int)samples;
}

void WaveformRenderMark::slotUpdateTrackSampleRate(double sampleRate) {
    //qDebug() << "WaveformRenderMark :: sampleRate = " << int(sampleRate);

    // f = z * m * n
    double m = m_pParent->getSubpixelsPerPixel();
    double f = sampleRate;
    double z = m_pParent->getPixelsPerSecond();
    double n = f / (m*z);

    m_iSampleRate = static_cast<int>(sampleRate);
    m_dSamplesPerDownsample = n;
}

void WaveformRenderMark::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
}

void WaveformRenderMark::newTrack(TrackPointer pTrack) {
}

void WaveformRenderMark::setup(QDomNode node) {
    ConfigKey configKey;
    configKey.group = m_pGroup;
    configKey.item = WWidget::selectNodeQString(node, "Control");

    if (m_pMarkPoint) {
        // Disconnect the old control
        disconnect(m_pMarkPoint, 0, this, 0);
        delete m_pMarkPoint;
        m_pMarkPoint = NULL;
    }

    m_pMarkPoint = new ControlObjectThreadMain(
        ControlObject::getControl(configKey));
    slotUpdateMarkPoint(m_pMarkPoint->get());
    connect(m_pMarkPoint, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateMarkPoint(double)));

    // Read the mark color, otherwise get MarkerColor of the Visual element
    QString markColor = WWidget::selectNodeQString(node, "Color");
    if (markColor == "") {
        // As a fallback, grab the mark color from the parent's MarkerColor
        markColor = WWidget::selectNodeQString(node.parentNode(), "MarkerColor");
        qDebug() << "Didn't get mark Color, using parent's MarkerColor:"
                 << markColor;
        m_markColor.setNamedColor(markColor);
        // m_markColor = QColor(255 - m_markColor.red(),
        //                      255 - m_markColor.green(),
        //                      255 - m_markColor.blue());
    } else {
        m_markColor.setNamedColor(markColor);
    }
    m_markColor = WSkinColor::getCorrectColor(m_markColor);

    // Read the text color, otherwise use the parent's BgColor.
    QString textColor = WWidget::selectNodeQString(node, "TextColor");
    if (textColor == "") {
        textColor = WWidget::selectNodeQString(node.parentNode(), "BgColor");
        qDebug() << "Didn't get mark TextColor, using parent's BgColor:"
                 << textColor;
        m_textColor.setNamedColor(textColor);
        // m_textColor = QColor(255 - m_textColor.red(),
        //                      255 - m_textColor.green(),
        //                      255 - m_textColor.blue());
    } else {
        m_textColor.setNamedColor(textColor);
    }
    m_textColor = WSkinColor::getCorrectColor(m_textColor);

    QString markAlign = WWidget::selectNodeQString(node, "Align");
    if (markAlign.compare("center", Qt::CaseInsensitive) == 0) {
        m_markAlign = WaveformRenderMark::CENTER;
    } else if (markAlign.compare("bottom", Qt::CaseInsensitive) == 0) {
        m_markAlign = WaveformRenderMark::BOTTOM;
    } else {
        // Default
        m_markAlign = WaveformRenderMark::TOP;
    }

    // Read the mark's text
    m_markText = WWidget::selectNodeQString(node, "Text");
    m_markPixmapPath = WWidget::selectNodeQString(node,"Pixmap");

    setupMarkPixmap();
}


void WaveformRenderMark::draw(QPainter *pPainter, QPaintEvent *event,
                              QVector<float> *buffer, double dPlayPos,
                              double rateAdjust) {
    if (m_iSampleRate == -1 || m_iSampleRate == 0 || m_iNumSamples == 0)
        return;

    // necessary?
    if (buffer == NULL)
        return;

    double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel()*(1.0+rateAdjust);

    pPainter->save();
    pPainter->scale(1.0/subpixelsPerPixel,1.0);
    QPen oldPen = pPainter->pen();
    QBrush oldBrush = pPainter->brush();

    double subpixelWidth = m_iWidth * subpixelsPerPixel;
    double subpixelHalfWidth = subpixelWidth / 2.0;
    double halfh = m_iHeight/2;

    if (m_iMarkPoint != -1) {
        double markPointMono = m_iMarkPoint >> 1;
        double curPos = dPlayPos * (m_iNumSamples/2);
        double i = (markPointMono - curPos)/m_dSamplesPerDownsample;

        if (abs(i) < subpixelHalfWidth) {
            double x = (i+subpixelHalfWidth);
            QPen newPen = QPen(m_markColor);
            newPen.setWidth(subpixelsPerPixel*2);
            pPainter->setPen(newPen);
            pPainter->drawLine(QLineF(x, halfh, x, -halfh));

            if (!m_bHasCustomPixmap) {
                // If no custom pixmap is provided, draw triangles at top and
                // bottom of the mark.
                pPainter->setPen(m_markColor);
                pPainter->setBrush(QBrush(m_markColor));
                QPolygonF topTriangle;
                QPolygonF bottomTriangle;
                double triWidth = subpixelsPerPixel * 8.0;
                double triHeight = 10.0;
                topTriangle << QPointF(x - 1 - triWidth/2.0f, halfh)
                            << QPointF(x + 1 + triWidth/2.0f, halfh)
                            << QPointF(x, halfh - triHeight);
                bottomTriangle << QPointF(x - triWidth/2.0f, -halfh)
                               << QPointF(x + 1 + triWidth/2.0f, -halfh)
                               << QPointF(x, -halfh + triHeight);
                pPainter->drawPolygon(topTriangle);
                pPainter->drawPolygon(bottomTriangle);
            }

            if (!m_markPixmap.isNull()) {
                pPainter->scale(subpixelsPerPixel, -1.0);
                x = x / subpixelsPerPixel;
                int pw = m_markPixmap.width();
                int ph = m_markPixmap.height();

                // Draw the pixmap in the right place
                switch (m_markAlign) {
                    case WaveformRenderMark::BOTTOM:
                        // Bottom
                        pPainter->drawPixmap(x - pw/2.0, halfh - ph, m_markPixmap);
                        break;
                    case WaveformRenderMark::CENTER:
                        // Center
                        pPainter->drawPixmap(x - pw/2.0, 0 - ph/2.0, m_markPixmap);
                        break;
                    case WaveformRenderMark::TOP:
                    default:
                        // Top
                        pPainter->drawPixmap(x - pw/2.0, -halfh + 2.0, m_markPixmap);
                        break;
                }
            }
        }
    }

    pPainter->setPen(oldPen);
    pPainter->setBrush(oldBrush);
    pPainter->restore();
}

void WaveformRenderMark::setupMarkPixmap() {
    // Load the pixmap from file -- takes precedence over text.
    if (m_markPixmapPath != "") {
        // TODO(XXX) We could use WPixmapStore here, which would recolor the
        // pixmap according to the theme. Then we would have to worry about
        // deleting it -- for now we'll just load the pixmap directly.
        m_markPixmap = QPixmap(WWidget::getPath(m_markPixmapPath));

        // If loading the pixmap didn't fail, then we're done. Otherwise fall
        // through and render a label.
        if (!m_markPixmap.isNull()) {
            m_bHasCustomPixmap = true;
            return;
        }
    }

    // If no text is provided, leave m_markPixmap as a null pixmap
    if (m_markText == "") {
        return;
    }

    //QFont font("Bitstream Vera Sans");
    //QFont font("Helvetica");
    QFont font; // Uses the application default
    font.setPointSize(8);
    //font.setWeight(QFont::Bold);
    //font.setLetterSpacing(QFont::AbsoluteSpacing, -1);

    QFontMetrics metrics(font);

    // Add left and right margins of one characters worth (based on average
    // pixels / character).
    double wordWidth = metrics.boundingRect(m_markText).width();
    double wordHeight = metrics.height();

    // A sensible margin for the horizontal is a quarter of the average
    // character width.
    //int marginX = wordWidth/m_markText.size()/4;
    //int marginX = metrics.maxWidth() / 4;
    double marginX = metrics.averageCharWidth() / 4.0;

    double marginY = 0; // .1 * wordHeight

    double markWidth = wordWidth + 2*marginX;
    double markHeight = wordHeight + 2*marginY;

    QRectF internalRect(marginX, marginY, wordWidth-1, wordHeight-1);
    QRectF externalRect(0, 0, markWidth-1, markHeight-1);

    m_markPixmap = QPixmap(markWidth, markHeight);

    // Fill with transparent pixels
    m_markPixmap.fill(QColor(0,0,0,0));

    QPainter painter(&m_markPixmap);
    painter.setRenderHint(QPainter::TextAntialiasing);
    //painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setFont(font);
    QColor color = m_textColor;
    color = QColor(0xff - color.red(),
                   0xff - color.green(),
                   0xff - color.blue(),
                   128);
    painter.setPen(color);
    painter.setBrush(QBrush(color));

    // Stuff to test that the rectangles are correct.
    //painter.setBrush(QBrush());
    //painter.drawRect(externalRect);
    //painter.drawRect(internalRect);

    //painter.setBrush(QBrush());
    //painter.drawRoundedRect(externalRect, 25, 60, Qt::RelativeSize);
    painter.drawRoundedRect(externalRect, 2, 2);

    painter.setPen(m_textColor);
    painter.drawText(internalRect,
                     Qt::AlignCenter,
                     m_markText);
}
