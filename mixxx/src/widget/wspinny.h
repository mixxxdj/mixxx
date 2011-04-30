
#ifndef _WSPINNY_H
#define _WSPINNY_H

#include "wwidget.h"
#include "midi/pitchfilter.h"
//TODO: Use a pointer to pitchfilter and get rid of include. (fwd decl)

class ControlObjectThreadMain;

class WSpinny : public WWidget
{
    Q_OBJECT
    public:
        WSpinny(QWidget* parent);
        ~WSpinny();
        void setup(QDomNode node);
    public slots:
        void updateAngle(double);
        void updateAngleForGhost();
        void updatePitchFilter();
    protected:
        //QWidget:
        void paintEvent(QPaintEvent*);
        void mouseMoveEvent(QMouseEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * e);
        void wheelEvent(QWheelEvent *e);

        double calculateAngle(double playpos);
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
        ControlObjectThreadMain* m_pBPM;
        ControlObjectThreadMain* m_pScratch;
        ControlObjectThreadMain* m_pScratchToggle;
        ControlObjectThreadMain* m_pScratchPos;
        float m_fAngle; //Degrees
        float m_fGhostAngle; 
        QElapsedTimer m_time;
        QElapsedTimer m_velocityTime;
        double m_dPausedPosition;
        bool m_bGhostPlayback;
        bool m_bScratchPlayback;
        QTimer m_ghostTimer;
        int m_iStartMouseX;
        int m_iStartMouseY;
        int m_iCompleteRotations;
        double m_dPrevTheta;
        PitchFilter m_pitchFilter;
        double m_dPrevPosOffset; /** Used for the pitch filter */
        QTimer m_pitchFilterTimer; 
        double m_dVelocity;
        double m_dPrevVelocity;
        double m_dAcceleration;
        double m_dDeltaPosInSeconds;
        double m_dTheta;
        double m_dPrevY;
};

#endif //_WSPINNY_H
