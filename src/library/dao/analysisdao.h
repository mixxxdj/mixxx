#pragma once

#include <QObject>
#include <QDir>
#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "library/dao/dao.h"
#include "track/trackid.h"
#include "waveform/waveform.h"

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
    ~AnalysisDao() override = default;

    // The following functions can be used with a custom database
    // connection and independent of whether the DAO has been
    // initialized or not.
    bool deleteAnalysesByType(
            const QSqlDatabase& database,
            AnalysisType type) const;
    size_t getDiskUsageInBytes(
            const QSqlDatabase& database,
            AnalysisType type) const;

    QList<AnalysisInfo> getAnalysesForTrackByType(TrackId trackId, AnalysisType type);
    QList<AnalysisInfo> getAnalysesForTrack(TrackId trackId);
    bool saveAnalysis(AnalysisInfo* analysis);
    bool deleteAnalysis(const int analysisId);
    void deleteAnalyses(const QList<TrackId>& trackIds);
    bool deleteAnalysesForTrack(TrackId trackId);

    void saveTrackAnalyses(
            TrackId trackId,
            ConstWaveformPointer pWaveform,
            ConstWaveformPointer pWaveSummary);

  private:
    QDir getAnalysisStoragePath() const;
    QByteArray loadDataFromFile(const QString& fileName) const;
    bool saveDataToFile(const QString& fileName, const QByteArray& data) const;
    bool deleteFile(const QString& filename) const;
    QList<AnalysisInfo> loadAnalysesFromQuery(TrackId trackId, QSqlQuery* query);

    const UserSettingsPointer m_pConfig;
};
