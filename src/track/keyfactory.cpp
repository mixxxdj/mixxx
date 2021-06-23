#include <QtDebug>
#include <QStringList>

#include "track/keyfactory.h"
#include "track/keys.h"
#include "track/keyutils.h"

using mixxx::track::io::key::KeyMap;

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
Keys KeyFactory::makeBasicKeys(mixxx::track::io::key::ChromaticKey global_key,
                               mixxx::track::io::key::Source source) {
    KeyMap key_map;
    key_map.set_global_key(global_key);
    key_map.set_source(source);
    return Keys(key_map);
}

// static
Keys KeyFactory::makeBasicKeysFromText(const QString& global_key_text,
                                       mixxx::track::io::key::Source source) {
    KeyMap key_map;
    key_map.set_source(source);
    mixxx::track::io::key::ChromaticKey global_key = KeyUtils::guessKeyFromText(
        global_key_text);
    if (global_key != mixxx::track::io::key::INVALID) {
        key_map.set_global_key(global_key);
    } else {
        // If we couldn't understand the key, save it as text.
        key_map.set_global_key_text(global_key_text.toStdString());
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
        const int iSampleRate, const int iTotalSamples) {
    Q_UNUSED(iSampleRate);

    const QString version = getPreferredVersion();
    const QString subVersion = getPreferredSubVersion(extraVersionInfo);

    if (version == KEY_MAP_VERSION) {
        KeyMap key_map;
        for (const auto* it = key_changes.constBegin();
                it != key_changes.constEnd();
                ++it) {
            // Key position is in frames. Do not accept fractional frames.
            double frame = floor(it->second);

            KeyMap::KeyChange* pChange = key_map.add_key_change();
            pChange->set_key(it->first);
            pChange->set_frame_position(static_cast<int>(frame));
        }
        key_map.set_global_key(KeyUtils::calculateGlobalKey(key_changes, iTotalSamples, iSampleRate));
        key_map.set_source(mixxx::track::io::key::ANALYZER);
        Keys keys(key_map);
        keys.setSubVersion(subVersion);
        return keys;
    }

    qDebug() << "ERROR: Could not determine what type of keys to create.";
    return Keys();
}
