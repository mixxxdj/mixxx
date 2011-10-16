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
      m_pVinylControl(NULL),
      m_iSize(MIXXX_VINYL_SCOPE_SIZE),
      m_qImage(),
      m_bVinylActive(FALSE),
      m_imageData(NULL) {
}

void VinylControlSignalWidget::setSize(int size)
{
    m_iSize = size;
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    setMinimumSize(size, size);
    setMaximumSize(size, size);
    m_imageData = new uchar[size * size * 4];
    m_qImage = QImage(m_imageData, size, size, 0, QImage::Format_ARGB32);
}

VinylControlSignalWidget::~VinylControlSignalWidget()
{
    delete [] m_imageData;
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

void VinylControlSignalWidget::setVinylActive(bool active)
{
    if (m_bVinylActive != active && !active)
        resetWidget();
    m_bVinylActive = active;
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

void VinylControlSignalWidget::timerEvent(QTimerEvent *event)
{
    if (m_pVinylControl) {
        m_iAngle = (int)m_pVinylControl->getAngle();

        unsigned char * buf = m_pVinylControl->getScopeBytemap();

        int r,g,b;

        QColor qual_color = QColor();
        m_fSignalQuality = m_pVinylControl->getTimecodeQuality();

        //color is related to signal quality
        //hsv:  s=1, v=1
        //h is the only variable.
        //h=0 is red, h=120 is green
        qual_color.setHsv((int)(120.0 * m_fSignalQuality), 255, 255);
        qual_color.getRgb(&r, &g, &b);

        if (buf) {
            for (int x=0; x<m_iSize; x++) {
                for(int y=0; y<m_iSize; y++) {
                    //XXX: endianness means this is backwards....
                    //does this break on other platforms?
                    m_imageData[4*(x+m_iSize*y)+0] = (uchar)b;
                    m_imageData[4*(x+m_iSize*y)+1] = (uchar)g;
                    m_imageData[4*(x+m_iSize*y)+2] = (uchar)r;
                    m_imageData[4*(x+m_iSize*y)+3] = (uchar)buf[x+m_iSize*y];
                }
            }
        }
    } else {
        m_fSignalQuality = 0.0;
    }
    update();
}

void VinylControlSignalWidget::resetWidget()
{
    m_controlLock.lock();
    for (int type = 0; type < (int)VINYLCONTROL_SIGTYPE_NUM; type++) {
        m_fRMSvolumeSum[type] = 0.0f;
        m_fRMSvolume[type] = 0.0f;
        m_samplesCalculated[type] = 0;
    }
    memset(m_imageData, 0, sizeof(uchar) * m_iSize * m_iSize * 4);
    m_controlLock.unlock();
}

void VinylControlSignalWidget::paintEvent(QPaintEvent* event)
{
    int sizeX = this->width();
    int sizeY = this->height();

    QPainter painter(this);
    painter.fillRect(this->rect(), QBrush(QColor(0, 0, 0)));

    if (m_bVinylActive) { //if timer is stopped, only draw the BG
        //main axes
        painter.setPen(QColor(0, 255, 0));
        painter.drawLine(sizeX / 2, 0, sizeX / 2, sizeY);
        painter.drawLine(0, sizeY / 2, sizeX, sizeY / 2);

        //quarter axes
        painter.setPen(QColor(0, 127, 0));
        painter.drawLine(sizeX * 0.25, 0, sizeX * 0.25, sizeY);
        painter.drawLine(sizeX * 0.75, 0, sizeX * 0.75, sizeY);
        painter.drawLine(0, sizeY * 0.25, sizeX, sizeY * 0.25);
        painter.drawLine(0, sizeY * 0.75, sizeX, sizeY * 0.75);

        //sweep
        if (m_iAngle >= 0) {
            //sweep fades along with signal quality
            painter.setPen(QColor(255, 255, 255, (int)(127.0 * m_fSignalQuality)));
            painter.setBrush(QColor(255, 255, 255, (int)(127.0 * m_fSignalQuality)));
            painter.drawPie(0, 0, sizeX, sizeY, m_iAngle*16, 6*16);
        }

        if (!m_qImage.isNull()) {
            //vinyl signal -- thanks xwax!
            painter.drawImage(this->rect(), m_qImage);
        }
    }
}
