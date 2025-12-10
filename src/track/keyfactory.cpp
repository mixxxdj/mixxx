#include <QtDebug>
#include <QStringList>
#include <QRegularExpression>
#include <cmath>

#include "track/keyfactory.h"
#include "track/keys.h"
#include "track/keyutils.h"

using mixxx::track::io::key::KeyMap;

// Static regex for parsing RapidEvolution tuning offsets (e.g., "A +50")
// Compiled once for performance during library scans
static const QRegularExpression s_offsetRegex(QStringLiteral(R"(([+-]\d+)\s*$)"));

// static
Keys KeyFactory::loadKeysFromByteArray(const QString& keysVersion,
                                       const QString& keysSubVersion,
                                       QByteArray* keysSerialized) {
    if (keysVersion == KEY_MAP_VERSION) {
        Keys keys(keysSerialized);
        keys.setSubVersion(keysSubVersion);
        qDebug() << "Successfully deserialized KeyMap";
        return keys;
    }

    return Keys();
}

// static
Keys KeyFactory::makeBasicKeys(
        mixxx::track::io::key::ChromaticKey global_key,
        mixxx::track::io::key::Source source) {
    KeyMap key_map;
    key_map.set_global_key(global_key);
    QString global_key_text = KeyUtils::keyToString(
            global_key, KeyUtils::KeyNotation::ID3v2);
    key_map.set_global_key_text(global_key_text.toStdString());
    key_map.set_source(source);
    return Keys(key_map);
}

// static
Keys KeyFactory::makeBasicKeysNormalized(
        const QString& global_key_text,
        mixxx::track::io::key::Source source) {
    mixxx::track::io::key::ChromaticKey global_key = KeyUtils::guessKeyFromText(global_key_text);
    return KeyFactory::makeBasicKeys(global_key, source);
}

// static
Keys KeyFactory::makeBasicKeysKeepText(
        const QString& global_key_text,
        mixxx::track::io::key::Source source) {
    KeyMap key_map;
    key_map.set_source(source);
    key_map.set_global_key_text(global_key_text.toStdString());
    mixxx::track::io::key::ChromaticKey global_key = KeyUtils::guessKeyFromText(global_key_text);
    key_map.set_global_key(global_key);
    // RapidEvolution writes tuning offset in cents after the key text, e.g. "A#m +50".
    // Preserve that as tuning_frequency_hz if present.
    const auto offsetMatch = s_offsetRegex.match(global_key_text);
    if (offsetMatch.hasMatch()) {
        bool ok = false;
        const int cents = offsetMatch.captured(1).toInt(&ok);
        // Validate cents range: +/- 1 octave (1200 cents) is reasonable
        if (ok && cents >= -1200 && cents <= 1200) {
            const double tuningHz = 440.0 * std::pow(2.0, static_cast<double>(cents) / 1200.0);
            // Additional sanity check: ensure frequency is in reasonable range
            if (tuningHz >= 220.0 && tuningHz <= 880.0) {
                key_map.set_tuning_frequency_hz(tuningHz);
            }
        }
    }
    return Keys(key_map);
}

// static
QString KeyFactory::getPreferredVersion() {
    return KEY_MAP_VERSION;
}

// static
QString KeyFactory::getPreferredSubVersion(
        const QHash<QString, QString>& extraVersionInfo) {
    const char* kSubVersionKeyValueSeparator = "=";
    const char* kSubVersionFragmentSeparator = "|";
    QStringList fragments;

    QHashIterator<QString, QString> it(extraVersionInfo);
    while (it.hasNext()) {
        it.next();
        if (it.key().contains(kSubVersionKeyValueSeparator) ||
            it.key().contains(kSubVersionFragmentSeparator) ||
            it.value().contains(kSubVersionKeyValueSeparator) ||
            it.value().contains(kSubVersionFragmentSeparator)) {
            qDebug() << "ERROR: Your analyzer key/value contains invalid characters:"
                     << it.key() << ":" << it.value() << "Skipping.";
            continue;
        }
        fragments << QString("%1%2%3").arg(
            it.key(), kSubVersionKeyValueSeparator, it.value());
    }

    std::sort(fragments.begin(), fragments.end());
    return (fragments.size() > 0) ? fragments.join(kSubVersionFragmentSeparator) : "";
}

// static
Keys KeyFactory::makePreferredKeys(
        const KeyChangeList& key_changes,
        const QHash<QString, QString>& extraVersionInfo,
        const mixxx::audio::SampleRate sampleRate,
        SINT totalFrames,
        double tuningFrequencyHz) {
    const QString version = getPreferredVersion();
    const QString subVersion = getPreferredSubVersion(extraVersionInfo);

    if (version == KEY_MAP_VERSION) {
        KeyMap key_map;
        for (auto it = key_changes.constBegin();
                it != key_changes.constEnd();
                ++it) {
            // Key position is in frames. Do not accept fractional frames.
            double frame = floor(it->second);

            KeyMap::KeyChange* pChange = key_map.add_key_change();
            pChange->set_key(it->first);
            pChange->set_frame_position(static_cast<int>(frame));
        }

        mixxx::track::io::key::ChromaticKey global_key =
                KeyUtils::calculateGlobalKey(
                        key_changes, totalFrames, sampleRate);
        key_map.set_global_key(global_key);
        QString global_key_text = KeyUtils::keyToString(
                global_key, KeyUtils::KeyNotation::ID3v2);
        key_map.set_global_key_text(global_key_text.toStdString());
        key_map.set_source(mixxx::track::io::key::ANALYZER);
        key_map.set_tuning_frequency_hz(tuningFrequencyHz);
        Keys keys(key_map);
        keys.setSubVersion(subVersion);
        return keys;
    }

    qDebug() << "ERROR: Could not determine what type of keys to create.";
    return Keys();
}
