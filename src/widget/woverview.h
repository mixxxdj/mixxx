//
// C++ Interface: woverview
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#pragma once

#include <QColor>
#include <QList>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>

#include "analyzer/analyzerprogress.h"
#include "skin/legacy/skincontext.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/color/color.h"
#include "util/parented_ptr.h"
#include "waveform/renderers/waveformmarkrange.h"
#include "waveform/renderers/waveformmarkset.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/waveform.h"
#include "widget/trackdroptarget.h"
#include "widget/wcuemenupopup.h"
#include "widget/wwidget.h"

class PlayerManager;
class PainterScope;

class WOverview : public WWidget, public TrackDropTarget {
    Q_OBJECT
  public:
    void setup(const QDomNode& node, const SkinContext& context);

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue) override;
    void slotTrackLoaded(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void onTrackAnalyzerProgress(TrackId trackId,
            AnalyzerProgress analyzerProgress);

  signals:
    void trackDropped(const QString& filename, const QString& group) override;
    void cloneDeck(const QString& sourceGroup, const QString& targetGroup) override;

  protected:
    WOverview(
            const QString& group,
            PlayerManager* pPlayerManager,
            UserSettingsPointer pConfig,
            QWidget* parent = nullptr);

    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* /*unused*/) override;
    void resizeEvent(QResizeEvent* /*unused*/) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    inline int length() {
        return m_orientation == Qt::Horizontal ? width() : height();
    }

    inline int breadth() {
        return m_orientation == Qt::Horizontal ? height() : width();
    }

    ConstWaveformPointer getWaveform() const {
        return m_pWaveform;
    }

    QImage m_waveformSourceImage;
    QImage m_waveformImageScaled;

    WaveformSignalColors m_signalColors;

    // Hold the last visual sample processed to generate the pixmap
    int m_actualCompletion;

    bool m_pixmapDone;
    float m_waveformPeak;

    float m_diffGain;
    qreal m_devicePixelRatio;

  private slots:
    void onEndOfTrackChange(double v);

    void onMarkChanged(double v);
    void onMarkRangeChange(double v);
    void onRateRatioChange(double v);
    void onPassthroughChange(double v);
    void receiveCuesUpdated();

    void slotWaveformSummaryUpdated();
    void slotCueMenuPopupAboutToHide();

  private:
    // Append the waveform overview pixmap according to available data
    // in waveform
    virtual bool drawNextPixmapPart() = 0;
    void drawEndOfTrackBackground(QPainter* pPainter);
    void drawAxis(QPainter* pPainter);
    void drawWaveformPixmap(QPainter* pPainter);
    void drawPlayedOverlay(QPainter* pPainter);
    void drawPlayPosition(QPainter* pPainter);
    void drawEndOfTrackFrame(QPainter* pPainter);
    void drawAnalyzerProgress(QPainter* pPainter);
    void drawRangeMarks(QPainter* pPainter, const float& offset, const float& gain);
    void drawMarks(QPainter* pPainter, const float offset, const float gain);
    void drawPickupPosition(QPainter* pPainter);
    void drawTimeRuler(QPainter* pPainter);
    void drawMarkLabels(QPainter* pPainter, const float offset, const float gain);
    void drawPassthroughOverlay(QPainter* pPainter);
    void paintText(const QString& text, QPainter* pPainter);
    double samplePositionToSeconds(double sample);
    inline int valueToPosition(double value) const {
        return static_cast<int>(m_a * value - m_b);
    }
    inline double positionToValue(int position) const {
        return (static_cast<double>(position) + m_b) / m_a;
    }

    void updateCues(const QList<CuePointer> &loadedCues);

    const QString m_group;
    UserSettingsPointer m_pConfig;
    ControlProxy* m_endOfTrackControl;
    bool m_endOfTrack;
    bool m_bPassthroughEnabled;
    ControlProxy* m_pRateRatioControl;
    ControlProxy* m_trackSampleRateControl;
    ControlProxy* m_trackSamplesControl;
    ControlProxy* m_playpositionControl;
    ControlProxy* m_pPassthroughControl;

    // Current active track
    TrackPointer m_pCurrentTrack;
    ConstWaveformPointer m_pWaveform;

    parented_ptr<WCueMenuPopup> m_pCueMenuPopup;
    bool m_bShowCueTimes;

    int m_iPosSeconds;
    // True if pick-up is dragged. Only used when m_bEventWhileDrag is false
    bool m_bLeftClickDragging;
    // Internal storage of slider position in pixels
    int m_iPickupPos;
    // position of the overlay shadow
    int m_iPlayPos;

    WaveformMarkPointer m_pHoveredMark;
    bool m_bTimeRulerActive;
    QPointF m_timeRulerPos;
    WaveformMarkLabel m_timeRulerPositionLabel;
    WaveformMarkLabel m_timeRulerDistanceLabel;

    Qt::Orientation m_orientation;

    QPixmap m_backgroundPixmap;
    QString m_backgroundPixmapPath;
    QColor m_backgroundColor;
    int m_iLabelFontSize;
    QColor m_labelTextColor;
    QColor m_labelBackgroundColor;
    QColor m_axesColor;
    QColor m_playPosColor;
    QColor m_endOfTrackColor;
    QColor m_passthroughOverlayColor;
    QColor m_playedOverlayColor;
    QColor m_lowColor;
    int m_dimBrightThreshold;
    QLabel* m_pPassthroughLabel;

    // All WaveformMarks
    WaveformMarkSet m_marks;
    // List of visible WaveformMarks sorted by the order they appear in the track
    QList<WaveformMarkPointer> m_marksToRender;
    std::vector<WaveformMarkRange> m_markRanges;
    WaveformMarkLabel m_cuePositionLabel;
    WaveformMarkLabel m_cueTimeDistanceLabel;

    // Coefficient value-position linear transposition
    double m_a;
    double m_b;

    AnalyzerProgress m_analyzerProgress;
    bool m_trackLoaded;
    double m_scaleFactor;
};
