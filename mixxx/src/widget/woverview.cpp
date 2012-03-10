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

WOverview::WOverview(const char *pGroup, QWidget * parent)
        : WWidget(parent),
          m_pGroup(pGroup) {
    m_waveformSummary = QByteArray();
    m_liSampleDuration = 0;
    m_iPos = 0;
    m_bDrag = false;
    m_pScreenBuffer = 0;

    // Analyse progress
    m_iProgress = 0;
    m_analysing = false;

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

    waveformChanged = false;
}

WOverview::~WOverview()
{
    if(m_pScreenBuffer != NULL) {
        delete m_pScreenBuffer;
        m_pScreenBuffer = NULL;
    }
}

void WOverview::setup(QDomNode node)
{
    // Set constants for line drawing
/*
    m_qMarkerPos1.setX(x/2);
    m_qMarkerPos1.setY(0);
    m_qMarkerPos2.setX(x/2);
    m_qMarkerPos2.setY(y);
    m_qMousePos.setX(x/2);
    m_qMousePos.setY(y/2);
 */

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

    // If we're doing a warm boot, free the pixmap, and flag it to be regenerated.
    if(m_pScreenBuffer != NULL) {

        // Since setup is called from MixxxView, we know that
        // the current thread is the GUI thread. That way we know
        // that paintEvent is not going on right now, and m_pScreenBuffer
        // is therefore safe to delete.

        delete m_pScreenBuffer;
    }

    m_pScreenBuffer = new QPixmap(size());
    // Flag the pixmap for regeneration.
    waveformChanged = true;

    m_qColorSignal.setNamedColor(selectNodeQString(node, "SignalColor"));
    m_qColorSignal = WSkinColor::getCorrectColor(m_qColorSignal);
    m_qColorMarker.setNamedColor(selectNodeQString(node, "MarkerColor"));
    m_qColorMarker = WSkinColor::getCorrectColor(m_qColorMarker);

    m_qColorProgress = m_qColorSignal;
    if (!selectNode(node, "ProgressColor").isNull()) {
        m_qColorProgress.setNamedColor(selectNodeQString(node, "ProgressColor"));
        m_qColorProgress = WSkinColor::getCorrectColor(m_qColorProgress);
    }

    m_iProgressAlpha = 80;
    if (!selectNode(node, "ProgressAlpha").isNull()) {
        m_iProgressAlpha = selectNodeInt(node, "ProgressAlpha");
    }
}

void WOverview::setValue(double fValue)
{
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

void WOverview::slotTrackLoaded(TrackPointer pTrack)
{
    // Set current track of this overview widget
    m_pCurrentTrack = pTrack;

    // If the track already has been analysed slotLoadNewWaveform will reset
    // this parameter.
    m_analysing = true;
    m_iProgress = 0;
    update();

    if (pTrack) {
        TrackInfoObject* pTrackInfo = pTrack.data();

        // Connect wavesummaryUpdated signals to our update slots.
        connect(pTrackInfo, SIGNAL(wavesummaryUpdated(TrackInfoObject*)),
                this, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
        // Now in case the track's wavesummary is already done, load it.
        slotLoadNewWaveform(pTrackInfo);

    }
}

void WOverview::slotLoadNewWaveform(TrackInfoObject* pTrack)
{
    //Update this widget with new waveform summary data from the new track.
    setData(pTrack->getWaveSummary(),
            pTrack->getDuration()*pTrack->getSampleRate()*pTrack->getChannels());

    // If no data is available, we can expect it to be analysed.
    if (!pTrack->getWaveSummary()->isNull() && !pTrack->getWaveSummary()->isEmpty())
    {
        m_analysing = false;
        m_iProgress = 0;
    }

    update();
}

void WOverview::slotUnloadTrack(TrackPointer pTrack) {
    // Unset current track of this overview widget
    m_pCurrentTrack.clear();

    if (pTrack) {
        disconnect(pTrack.data(), SIGNAL(wavesummaryUpdated(TrackInfoObject*)),
                   this, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
    }
    QByteArray ba;
    setData(&ba, 0);
    m_analysing = false;
    m_iProgress = 0;
    update();
}

void WOverview::slotTrackProgress(TrackPointer pTrack, int progress)
{
    if (pTrack == m_pCurrentTrack) {
        if (progress != m_iProgress) {
            m_iProgress = progress;
            update();
        }
    }
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

void WOverview::setData(const QByteArray* pWaveformSummary, long liSampleDuration)
{
    //COPY THE EFFING WAVESUMMARY BYTES so we don't end up with
    //a bad pointer in case the TrackInfoObject that pWavefromSummary originates
    //from is deallocated. -- Albert Dec 22, 2009
    m_waveformSummary = *pWaveformSummary;
    m_liSampleDuration = liSampleDuration;

    waveformChanged = true;
}

void WOverview::redrawPixmap() {
    // Fill with transparent pixels
    m_pScreenBuffer->fill(m_qColorBackground);

    QPainter paint(m_pScreenBuffer);
    if (!m_backgroundPixmap.isNull()) {
        paint.drawTiledPixmap(m_pScreenBuffer->rect(), m_backgroundPixmap, QPoint(0,0));
    } else {
        paint.fillRect(m_pScreenBuffer->rect(), m_qColorBackground);
    }

    if (!m_waveformSummary.size()) {
        update();
        return;
    }

    float yscale = (((float)(height()-2)/2.)/128.); //32768.;
    float xscale = (float)m_waveformSummary.size()/width();

    float hfcMax = 0.;
    for (unsigned int i=2; i<m_waveformSummary.size(); i=i+3)
    {
        if (m_waveformSummary.at(i)+128>hfcMax)
            hfcMax = m_waveformSummary.at(i)+128;
    }

    // Draw waveform using hfc to determine color
    for (int i=0; i<width(); ++i)
    {
        //const QColor kqLightColor("#2bff00");
        const QColor kqLightColor = m_qColorSignal;
        const QColor kqDarkColor("#1ba200");

        float fMin = 0.;
        float fMax = 0.;

        if ((unsigned int)width()>m_waveformSummary.size()/3)
        {
            float pos = (float)(m_waveformSummary.size()-3)/3.*(float)i/(float)width();
            int idx = (int)floor(pos);
            float fraction = pos-(float)idx;

            char v1, v2;

            float hfc = (m_waveformSummary.at(idx*3+2)+128.)/hfcMax;

            int r = kqDarkColor.red()  + (int)((kqLightColor.red()-kqDarkColor.red())*hfc);
            int g = kqDarkColor.green()+ (int)((kqLightColor.green()-kqDarkColor.green())*hfc);
            int b = kqDarkColor.blue() + (int)((kqLightColor.blue()-kqDarkColor.blue())*hfc);
            paint.setPen(QColor(r,g,b));

            v1 = m_waveformSummary.at(idx*3);
            v2 = m_waveformSummary.at((idx+1)*3);
            fMin = v1 + (v2-v1)*fraction;

            v1 = m_waveformSummary.at(idx*3+1);
            v2 = m_waveformSummary.at((idx+1)*3+1);
            fMax = v1 + (v2-v1)*fraction;
        }
        else
        {
            int idxStart = (int)(i*xscale);
            while (idxStart%3!=0 && idxStart>0)
                idxStart--;

            int idxEnd = (int)math_min((i+1)*xscale,m_waveformSummary.size());
            while (idxEnd%3!=0 && idxEnd>0)
                idxEnd--;

            for (int j=idxStart; j<idxEnd; j+=3)
            {
                fMin += m_waveformSummary.at(j);
                fMax += m_waveformSummary.at(j+1);
            }
            fMin /= ((idxEnd-idxStart)/2.);
            fMax /= ((idxEnd-idxStart)/2.);

            float hfc = (m_waveformSummary.at(idxStart+2)+128.)/hfcMax;
            //qDebug() << "hfc " << hfc;
            int r = kqDarkColor.red()  + (int)((kqLightColor.red()-kqDarkColor.red())*hfc);
            int g = kqDarkColor.green()+ (int)((kqLightColor.green()-kqDarkColor.green())*hfc);
            int b = kqDarkColor.blue() + (int)((kqLightColor.blue()-kqDarkColor.blue())*hfc);
            paint.setPen(QColor(r,g,b));
        }
//         qDebug() << "min " << fMin << ", max " << fMax;
        paint.drawLine(i, height()/2-(int)(fMin*yscale), i, height()/2-(int)(fMax*yscale));
    }

    paint.end();
    update();
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
    if(!m_pScreenBuffer)
        return;

    if (waveformChanged) {
      redrawPixmap();
      waveformChanged = false;
    }

    QPainter paint(this);
    // Fill with transparent pixels
    paint.fillRect(rect(), QColor(0,0,0,0));

    // Draw waveform, then playpos
    paint.drawPixmap(rect(), *m_pScreenBuffer, m_pScreenBuffer->rect());

    // Draw progress bar for track analysis
    paintTrackProgress(paint);

    if (m_liSampleDuration > 0) {
        // Draw play position
        paint.setPen(m_qColorMarker);
        paint.drawLine(m_iPos,   0, m_iPos,   height());
        paint.drawLine(m_iPos+1, 0, m_iPos+1, height());
        //paint.drawLine(m_iPos-1, 0, m_iPos-1, height());

        float fPos;

        // Draw loop markers
        QColor loopColor = m_qColorMarker;
        if (!m_bLoopEnabled) {
            loopColor = m_qColorSignal;
        }
        paint.setPen(loopColor);
        if (m_dLoopStart != -1.0) {
            fPos = m_dLoopStart * (width() - 2) / m_liSampleDuration;
            paint.drawLine(fPos, 0, fPos, height());
        }
        if (m_dLoopEnd != -1.0) {
            fPos = m_dLoopEnd * (width() - 2) / m_liSampleDuration;
            paint.drawLine(fPos, 0, fPos, height());
        }

        if (m_dLoopStart != -1.0 && m_dLoopEnd != -1.0) {
            //loopColor.setAlphaF(0.5);
            paint.setOpacity(0.5);
            //paint.setPen(loopColor);
            paint.setBrush(QBrush(loopColor));
            float sPos = m_dLoopStart * (width() - 2) / m_liSampleDuration;
            float ePos = m_dLoopEnd * (width() - 2) / m_liSampleDuration;
            QRectF rect(QPointF(sPos, 0), QPointF(ePos, height()-1));
            paint.drawRect(rect);
            paint.setOpacity(1.0);
        }

        QFont font;
        font.setBold(false);
        int textWidth = 8;
        int textHeight = 10;
        font.setPixelSize(2*textHeight);
        paint.setPen(m_qColorMarker);
        paint.setFont(font);

        // Draw hotcues
        for (int i = 0; i < m_hotcues.size(); ++i) {
            int position = m_hotcues[i];
            if (position == -1)
                continue;
            fPos = float(position) * (width()-2) / m_liSampleDuration;
            //qDebug() << "Drawing cue" << i << "at" << fPos;

            paint.drawLine(fPos, 0,
                           fPos, height());
            // paint.drawLine(fPos+1, 0,
            //                fPos+1, height());

            // int halfHeight = height()/2;
            // QRectF rect(QPointF(fPos-textWidth, halfHeight-textHeight),
            //             QPointF(fPos+textWidth, halfHeight+textHeight));

            // paint.drawText(rect, Qt::AlignCenter, QString("%1").arg(i+1));
        }
    }

    paint.end();
}

void WOverview::paintTrackProgress(QPainter& pPainter) {
    if (m_analysing) {
        // Prepare rectangle
        QRectF buf = m_pScreenBuffer->rect();
        qreal width = static_cast<float>(buf.width() * m_iProgress) / 100.0f;
        qreal height = buf.height();
        QRectF bar(0, 0, width, height);

        // Prepare color
        QColor color = m_qColorProgress;
        color.setAlpha(m_iProgressAlpha);

        // Paint translucent rectangle representing analysis progress
        pPainter.fillRect(bar, color);
    }
}

QColor WOverview::getMarkerColor() {
   return m_qColorMarker;
}

QColor WOverview::getSignalColor() {
   return m_qColorSignal;
}

void WOverview::dragEnterEvent(QDragEnterEvent* event) {
    // Accept the enter event if the thing is a filepath and nothing's playing
    // in this deck.
    if (event->mimeData()->hasUrls() &&
        event->mimeData()->urls().size() > 0) {
        ControlObject *pPlayCO = ControlObject::getControl(
            ConfigKey(m_pGroup, "play"));
        if (pPlayCO && pPlayCO->get()) {
            event->ignore();
        } else {
            event->acceptProposedAction();
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
