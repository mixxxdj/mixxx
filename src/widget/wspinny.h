#pragma once

#include <QEvent>
#include <QGLWidget>
#include <QHideEvent>
#include <QShowEvent>

#include "library/dlgcoverartfullsize.h"
#include "mixer/basetrackplayer.h"
#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "track/track_decl.h"
#include "vinylcontrol/vinylsignalquality.h"
#include "widget/trackdroptarget.h"
#include "widget/wbasewidget.h"
#include "widget/wcoverartmenu.h"
#include "widget/wwidget.h"

class ControlProxy;
class VisualPlayPosition;
class VinylControlManager;
class VSyncThread;

class WSpinny : public QGLWidget, public WBaseWidget, public VinylSignalQualityListener,
                public TrackDropTarget {
    Q_OBJECT
  public:
    WSpinny(QWidget* parent, const QString& group,
            UserSettingsPointer pConfig,
            VinylControlManager* pVCMan,
            BaseTrackPlayer* pPlayer);
    ~WSpinny() override;

    void onVinylSignalQualityUpdate(const VinylSignalQualityReport& report) override;

    void setup(const QDomNode& node, const SkinContext& context);
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  public slots:
    void slotLoadTrack(TrackPointer);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void updateVinylControlSpeed(double rpm);
    void updateVinylControlEnabled(double enabled);
    void updateVinylControlSignalEnabled(double enabled);
    void updateSlipEnabled(double enabled);
    void render(VSyncThread* vSyncThread);
    void swap();

  protected slots:
    void slotCoverFound(
            const QObject* pRequestor,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap,
            quint16 requestedHash,
            bool coverInfoUpdated);
    void slotCoverInfoSelected(const CoverInfoRelative& coverInfo);
    void slotReloadCoverArt();
    void slotTrackCoverArtUpdated();


  signals:
    void trackDropped(const QString& filename, const QString& group) override;
    void cloneDeck(const QString& sourceGroup, const QString& targetGroup) override;

  protected:
    //QWidget:
    void paintEvent(QPaintEvent* /*unused*/) override;
    void mouseMoveEvent(QMouseEvent * e) override;
    void mousePressEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent * e) override;
    void resizeEvent(QResizeEvent* /*unused*/) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool event(QEvent* pEvent) override;

    double calculateAngle(double playpos);
    int calculateFullRotations(double playpos);
    double calculatePositionFromAngle(double angle);
    QPixmap scaledCoverArt(const QPixmap& normal);

  private:
    const QString m_group;
    UserSettingsPointer m_pConfig;
    std::shared_ptr<QImage> m_pBgImage;
    std::shared_ptr<QImage> m_pMaskImage;
    std::shared_ptr<QImage> m_pFgImage;
    QImage m_fgImageScaled;
    std::shared_ptr<QImage> m_pGhostImage;
    QImage m_ghostImageScaled;
    ControlProxy* m_pPlayPos;
    QSharedPointer<VisualPlayPosition> m_pVisualPlayPos;
    ControlProxy* m_pTrackSamples;
    ControlProxy* m_pTrackSampleRate;
    ControlProxy* m_pScratchToggle;
    ControlProxy* m_pScratchPos;
    ControlProxy* m_pVinylControlSpeedType;
    ControlProxy* m_pVinylControlEnabled;
    ControlProxy* m_pSignalEnabled;
    ControlProxy* m_pSlipEnabled;

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
    // Speed of the vinyl rotation.
    double m_dRotationsPerSecond;
    bool m_bClampFailedWarning;
    bool m_bGhostPlayback;

    BaseTrackPlayer* m_pPlayer;
    DlgCoverArtFullSize* m_pDlgCoverArt;
    WCoverArtMenu* m_pCoverMenu;
};
