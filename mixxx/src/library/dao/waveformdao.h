#ifndef WAVEFORMDAO_H
#define WAVEFORMDAO_H

#include "dao.h"

#include "trackinfoobject.h"

#include <QObject>
#include <QSqlDatabase>

#define WAVEFORM_TABLE "waveforms"

class WaveformDao : public QObject, public DAO
{
    Q_OBJECT
public:
    explicit WaveformDao(QObject *parent = 0);
    virtual ~WaveformDao();
    
    virtual void initialize();
    void setDatabase( const QSqlDatabase& database);

    bool getWaveform( const TrackInfoObject& tio);
    bool saveWaveform( const TrackInfoObject& tio);
    bool removeWaveform( const TrackInfoObject& tio);

signals:
    
public slots:

private:
    QSqlDatabase m_db;
    
};

#endif // WAVEFORMDAO_H
