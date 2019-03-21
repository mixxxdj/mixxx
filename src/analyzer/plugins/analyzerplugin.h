#ifndef ANALYZER_PLUGINS_ANALYZERPLUGIN_H
#define ANALYZER_PLUGINS_ANALYZERPLUGIN_H

#include <QString>

#include "track/beats.h"
#include "track/keys.h"
#include "util/types.h"

namespace mixxx {

struct AnalyzerPluginInfo {
    AnalyzerPluginInfo(const QString& id,
                       const QString& author,
                       const QString& name)
            : id(id), author(author), name(name) { }
    QString id;
    QString author;
    QString name;
};

class AnalyzerPlugin {
  public:
    virtual ~AnalyzerPlugin() = default;

    virtual QString id() const {
        return info().id;
    }
    virtual QString author() const {
        return info().author;
    }
    virtual QString name() const {
        return info().name;
    }
    virtual AnalyzerPluginInfo info() const = 0;

    virtual bool initialize(int samplerate) = 0;
    virtual bool process(const CSAMPLE* pIn, const int iLen) = 0;
    virtual bool finalize() = 0;
};

class AnalyzerBeatsPlugin : public AnalyzerPlugin {
  public:
    ~AnalyzerBeatsPlugin() override = default;

    virtual bool supportsBeatTracking() const = 0;
    virtual float getBpm() const {
        return 0.0f;
    }
    virtual QVector<double> getBeats() const {
        return QVector<double>();
    }
};

class AnalyzerKeyPlugin : public AnalyzerPlugin {
  public:
    ~AnalyzerKeyPlugin() override = default;

    virtual KeyChangeList getKeyChanges() const = 0;
};

}  // namespace mixxx

#endif /* ANALYZER_PLUGINS_ANALYZERPLUGIN_H */
