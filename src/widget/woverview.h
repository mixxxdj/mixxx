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
#ifndef WOVERVIEW_H
#define WOVERVIEW_H

#include <QPaintEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QColor>
#include <QList>

#include "track/track.h"
#include "widget/trackdroptarget.h"
#include "widget/wwidget.h"
#include "analyzer/analyzerprogress.h"

#include "util/color/color.h"

#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/renderers/waveformmarkset.h"
#include "waveform/renderers/waveformmarkrange.h"
#include "skin/skincontext.h"

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
    void trackDropped(QString filename, QString group);
    void cloneDeck(QString source_group, QString target_group);

  protected:
    WOverview(
            const char* group,
            PlayerManager* pPlayerManager,
            UserSettingsPointer pConfig,
            QWidget* parent = nullptr);

    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent * /*unused*/) override;
    void resizeEvent(QResizeEvent * /*unused*/) override;
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

    int m_diffGain;
    qreal m_devicePixelRatio;

  private slots:
    void onEndOfTrackChange(double v);

    void onMarkChanged(double v);
    void onMarkRangeChange(double v);
    void onRateSliderChange(double v);
    void receiveCuesUpdated();

    void slotWaveformSummaryUpdated();

  private:
    // Append the waveform overview pixmap according to available data
    // in waveform
    virtual bool drawNextPixmapPart() = 0;
    void drawEndOfTrackBackground(QPainter* pPainter);
    void drawAxis(QPainter* pPainter);
    void drawWaveformPixmap(QPainter* pPainter);
    void drawEndOfTrackFrame(QPainter* pPainter);
    void drawAnalyzerProgress(QPainter* pPainter);
    void drawRangeMarks(QPainter* pPainter, const float& offset, const float& gain);
    void drawMarks(QPainter* pPainter, const float offset, const float gain);
    void drawCurrentPosition(QPainter* pPainter);
    void paintText(const QString& text, QPainter* pPainter);
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
    ControlProxy* m_pRateDirControl;
    ControlProxy* m_pRateRangeControl;
    ControlProxy* m_pRateSliderControl;
    ControlProxy* m_trackSampleRateControl;
    ControlProxy* m_trackSamplesControl;

    // Current active track
    TrackPointer m_pCurrentTrack;
    ConstWaveformPointer m_pWaveform;

    // True if slider is dragged. Only used when m_bEventWhileDrag is false
    bool m_bDrag;
    // Internal storage of slider position in pixels
    int m_iPos;

    Qt::Orientation m_orientation;

    QPixmap m_backgroundPixmap;
    QString m_backgroundPixmapPath;
    QColor m_qColorBackground;
    QColor m_endOfTrackColor;

    PredefinedColorsRepresentation m_predefinedColorsRepresentation;
    WaveformMarkSet m_marks;
    std::vector<WaveformMarkRange> m_markRanges;

    // Coefficient value-position linear transposition
    double m_a;
    double m_b;

    AnalyzerProgress m_analyzerProgress;
    bool m_trackLoaded;
    double m_scaleFactor;
};

#endif
