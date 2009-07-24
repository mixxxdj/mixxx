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
    
    void resetWidget();
    void startDrawing();
    void stopDrawing();
    void updateScene();
    void setupWidget();
                      
public slots:
    void updateSignalQuality(VinylControlSignalType type, double value);

protected:
    void timerEvent (QTimerEvent* event);
    
private:
    QMutex m_controlLock;
    QGraphicsScene m_signalScene;
    QGraphicsRectItem* m_signalRectItem[VINYLCONTROL_SIGTYPE_NUM];
    QGraphicsTextItem* m_textItem;
    QRectF m_signalRect[VINYLCONTROL_SIGTYPE_NUM];
    
    QLinearGradient m_signalGradGood;
    QLinearGradient m_signalGradBad;

    float m_fRMSvolumeSum[VINYLCONTROL_SIGTYPE_NUM];
    float m_fRMSvolume[VINYLCONTROL_SIGTYPE_NUM];
    long m_samplesCalculated[VINYLCONTROL_SIGTYPE_NUM];

    int m_iTimerId;
};

#endif /* VINYLCONTROLSIGNALWIDGET_H_ */
