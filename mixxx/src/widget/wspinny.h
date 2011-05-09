
#ifndef _WSPINNY_H
#define _WSPINNY_H

#include <QGLWidget>
#include "wwidget.h"

class ControlObjectThreadMain;

class WSpinny : public QGLWidget
{
    Q_OBJECT
    public:
        WSpinny(QWidget* parent);
        ~WSpinny();
        void setup(QDomNode node, QString group);
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
    public slots:
        void updateAngle(double);
        void updateAngleForGhost();
    signals:
        void trackDropped(QString filename, QString group);
    protected:
        //QWidget:
        void paintEvent(QPaintEvent*);
        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);
        void wheelEvent(QWheelEvent *e);

        double calculateAngle(double playpos);
        int calculateFullRotations(double playpos);
        double calculatePositionFromAngle(double angle);
    private:
        QPixmap* m_pBG;
        QPixmap* m_pFG;
        QPixmap* m_pGhost;
        ControlObjectThreadMain* m_pPlay;
        ControlObjectThreadMain* m_pPlayPos;
        ControlObjectThreadMain* m_pVisualPlayPos;
        ControlObjectThreadMain* m_pDuration;
        ControlObjectThreadMain* m_pTrackSamples;
        ControlObjectThreadMain* m_pTrackSampleRate;
        ControlObjectThreadMain* m_pBPM;
        ControlObjectThreadMain* m_pScratch;
        ControlObjectThreadMain* m_pScratchToggle;
        ControlObjectThreadMain* m_pScratchPos;
        QString m_group;
        float m_fAngle; //Degrees
        float m_fGhostAngle; 
        QTime m_time;
        double m_dPausedPosition;
        bool m_bGhostPlayback;
        QTimer m_ghostPaintTimer;
        int m_iStartMouseX;
        int m_iStartMouseY;
        int m_iFullRotations;
        double m_dPrevTheta;
        double m_dTheta;
};

#endif //_WSPINNY_H
