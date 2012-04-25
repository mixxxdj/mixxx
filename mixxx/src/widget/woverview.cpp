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
#include <qpainter.h>
#include <QtDebug>
#include <qpixmap.h>
#include <qapplication.h>

#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "woverview.h"
#include "wskincolor.h"
#include "trackinfoobject.h"
#include "mathstuff.h"

#include "waveform/waveform.h"

WOverview::WOverview(const char *pGroup, ConfigObject<ConfigValue>* pConfig, QWidget * parent)
    : WWidget(parent),
      m_pGroup(pGroup),
      m_pConfig(pConfig) {

    m_sampleDuration = 0;
    m_iPos = 0;
    m_bDrag = false;

    setAcceptDrops(true);

    m_pLoopStart = ControlObject::getControl(
                ConfigKey(m_pGroup, "loop_start_position"));
    connect(m_pLoopStart, SIGNAL(valueChanged(double)),
            this, SLOT(loopStartChanged(double)));
    connect(m_pLoopStart, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(loopStartChanged(double)));
    loopStartChanged(m_pLoopStart->get());
    m_pLoopEnd = ControlObject::getControl(
                ConfigKey(m_pGroup, "loop_end_position"));
    connect(m_pLoopEnd, SIGNAL(valueChanged(double)),
            this, SLOT(loopEndChanged(double)));
    connect(m_pLoopEnd, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(loopEndChanged(double)));
    loopEndChanged(m_pLoopEnd->get());
    m_pLoopEnabled = ControlObject::getControl(
                ConfigKey(m_pGroup, "loop_enabled"));
    connect(m_pLoopEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(loopEnabledChanged(double)));
    connect(m_pLoopEnabled, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(loopEnabledChanged(double)));
    loopEnabledChanged(m_pLoopEnabled->get());

    QString pattern = "hotcue_%1_position";

    int i = 1;
    ConfigKey hotcueKey;
    hotcueKey.group = m_pGroup;
    hotcueKey.item = pattern.arg(i);
    ControlObject* pControl = ControlObject::getControl(hotcueKey);

    //qDebug() << "Connecting hotcue controls.";
    while (pControl) {
        m_hotcueControls.push_back(pControl);
        m_hotcues.push_back(pControl->get());
        m_hotcueMap[pControl] = i;

        //qDebug() << "Connecting hotcue" << hotcueKey.group << hotcueKey.item;

        connect(pControl, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(cueChanged(double)));
        connect(pControl, SIGNAL(valueChanged(double)),
                this, SLOT(cueChanged(double)));

        hotcueKey.item = pattern.arg(++i);
        pControl = ControlObject::getControl(hotcueKey);
    }

    //vrince
    m_waveform = NULL;
    m_waveformPixmap = QPixmap();
    m_actualCompletion = 0;
    m_visualSamplesByPixel = 0.0;

    m_timerPixmapRefresh = -1;
    m_renderSampleLimit = 1000;
}

WOverview::~WOverview() {
}

void WOverview::setup(QDomNode node) {
    // Background color and pixmap, default background color to transparent
    m_qColorBackground = QColor(0, 0, 0, 0);
    if (!selectNode(node, "BgColor").isNull()) {
        m_qColorBackground.setNamedColor(selectNodeQString(node, "BgColor"));
        m_qColorBackground = WSkinColor::getCorrectColor(m_qColorBackground);
    }

    // Clear the background pixmap, if it exists.
    m_backgroundPixmap = QPixmap();
    m_backgroundPixmapPath = WWidget::selectNodeQString(node, "BgPixmap");
    if (m_backgroundPixmapPath != "") {
        m_backgroundPixmap = QPixmap(WWidget::getPath(m_backgroundPixmapPath));
    }

    QPalette palette; //Qt4 update according to http://doc.trolltech.com/4.4/qwidget-qt3.html#setBackgroundColor (this could probably be cleaner maybe?)
    palette.setColor(this->backgroundRole(), m_qColorBackground);
    setPalette(palette);

    m_signalColors.setup(node);

    m_qColorMarker.setNamedColor(selectNodeQString(node, "MarkerColor"));
    m_qColorMarker = WSkinColor::getCorrectColor(m_qColorMarker);

    m_waveformPixmap = QPixmap(size());

    m_waveformPixmap.fill( QColor(0,0,0,0));
}

void WOverview::setValue(double fValue) {
    if (!m_bDrag)
    {
        // Calculate handle position
        int iPos = (int)(((fValue-14.)/100.)*((double)width()-2.));
        if (iPos != m_iPos) {
            m_iPos = iPos;
            update();
        }
    }
}

void WOverview::slotWaveformSummaryUpdated() {
    if (!m_pCurrentTrack) {
        return;
    }
    m_waveform = m_pCurrentTrack->getWaveformSummary();
    if (m_timerPixmapRefresh == -1) {
        m_timerPixmapRefresh = startTimer(60);
    }
    update();
}

void WOverview::slotLoadNewTrack(TrackPointer pTrack) {

    //qDebug() << "WOverview::slotLoadNewTrack(TrackPointer pTrack)";

    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), SIGNAL(waveformSummaryUpdated()),
                   this, SLOT(slotWaveformSummaryUpdated()));
    }

    if (pTrack) {
        m_pCurrentTrack = pTrack;

        m_sampleDuration = pTrack->getDuration()*pTrack->getSampleRate()*pTrack->getChannels();

        connect(pTrack.data(), SIGNAL(waveformSummaryUpdated()),
                this, SLOT(slotWaveformSummaryUpdated()));
        slotWaveformSummaryUpdated();
        //qDebug() << "WOverview::slotLoadNewTrack - startTimer";
    }

    m_actualCompletion = 0;
    m_visualSamplesByPixel = 0.0;

    m_waveformPixmap.fill(QColor(0, 0, 0, 0));
    update();
}

void WOverview::slotUnloadTrack(TrackPointer /*pTrack*/) {
    m_pCurrentTrack.clear();
    m_waveform = NULL;
    m_actualCompletion = 0;
    m_visualSamplesByPixel = 0.0;

    //qDebug() << "WOverview::slotUnloadTrack - kill Timer";
    if (m_timerPixmapRefresh != -1) {
        killTimer(m_timerPixmapRefresh);
        m_timerPixmapRefresh = -1;
    }

    update();
}

void WOverview::cueChanged(double v) {
    //qDebug() << "WOverview::cueChanged()";
    QObject* pSender = sender();
    if (!pSender)
        return;

    if (!m_hotcueMap.contains(pSender))
        return;

    int hotcue = m_hotcueMap[pSender];
    m_hotcues[hotcue] = v;
    //qDebug() << "hotcue" << hotcue << "position" << v;
    update();
}

void WOverview::loopStartChanged(double v) {
    m_dLoopStart = v;
    update();
}

void WOverview::loopEndChanged(double v) {
    m_dLoopEnd = v;
    update();
}

void WOverview::loopEnabledChanged(double v) {
    m_bLoopEnabled = !(v == 0.0f);
    update();
}

bool WOverview::drawNextPixmapPart() {

    //qDebug() << "WOverview::drawNextPixmapPart() - m_waveform" << m_waveform;

    if (!m_waveform) {
        return false;
    }

    // Safe without locking because it is atomic.
    const int dataSize = m_waveform->getDataSize();
    if (m_waveform->getDataSize() == 0) {
        return false;
    }

    const WaveformData* data = m_waveform->data();
    if (data == NULL) {
        return false;
    }

    //test id there is some new to draw (at least of pixel width)

    // Safe without locking because it is atomic.
    const int waveformCompletion = m_waveform->getCompletion();
    int completionIncrement = waveformCompletion - m_actualCompletion;

    //qDebug() << "WOverview::drawNextPixmapPart() - nextCompletion" << nextCompletion;

    if ((double)completionIncrement < m_visualSamplesByPixel) {
        return false;
    }

    //qDebug() << "WOverview::drawNextPixmapPart() - m_actualCompletion" << m_actualCompletion
    //         << "m_waveform->getCompletion()" << waveformCompletion
    //         << "nextCompletion" << completionIncrement;

    completionIncrement = std::min(completionIncrement,m_renderSampleLimit);
    const int nextCompletion = m_actualCompletion + completionIncrement;

    QPainter painter(&m_waveformPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.translate(0.0,height()/2.0);
    painter.scale(1.0,2.0*(double)(height()-2)/255.0);

    //draw only the new part
    const float pixelStartPosition = (float)m_actualCompletion / (float)dataSize * (float)width();
    const float pixelByVisualSamples = 1.0 / m_visualSamplesByPixel;

    const float alpha = math_min( 1.0, 2.0*math_max( 0.0, pixelByVisualSamples));

    QColor lowColor = m_signalColors.getLowColor();
    lowColor.setAlphaF(alpha);
    QPen lowColorPen( QBrush(lowColor), 1.25, Qt::SolidLine, Qt::RoundCap);

    QColor midColor = m_signalColors.getMidColor();
    midColor.setAlphaF(alpha);
    QPen midColorPen( QBrush(midColor), 1.25, Qt::SolidLine, Qt::RoundCap);

    QColor highColor = m_signalColors.getHighColor();
    highColor.setAlphaF(alpha);
    QPen highColorPen( QBrush(highColor), 1.25, Qt::SolidLine, Qt::RoundCap);

    int currentCompletion = m_actualCompletion;
    float pixelPosition = pixelStartPosition;
    for( ; currentCompletion < nextCompletion; currentCompletion += 2) {
        const WaveformData& datum = *(data + currentCompletion);
        const WaveformData& next_datum = *(data + currentCompletion + 1);
        painter.setPen( lowColorPen);
        painter.drawLine( QPointF(pixelPosition, - next_datum.filtered.low - 1.f),
                          QPointF(pixelPosition, datum.filtered.low + 1.f));
        pixelPosition += 2.0*pixelByVisualSamples;

    }

    currentCompletion = m_actualCompletion;
    pixelPosition = pixelStartPosition;
    for( ; currentCompletion < nextCompletion; currentCompletion += 2) {
        const WaveformData& datum = *(data + currentCompletion);
        const WaveformData& next_datum = *(data + currentCompletion + 1);
        painter.setPen( midColorPen);
        painter.drawLine( QPointF(pixelPosition, - next_datum.filtered.mid - 1.f),
                          QPointF(pixelPosition, datum.filtered.mid + 1.f));
        pixelPosition += 2.0*pixelByVisualSamples;
    }

    currentCompletion = m_actualCompletion;
    pixelPosition = pixelStartPosition;
    for( ; currentCompletion < nextCompletion; currentCompletion += 2) {
        const WaveformData& datum = *(data + currentCompletion);
        const WaveformData& next_datum = *(data + currentCompletion + 1);
        painter.setPen( highColorPen);
        painter.drawLine( QPointF(pixelPosition, - next_datum.filtered.high - 1.f),
                          QPointF(pixelPosition, datum.filtered.high + 1.f));
        pixelPosition += 2.0*pixelByVisualSamples;
    }

    m_actualCompletion = nextCompletion;
    return true;
}

void WOverview::mouseMoveEvent(QMouseEvent * e)
{
    m_iPos = e->x()-m_iStartMousePos;

    if (m_iPos>width()-2)
        m_iPos = width()-2;
    else if (m_iPos<0)
        m_iPos = 0;


    // Update display
    update();
}

void WOverview::mouseReleaseEvent(QMouseEvent * e)
{
    mouseMoveEvent(e);

    // value ranges from 0 to 127 map to -1 to 1
    float fValue = ((double)m_iPos*(100./(double)(width()-2))) + 14.;

    if (e->button()==Qt::RightButton)
        emit(valueChangedRightUp(fValue));
    else
        emit(valueChangedLeftUp(fValue));

    m_bDrag = false;
}

void WOverview::mousePressEvent(QMouseEvent * e)
{
    m_iStartMousePos = 0;
    mouseMoveEvent(e);
    m_bDrag = true;
}

void WOverview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    // Fill with transparent pixels
    painter.drawPixmap(rect(), m_backgroundPixmap);

    // Draw waveform, then playpos
    if (m_waveform) {
        painter.drawPixmap(rect(), m_waveformPixmap);
    }

    if (m_sampleDuration > 0) {
        // Draw play position
        //TODO(vrince) could be nice to have a precomputed pixmap for that
        painter.setPen(m_qColorMarker);
        painter.setOpacity(0.9);
        painter.drawLine(m_iPos,   0, m_iPos,   height());

        painter.drawLine(m_iPos-2,0,m_iPos,2);
        painter.drawLine(m_iPos,2,m_iPos+2,0);
        painter.drawLine(m_iPos-2,0,m_iPos+2,0);

        painter.drawLine(m_iPos-2,height()-1,m_iPos,height()-3);
        painter.drawLine(m_iPos,height()-3,m_iPos+2,height()-1);
        painter.drawLine(m_iPos-2,height()-1,m_iPos+2,height()-1);

        painter.setPen(m_qColorMarker.darker(200));
        painter.setOpacity(0.4);
        painter.drawLine(m_iPos+1, 0, m_iPos+1, height());
        painter.drawLine(m_iPos-1, 0, m_iPos-1, height());

        float fPos;

        // Draw loop markers
        QColor loopColor = m_qColorMarker;
        if (!m_bLoopEnabled) {
            loopColor = loopColor.darker(150);
        }
        painter.setPen(loopColor);
        if (m_dLoopStart != -1.0) {
            fPos = m_dLoopStart * (width() - 2) / m_sampleDuration;
            painter.drawLine(fPos, 0, fPos, height());
        }
        if (m_dLoopEnd != -1.0) {
            fPos = m_dLoopEnd * (width() - 2) / m_sampleDuration;
            painter.drawLine(fPos, 0, fPos, height());
        }

        if (m_dLoopStart != -1.0 && m_dLoopEnd != -1.0) {
            //loopColor.setAlphaF(0.5);
            painter.setOpacity(0.5);
            //paint.setPen(loopColor);
            painter.setBrush(QBrush(loopColor));
            float sPos = m_dLoopStart * (width() - 2) / m_sampleDuration;
            float ePos = m_dLoopEnd * (width() - 2) / m_sampleDuration;
            QRectF rect(QPointF(sPos, 0), QPointF(ePos, height()-1));
            painter.drawRect(rect);
            painter.setOpacity(1.0);
        }

        QFont font;
        font.setBold(false);
        int textWidth = 8;
        int textHeight = 10;
        font.setPixelSize(2*textHeight);
        painter.setPen(m_qColorMarker);
        painter.setFont(font);

        // Draw hotcues
        for (int i = 0; i < m_hotcues.size(); ++i) {
            int position = m_hotcues[i];
            if (position == -1)
                continue;
            fPos = float(position) * (width()-2) / m_sampleDuration;
            //qDebug() << "Drawing cue" << i << "at" << fPos;

            painter.drawLine(fPos, 0,
                             fPos, height());
            // paint.drawLine(fPos+1, 0,
            //                fPos+1, height());

            // int halfHeight = height()/2;
            // QRectF rect(QPointF(fPos-textWidth, halfHeight-textHeight),
            //             QPointF(fPos+textWidth, halfHeight+textHeight));

            // paint.drawText(rect, Qt::AlignCenter, QString("%1").arg(i+1));
        }
    }
    painter.end();
}

void WOverview::timerEvent(QTimerEvent* timer) {

    if (timer->timerId() == m_timerPixmapRefresh) {
        if (m_waveform == NULL) {
            return;
        }
        // Safe without locking because it is atomic.
        const int dataSize = m_waveform->getDataSize();

        //qDebug() << "timerEvent - m_timerPixmapRefresh";
        m_visualSamplesByPixel = (double)dataSize / (double)width();

        if (drawNextPixmapPart())
            update();

        //qDebug() << "timerEvent - m_actualCompletion" << m_actualCompletion << "m_waveform->size()" << m_waveform->size();

        //if m_waveform is empty ... actual computation do not start !
        //it must be in the analyser queue, we need to wait until it ready to display
        if (dataSize > 0 && m_actualCompletion + m_visualSamplesByPixel >= dataSize) {
            //qDebug() << " WOverview::timerEvent - kill timer";
            killTimer(m_timerPixmapRefresh);
            m_timerPixmapRefresh = -1;
        }
    }
}

QColor WOverview::getMarkerColor() {
    return m_qColorMarker;
}

void WOverview::dragEnterEvent(QDragEnterEvent* event) {
    // Accept the enter event if the thing is a filepath and nothing's playing
    // in this deck or the settings allow to interrupt the playing deck.
    if (event->mimeData()->hasUrls() &&
            event->mimeData()->urls().size() > 0) {
        ControlObject *pPlayCO = ControlObject::getControl(
                    ConfigKey(m_pGroup, "play"));
        if (pPlayCO && (!pPlayCO->get() ||
            m_pConfig->getValueString(ConfigKey("[Controls]","AllowTrackLoadToPlayingDeck")).toInt())) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }
}

void WOverview::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls() &&
            event->mimeData()->urls().size() > 0) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url = urls.first();
        QString name = url.toLocalFile();
        //If the file is on a network share, try just converting the URL to a string...
        if (name == "") {
            name = url.toString();
        }
        event->accept();
        emit(trackDropped(name, m_pGroup));
    } else {
        event->ignore();
    }
}
