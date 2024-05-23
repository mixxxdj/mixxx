#pragma once

#include "track/track_decl.h"
#include "util/parented_ptr.h"
#include "waveform/renderers/waveformmark.h"
#include "widget/trackdroptarget.h"
#include "widget/wwidget.h"

class ControlProxy;
class WaveformWidgetAbstract;
class WStemControlBox;
class WCueMenuPopup;
class QDomNode;
class SkinContext;

class WWaveformViewer : public WWidget, public TrackDropTarget {
    Q_OBJECT
  public:
    enum StemControlAlignment {
        Left = Qt::AlignLeft,
        Right = Qt::AlignRight
    };

    WWaveformViewer(
            const QString& group,
            UserSettingsPointer pConfig,
            QWidget* parent = nullptr);
    ~WWaveformViewer() override;

    const QString& getGroup() const {
        return m_group;
    }
    void setup(const QDomNode& node, const SkinContext& context);

    bool handleDragAndDropEventFromWindow(QEvent* pEvent) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void mousePressEvent(QMouseEvent * /*unused*/) override;
    void mouseMoveEvent(QMouseEvent * /*unused*/) override;
    void mouseReleaseEvent(QMouseEvent * /*unused*/) override;
    void leaveEvent(QEvent* /*unused*/) override;

#ifdef __STEM__
    // Used by LegacySkinParser to inject the stem toolbox if defined
    WStemControlBox* stemControlWidget() const {
        return m_stemControlWidget.get();
    }

    bool eventFilter(QObject* object, QEvent* event) override;
#endif
  signals:
    void trackDropped(const QString& filename, const QString& group) override;
    void cloneDeck(const QString& sourceGroup, const QString& targetGroup) override;
    void passthroughChanged(double value);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer pOldTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
#ifdef __STEM__
    void slotSelectStem(uint stemIdx);
#endif

  protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private slots:
    void onZoomChange(double zoom);

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
    parented_ptr<ControlProxy> m_pPassthroughEnabled;
    bool m_bScratching;
    bool m_bBending;
    QPoint m_mouseAnchor;
    parented_ptr<WCueMenuPopup> m_pCueMenuPopup;
    WaveformMarkPointer m_pHoveredMark;

    WaveformWidgetAbstract* m_waveformWidget;
#ifdef __STEM__
    std::unique_ptr<WStemControlBox> m_stemControlWidget;
    QWidget* m_mainWindow;

    void adjustStemControl();
#endif

    StemControlAlignment m_stemControlWidgetAlignment;

    int m_dimBrightThreshold;

    friend class WaveformWidgetFactory;

    CuePointer getCuePointerFromCueMark(WaveformMarkPointer pMark) const;
    void highlightMark(WaveformMarkPointer pMark);
    void unhighlightMark(WaveformMarkPointer pMark);
    bool isPlaying() const;
};
