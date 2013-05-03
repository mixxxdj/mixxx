#ifndef KEYFACTORY_H
#define KEYFACTORY_H

#include <QHash>
#include <QVector>
#include <QString>

#include "track/keys.h"
#include "proto/keys.pb.h"
#include "trackinfoobject.h"

class KeyFactory {
  public:
    static Keys loadKeysFromByteArray(TrackPointer pTrack,
                                      QString keysVersion,
                                      QString keysSubVersion,
                                      QByteArray* keysSerialized);
    static Keys makeBasicKeys(TrackInfoObject* pTrack,
                              mixxx::track::io::key::ChromaticKey global_key,
                              mixxx::track::io::key::Source source);
    static Keys makeBasicKeysFromText(TrackInfoObject* pTrack,
                                      QString global_key_text,
                                      mixxx::track::io::key::Source source);

    static QString getPreferredVersion();

    static QString getPreferredSubVersion(
        const QHash<QString, QString> extraVersionInfo);

    static Keys makePreferredKeys(
        TrackPointer pTrack, const KeyChangeList& key_changes,
        const QHash<QString, QString> extraVersionInfo,
        const int iSampleRate, const int iTotalSamples);
};

#endif /* KEYFACTORY_H */
