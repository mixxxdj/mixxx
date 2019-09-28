#include "track/trackrecord.h"

#include "track/keyfactory.h"


namespace mixxx {

TrackRecord::TrackRecord(TrackId id)
        : m_id(std::move(id)),
          m_metadataSynchronized(false),
          m_rating(0),
          m_bpmLocked(false) {
}

void TrackRecord::setKeys(const Keys& keys) {
    refMetadata().refTrackInfo().setKey(KeyUtils::getGlobalKeyText(keys));
    m_keys = std::move(keys);
}

bool TrackRecord::updateGlobalKey(
        mixxx::track::io::key::ChromaticKey key,
        mixxx::track::io::key::Source keySource) {
    if (key == mixxx::track::io::key::INVALID) {
        return false;
    } else {
        Keys keys = KeyFactory::makeBasicKeys(key, keySource);
        if (m_keys.getGlobalKey() != keys.getGlobalKey()) {
            setKeys(keys);
            return true;
        }
    }
    return false;
}

bool TrackRecord::updateGlobalKeyText(
        const QString& keyText,
        mixxx::track::io::key::Source keySource) {
    Keys keys = KeyFactory::makeBasicKeysFromText(keyText, keySource);
    if (keys.getGlobalKey() == mixxx::track::io::key::INVALID) {
        return false;
    } else {
        if (m_keys.getGlobalKey() != keys.getGlobalKey()) {
            setKeys(keys);
            return true;
        }
    }
    return false;
}

} //namespace mixxx
