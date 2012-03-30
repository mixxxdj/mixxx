#include "waveformdao.h"

#include "waveform/waveform.h"

WaveformDao::WaveformDao(QObject *parent) :
    QObject(parent) {
}

WaveformDao::~WaveformDao() {
}

void WaveformDao::initialize() {
}

void WaveformDao::setDatabase(const QSqlDatabase& database) {
    m_db = QSqlDatabase(database);
}

bool WaveformDao::getWaveform(const TrackInfoObject &tio) {
    //TODO: (vrince)
    return true;
}

bool WaveformDao::saveWaveform(const TrackInfoObject &tio) {
    //TODO: (vrince)
    return true;
}

bool WaveformDao::removeWaveform(const TrackInfoObject &tio){
    //TODO: (vrince)
    return true;
}
