#include <QMutexLocker>
#include <QtDebug>

#include "track/timbre.h"

using mixxx::track::io::timbre::TimbreModel;

Timbre::Timbre(const QByteArray* pByteArray) {
    if (pByteArray) {
        readByteArray(pByteArray);
    }
}

Timbre::Timbre(const TimbreModel& timbreModel)
        : m_timbreModel(timbreModel) {
}

Timbre::~Timbre() {
}

QByteArray* Timbre::toByteArray() const {
    QMutexLocker locker(&m_mutex);
    std::string output;
    m_timbreModel.SerializeToString(&output);
    QByteArray* pByteArray = new QByteArray(output.data(), output.length());
    return pByteArray;
}

QString Timbre::getVersion() const {
    return TIMBRE_MODEL_VERSION;
}

QString Timbre::getSubVersion() const {
    QMutexLocker locker(&m_mutex);
    return m_subVersion;
}

void Timbre::setSubVersion(QString subVersion) {
    QMutexLocker locker(&m_mutex);
    m_subVersion = subVersion;
}

const TimbreModel& Timbre::getTimbreModel() {
    QMutexLocker locker(&m_mutex);
    return m_timbreModel;
}

void Timbre::readByteArray(const QByteArray* pByteArray) {
    if (!m_timbreModel.ParseFromArray(pByteArray->constData(), pByteArray->size())) {
        qDebug() << "ERROR: Could not parse TimbreModel from QByteArray of size"
                 << pByteArray->size();
        return;
    }
}
