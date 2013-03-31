
#ifndef _WSPINNY_H
#define _WSPINNY_H

#include <QGLWidget>
#include "wwidget.h"
#include "vinylcontrol/vinylcontrolmanager.h"
#ifdef __VINYLCONTROL__
#include "vinylcontrol/vinylcontrolproxy.h"
#include "vinylcontrol/vinylcontrol.h"
#endif

class ControlObjectThreadMain;
class VisualPlayPosition;

class WSpinny : public QGLWidget {
    Q_OBJECT
  public:
    WSpinny(QWidget* parent, VinylControlManager* pVCMan);
    virtual ~WSpinny();

    void setup(QDomNode node, QString group);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

  public slots:
    void updateVinylControlSpeed(double rpm);
    void updateVinylControlEnabled(double enabled);
    void invalidateVinylControl();

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
    QImage* m_pBgImage;
    QImage* m_pFgImage;
    QImage* m_pGhostImage;
    ControlObjectThreadMain* m_pPlay;
    ControlObjectThreadMain* m_pPlayPos;
    VisualPlayPosition* m_pVisualPlayPos;
    ControlObjectThreadMain* m_pDuration;
    ControlObjectThreadMain* m_pTrackSamples;
    ControlObjectThreadMain* m_pTrackSampleRate;
    ControlObjectThreadMain* m_pScratch;
    ControlObjectThreadMain* m_pScratchToggle;
    ControlObjectThreadMain* m_pScratchPos;
    ControlObjectThreadMain* m_pRate;
    ControlObjectThreadMain* m_pVinylControlSpeedType;
    ControlObjectThreadMain* m_pVinylControlEnabled;
    ControlObjectThreadMain* m_pSignalEnabled;
    ControlObjectThreadMain* m_pSlipEnabled;

#ifdef __VINYLCONTROL__
    VinylControlManager* m_pVCManager;
    VinylControlProxy* m_pVinylControl;
#endif
    double m_dInitialPos;
    bool m_bVinylActive;
    bool m_bSignalActive;
    QImage m_qImage;
    int m_iSize;
    int m_iTimerId;
    int m_iSignalUpdateTick;
    QString m_group;
    float m_fAngle; //Degrees
    double m_dAngleLastPlaypos;
    float m_fGhostAngle;
    double m_dGhostAngleLastPlaypos;
    int m_iStartMouseX;
    int m_iStartMouseY;
    int m_iFullRotations;
    double m_dPrevTheta;
    double m_dTheta;
    /** Speed of the vinyl rotation. */
    double m_dRotationsPerSecond;
    bool m_bClampFailedWarning;
};

#endif //_WSPINNY_H
