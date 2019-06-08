#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QtDebug>

#include "waveform/renderers/waveformrenderbeat.h"

#include "control/controlobject.h"
#include "track/beats.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
    m_beats.resize(128);
}

WaveformRenderBeat::~WaveformRenderBeat() {
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor.setNamedColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();
    m_barColor.setNamedColor(context.selectString(node, "BarColor"));
    m_barColor = WSkinColor::getCorrectColor(m_barColor).toRgb();
    m_phraseColor.setNamedColor(context.selectString(node, "PhraseColor"));
    m_phraseColor = WSkinColor::getCorrectColor(m_phraseColor).toRgb();
}

void WaveformRenderBeat::draw(QPainter* painter, QPaintEvent* /*event*/) {
	m_showBarAndPhrase = WaveformWidgetFactory::instance()->getShowBarAndPhrase();
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo)
        return;

    BeatsPointer trackBeats = trackInfo->getBeats();
    if (!trackBeats)
        return;

    int alpha = m_waveformRenderer->beatGridAlpha();
    if (alpha == 0)
        return;
    m_beatColor.setAlphaF(alpha/100.0);
    m_barColor.setAlphaF(alpha/100.0);
    m_phraseColor.setAlphaF(alpha/100.0);

    const int trackSamples = m_waveformRenderer->getNumberOfSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition();

    // qDebug() << "trackSamples" << trackSamples
    //          << "firstDisplayedPosition" << firstDisplayedPosition
    //          << "lastDisplayedPosition" << lastDisplayedPosition;

    std::unique_ptr<BeatIterator> it(trackBeats->findBeats(
            firstDisplayedPosition * trackSamples,
            lastDisplayedPosition * trackSamples));

    // if no beat do not waste time saving/restoring painter
    if (!it || !it->hasNext()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPen beatPen(m_beatColor);
    beatPen.setWidthF(std::max(1.0, scaleFactor()));

    QPen barPen(m_barColor);
    barPen.setWidthF(std::max(1.0, scaleFactor()*2));

    QPen phrasePen(m_phraseColor);
    phrasePen.setWidthF(std::max(1.0, scaleFactor()*2));

    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();
    const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();

    while (it->hasNext()) {
        BeatData beat = it->next();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(beat.sample);

        xBeatPoint = qRound(xBeatPoint);


        // Selects the right pen, if we are in phrase also paints the phrase tag
        if(beat.phraseNumber != -1 && m_showBarAndPhrase) {
            // Selects the font
            QFont font; // Uses the application default
            font.setPointSizeF(10 * scaleFactor());
            font.setStretch(100);
            font.setWeight(75);

            QFontMetrics metrics(font);

            // Calculates the size of the box
            QString label(QString::number(beat.phraseNumber));
            QRect wordRect = metrics.tightBoundingRect(label);
            const int marginX = 1 * scaleFactor();
            const int marginY = 1 * scaleFactor();
            wordRect.moveTop(marginX + 1);
            wordRect.moveLeft(marginY + 1);
            wordRect.setHeight(wordRect.height() + (wordRect.height()%2));
            wordRect.setWidth(wordRect.width() + (wordRect.width())%2);
            //even wordrect to have an even Image >> draw the line in the middle !

            int labelRectWidth = wordRect.width() + 2 * marginX + 4;
            int labelRectHeight = wordRect.height() + 2 * marginY + 4 * scaleFactor();

            QRectF labelRect(xBeatPoint-labelRectWidth,
                    rendererHeight-labelRectHeight,
                    (float)labelRectWidth,
                    (float)labelRectHeight);

            // Draw the label rect
            QColor rectColor = m_phraseColor;
            rectColor.setAlpha(200);
            painter->setPen(m_phraseColor);
            painter->setBrush(QBrush(rectColor));
            painter->drawRoundedRect(labelRect, 2 * scaleFactor(), 2 * scaleFactor());
            // Draw the text
            painter->setBrush(QBrush(QColor(0,0,0,0)));
            painter->setFont(font);
            painter->setPen(Qt::black);
            painter->drawText(labelRect, Qt::AlignCenter, label);

            painter->setPen(phrasePen);
        } else if(beat.barNumber != -1 && m_showBarAndPhrase) {
            painter->setPen(barPen);
        }
        else {
            painter->setPen(beatPen);

        }

        // Paints the beat line
        if (orientation == Qt::Horizontal) {
            painter->drawLine(xBeatPoint, 0.0f, xBeatPoint, rendererHeight);
        } else {
            painter->drawLine(0.0f, xBeatPoint, rendererWidth, xBeatPoint);
        }
    }

    painter->restore();
}
