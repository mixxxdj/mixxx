#ifndef TIMBRE_H
#define TIMBRE_H

#define TIMBRE_MODEL_VERSION "Timbre-1.0"

#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <QSharedPointer>

#include "proto/timbre.pb.h"

class TimbreFactory;

class Timbre;
typedef QSharedPointer<Timbre> TimbrePointer;

class Timbre : public QObject {
    Q_OBJECT
  public:
    explicit Timbre(const QByteArray* pByteArray=NULL);
    ~Timbre();
    QByteArray* toByteArray() const;
    QString getVersion() const;
    QString getSubVersion() const;
    void setSubVersion(QString subVersion);
    const mixxx::track::io::timbre::TimbreModel& getTimbreModel();
  private:
    Timbre(const mixxx::track::io::timbre::TimbreModel& m_timbreModel);
    void readByteArray(const QByteArray* pByteArray);

    mutable QMutex m_mutex;
    QString m_subVersion;
    mixxx::track::io::timbre::TimbreModel m_timbreModel;

    // For private constructor access.
    friend class TimbreFactory;
};

#endif // TIMBRE_H
