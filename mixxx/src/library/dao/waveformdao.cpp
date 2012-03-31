#include "waveformdao.h"

#include "waveform/waveform.h"

#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlError>

#include <QDebug>

const QString WaveformDao::s_waveformTableName = "waveforms";
const QString WaveformDao::s_waveformSummaryTableName = "waveform_summaries";

WaveformDao::WaveformDao() {
}

WaveformDao::~WaveformDao() {
}

void WaveformDao::initialize() {
}

void WaveformDao::setDatabase(const QSqlDatabase& database) {
    m_db = QSqlDatabase(database);
}

bool WaveformDao::getWaveform(TrackInfoObject& tio) {
    bool result = true;
    result &= loadWaveform(tio,tio.getWaveform(),s_waveformTableName);
    result &= loadWaveform(tio,tio.getWaveformSummary(),s_waveformSummaryTableName);
    return result;
}

bool WaveformDao::saveWaveform(const TrackInfoObject &tio) {
    bool result = true;
    result &= saveWaveform( tio, tio.getWaveform(), s_waveformTableName);
    result &= saveWaveform( tio, tio.getWaveformSummary(), s_waveformSummaryTableName);
    return result;
}

bool WaveformDao::saveWaveform(const TrackInfoObject &tio, const Waveform* waveform, const QString& tableName) {
    if( !m_db.isOpen())
        return false;

    if( !waveform)
        return false;

    QSqlQuery query(m_db);
    query.prepare("SELECT track_id FROM " + tableName + " " +
                  "WHERE track_id = :trackId");
    query.bindValue(":trackId", tio.getId());

    if( !query.exec()) {
        qWarning() << "WaveformDao::saveWaveform - error" << query.lastError().text();
        return false;
    }

    int id = -1;
    if( query.next())
        id = query.value(0).toInt();

    if( id == -1) { //create new entry
        query.prepare("INSERT INTO " + tableName + " (track_id,actual_size,data_size,visual_sample_rate,audio_visual_ratio,data) " +
                      "VALUES (:trackId,:actualSize,:dataSize,:visualSampleRate,:audioVisualRatio,:data)");
        query.bindValue(":trackId",tio.getId());
        query.bindValue(":actualSize",waveform->getActualSize());
        query.bindValue(":dataSize",waveform->getDataSize());
        query.bindValue(":visualSampleRate",waveform->getVisualSampleRate());
        query.bindValue(":audioVisualRatio",waveform->getAudioVisualRatio());
        query.bindValue(":data",QByteArray( (const char*)waveform->data(),
                                            waveform->getDataSize()*sizeof(WaveformData)));

        if( !query.exec()) {
            qWarning() << "WaveformDao::saveWaveform - insert error" << query.lastError().text();
            return false;
        }
    } else { //need and update
        query.prepare("UPDATE " + tableName + " SET " \
                      "actual_size = :actualSize," \
                      "data_size = :dataSize," \
                      "visual_sample_rate = :visualSampleRate," \
                      "audio_visual_ratio = :audioVisualRatio," \
                      "data = :data " \
                      "WHERE track_id = :trackId");

        query.bindValue(":trackId",tio.getId());
        query.bindValue(":actualSize",waveform->getActualSize());
        query.bindValue(":dataSize",waveform->getDataSize());
        query.bindValue(":visualSampleRate",waveform->getVisualSampleRate());
        query.bindValue(":audioVisualRatio",waveform->getAudioVisualRatio());
        query.bindValue(":data",QByteArray( (const char*)waveform->data(),
                                            waveform->getDataSize()*sizeof(WaveformData)));

        if( !query.exec()) {
            qWarning() << "WaveformDao::saveWaveform - update error" << query.lastError().text();
            return false;
        }
    }
    return true;
}

bool WaveformDao::loadWaveform(const TrackInfoObject &tio, Waveform *waveform, const QString &tableName) {
    if( !m_db.isOpen())
        return false;

    if( !waveform)
        return false;

    QSqlQuery query(m_db);
    query.prepare("SELECT actual_size, data_size, visual_sample_rate, audio_visual_ratio, data FROM " + tableName + " " +
                  "WHERE track_id = :trackId");
    query.bindValue(":trackId", tio.getId());

    if( !query.exec()) {
        qWarning() << "WaveformDao::saveWaveform - select error" << query.lastError().text();
        return false;
    }

    if( query.next()) {
        waveform->m_actualSize = query.value(0).toDouble();
        int dataSize = query.value(1).toInt();
        waveform->m_visualSampleRate = query.value(2).toDouble();
        waveform->m_audioVisualRatio = query.value(3).toDouble();
        QByteArray data = query.value(4).toByteArray();

        waveform->resize(dataSize);
        memcpy((void*)(&waveform->m_data[0]),(void*)data.data(),dataSize*sizeof(WaveformData));

        waveform->m_completion = dataSize;

        return true;
    }

    return false;
}

bool WaveformDao::removeWaveform(const TrackInfoObject &tio){
    //TODO: (vrince)
    return true;
}
