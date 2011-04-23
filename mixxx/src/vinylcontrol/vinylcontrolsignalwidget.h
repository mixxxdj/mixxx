/*
 * vinylcontrolsignalwidget.h
 *
 *  Created on: 5-Jul-2008
 *      Author: asantoni
 */

#ifndef VINYLCONTROLSIGNALWIDGET_H_
#define VINYLCONTROLSIGNALWIDGET_H_

#include <QtGui>
#include <QPainter>
#include <QTimerEvent>

class VinylControlProxy;

enum VinylControlSignalType {
    VINYLCONTROL_SIGQUALITY = 0,
    VINYLCONTROL_SIGLEFTCHANNEL,
    VINYLCONTROL_SIGRIGHTCHANNEL,
    VINYLCONTROL_SIGTYPE_NUM
};

class VinylControlSignalWidget : public QWidget
{
    Q_OBJECT
public:
    VinylControlSignalWidget();
    VinylControlSignalWidget(int size);
    ~VinylControlSignalWidget();
    void setVinylControlProxy(VinylControlProxy* vc);
    void paintEvent(QPaintEvent* event);
    void setSize(int size);
    void setVinylActive(bool active);

    void resetWidget();
    void startDrawing();
    void stopDrawing();

public slots:
    void invalidateVinylControl();

protected:
    void timerEvent(QTimerEvent* event);

private:
    QMutex m_controlLock;
    VinylControlProxy* m_pVinylControl;

    float m_fRMSvolumeSum[VINYLCONTROL_SIGTYPE_NUM];
    float m_fRMSvolume[VINYLCONTROL_SIGTYPE_NUM];
    long m_samplesCalculated[VINYLCONTROL_SIGTYPE_NUM];

    int m_iTimerId;
    int m_iSize;

    QImage m_qImage;
    unsigned char * m_imageData;
    int m_iAngle;
    float m_fSignalQuality;
    bool m_bVinylActive;
};

#endif /* VINYLCONTROLSIGNALWIDGET_H_ */
