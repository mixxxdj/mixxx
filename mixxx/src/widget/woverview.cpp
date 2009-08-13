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
#include "woverview.h"
#include "wskincolor.h"
#include "trackinfoobject.h"
#include <qpainter.h>
#include <QtDebug>
#include <qpixmap.h>
//Added by qt3to4:
#include <Q3MemArray>
#include <QMouseEvent>
#include <Q3ValueList>
#include <QPaintEvent>
#include "mathstuff.h"
#include <qapplication.h>

WOverview::WOverview(QWidget * parent) : WWidget(parent)
{
    m_pWaveformSummary = 0;
    m_pSegmentation = 0;
    m_liSampleDuration = 0;
    m_iPos = 0;
    m_iVirtualPos = -1;
    m_bDrag = false;
    m_pScreenBuffer = 0;

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
    // Setup position and connections
    WWidget::setup(node);

    // Size
    QString size = selectNodeQString(node, "Size");
    int x = size.left(size.indexOf(",")).toInt();
    int y = size.mid(size.indexOf(",")+1).toInt();
    setFixedSize(x,y);

    // Set constants for line drawing
/*
    m_qMarkerPos1.setX(x/2);
    m_qMarkerPos1.setY(0);
    m_qMarkerPos2.setX(x/2);
    m_qMarkerPos2.setY(y);
    m_qMousePos.setX(x/2);
    m_qMousePos.setY(y/2);
 */

    // Background color
    QColor c(255,255,255);
    if (!selectNode(node, "BgColor").isNull())
    {
        c.setNamedColor(selectNodeQString(node, "BgColor"));
    }
    
    QPalette palette; //Qt4 update according to http://doc.trolltech.com/4.4/qwidget-qt3.html#setBackgroundColor (this could probably be cleaner maybe?)
    palette.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(c));
    this->setPalette(palette);
    

    // If we're doing a warm boot, free the pixmap, and flag it to be regenerated.
    if(m_pScreenBuffer != NULL) {

        // Since setup is called from MixxxView, we know that
        // the current thread is the GUI thread. That way we know
        // that paintEvent is not going on right now, and m_pScreenBuffer
        // is therefore safe to delete.
        
        delete m_pScreenBuffer;
        
        // Flag the pixmap for regeneration.
        waveformChanged = true;
    }
    
    m_pScreenBuffer = new QPixmap(this->size());
    m_pScreenBuffer->fill(this->palette().color(QPalette::Background));
    
    m_qColorSignal.setNamedColor(selectNodeQString(node, "SignalColor"));
    m_qColorSignal = WSkinColor::getCorrectColor(m_qColorSignal);
    m_qColorMarker.setNamedColor(selectNodeQString(node, "MarkerColor"));
    m_qColorMarker = WSkinColor::getCorrectColor(m_qColorMarker);
}

void WOverview::setValue(double fValue)
{
    if (!m_bDrag)
    {
        // Calculate handle position
        m_iPos = (int)((fValue/127.)*((double)width()-2.));
        update();
    }
}

void WOverview::setVirtualPos(double fValue)
{
    if (!m_bDrag)
    {
        // Calculate virtual position
        m_iVirtualPos = (int)((fValue/127.)*((double)width()-2.));
        update();
    }
}

void WOverview::slotLoadNewWaveform(TrackInfoObject* pTrack)
{
	//Update this widget with new waveform summary data from the new track.
    this->setData(pTrack->getWaveSummary(), pTrack->getSegmentationSummary(), 
    			  pTrack->getDuration()*pTrack->getSampleRate()*pTrack->getChannels());
    update();
}

void WOverview::setData(QByteArray* pWaveformSummary, Q3ValueList<long> * pSegmentation, long liSampleDuration)
{
    m_pWaveformSummary = pWaveformSummary;
    m_pSegmentation = pSegmentation;
    m_liSampleDuration = liSampleDuration;

    waveformChanged = true;
}

void WOverview::redrawPixmap() {

    if (!m_pWaveformSummary || !m_pWaveformSummary->size())
        return;

    // Erase background
    m_pScreenBuffer->fill(this->palette().color(this->backgroundRole()));
    
    QPainter paint(m_pScreenBuffer);

    float yscale = (((float)(height()-2)/2.)/128.); //32768.;
    float xscale = (float)m_pWaveformSummary->size()/width();

    float hfcMax = 0.;
    for (unsigned int i=2; i<m_pWaveformSummary->size(); i=i+3)
    {
        if (m_pWaveformSummary->at(i)+128>hfcMax)
            hfcMax = m_pWaveformSummary->at(i)+128;
    }

    // Draw waveform using hfc to determine color
    for (int i=0; i<width(); ++i)
    {
        //const QColor kqLightColor("#2bff00");
        const QColor kqLightColor = m_qColorSignal;
        const QColor kqDarkColor("#1ba200");

        float fMin = 0.;
        float fMax = 0.;

        if ((unsigned int)width()>m_pWaveformSummary->size()/3)
        {
            float pos = (float)(m_pWaveformSummary->size()-3)/3.*(float)i/(float)width();
            int idx = (int)floor(pos);
            float fraction = pos-(float)idx;

            char v1, v2;

            float hfc = (m_pWaveformSummary->at(idx*3+2)+128.)/hfcMax;

            int r = kqDarkColor.red()  + (int)((kqLightColor.red()-kqDarkColor.red())*hfc);
            int g = kqDarkColor.green()+ (int)((kqLightColor.green()-kqDarkColor.green())*hfc);
            int b = kqDarkColor.blue() + (int)((kqLightColor.blue()-kqDarkColor.blue())*hfc);
            paint.setPen(QColor(r,g,b));

            v1 = m_pWaveformSummary->at(idx*3);
            v2 = m_pWaveformSummary->at((idx+1)*3);
            fMin = v1 + (v2-v1)*fraction;

            v1 = m_pWaveformSummary->at(idx*3+1);
            v2 = m_pWaveformSummary->at((idx+1)*3+1);
            fMax = v1 + (v2-v1)*fraction;
        }
        else
        {
            int idxStart = (int)(i*xscale);
            while (idxStart%3!=0 && idxStart>0)
                idxStart--;

            int idxEnd = (int)math_min((i+1)*xscale,m_pWaveformSummary->size());
            while (idxEnd%3!=0 && idxEnd>0)
                idxEnd--;

            for (int j=idxStart; j<idxEnd; j+=3)
            {
                fMin += m_pWaveformSummary->at(j);
                fMax += m_pWaveformSummary->at(j+1);
            }
            fMin /= ((idxEnd-idxStart)/2.);
            fMax /= ((idxEnd-idxStart)/2.);

            float hfc = (m_pWaveformSummary->at(idxStart+2)+128.)/hfcMax;
            //qDebug() << "hfc " << hfc;
            int r = kqDarkColor.red()  + (int)((kqLightColor.red()-kqDarkColor.red())*hfc);
            int g = kqDarkColor.green()+ (int)((kqLightColor.green()-kqDarkColor.green())*hfc);
            int b = kqDarkColor.blue() + (int)((kqLightColor.blue()-kqDarkColor.blue())*hfc);
            paint.setPen(QColor(r,g,b));
        }
//         qDebug() << "min " << fMin << ", max " << fMax;
        paint.drawLine(i, height()/2-(int)(fMin*yscale), i, height()/2-(int)(fMax*yscale));
    }

    // Draw segmentation points
/*
    paint.setPen(QColor("#FF9900"));
    if (m_pSegmentation)
    {
        for (unsigned int i=0; i<m_pSegmentation->size(); ++i)
        {
            int point = (int)((double)width()*((double)(*m_pSegmentation->at(i))/(double)m_liSampleDuration));
            qDebug() << "i " << i << ", seg " << (*m_pSegmentation->at(i)) << "i, dur " << m_liSampleDuration << "i, point " << point << ", width " << width();
            paint.drawLine(point, 0, point, height());
        }
    }
 */
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

    // value ranges from 0 to 127
    float fValue = (double)m_iPos*(127./(double)(width()-2));

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
    // Draw waveform, then playpos
    paint.drawPixmap(0, 0, *m_pScreenBuffer);

    if (m_pWaveformSummary)
    {
        // Draw play position
        paint.setPen(m_qColorMarker);
        paint.drawLine(m_iPos,   0, m_iPos,   height());
        paint.drawLine(m_iPos+1, 0, m_iPos+1, height());
        //paint.drawLine(m_iPos-1, 0, m_iPos-1, height());

        // Draw virtual pos pointer
        if (m_iVirtualPos>=0)
        {
            int dist = math_min(10,abs(m_iVirtualPos-m_iPos));
            //qDebug() << "dist " << dist;
            paint.drawLine(m_iPos, height()/2,   m_iVirtualPos, height()/2);
            paint.drawLine(m_iPos, height()/2+1, m_iVirtualPos, height()/2+1);

            if (m_iVirtualPos>m_iPos)
            {
                paint.drawLine(m_iVirtualPos, height()/2,   m_iVirtualPos-dist, height()/2-dist);
                paint.drawLine(m_iVirtualPos, height()/2+1, m_iVirtualPos-dist, height()/2+dist+1);
            }
            else
            {
                paint.drawLine(m_iVirtualPos, height()/2,   m_iVirtualPos+dist, height()/2-dist);
                paint.drawLine(m_iVirtualPos, height()/2+1, m_iVirtualPos+dist, height()/2+dist+1);
            }
        }
    }
    paint.end();
}


QColor WOverview::getMarkerColor() {
   return m_qColorMarker;
}

QColor WOverview::getSignalColor() {
   return m_qColorSignal;
}
