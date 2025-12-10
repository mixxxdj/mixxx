#pragma once

#include <QString>

#include "audio/frame.h"
#include "track/beats.h"
#include "track/bpm.h"
#include "track/keys.h"
#include "util/types.h"

namespace mixxx {

class AnalyzerPluginInfo {
  public:
    AnalyzerPluginInfo(const QString& id,
            const QString& author,
            const QString& name,
            bool isConstantTempoSupported)
            : m_id(id),
              m_author(author),
              m_name(name),
              m_isConstantTempoSupported(isConstantTempoSupported) {
    }

    const QString& id() const {
        return m_id;
    }

    const QString& author() const {
        return m_author;
    }

    const QString& name() const {
        return m_name;
    }

    bool isConstantTempoSupported() const {
        return m_isConstantTempoSupported;
    }

  private:
    QString m_id;
    QString m_author;
    QString m_name;
    bool m_isConstantTempoSupported;
};

class AnalyzerPlugin {
  public:
    virtual ~AnalyzerPlugin() = default;

    virtual QString id() const {
        return info().id();
    }
    virtual QString author() const {
        return info().author();
    }
    virtual QString name() const {
        return info().name();
    }
    virtual AnalyzerPluginInfo info() const = 0;

    virtual bool initialize(mixxx::audio::SampleRate sampleRate) = 0;
    virtual bool processSamples(const CSAMPLE* pIn, SINT iLen) = 0;
    virtual bool finalize() = 0;
};

class AnalyzerBeatsPlugin : public AnalyzerPlugin {
  public:
    ~AnalyzerBeatsPlugin() override = default;

    virtual bool supportsBeatTracking() const = 0;
    virtual mixxx::Bpm getBpm() const {
        return {};
    }
    virtual QVector<mixxx::audio::FramePos> getBeats() const {
        return {};
    }
};

class AnalyzerKeyPlugin : public AnalyzerPlugin {
  public:
    ~AnalyzerKeyPlugin() override = default;

    virtual KeyChangeList getKeyChanges() const = 0;

    // Enable 432Hz detection mode (analyze at both 440Hz and 432Hz)
    virtual void setDetect432Hz(bool enabled) {
        Q_UNUSED(enabled);
    }

    // Returns the detected tuning frequency in Hz (default 440.0 Hz)
    // This is the reference frequency A4 that best matches the track's tuning
    // Stored as double to preserve cents precision
    virtual double getTuningFrequencyHz() const {
        return 440.0;
    }

    // Configure the tuning frequency detection range
    // minFreq: Minimum frequency to test (e.g., 427Hz)
    // maxFreq: Maximum frequency to test (e.g., 447Hz)
    // stepFreq: Step size for scanning (e.g., 1Hz)
    virtual void setTuningDetectionRange(int minFreq, int maxFreq, int stepFreq) {
        Q_UNUSED(minFreq);
        Q_UNUSED(maxFreq);
        Q_UNUSED(stepFreq);
    }

    // Enable dynamic tuning frequency detection
    virtual void setDetectTuningFrequency(bool enabled) {
        Q_UNUSED(enabled);
    }
};

} // namespace mixxx
