#include "aoide/json/playlist.h"

#include "util/assert.h"

namespace aoide {

namespace json {

std::optional<QDateTime> Playlist::collectedAt() const {
    return importDateTime(m_jsonObject.value(QLatin1String("collectedAt")));
}

void Playlist::setCollectedAt(const QDateTime& collectedAt) {
    putOptionalNonEmpty(QLatin1String("collectedAt"), exportDateTime(collectedAt));
}

EntityHeader PlaylistEntity::header() const {
    return EntityHeader(m_jsonArray.at(0).toArray());
}

Playlist PlaylistEntity::body() const {
    return Playlist(m_jsonArray.at(1).toObject());
}

std::optional<QDateTime> PlaylistEntry::addedAt() const {
    return importDateTime(m_jsonObject.value(QLatin1String("addedAt")));
}

void PlaylistEntry::setAddedAt(QDateTime addedAt) {
    putOptionalNonEmpty(QLatin1String("addedAt"), exportDateTime(std::move(addedAt)));
}

QString PlaylistEntry::trackUid() const {
    return m_jsonObject.value(QLatin1String("track"))
            .toObject()
            .value(QLatin1String("uid"))
            .toString();
}

/*static*/ PlaylistEntry PlaylistEntry::newSeparator() {
    PlaylistEntry entry;
    entry.m_jsonObject.insert(QLatin1String("separator"), {});
    return entry;
}

/*static*/ PlaylistEntry PlaylistEntry::newTrack(QString trackUid) {
    DEBUG_ASSERT(!trackUid.isEmpty());
    QJsonObject itemObject;
    itemObject.insert(QLatin1String("uid"), std::move(trackUid));
    PlaylistEntry entry;
    entry.m_jsonObject.insert(QLatin1String("track"), std::move(itemObject));
    return entry;
}

QJsonArray PlaylistWithEntries::entries() const {
    return m_jsonObject.value(QLatin1String("entries")).toArray();
}

void PlaylistWithEntries::setEntries(QJsonArray entries) {
    // Entries array is mandatory!
    m_jsonObject.insert(QLatin1String("entries"), std::move(entries));
}

EntityHeader PlaylistWithEntriesEntity::header() const {
    return EntityHeader(m_jsonArray.at(0).toArray());
}

PlaylistWithEntries PlaylistWithEntriesEntity::body() const {
    return PlaylistWithEntries(m_jsonArray.at(1).toObject());
}

quint64 PlaylistEntriesSummary::totalCount() const {
    const auto v = m_jsonObject.value(QLatin1String("totalCount")).toVariant();
    DEBUG_ASSERT(v.isValid());
    DEBUG_ASSERT(v.canConvert(QMetaType::ULongLong));
    return v.toULongLong();
}

quint64 PlaylistEntriesSummary::totalTracksCount() const {
    const auto v = m_jsonObject.value(QLatin1String("tracks"))
                           .toObject()
                           .value(QLatin1String("totalCount"))
                           .toVariant();
    DEBUG_ASSERT(v.isValid());
    DEBUG_ASSERT(v.canConvert(QMetaType::ULongLong));
    return v.toULongLong();
}

std::optional<QDateTime> PlaylistEntriesSummary::addedAtMin() const {
    auto minmaxArray = m_jsonObject.value(QLatin1String("addedAtMinMax")).toArray();
    if (minmaxArray.size() != 2) {
        return std::nullopt;
    }
    return importDateTime(minmaxArray.at(0));
}

std::optional<QDateTime> PlaylistEntriesSummary::addedAtMax() const {
    auto minmaxArray = m_jsonObject.value(QLatin1String("addedAtMinMax")).toArray();
    if (minmaxArray.size() != 2) {
        return std::nullopt;
    }
    return importDateTime(minmaxArray.at(1));
}

PlaylistEntriesSummary PlaylistWithEntriesSummary::entries() const {
    return PlaylistEntriesSummary(m_jsonObject.value("entries").toObject());
}

EntityHeader PlaylistWithEntriesSummaryEntity::header() const {
    return EntityHeader(m_jsonArray.at(0).toArray());
}

PlaylistWithEntriesSummary PlaylistWithEntriesSummaryEntity::body() const {
    return PlaylistWithEntriesSummary(m_jsonArray.at(1).toObject());
}

} // namespace json

} // namespace aoide
