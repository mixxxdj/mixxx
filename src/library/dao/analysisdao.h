#ifndef ANALYSISDAO_H
#define ANALYSISDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "configobject.h"
#include "library/dao/dao.h"
#include "trackinfoobject.h"

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

    AnalysisDao(QSqlDatabase& database, ConfigObject<ConfigValue>* pConfig);
    virtual ~AnalysisDao();

    virtual void initialize();
    void setDatabase(QSqlDatabase& database);

    bool getWaveform(TrackInfoObject& tio);
    bool saveWaveform(const TrackInfoObject& tio);
    bool removeWaveform(const TrackInfoObject& tio);

    QList<AnalysisInfo> getAnalysesForTrackByType(TrackId trackId, AnalysisType type);
    QList<AnalysisInfo> getAnalysesForTrack(TrackId trackId);
    bool saveAnalysis(AnalysisInfo* analysis);
    bool deleteAnalysis(const int analysisId);
    void deleteAnalyses(const QList<TrackId>& trackIds);
    bool deleteAnalysesForTrack(TrackId trackId);

    void saveTrackAnalyses(TrackInfoObject* pTrack);

  private:
    bool saveWaveform(const TrackInfoObject& tio,
                      const Waveform& waveform,
                      AnalysisType type);
    bool loadWaveform(const TrackInfoObject& tio,
                      Waveform* waveform, AnalysisType type);
    QDir getAnalysisStoragePath() const;
    QByteArray loadDataFromFile(const QString& fileName) const;
    bool saveDataToFile(const QString& fileName, const QByteArray& data) const;
    bool deleteFile(const QString& filename) const;
    QList<AnalysisInfo> loadAnalysesFromQuery(TrackId trackId, QSqlQuery* query);

    ConfigObject<ConfigValue>* m_pConfig;
    QSqlDatabase m_db;
};

#endif // ANALYSISDAO_H
