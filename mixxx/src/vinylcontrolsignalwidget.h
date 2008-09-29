/*
 * vinylcontrolsignalwidget.h
 *
 *  Created on: 5-Jul-2008
 *      Author: asantoni
 */

#ifndef VINYLCONTROLSIGNALWIDGET_H_
#define VINYLCONTROLSIGNALWIDGET_H_


#include <QtGui>

enum VinylControlSignalType {
    VINYLCONTROL_SIGQUALITY = 0,
    VINYLCONTROL_SIGLEFTCHANNEL,
    VINYLCONTROL_SIGRIGHTCHANNEL,
    VINYLCONTROL_SIGTYPE_NUM
};

class VinylControlSignalWidget : public QGraphicsView
{
    Q_OBJECT
public:
    VinylControlSignalWidget();
    ~VinylControlSignalWidget();
    void setupWidget();
    void resetWidget();
public slots:
    void updateSignalQuality(VinylControlSignalType type, double value);

private:
    QGraphicsScene m_signalScene;
    QGraphicsRectItem* m_signalRectItem[VINYLCONTROL_SIGTYPE_NUM];
    QRectF m_signalRect[VINYLCONTROL_SIGTYPE_NUM];
    QBrush m_signalBrush[VINYLCONTROL_SIGTYPE_NUM];
    QPixmap *m_bg[VINYLCONTROL_SIGTYPE_NUM];

    float m_fRMSvolumeSum[VINYLCONTROL_SIGTYPE_NUM];
    float m_fRMSvolume[VINYLCONTROL_SIGTYPE_NUM];
    long m_samplesCalculated[VINYLCONTROL_SIGTYPE_NUM];

};

#endif /* VINYLCONTROLSIGNALWIDGET_H_ */
