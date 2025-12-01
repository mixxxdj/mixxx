#pragma once

#include "control/pollingcontrolproxy.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"
#include "vinylcontrol/vinylsignalquality.h"
#include "widget/trackdroptarget.h"
#include "widget/wbasewidget.h"
#include "widget/wcoverartmenu.h"
#include "widget/wglwidget.h"

class ConfigKey;
class ControlProxy;
class VisualPlayPosition;
class VinylControlManager;
class VSyncThread;
class QDomNode;
class SkinContext;
class BaseTrackPlayer;
class DlgCoverArtFullSize;

class WSpinnyBase : public WGLWidget,
                    public WBaseWidget,
                    public VinylSignalQualityListener,
                    public TrackDropTarget {
    Q_OBJECT
  public:
    WSpinnyBase(QWidget* pParent,
            const QString& group,
            UserSettingsPointer pConfig,
            VinylControlManager* pVCMan,
            BaseTrackPlayer* pPlayer);
    ~WSpinnyBase() override;

    void onVinylSignalQualityUpdate(const VinylSignalQualityReport& report) override;

    virtual void setupVinylSignalQuality() = 0;
    virtual void updateVinylSignalQualityImage(
            const QColor& qual_color, const unsigned char* data) = 0;

    void setup(const QDomNode& node,
            const SkinContext& context,
            const ConfigKey& showCoverConfigKey);
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

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
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);
    void slotCoverInfoSelected(const CoverInfoRelative& coverInfo);
    void slotReloadCoverArt();
    void slotTrackCoverArtUpdated();

  signals:
    void trackDropped(const QString& filename, const QString& group) override;
    void cloneDeck(const QString& sourceGroup, const QString& targetGroup) override;

  protected:
    // QWidget:
    void paintEvent(QPaintEvent* /*unused*/) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void resizeEvent(QResizeEvent* /*unused*/) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool event(QEvent* pEvent) override;

    // TrackDropTarget:
    bool handleDragAndDropEventFromWindow(QEvent* pEvent) override;

    double calculateAngle(double playpos);
    int calculateFullRotations(double playpos);
    double calculatePositionFromAngle(double angle);

    void setLoadedCover(const QPixmap& pixmap);

    bool shouldDrawVinylQuality() const;

  private:
    virtual void draw() = 0;
    virtual void coverChanged() = 0;

    QPixmap scaleToSize(const QPixmap& pixmap) const;
    QImage scaleToSize(const QImage& image) const;
    QImage scaleToSize(const std::shared_ptr<QImage>& image) const;

    const QString m_group;
    UserSettingsPointer m_pConfig;

  protected:
    std::shared_ptr<QImage> m_pBgImage;
    std::shared_ptr<QImage> m_pMaskImage;
    std::shared_ptr<QImage> m_pFgImage;
    QImage m_fgImageScaled;
    std::shared_ptr<QImage> m_pGhostImage;
    QImage m_ghostImageScaled;

    QSharedPointer<VisualPlayPosition> m_pVisualPlayPos;
    PollingControlProxy m_pPlayPos;
    PollingControlProxy m_pTrackSamples;
    PollingControlProxy m_pTrackSampleRate;
    PollingControlProxy m_pScratchToggle;
    PollingControlProxy m_pScratchPos;
    parented_ptr<ControlProxy> m_pVinylControlSpeedType;
    parented_ptr<ControlProxy> m_pVinylControlEnabled;
    parented_ptr<ControlProxy> m_pSignalEnabled;
    parented_ptr<ControlProxy> m_pSlipEnabled;
    parented_ptr<ControlProxy> m_pShowCoverProxy;

    TrackPointer m_pLoadedTrack;
    QPixmap m_loadedCover;
    QPixmap m_loadedCoverScaled;
    CoverInfo m_lastRequestedCover;
    bool m_bShowCover;

    VinylControlManager* m_pVCManager;
    double m_dInitialPos;

    int m_iVinylInput;
    bool m_bVinylActive;
    bool m_bSignalActive;
    bool m_bDrawVinylSignalQuality;
    int m_iVinylScopeSize;

    float m_fAngle; // Degrees
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
    parented_ptr<WCoverArtMenu> m_pCoverMenu;
    parented_ptr<DlgCoverArtFullSize> m_pDlgCoverArt;
};
