/***************************************************************************
                          vinylcontrolsignalwidget.cpp
                             -------------------
    begin                : July 5, 2008
    copyright            : (C) 2008 by Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/**
    TODO(XXX): Feb 2011 - asantoni 
        * This class probably doesn't need the locking anymore. 

*/

#include <QtCore>
#include <QtGui>
#include <math.h>
#include "vinylcontrolsignalwidget.h"
#include "vinylcontrolproxy.h"

VinylControlSignalWidget::VinylControlSignalWidget()
    : QWidget(),
      m_iTimerId(0),
      m_pVinylControl(NULL) {
    for (int type = 0; type < (int) VINYLCONTROL_SIGTYPE_NUM; type++) {
    }

}

VinylControlSignalWidget::~VinylControlSignalWidget()
{
}

/** This gets called before the VinylControlProxy objects get destroyed, in
    order to prevent us from having bad pointers dangling. */
void VinylControlSignalWidget::invalidateVinylControl()
{
    m_pVinylControl = NULL;
}

void VinylControlSignalWidget::setVinylControlProxy(VinylControlProxy* vc)
{

    m_pVinylControl = vc;
    //Catch when the VinylControl objects get deleted (like when
    //you change your vinyl type)
    connect(m_pVinylControl, SIGNAL(destroyed()),
            this, SLOT(invalidateVinylControl()));
}

void VinylControlSignalWidget::startDrawing() {
    if (m_iTimerId == 0) {
        m_iTimerId = startTimer(60);
    }
}

void VinylControlSignalWidget::stopDrawing() {
    if (m_iTimerId != 0) {
        killTimer(m_iTimerId);
        m_iTimerId = 0;
    }
}

void VinylControlSignalWidget::timerEvent(QTimerEvent *event) {
    if (m_pVinylControl) {
        updateSignalQuality(VINYLCONTROL_SIGQUALITY, 
                            m_pVinylControl->getTimecodeQuality());
        updateSignalQuality(VINYLCONTROL_SIGLEFTCHANNEL, 
                            m_pVinylControl->getSignalLeft());
        updateSignalQuality(VINYLCONTROL_SIGRIGHTCHANNEL, 
                            m_pVinylControl->getSignalRight());
    } 
    else {
        updateSignalQuality(VINYLCONTROL_SIGQUALITY, 
                            0.0f);
        updateSignalQuality(VINYLCONTROL_SIGLEFTCHANNEL, 
                            0.0f);
        updateSignalQuality(VINYLCONTROL_SIGRIGHTCHANNEL, 
                            0.0f);
    }
    update();
}

void VinylControlSignalWidget::resetWidget()
{
    m_controlLock.lock();
    for (int type = 0; type < (int)VINYLCONTROL_SIGTYPE_NUM; type++)
    {
        m_fRMSvolumeSum[type] = 0.0f;
        m_fRMSvolume[type] = 0.0f;
        m_samplesCalculated[type] = 0;
    }
    m_controlLock.unlock();
}

void VinylControlSignalWidget::paintEvent(QPaintEvent* event)
{
    int sizeX = this->width();
    int sizeY = this->height();

    QPainter painter(this);
    //painter.fillRect(this->rect(), QBrush(Qt::black));

    //Paint the background as a measure of whether or not the timecode is
    //being successfully decoded.
    painter.fillRect(this->rect(), QBrush(QColor(0, m_fRMSvolume[0]*255, 0)));

    for (int type = 0; type < (int)VINYLCONTROL_SIGTYPE_NUM; type++) {
        
        if (m_samplesCalculated[type] == 0)
            continue;
        QBrush brush;
        if (type == VINYLCONTROL_SIGQUALITY) {
            //Hmmm.
        }
        else { //For the left/right channel signals.
            
            painter.setPen(QPen(Qt::white));
            int width = m_fRMSvolume[1]*sizeX;
            int height = m_fRMSvolume[2]*sizeY;
            int centerX = (sizeX-width) / 2;
            int centerY = (sizeY-height) / 2;
            painter.drawEllipse(centerX,
                                centerY,
                                width,
                                height);
        }

        // Reset calculation:
        m_fRMSvolumeSum[type] = 0;
        m_samplesCalculated[type] = 0;
    }
}


/** @brief Wraps Updates the signal quality indicators with a given signal strength
  * @param indicator_index Identifies the corresponding channel and indicator to be updated.
  * @param value The new signal quality level for the specified channel (0.0f-1.0f)
  */
void VinylControlSignalWidget::updateSignalQuality(VinylControlSignalType type,
                                                   double value)
{
    const float ATTACK_SMOOTHING = .3;
    const float DECAY_SMOOTHING  = .1;//.16//.4
        
    m_controlLock.lock();
    
    m_fRMSvolumeSum[type] += value;
    
    float m_fRMSvolumePrev = m_fRMSvolume[type];
    
    //Use a log10 here so that we display dB.
    m_fRMSvolume[type] = log10(1+value*9); //log10(m_fRMSvolumeSum/(samplesCalculated*1000)+1);
    
    //Smooth the output
    float smoothFactor = (m_fRMSvolumePrev > m_fRMSvolume[type]) ? DECAY_SMOOTHING : ATTACK_SMOOTHING;
    
    m_fRMSvolume[type] = m_fRMSvolumePrev + smoothFactor * (m_fRMSvolume[type] - m_fRMSvolumePrev);

    m_samplesCalculated[type]++;
    m_controlLock.unlock();
}
