#ifndef WAVEFORMDAO_H
#define WAVEFORMDAO_H

#include "dao.h"

#include "trackinfoobject.h"

#include <QObject>
#include <QSqlDatabase>

class WaveformDao : public DAO
{
public:
    static const QString s_waveformTableName;
    static const QString s_waveformSummaryTableName;

public:
    WaveformDao();
    virtual ~WaveformDao();
    
    virtual void initialize();
    void setDatabase( const QSqlDatabase& database);

    bool getWaveform( TrackInfoObject& tio);
    bool saveWaveform( const TrackInfoObject& tio);

    bool removeWaveform( const TrackInfoObject& tio);

protected:
    bool saveWaveform( const TrackInfoObject& tio, const Waveform* waveform, const QString& tableName);
    bool loadWaveform( const TrackInfoObject& tio, Waveform* waveform, const QString& tableName);

private:
    QSqlDatabase m_db;
    
};

#endif // WAVEFORMDAO_H
