
#pragma once

#include <QEvent>
#include <QHideEvent>
#include <QShowEvent>

#include "library/dlgcoverartfullsize.h"
#include "mixer/basetrackplayer.h"
#include "preferences/usersettings.h"
#include "track/track.h"
#include "widget/trackdroptarget.h"
#include "widget/wbasewidget.h"
#include "widget/wglwidget.h"
#include "widget/wwidget.h"

class ControlProxy;
class SkinContext;
class VisualPlayPosition;
class VinylControlManager;
class VSyncThread;

class WStrobe : public WGLWidget, public WBaseWidget, public TrackDropTarget {
    Q_OBJECT
  public:
    WStrobe(
            QWidget* pParent,
            const QString& group,
            UserSettingsPointer pConfig,
            BaseTrackPlayer* pPlayer);
    ~WStrobe() override;

    void setup(const QDomNode& node, const SkinContext& context);
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

  public slots:
    void slotLoadTrack(TrackPointer);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void render(VSyncThread* vSyncThread);
    void swap();

  signals:
    void trackDropped(const QString& filename, const QString& group) override;
    void cloneDeck(const QString& source_group, const QString& target_group) override;

  protected:
    // QWidget:
    void paintEvent(QPaintEvent* /*unused*/) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool event(QEvent* pEvent) override;

    QPixmap scaledCoverArt(const QPixmap& normal);

  private:
    double calculateAngle(double playpos);

    QString m_group;
    UserSettingsPointer m_pConfig;
    std::shared_ptr<QImage> m_pDot60PosImage;
    std::shared_ptr<QImage> m_pDot33PosImage;
    std::shared_ptr<QImage> m_pDotUnityImage;
    std::shared_ptr<QImage> m_pDot33NegImage;
    QImage m_fgImageScaled;
    std::shared_ptr<QImage> m_pGhostImage;
    QImage m_ghostImageScaled;
    ControlProxy* m_pPlay;
    ControlProxy* m_pPlayPos;
    QSharedPointer<VisualPlayPosition> m_pVisualPlayPos;
    ControlProxy* m_pTrackSamples;
    ControlProxy* m_pTrackSampleRate;
    ControlProxy* m_pScratchToggle;
    ControlProxy* m_pScratchPos;
    ControlProxy* m_pSignalEnabled;

    TrackPointer m_loadedTrack;

    double m_dInitialPos;

    int m_iVinylInput;
    bool m_bVinylActive;
    bool m_bSignalActive;
    QImage m_qImage;
    int m_iVinylScopeSize;

    double m_fAngle; // Degrees
    double m_dAngleCurrentPlaypos;
    double m_dGhostAngleCurrentPlaypos;
    double m_dAngleLastPlaypos;
    int m_iStartMouseX;
    int m_iStartMouseY;
    int m_iFullRotations;
    double m_dPrevTheta;
    double m_dTheta;
    // Speed of the vinyl rotation.
    double m_dRotationsPerSecond;
    bool m_bClampFailedWarning;

    BaseTrackPlayer* m_pPlayer;
};
