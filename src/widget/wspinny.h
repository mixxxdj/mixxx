
#ifndef _WSPINNY_H
#define _WSPINNY_H

#include <QGLWidget>
#include <QShowEvent>
#include <QHideEvent>
#include <QEvent>

#include "configobject.h"
#include "skin/skincontext.h"
#include "trackinfoobject.h"
#include "vinylcontrol/vinylsignalquality.h"
#include "widget/wbasewidget.h"
#include "widget/wwidget.h"

class ControlObjectThread;
class VisualPlayPosition;
class VinylControlManager;

class WSpinny : public QGLWidget, public WBaseWidget, public VinylSignalQualityListener {
    Q_OBJECT
  public:
    WSpinny(QWidget* parent, const QString& group,
            ConfigObject<ConfigValue>* pConfig,
            VinylControlManager* pVCMan);
    virtual ~WSpinny();

    void onVinylSignalQualityUpdate(const VinylSignalQualityReport& report);

    void setup(QDomNode node, const SkinContext& context);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

  public slots:
    void slotLoadTrack(TrackPointer);
    void slotReset();
    void updateVinylControlSpeed(double rpm);
    void updateVinylControlEnabled(double enabled);
    void updateVinylControlSignalEnabled(double enabled);
    void updateSlipEnabled(double enabled);

  protected slots:
    void maybeUpdate();
    void slotCoverFound(const QObject* pRequestor, int requestReference,
                        const CoverInfo& info, QPixmap pixmap, bool fromCache);
    void slotTrackCoverArtUpdated();


  signals:
    void trackDropped(QString filename, QString group);

  protected:
    //QWidget:
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);
    void resizeEvent(QResizeEvent*);
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    bool event(QEvent* pEvent);

    double calculateAngle(double playpos);
    int calculateFullRotations(double playpos);
    double calculatePositionFromAngle(double angle);
    QPixmap scaledCoverArt(const QPixmap& normal);

  private:
    QString m_group;
    ConfigObject<ConfigValue>* m_pConfig;
    QImage* m_pBgImage;
    QImage* m_pFgImage;
    QImage* m_pGhostImage;
    ControlObjectThread* m_pPlay;
    ControlObjectThread* m_pPlayPos;
    QSharedPointer<VisualPlayPosition> m_pVisualPlayPos;
    ControlObjectThread* m_pTrackSamples;
    ControlObjectThread* m_pTrackSampleRate;
    ControlObjectThread* m_pScratchToggle;
    ControlObjectThread* m_pScratchPos;
    ControlObjectThread* m_pVinylControlSpeedType;
    ControlObjectThread* m_pVinylControlEnabled;
    ControlObjectThread* m_pSignalEnabled;
    ControlObjectThread* m_pSlipEnabled;

    TrackPointer m_loadedTrack;
    QPixmap m_loadedCover;
    QPixmap m_loadedCoverScaled;
    CoverInfo m_lastRequestedCover;
    bool m_bShowCover;


    VinylControlManager* m_pVCManager;
    double m_dInitialPos;

    int m_iVinylInput;
    bool m_bVinylActive;
    bool m_bSignalActive;
    QImage m_qImage;
    int m_iVinylScopeSize;

    float m_fAngle; //Degrees
    double m_dAngleCurrentPlaypos;
    double m_dAngleLastPlaypos;
    float m_fGhostAngle;
    double m_dGhostAngleCurrentPlaypos;
    double m_dGhostAngleLastPlaypos;
    int m_iStartMouseX;
    int m_iStartMouseY;
    int m_iFullRotations;
    double m_dPrevTheta;
    double m_dTheta;
    // Speed of the vinyl rotation.
    double m_dRotationsPerSecond;
    bool m_bClampFailedWarning;
    bool m_bGhostPlayback;
    bool m_bWidgetDirty;
};

#endif //_WSPINNY_H
