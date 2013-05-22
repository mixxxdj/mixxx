/*
 * vinylcontrolsignalwidget.h
 *
 *  Created on: 5-Jul-2008
 *      Author: asantoni
 */

#ifndef VINYLCONTROLSIGNALWIDGET_H
#define VINYLCONTROLSIGNALWIDGET_H

#include <QtGui>
#include <QPainter>
#include <QTimerEvent>

#include "vinylcontrol/vinylsignalquality.h"

class VinylControlSignalWidget : public QWidget, public VinylSignalQualityListener {
    Q_OBJECT
  public:
    VinylControlSignalWidget();
    VinylControlSignalWidget(int size);
    virtual ~VinylControlSignalWidget();

    void onVinylSignalQualityUpdate(const VinylSignalQualityReport& report);

    void paintEvent(QPaintEvent* event);
    void setVinylInput(int input);
    void setSize(int size);
    void setVinylActive(bool active);

    void resetWidget();

  private:
    int m_iVinylInput;
    int m_iSize;

    QImage m_qImage;
    unsigned char * m_imageData;
    int m_iAngle;
    float m_fSignalQuality;
    bool m_bVinylActive;
};

#endif /* VINYLCONTROLSIGNALWIDGET_H */
