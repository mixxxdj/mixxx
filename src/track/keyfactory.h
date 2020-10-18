#ifndef KEYFACTORY_H
#define KEYFACTORY_H

#include <QHash>
#include <QString>
#include <QVector>

#include "proto/keys.pb.h"
#include "track/keys.h"

class KeyFactory {
  public:
    static Keys loadKeysFromByteArray(const QString& keysVersion,
                                      const QString& keysSubVersion,
                                      QByteArray* keysSerialized);

    static Keys makeBasicKeys(mixxx::track::io::key::ChromaticKey global_key,
                              mixxx::track::io::key::Source source);

    static Keys makeBasicKeysFromText(const QString& global_key_text,
                                      mixxx::track::io::key::Source source);

    static QString getPreferredVersion();

    static QString getPreferredSubVersion(
        const QHash<QString, QString>& extraVersionInfo);

    static Keys makePreferredKeys(
        const KeyChangeList& key_changes,
        const QHash<QString, QString>& extraVersionInfo,
        const int iSampleRate, const int iTotalSamples);
};

#endif /* KEYFACTORY_H */
