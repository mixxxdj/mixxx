#pragma once

#include <QPainter>
#include <QTime>
#include <QVector>
#include <QtDebug>

#include "track/track_decl.h"
#include "util/class.h"
#include "util/performancetimer.h"

// Stripped-down WaveformWidgetRenderer and using ISyncTimeProvider instead of
// VSyncThread.
// TODO @m0dB remove code duplication by making WaveformWidgetRenderer a subclass

class ControlProxy;
class VisualPlayPosition;
class ISyncTimeProvider;

class WaveformDisplayRange {
  public:
    static const double s_waveformMinZoom;
    static const double s_waveformMaxZoom;
    static const double s_waveformDefaultZoom;
    static const double s_defaultPlayMarkerPosition;

  public:
    explicit WaveformDisplayRange(const QString& group);
    virtual ~WaveformDisplayRange();

    bool init();
    virtual bool onInit() {
        return true;
    }

    void onPreRender(ISyncTimeProvider* syncTimeProvider);

    const QString& getGroup() const {
        return m_group;
    }
    const TrackPointer getTrackInfo() const {
        return m_pTrack;
    }

    double getFirstDisplayedPosition() const {
        return m_firstDisplayedPosition;
    }
    double getLastDisplayedPosition() const {
        return m_lastDisplayedPosition;
    }

    void setZoom(double zoom);

    void setDisplayBeatGrid(bool set);
    void setDisplayBeatGridAlpha(int alpha);

    double getVisualSamplePerPixel() const {
        return m_visualSamplePerPixel;
    }
    double getAudioSamplePerPixel() const {
        return m_audioSamplePerPixel;
    }

    // those function replace at its best sample position to an admissible
    // sample position according to the current visual resampling
    // this make mark and signal deterministic
    void regulateVisualSample(int& sampleIndex) const;

    // this "regulate" against visual sampling to make the position in widget
    // stable and deterministic
    // Transform sample index to pixel in track.
    inline double transformSamplePositionInRendererWorld(double samplePosition) const {
        if (std::abs(samplePosition - m_truePosSample) < 1.f) {
            // When asked for the sample position that corresponds with the play
            // marker, return the play market pixel position. This avoids a rare
            // rounding issue where a marker at that sample position would be
            // 1 pixel off.
            return m_playMarkerPosition * getLength();
        }
        const double relativePosition = samplePosition / m_trackSamples;
        return transformPositionInRendererWorld(relativePosition);
    }
    // Transform position (percentage of track) to pixel in track.
    inline double transformPositionInRendererWorld(double position) const {
        return m_trackPixelCount * (position - m_firstDisplayedPosition);
    }

    double getPlayPos() const {
        return m_playPos;
    }
    int getPlayPosVSample() const {
        return m_playPosVSample;
    }
    int getTotalVSample() const {
        return m_totalVSamples;
    }
    double getZoomFactor() const {
        return m_zoomFactor;
    }
    double getGain() const {
        return m_gain;
    }
    double getTrackSamples() const {
        return m_trackSamples;
    }

    int getBeatGridAlpha() const {
        return m_alphaBeatGrid;
    }

    void resize(int width, int height, float devicePixelRatio);
    int getHeight() const {
        return m_height;
    }
    int getWidth() const {
        return m_width;
    }
    float getDevicePixelRatio() const {
        return m_devicePixelRatio;
    }
    int getLength() const {
        return m_orientation == Qt::Horizontal ? m_width : m_height;
    }
    int getBreadth() const {
        return m_orientation == Qt::Horizontal ? m_height : m_width;
    }
    Qt::Orientation getOrientation() const {
        return m_orientation;
    }
    int getDimBrightThreshold() {
        return m_dimBrightThreshold;
    }

    void setTrack(TrackPointer track);

  protected:
    const QString m_group;
    TrackPointer m_pTrack;
    Qt::Orientation m_orientation;
    int m_dimBrightThreshold;
    int m_height;
    int m_width;
    float m_devicePixelRatio;

    double m_firstDisplayedPosition;
    double m_lastDisplayedPosition;
    double m_trackPixelCount;

    double m_zoomFactor;
    double m_visualSamplePerPixel;
    double m_audioSamplePerPixel;
    double m_audioVisualRatio;

    int m_alphaBeatGrid;

    //TODO: vRince create some class to manage control/value
    //ControlConnection
    QSharedPointer<VisualPlayPosition> m_visualPlayPosition;
    int m_playPosVSample;
    int m_totalVSamples;
    ControlProxy* m_pRateRatioCO;
    double m_rateRatio;
    ControlProxy* m_pGainControlObject;
    double m_gain;
    ControlProxy* m_pTrackSamplesControlObject;
    double m_trackSamples;
    double m_scaleFactor;
    double m_playMarkerPosition; // 0.0 - left, 0.5 - center, 1.0 - right

  private:
    DISALLOW_COPY_AND_ASSIGN(WaveformDisplayRange);
    double m_playPos;
    double m_truePosSample;
};
