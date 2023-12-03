#include "waveform/renderers/waveformdisplayrange.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "track/track.h"
#include "util/math.h"
#include "util/performancetimer.h"
#include "waveform/visualplayposition.h"
#include "waveform/waveform.h"
#include "widget/wwidget.h"

const double WaveformDisplayRange::s_waveformMinZoom = 1.0;
const double WaveformDisplayRange::s_waveformMaxZoom = 10.0;
const double WaveformDisplayRange::s_waveformDefaultZoom = 3.0;
const double WaveformDisplayRange::s_defaultPlayMarkerPosition = 0.5;

namespace {
constexpr int kDefaultDimBrightThreshold = 127;
} // namespace

WaveformDisplayRange::WaveformDisplayRange(const QString& group)
        : m_group(group),
          m_orientation(Qt::Horizontal),
          m_dimBrightThreshold(kDefaultDimBrightThreshold),
          m_height(-1),
          m_width(-1),
          m_devicePixelRatio(1.0f),

          m_firstDisplayedPosition(0.0),
          m_lastDisplayedPosition(0.0),
          m_trackPixelCount(0.0),

          m_zoomFactor(1.0),
          m_visualSamplePerPixel(1.0),
          m_audioSamplePerPixel(1.0),
          m_audioVisualRatio(1.0),
          m_alphaBeatGrid(90),
          // Really create some to manage those;
          m_visualPlayPosition(nullptr),
          m_playPosVSample(0),
          m_totalVSamples(0),
          m_pRateRatioCO(nullptr),
          m_rateRatio(1.0),
          m_pGainControlObject(nullptr),
          m_gain(1.0),
          m_pTrackSamplesControlObject(nullptr),
          m_trackSamples(0.0),
          m_scaleFactor(1.0),
          m_playMarkerPosition(s_defaultPlayMarkerPosition),
          m_playPos(-1.0),
          m_truePosSample(-1.0) {
    //qDebug() << "WaveformDisplayRange";
}

WaveformDisplayRange::~WaveformDisplayRange() {
    //qDebug() << "~WaveformDisplayRange";

    delete m_pRateRatioCO;
    delete m_pGainControlObject;
    delete m_pTrackSamplesControlObject;
}

bool WaveformDisplayRange::init() {
    //qDebug() << "WaveformDisplayRange::init, m_group=" << m_group;

    m_visualPlayPosition = VisualPlayPosition::getVisualPlayPosition(m_group);

    m_pRateRatioCO = new ControlProxy(
            m_group, "rate_ratio");
    m_pGainControlObject = new ControlProxy(
            m_group, "total_gain");
    m_pTrackSamplesControlObject = new ControlProxy(
            m_group, "track_samples");

    return true;
}

void WaveformDisplayRange::onPreRender(ISyncTimeProvider* syncTimeProvider) {
    // FIXME (ac) this seems to sometime be called before init
    if (!m_trackSamples) {
        return;
    }

    // For a valid track to render we need
    m_trackSamples = m_pTrackSamplesControlObject->get();
    if (m_trackSamples <= 0) {
        return;
    }

    //Fetch parameters before rendering in order the display all sub-renderers with the same values
    m_rateRatio = m_pRateRatioCO->get();

    // This gain adjustment compensates for an arbitrary /2 gain chop in
    // EnginePregain. See the comment there.
    m_gain = m_pGainControlObject->get() * 2;

    // Compute visual sample to pixel ratio
    // Allow waveform to spread one visual sample across a hundred pixels
    // NOTE: The hundred pixel limit is totally arbitrary. Theoretically,
    // there should be no limit to how far the waveforms can be zoomed in.
    double visualSamplePerPixel = m_zoomFactor * m_rateRatio / m_scaleFactor;
    m_visualSamplePerPixel = math_max(0.01, visualSamplePerPixel);

    TrackPointer pTrack = m_pTrack;
    if (pTrack) {
        ConstWaveformPointer pWaveform = pTrack->getWaveform();
        if (pWaveform) {
            m_audioVisualRatio = pWaveform->getAudioVisualRatio();
        }
    }

    m_audioSamplePerPixel = m_visualSamplePerPixel * m_audioVisualRatio;

    double truePlayPos = m_visualPlayPosition->getAtNextVSync(syncTimeProvider);
    // truePlayPos = -1 happens, when a new track is in buffer but m_visualPlayPosition was not updated

    if (m_audioSamplePerPixel > 0 && truePlayPos != -1) {
        // Track length in pixels.
        m_trackPixelCount = m_trackSamples / 2.0 / m_audioSamplePerPixel;

        // Avoid pixel jitter in play position by rounding to the nearest track
        // pixel.
        m_playPos = round(truePlayPos * m_trackPixelCount) / m_trackPixelCount;
        // TODO m0dB shouldn't this be:
        //        round(truePlayPos * m_trackPixelCount * m_devicePixelRatio) /
        //        (m_trackPixelCount * m_devicePixelRatio);
        m_totalVSamples = static_cast<int>(m_trackPixelCount * m_visualSamplePerPixel);
        m_playPosVSample = static_cast<int>(m_playPos * m_totalVSamples);
        m_truePosSample = truePlayPos * static_cast<double>(m_trackSamples);
        double leftOffset = m_playMarkerPosition;
        double rightOffset = 1.0 - m_playMarkerPosition;

        double displayedLengthLeft =
                (static_cast<double>(getLength()) / m_trackPixelCount) *
                leftOffset;
        double displayedLengthRight =
                (static_cast<double>(getLength()) / m_trackPixelCount) *
                rightOffset;

        //qDebug() << "WaveformDisplayRange::onPreRender" <<
        //        "m_playMarkerPosition=" << m_playMarkerPosition <<
        //        "leftOffset=" << leftOffset <<
        //        "rightOffset=" << rightOffset <<
        //        "displayedLengthLeft=" << displayedLengthLeft <<
        //        "displayedLengthRight=" << displayedLengthRight;

        m_firstDisplayedPosition = m_playPos - displayedLengthLeft;
        m_lastDisplayedPosition = m_playPos + displayedLengthRight;
    } else {
        m_playPos = -1.0; // disable renderers
        m_truePosSample = -1.0;
    }

    //qDebug() << "WaveformDisplayRange::onPreRender" <<
    //        "m_group" << m_group <<
    //        "m_trackSamples" << m_trackSamples <<
    //        "m_playPos" << m_playPos <<
    //        "m_rateRatio" << m_rate <<
    //        "m_gain" << m_gain;
}

void WaveformDisplayRange::resize(int width, int height, float devicePixelRatio) {
    m_width = width;
    m_height = height;
    m_devicePixelRatio = devicePixelRatio;
}

void WaveformDisplayRange::setZoom(double zoom) {
    //qDebug() << "WaveformDisplayRange::setZoom" << zoom;
    m_zoomFactor = math_clamp<double>(zoom, s_waveformMinZoom, s_waveformMaxZoom);
}

void WaveformDisplayRange::setDisplayBeatGridAlpha(int alpha) {
    m_alphaBeatGrid = alpha;
}

void WaveformDisplayRange::setTrack(TrackPointer track) {
    m_pTrack = track;
    //used to postpone first display until track sample is actually available
    m_trackSamples = -1.0;
}
