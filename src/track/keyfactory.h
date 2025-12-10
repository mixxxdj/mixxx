#pragma once

#include <QHash>
#include <QString>

#include "audio/types.h"
#include "proto/keys.pb.h"
#include "track/keys.h"
#include "util/types.h"

class KeyFactory {
  public:
    static Keys loadKeysFromByteArray(const QString& keysVersion,
                                      const QString& keysSubVersion,
                                      QByteArray* keysSerialized);

    static Keys makeBasicKeys(mixxx::track::io::key::ChromaticKey global_key,
                              mixxx::track::io::key::Source source);

    /// This function creates a Keys object and normalizes the given text
    /// This can be used for user input
    static Keys makeBasicKeysNormalized(
            const QString& global_key_text,
            mixxx::track::io::key::Source source);

    /// This function creates a Keys object and stores the given text
    /// as it is. This can be used for library or file metadata keys
    /// Where the text must not be altered to avoid unnecessary changes
    /// with a risk of losing extra data
    static Keys makeBasicKeysKeepText(
            const QString& global_key_text,
            mixxx::track::io::key::Source source);

    static QString getPreferredVersion();

    static QString getPreferredSubVersion(
            const QHash<QString, QString>& extraVersionInfo);

    static Keys makePreferredKeys(
            const KeyChangeList& key_changes,
            const QHash<QString, QString>& extraVersionInfo,
            mixxx::audio::SampleRate sampleRate,
            SINT totalFrames,
            bool is432Hz = false,
            int tuningFrequencyHz = 440);
};
