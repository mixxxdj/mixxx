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

    AnalysisDao(QSqlDatabase& database, UserSettingsPointer pConfig);
    virtual ~AnalysisDao();

    virtual void initialize();
    void setDatabase(QSqlDatabase& database);

    bool getWaveform(Track& tio);
    bool saveWaveform(const Track& tio);
    bool removeWaveform(const Track& tio);

    QList<AnalysisInfo> getAnalysesForTrackByType(TrackId trackId, AnalysisType type);
    QList<AnalysisInfo> getAnalysesForTrack(TrackId trackId);
    bool saveAnalysis(AnalysisInfo* analysis);
    bool deleteAnalysis(const int analysisId);
    void deleteAnalyses(const QList<TrackId>& trackIds);
    bool deleteAnalysesForTrack(TrackId trackId);
    bool deleteAnalysesByType(AnalysisType type);

    void saveTrackAnalyses(Track* pTrack);

    size_t getDiskUsageInBytes(AnalysisType type);

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
