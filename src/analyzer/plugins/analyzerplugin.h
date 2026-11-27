#pragma once

#include <QString>
#include <optional>

#include "analyzer/plugins/constantbeatgrid.h"
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
    // A constant tempo grid that the plugin estimated over the whole track,
    // or, with the beat map enabled, that it found while building its beat
    // map. It is used instead of getBeats() and getBeatMap().
    virtual std::optional<ConstantBeatGrid> getConstantBeatGrid() const {
        return std::nullopt;
    }
    // A beat map that the plugin built from getBeats() itself. With the beat
    // map enabled and no constant grid, it is used instead of ironing
    // getBeats() with BeatUtils if it has at least two beats.
    virtual QVector<mixxx::audio::FramePos> getBeatMap() const {
        return {};
    }
    // Whether finalize() builds getBeatMap(), which is not used with the fixed
    // tempo assumption. Enabled by default.
    virtual void setBeatMapEnabled(bool /*enabled*/) {
    }
};

class AnalyzerKeyPlugin : public AnalyzerPlugin {
  public:
    ~AnalyzerKeyPlugin() override = default;

    virtual KeyChangeList getKeyChanges() const = 0;
};

} // namespace mixxx
