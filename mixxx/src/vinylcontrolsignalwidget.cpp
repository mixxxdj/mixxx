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



#include <QtCore>
#include <QtGui>
#include "vinylcontrolsignalwidget.h"
#include <math.h>
#include <stdlib.h>

VinylControlSignalWidget::VinylControlSignalWidget()
    : QGraphicsView(),
      m_iTimerId(0) {
    for (int type = 0; type < (int) VINYLCONTROL_SIGTYPE_NUM; type++) {
        m_signalRectItem[type] = NULL;
    }
    setupWidget();
}

VinylControlSignalWidget::~VinylControlSignalWidget()
{
}

void VinylControlSignalWidget::startDrawing() {
    if (m_iTimerId == 0) {
        m_iTimerId = startTimer(50);
    }
}

void VinylControlSignalWidget::stopDrawing() {
    if (m_iTimerId != 0) {
        killTimer(m_iTimerId);
        m_iTimerId = 0;
    }
}

void VinylControlSignalWidget::timerEvent(QTimerEvent *event) {
    updateScene();
}

void VinylControlSignalWidget::updateScene() {
    m_controlLock.lock();
    for (int type = 0; type < (int)VINYLCONTROL_SIGTYPE_NUM; type++) {
        
        if (m_samplesCalculated[type] == 0)
            continue;
        
        QBrush brush;
        if (type == VINYLCONTROL_SIGQUALITY) {
            if (m_fRMSvolume[type] >= 0.990f) {
                m_textItem->setPlainText(tr("OK"));
                brush = QBrush(m_signalGradGood);
            }
            else {
                m_textItem->setPlainText(tr(""));
                brush = QBrush(m_signalGradBad);
            }
        }
        else { //For the left/right channel signals.
            if (m_fRMSvolume[type] < 0.90f && m_fRMSvolume[type] > 0.10f) { //This is totally empirical.
                brush = QBrush(m_signalGradGood);
            } else {
                brush = QBrush(m_signalGradBad);
            }
        }
            
        //The QGraphicsView coord system is upside down...
        int sizeY = this->height();
        m_signalRect[type].setHeight(-m_fRMSvolume[type] * sizeY);
        m_signalRectItem[type]->setBrush(brush);
        m_signalRectItem[type]->setRect(m_signalRect[type]);

        // Reset calculation:
        m_fRMSvolumeSum[type] = 0;
        m_samplesCalculated[type] = 0;
    }
    m_controlLock.unlock();
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


void VinylControlSignalWidget::setupWidget()
{
    int sizeX = this->width();
    int sizeY = this->height();

    m_signalScene.setSceneRect(0,0,sizeX, sizeY);
    m_signalScene.setBackgroundBrush(Qt::black);

    this->setInteractive(false);

    //Disable any scrollbars on the QGraphicsView
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //Initialize QPens
    QPen gridPen(Qt::green);
    QPen graphLinePen(Qt::white);
    QPen signalPen(Qt::black);

    m_signalGradGood = QLinearGradient(0, 0, 0, rect().height());
    m_signalGradBad = QLinearGradient(0, 0, 0, rect().height());
    m_signalGradGood.setColorAt(0, Qt::green);
    m_signalGradGood.setColorAt(1, Qt::darkGreen);    
    m_signalGradBad.setColorAt(0, Qt::red);
    m_signalGradBad.setColorAt(1, Qt::darkRed);
      
            
    //QBrush signalBrush[VINYLCONTROL_SIGTYPE_NUM];
    //QPixmap bg1(this->width() / 3, this->height());
   /*
    QPainter painter(m_bg[type]);
    painter.fillRect(m_bg1->rect(), QBrush(QColor(0, 0, 255)));
    painter.setPen(Qt::red);
    painter.setFont(QFont("Tahoma", 8));
    painter.drawText(rect(), tr("OK")); //Draw the OK text
    painter.end();
    */

    //draw grid
#define GRID_X_LINES 3
#define GRID_Y_LINES 3
    for(int i=1; i < GRID_X_LINES; i++)
    {
        QGraphicsItem* line = m_signalScene.addLine(QLineF(0, i *(sizeY/GRID_X_LINES),
                                                           sizeX,i *(sizeY/GRID_X_LINES)), gridPen);
        line->setZValue(0);
    }
    for(int i=1; i < GRID_Y_LINES; i++)
    {
        QGraphicsItem* line = m_signalScene.addLine(QLineF( i * (sizeX/GRID_Y_LINES), 0,
                                                            i * (sizeX/GRID_Y_LINES), sizeY), gridPen);
        line->setZValue(0);
    }

    for (int type = 0; type < (int)VINYLCONTROL_SIGTYPE_NUM; type++)
    {
        m_signalRect[type].setX(type * (sizeX / 3));
        m_signalRect[type].setY(sizeY);
        m_signalRect[type].setWidth(sizeX / 3);
        m_signalRect[type].setHeight(1);
        m_signalRectItem[type] = m_signalScene.addRect(m_signalRect[type],
                                                       signalPen,
                                                       QBrush(m_signalGradGood));
        m_signalRectItem[type]->setZValue(1);
    }

    m_textItem = m_signalScene.addText("", QFont("Tahoma", 8));
    m_textItem->setPos(QPointF(1, 1));
    m_textItem->setDefaultTextColor(QColor(0,0,0));
    m_textItem->setZValue(2);

    this->setScene(&m_signalScene);
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
