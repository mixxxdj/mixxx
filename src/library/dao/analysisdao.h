#pragma once

#include <QDir>

#include "library/dao/analysisinfo.h"
#include "library/dao/dao.h"
#include "preferences/usersettings.h"
#include "waveform/waveform.h"

class QSqlDatabase;

class AnalysisDao : public DAO {
  public:
    static const QString s_analysisTableName;

    // These aliases keep existing code that references AnalysisDao::AnalysisType,
    // AnalysisDao::AnalysisInfo, and AnalysisDao::TYPE_* working unchanged.
    using AnalysisType = ::AnalysisType;
    using AnalysisInfo = ::AnalysisInfo;
    static constexpr AnalysisType TYPE_UNKNOWN = ::TYPE_UNKNOWN;
    static constexpr AnalysisType TYPE_WAVEFORM = ::TYPE_WAVEFORM;
    static constexpr AnalysisType TYPE_WAVESUMMARY = ::TYPE_WAVESUMMARY;

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
