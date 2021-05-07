#include "aoide/json/entity.h"

#include "util/assert.h"

namespace aoide {

namespace json {

std::optional<EntityUid> EntityUid::fromQJsonValue(
        const QJsonValue& value) {
    DEBUG_ASSERT(!value.isUndefined());
    if (!value.isString()) {
        return std::nullopt;
    }
    return EntityUid(value.toString());
}

std::optional<EntityRevision> EntityRevision::fromQJsonValue(
        const QJsonValue& value) {
    DEBUG_ASSERT(!value.isUndefined());
    if (value.isNull()) {
        return std::nullopt;
    }
    const auto variant = value.toVariant();
    DEBUG_ASSERT(variant.isValid());
    if (!variant.canConvert(QMetaType::ULongLong)) {
        return std::nullopt;
    }
    return EntityRevision(variant.toULongLong());
}

EntityHeader::EntityHeader(
        QJsonArray jsonArray)
        : Array(std::move(jsonArray)) {
    DEBUG_ASSERT(m_jsonArray.size() == 2);
}

EntityUid EntityHeader::uid() const {
    VERIFY_OR_DEBUG_ASSERT(m_jsonArray.size() == 2) {
        return EntityUid();
    }
    const auto uid = EntityUid::fromQJsonValue(m_jsonArray.at(0));
    VERIFY_OR_DEBUG_ASSERT(uid) {
        return EntityUid();
    }
    return *uid;
}

EntityRevision EntityHeader::rev() const {
    VERIFY_OR_DEBUG_ASSERT(m_jsonArray.size() == 2) {
        return EntityRevision();
    }
    const auto rev = EntityRevision::fromQJsonValue(m_jsonArray.at(1));
    VERIFY_OR_DEBUG_ASSERT(rev) {
        return EntityRevision();
    }
    return *rev;
}

} // namespace json

} // namespace aoide
