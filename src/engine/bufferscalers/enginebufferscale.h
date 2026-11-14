#pragma once

#include <QObject>

#include "audio/signalinfo.h"

// kMaxSeekSpeed needs to be good and high to allow room for the very high
// instantaneous velocities of advanced scratching (Uzi) and spin-backs.
// (Yes, I can actually spin the SCS.1d faster than 15x nominal.
// Why do we even have this parameter? -- Sean)

constexpr double kMaxSeekSpeed = 100.0;
constexpr double kMinSeekSpeed = 0.010;

// I'll hurt you if you change kMinSeekSpeed. SoundTouch freaks out and
// just gives us stuttering if you set the speed to be lower than this.
// This took me ages to figure out.
// -- Albert July 17, 2010.

class EngineBufferScale : public QObject {
    Q_OBJECT
  public:
    EngineBufferScale();
    ~EngineBufferScale() override = default;

    // Sets the scaling parameters.
    // * The base rate (ratio of track sample rate to output sample rate).
    // * The tempoRatio describes the tempo change in fraction of
    //   original tempo. Put another way, it is the ratio of track seconds to
    //   real second. For example, a tempo of 1.0 is no change. A
    //   tempo of 2 is a 2x speedup (2 track seconds pass for every 1
    //   real second).
    // * The pitchRatio describes the pitch adjustment in fraction of
    //   the original pitch. For example, a pitch adjustment of 1.0 is no change and a
    //   pitch adjustment of 2.0 is a full octave shift up.
    //
    // If parameter settings are outside of acceptable limits, each setting will
    // be set to the value it was clamped to.
    virtual void setScaleParameters(double baseRate,
            double* pTempoRatio,
            double* pPitchRatio) {
        m_dBaseRate = baseRate;
        m_dTempoRatio = *pTempoRatio;
        m_dPitchRatio = *pPitchRatio;
    }

    // Set the desired output signal.
    void setSignal(
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::ChannelCount channelCout);

    const mixxx::audio::SignalInfo& getOutputSignal() const {
        return m_signal;
    }

    // Called from EngineBuffer when seeking, to ensure the buffers are flushed */
    virtual void clear() = 0;

    // Scale buffer
    // Returns the number of frames that have bean read from the unscaled
    // input buffer The number of frames copied to the output buffer is always
    // an integer value, while the number of frames read from the unscaled
    // input buffer might be partial number!
    // The size of the output buffer is given in samples, i.e. twice the number
    // of frames for an interleaved stereo signal.
    virtual double scaleBuffer(
            CSAMPLE* pOutputBuffer,
            SINT iOutputBufferSize) = 0;

  private:
    mixxx::audio::SignalInfo m_signal;

    virtual void onSignalChanged() = 0;

  protected:
    double m_dBaseRate;
    bool m_bSpeedAffectsPitch;
    double m_dTempoRatio;
    double m_dPitchRatio;

    // Due to the scaler latency, tempo and pitch changes are not immediately effective.
    double m_effectiveRate;
};
