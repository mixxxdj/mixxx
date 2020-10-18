#ifndef WWAVEFORMVIEWER_H
#define WWAVEFORMVIEWER_H

#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QList>
#include <QMutex>

#include "skin/skincontext.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"
#include "waveform/renderers/waveformmark.h"
#include "widget/trackdroptarget.h"
#include "widget/wcuemenupopup.h"
#include "widget/wwidget.h"

class ControlProxy;
class WaveformWidgetAbstract;
class ControlPotmeter;

class WWaveformViewer : public WWidget, public TrackDropTarget {
    Q_OBJECT
  public:
    WWaveformViewer(
            const QString& group,
            UserSettingsPointer pConfig,
            QWidget* parent = nullptr);
    ~WWaveformViewer() override;

    const QString& getGroup() const {
        return m_group;
    }
    void setup(const QDomNode& node, const SkinContext& context);

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void mousePressEvent(QMouseEvent * /*unused*/) override;
    void mouseMoveEvent(QMouseEvent * /*unused*/) override;
    void mouseReleaseEvent(QMouseEvent * /*unused*/) override;
    void leaveEvent(QEvent* /*unused*/) override;

  signals:
    void trackDropped(QString filename, QString group) override;
    void cloneDeck(QString source_group, QString target_group) override;

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private slots:
    void onZoomChange(double zoom);
    void slotWidgetDead() {
        m_waveformWidget = nullptr;
    }

  private:
    void setWaveformWidget(WaveformWidgetAbstract* waveformWidget);
    WaveformWidgetAbstract* getWaveformWidget() {
        return m_waveformWidget;
    }
    //direct access to let factory sync/set default zoom
    void setZoom(double zoom);
    void setDisplayBeatGridAlpha(int alpha);
    void setPlayMarkerPosition(double position);

  private:
    const QString m_group;
    UserSettingsPointer m_pConfig;
    int m_zoomZoneWidth;
    ControlProxy* m_pZoom;
    ControlProxy* m_pScratchPositionEnable;
    ControlProxy* m_pScratchPosition;
    ControlProxy* m_pWheel;
    ControlProxy* m_pPlayEnabled;
    bool m_bScratching;
    bool m_bBending;
    QPoint m_mouseAnchor;
    parented_ptr<WCueMenuPopup> m_pCueMenuPopup;
    WaveformMarkPointer m_pHoveredMark;

    WaveformWidgetAbstract* m_waveformWidget;

    friend class WaveformWidgetFactory;

    CuePointer getCuePointerFromCueMark(WaveformMarkPointer pMark) const;
    void highlightMark(WaveformMarkPointer pMark);
    void unhighlightMark(WaveformMarkPointer pMark);
    bool isPlaying() const;
};

#endif
