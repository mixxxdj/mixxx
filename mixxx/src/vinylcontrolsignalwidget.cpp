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


VinylControlSignalWidget::VinylControlSignalWidget() : QGraphicsView()
{
    resetWidget();

}

VinylControlSignalWidget::~VinylControlSignalWidget()
{

}

void VinylControlSignalWidget::resetWidget()
{
    for (int type = 0; type < (int)VINYLCONTROL_SIGTYPE_NUM; type++)
    {
        m_fRMSvolumeSum[type] = 0.0f;
        m_fRMSvolume[type] = 0.0f;
        m_samplesCalculated[type] = 0;
        updateSignalQuality((VinylControlSignalType)type, 0.0f);
    }
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
    QPen signalPen(Qt::blue);


    //QBrush signalBrush[VINYLCONTROL_SIGTYPE_NUM];
    //QPixmap bg1(this->width() / 3, this->height());
   /*
    QPainter painter(m_bg[type]);
    painter.fillRect(m_bg1->rect(), QBrush(QColor(0, 0, 255)));
    painter.setPen(Qt::red);
    painter.setFont(QFont("Tahoma", 8));
    painter.drawText(rect(), tr("OK")); //Draw the OK text
    painter.end();
    m_signalBrush.setTexture(*m_bg1);
    */

    //draw grid
#define GRID_X_LINES 3
#define GRID_Y_LINES 3
    for(int i=1; i < GRID_X_LINES; i++)
    {
        m_signalScene.addLine(QLineF(0, i *(sizeY/GRID_X_LINES),sizeX,i *(sizeY/GRID_X_LINES)), gridPen);
    }
    for(int i=1; i < GRID_Y_LINES; i++)
    {
        m_signalScene.addLine(QLineF( i * (sizeX/GRID_Y_LINES), 0, i * (sizeX/GRID_Y_LINES), sizeY), gridPen);
    }

    for (int type = 0; type < (int)VINYLCONTROL_SIGTYPE_NUM; type++)
    {
        m_bg[type] = new QPixmap(this->width(), this->height());
        m_signalRect[type].setX(type * (sizeX / 3));
        m_signalRect[type].setY(sizeY);
        m_signalRect[type].setWidth(sizeX / 3);
        m_signalRect[type].setHeight(1);
        m_signalRectItem[type] = m_signalScene.addRect(m_signalRect[type],
                                                       signalPen,
                                                       m_signalBrush[type]);
    }

    this->setScene(&m_signalScene);
}

/** @brief Wraps Updates the signal quality indicators with a given signal strength
  * @param indicator_index Identifies the corresponding channel and indicator to be updated.
  * @param value The new signal quality level for the specified channel (0.0f-1.0f)
  */
void VinylControlSignalWidget::updateSignalQuality(VinylControlSignalType type,
                                                   double value)
{
    m_fRMSvolumeSum[type] += value;

    const float ATTACK_SMOOTHING = .3;
    const float DECAY_SMOOTHING  = .1;//.16//.4

    if (m_samplesCalculated[type] > 1)
    {
        float m_fRMSvolumePrev = m_fRMSvolume[type];
        float smoothFactor;

        m_fRMSvolume[type] = value;//log10(m_fRMSvolumeSum/(samplesCalculated*1000)+1);
        //Smooth the output
        smoothFactor = (m_fRMSvolumePrev > m_fRMSvolume[type]) ? DECAY_SMOOTHING : ATTACK_SMOOTHING;
        m_fRMSvolume[type] = m_fRMSvolumePrev + smoothFactor * (m_fRMSvolume[type] - m_fRMSvolumePrev);

        QColor signalColour;
        signalColour.setRed((int)(255 - m_fRMSvolume[type]*255));
        signalColour.setGreen((int)m_fRMSvolume[type]*255);
           QPainter painter(m_bg[type]);
           painter.fillRect(m_bg[type]->rect(), QBrush(signalColour));
           painter.setPen(Qt::black);
           painter.setFont(QFont("Tahoma", 8));
           painter.drawText(rect(), tr("OK")); //Draw the OK text
           painter.end();
           m_signalBrush[type].setTexture(*m_bg[type]);
           m_signalRectItem[type]->setBrush(m_signalBrush[type]);

        int sizeY = this->height();
        m_signalRect[type].setHeight(-m_fRMSvolume[type] * sizeY); //The QGraphicsView coord system is upside down...
        m_signalRectItem[type]->setRect(m_signalRect[type]);

        // Reset calculation:
        m_samplesCalculated[type] = 0;
        m_fRMSvolumeSum[type] = 0;

    }

    m_samplesCalculated[type]++;

}
