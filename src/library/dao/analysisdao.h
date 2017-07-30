#ifndef ANALYSISDAO_H
#define ANALYSISDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "library/dao/dao.h"
#include "track/track.h"

class AnalysisDao : public DAO {
  public:
    static const QString s_analysisTableName;

    enum AnalysisType {
        TYPE_UNKNOWN = 0,
        TYPE_WAVEFORM,
        TYPE_WAVESUMMARY
    };

    struct AnalysisInfo {
        AnalysisInfo()
                : analysisId(-1),
                  type(TYPE_UNKNOWN) {
        }
        int analysisId;
        TrackId trackId;
        AnalysisType type;
        QString description;
        QString version;
        QByteArray data;
    };

    explicit AnalysisDao(UserSettingsPointer pConfig);
    ~AnalysisDao() override {}

    // The following functions can be used with a custom database
    // connection and independent of whether the DAO has been
    // initialized or not.
    bool deleteAnalysesByType(
            const QSqlDatabase& database,
            AnalysisType type) const;
    size_t getDiskUsageInBytes(
            const QSqlDatabase& database,
            AnalysisType type) const;

    void initialize(const QSqlDatabase& database) override {
        m_db = database;
    }

    QList<AnalysisInfo> getAnalysesForTrackByType(TrackId trackId, AnalysisType type);
    QList<AnalysisInfo> getAnalysesForTrack(TrackId trackId);
    bool saveAnalysis(AnalysisInfo* analysis);
    bool deleteAnalysis(const int analysisId);
    void deleteAnalyses(const QList<TrackId>& trackIds);
    bool deleteAnalysesForTrack(TrackId trackId);

    void saveTrackAnalyses(const Track& track);

  private:
    bool saveWaveform(const Track& tio,
                      const Waveform& waveform,
                      AnalysisType type);
    bool loadWaveform(const Track& tio,
                      Waveform* waveform, AnalysisType type);
    QDir getAnalysisStoragePath() const;
    QByteArray loadDataFromFile(const QString& fileName) const;
    bool saveDataToFile(const QString& fileName, const QByteArray& data) const;
    bool deleteFile(const QString& filename) const;
    QList<AnalysisInfo> loadAnalysesFromQuery(TrackId trackId, QSqlQuery* query);

    UserSettingsPointer m_pConfig;
    QSqlDatabase m_db;
};

#endif // ANALYSISDAO_H
